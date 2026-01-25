/*
@file writer.c
@author Muthuu SVS, musv6057@colorado.edu
@date 1/25/2026
@brief accepts 2 arguments: <WRITEFILE> full path to a file including file name,
and second argument is a string <WRITESTR> written into this file
exits w/ 1 on error and print statement

assume directories exist
setup syslog logging for utility using the LOG_USER facility

Use the syslog capability to write a message “Writing <string> to <file>”
where <string> is the text string written to file (second argument) and <file> is the file created by the script.  This should be written with LOG_DEBUG level.

Use the syslog capability to log any unexpected errors with LOG_ERR level
*/
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>

/*Main function*/
int main (int argc, char *argv[])
{
    openlog(NULL, 0, LOG_USER);

    //check for valid # of args
    if(argc != 3)
    {
        syslog(LOG_ERR, "Error: not enough arguments");
        return 1;
    }

    if(argv[1] == NULL || argv[2] == NULL)
    {
        syslog(LOG_ERR, "Error: null parameter(s)");
        return 1;
    }

    //open file descriptor to given filepath 
    // owner r/w, usr & grp rd only
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0)
    {
        syslog(LOG_ERR, "Error: file descriptor to %s failed to open", argv[1]);
        return 1;
    }

    //get length of string to write
    int bufcount = strlen(argv[2]);
   
    //write WRITESTR to WRITEFILE
    int bytes = write(fd, argv[2], bufcount);
    if(bytes < 0)
    {
        syslog(LOG_ERR, "Error: writing %s to %s failed", argv[2], argv[1]);
        return 1;
    }

    //send msg to syslog at LOG_DEBUG
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);

    return 0;
}