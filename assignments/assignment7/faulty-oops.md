# Faulty Kernel Module — Oops Analysis

## Overview

This document analyzes a kernel oops captured by running:

```bash
echo "hello_world" > /dev/faulty
```

on a QEMU AArch64 target running Linux 6.1.44 with the `faulty` kernel module loaded.

---

## Full Oops Output

```
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
Mem abort info:
  ESR = 0x0000000096000045
  EC = 0x25: DABT (current EL), IL = 32 bits
  SET = 0, FnV = 0
  EA = 0, S1PTW = 0
  FSC = 0x05: level 1 translation fault
Data abort info:
  ISV = 0, ISS = 0x00000045
  CM = 0, WnR = 1
user pgtable: 4k pages, 39-bit VAs, pgdp=000000004129a000
[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
Internal error: Oops: 0000000096000045 [#1] SMP
Modules linked in: hello(O) faulty(O) scull(O) [last unloaded: scull(O)]
CPU: 0 PID: 129 Comm: sh Tainted: G           O       6.1.44 #1
Hardware name: linux,dummy-virt (DT)
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
pc : faulty_write+0x10/0x20 [faulty]
lr : vfs_write+0xc8/0x390
sp : ffffffc008dabd20
x29: ffffffc008dabd80 x28: ffffff8001b9ea00 x27: 0000000000000000
x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
x23: 000000000000000c x22: 000000000000000c x21: ffffffc008dabdc0
x20: 000000555903cb50 x19: ffffff8001dd8d00 x18: 0000000000000000
x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000
x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
x5 : 0000000000000001 x4 : ffffffc000787000 x3 : ffffffc008dabdc0
x2 : 000000000000000c x1 : 0000000000000000 x0 : 0000000000000000
Call trace:
 faulty_write+0x10/0x20 [faulty]
 ksys_write+0x74/0x110
 __arm64_sys_write+0x1c/0x30
 invoke_syscall+0x54/0x130
 el0_svc_common.constprop.0+0x44/0xf0
 do_el0_svc+0x2c/0xc0
 el0_svc+0x2c/0x90
 el0t_64_sync_handler+0xf4/0x120
 el0t_64_sync+0x18c/0x190
Code: d2800001 d2800000 d503233f d50323bf (b900003f)
---[ end trace 0000000000000000 ]---
```

---

## Analysis

### 1. Oops Type — NULL Pointer Dereference

```
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
```

The kernel attempted a memory access at virtual address `0x0` (NULL). On AArch64, there is no
valid mapping at address 0, so the MMU raised a Data Abort exception.

The `ESR` (Exception Syndrome Register) value `0x96000045` breaks down as:
- `EC = 0x25` — Data Abort from the current Exception Level (EL1, kernel mode)
- `WnR = 1` — this was a **write** operation 
- `FSC = 0x05` — level 1 translation fault (no page table entry for address 0)

This confirms the fault was a **store to address 0x0**.

---

### 2. Faulting Location — Program Counter

```
pc : faulty_write+0x10/0x20 [faulty]
```

The program counter (PC) was at offset `+0x10` within `faulty_write`, which is a 32-byte
(`0x20`) function. This directly identifies `faulty_write()` in `misc-modules/faulty.c` as
the faulting function.

The link register (LR) shows the caller:
```
lr : vfs_write+0xc8/0x390
```
This confirms the write came from the VFS write path — i.e., the `echo` command's write
syscall was dispatched through `vfs_write` into `faulty_write`.

---

### 3. Root Cause — The Faulty Line

Looking at `misc-modules/faulty.c`:

```c
ssize_t faulty_write(struct file *filp, const char __user *buf,
                     size_t count, loff_t *pos)
{
    /* make a simple fault by dereferencing a NULL pointer */
    *(int *)0 = 0;   // <-- intentional NULL pointer write
    return 0;
}
```

The line `*(int *)0 = 0` casts integer `0` to an `int` pointer and writes `0` through it.
This is an explicit store to virtual address `0x0`, which is unmapped in the kernel address
space, causing the MMU fault seen in the oops.

The PC offset `+0x10/0x20` can be confirmed with `objdump`:

```bash
objdump -dS faulty.ko
```

The instruction at offset `+0x10` corresponds to the store instruction. The oops `Code:`
field also shows this directly:

```
Code: d2800001 d2800000 d503233f d50323bf (b900003f)
```

The faulting instruction is `b900003f` — on AArch64 this is `str wzr, [x1]`, a 32-bit store
of zero to the address in register `x1`. Since `x1 = 0x0`, this is exactly the NULL pointer store.

---

### 4. Call Trace — How We Got Here

```
faulty_write+0x10/0x20 [faulty]    - fault occurs here
ksys_write+0x74/0x110              - kernel write syscall implementation
__arm64_sys_write+0x1c/0x30        - AArch64 syscall entry wrapper
invoke_syscall+0x54/0x130
el0_svc_common.constprop.0+0x44/0xf0
do_el0_svc+0x2c/0xc0
el0_svc+0x2c/0x90
el0t_64_sync_handler+0xf4/0x120
el0t_64_sync+0x18c/0x190           - EL0 (userspace) synchronous exception entry
```

Reading bottom-up: a userspace `write()` syscall entered the kernel via the AArch64
synchronous exception vector (`el0t_64_sync`), was dispatched through the syscall layer
(`el0_svc` → `ksys_write`), which called `vfs_write`, which called the `faulty` driver's
`faulty_write` file operation — where the crash occurred.

---

### 5. How to Use This to Locate the Faulty Line

Even without source code, the PC offset `faulty_write+0x10` combined with `objdump` or
`addr2line` uniquely identifies the crashing instruction:

```bash
# Disassemble the module with source interleaving
objdump -dS faulty.ko | grep -A 5 "faulty_write"

# Or use addr2line if the module was built with debug symbols
addr2line -e faulty.ko 0x10
```

Both approaches will point to `faulty.c` line 53: `*(int *)0 = 0;`

This demonstrates how kernel oops output, even without a running debugger, gives enough
information (function name + byte offset) to precisely locate the crashing line in the
driver source code.
