#include "systemcalls.h"
#include <stdlib.h>//for system()
#include <unistd.h>//for fork(), execv(), dup2()
#include <sys/wait.h>//for waitpid()
#include <fcntl.h>//for open()
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
 * @reference https://man7.org/linux/man-pages/man3/system.3.html
 * https://stackoverflow.com/questions/8654089/return-value-of-system-in-c
 * https://forums.opensuse.org/t/handling-of-exit-codes-in-c-c/64931/3
*/
bool do_system(const char *cmd)
{
/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
//error checking 
    if(cmd == NULL)
    {
        return false;
    }

    int ret = system(cmd);

    //system call shell creation failed
    if(ret == -1)
    {
        perror("system call failed");
        return false;
    }

    //check system call success & exit status of command 
    if(WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
    {
        return true;
    }

    perror("command execution failed");
    //crashed/killed process or non-zero exit status
    return false;//any other case is failure
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*
* @reference https://forums.opensuse.org/t/handling-of-exit-codes-in-c-c/64931/3
* https://stackoverflow.com/questions/36754510/understanding-fork-exec-and-wait-in-c-linux
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    __pid_t pid = fork();
    //did we create child
    if(pid < 0)
    {
        // fork failed
        va_end(args);
        return false;
   }
    
    //child process
    if(pid == 0)
    {
        execv(command[0], command);
        // If execv returns, it means it failed
        perror("execv failed");
        va_end(args);
        _exit(1); // Exit child process with error code
    }

    //parent process see child finish
    int status = 0;
    __pid_t w = waitpid(pid, &status, 0);
    va_end(args);

    if(w < 0)
    {
        // waitpid failed
        return false;
    }
    
    // Check if child process terminated normally
    if(WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        return true; // Command executed successfully
    }

    return false; // Command failed
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
* @reference https://stackoverflow.com/a/13784315/1446624
* 
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    __pid_t pid = fork();
    //did we create child
    if(pid < 0)
    {
        // fork failed
        va_end(args);
        return false;
    }

    if(pid == 0)
    {
        // Child process
        //create/open output file
        int fd = open(outputfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if(fd < 0)
        {
            perror("open output file failed");
            va_end(args);
            _exit(1);
        }

        // Redirect stdout to the file
        if(dup2(fd, 1) < 0)
        {
            perror("dup2 failed");
            close(fd);
            va_end(args);
            _exit(1);
        }
        close(fd); // Close the original file descriptor

        execv(command[0], command);
        // If execv returns, it means it failed
        perror("execv failed");
        va_end(args);
        _exit(1); // Exit child process with error code
    }

    //parent process see child finish
    int status = 0;
    __pid_t w = waitpid(pid, &status, 0);
    va_end(args);

    if(w < 0)
    {
        // waitpid failed
        return false;
    }
    
    // Check if child process terminated normally
    if(WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        return true; // Command executed successfully
    }

    return false; // Command failed
}
