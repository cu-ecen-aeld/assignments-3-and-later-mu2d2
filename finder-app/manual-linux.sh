#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

#Editor: Muthuu SVS
#reference: https://kernelnewbies.org/KernelBuild
# https://ftp.kh.edu.tw/Linux/Redhat/en_6.2/doc/ref-guide/s1-sysadmin-build-kernel.htm#:~:text=It%20is%20important%20to%20begin,to%20be%20the%20default%20settings.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    echo "Building kernel for ${ARCH} architecture"
    # TODO: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper #kernel clean up arch specifc headers
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig

    echo "building image and modules"
    #building the modules and tree added benchmarking to test how fast build will take
    time make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} Image modules dtbs
   
    cp arch/${ARCH}/boot/Image ${OUTDIR}/Image
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
echo "Creating rootfs directories"
mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs

mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    echo "Configuring busybox"
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
echo "Building and installing busybox"
#benchmarking the build time
time make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}

echo "Installing busybox"
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install


# TODO: Add library dependencies to rootfs
echo "Library dependencies"
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)

#reference: https://community.unix.com/t/solved-help-with-using-tr-removing-white-spaces/296416
#TODO: figure out how to copy libraries outputted from grep and copy source here.
#reads the output of readelf, searches for program interpreter and shared lib, finds the path and sets to variable
#reference:https://stackoverflow.com/questions/31333337/how-can-i-get-the-source-code-paths-from-an-elf-files-debug-information#:~:text=7%20Answers,lmo%20Over%20a%20year%20ago
INTERPRETER=$( ${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | sed -n 's/.*Requesting program interpreter: \(.*\)\]/\1/p')
LIBCXX=$( ${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | sed -n 's/.*Shared library: \[\(.*\)\]/\1/p')

#copy process

#program interpreter to rootfs
mkdir -p ${OUTDIR}/rootfs/lib ${OUTDIR}/rootfs/lib64
cp -a ${SYSROOT}${INTERPRETER} ${OUTDIR}/rootfs${INTERPRETER}

#copy each needed shared library
#loop through each library in LIBCXX variable
for LIB in ${LIBCXX}; do
     if [ -e ${SYSROOT}/lib64/${LIB} ]; then #check if lib exists in lib64
    cp -a ${SYSROOT}/lib64/${LIB} ${OUTDIR}/rootfs/lib64/
  elif [ -e ${SYSROOT}/lib/${LIB} ]; then #check if lib exists in lib
    cp -a ${SYSROOT}/lib/${LIB} ${OUTDIR}/rootfs/lib/
  else #if not found in either location, print error and exit
    echo "ERROR: missing ${LIB} in sysroot"
    exit 1
  fi
done

# TODO: Make device nodes
echo "Creating device nodes"
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 600 ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility
echo "Building writer utility"
cd ${FINDER_APP_DIR}
make clean
make CROSS_COMPILE=${CROSS_COMPILE}
cp -a writer ${OUTDIR}/rootfs/home/


# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
echo "Copying finder scripts and executables"
cp -a finder.sh finder-test.sh ${OUTDIR}/rootfs/home/ 2>/dev/null 
cp -a conf ${OUTDIR}/rootfs/home/ 2>/dev/null 

# TODO: Chown the root directory
echo "Changing ownership to root"
sudo chown -R root:root ${OUTDIR}/rootfs
#setting the setuid bit for busybox
sudo chmod u+s ${OUTDIR}/rootfs/bin/busybox

# TODO: Create initramfs.cpio.gz
echo "Creating initramfs.cpio.gz file"
cd ${OUTDIR}/rootfs
find . | cpio -o -H newc | gzip > ${OUTDIR}/initramfs.cpio.gz