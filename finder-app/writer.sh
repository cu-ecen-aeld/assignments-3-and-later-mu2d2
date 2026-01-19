#!/bin/bash
#writer script for assignment 1
#Author: Muthuu SVS

#exits if command fails or unset variable referenced
set -e
set -u

WRITEFILE=$1 # first argument path to file
WRITESTR=$2 #2nd argument text string to write in file

#checks if exactly 2 arguments were passed
#this if check was created with help from https://stackoverflow.com/questions/7989486/check-that-there-are-at-least-two-arguments-given-in-a-bash-script
if [ "$#" -ne 2 ]
then
    echo "Error: 2 arguments not supplied." >&2
    echo "Usage: $0 <writefilepath> <writestring>"
    exit 1
fi

echo "Two arguments passed in."

#this function was used with help from https://www.geeksforgeeks.org/linux-unix/dirname-command-in-linux-with-examples/
#strip the directory name of the write file
DIRPATH=$(dirname "$WRITEFILE")

#checks if filedirectory is a valid directory
if [ -d "$DIRPATH" ]
then
    echo "Directory is valid"
else #creates directories needed if it doesnt exist
    mkdir -p "$DIRPATH"
fi

#checks if creating file succeded
if touch "$WRITEFILE";
then
    echo "File created successfully"
else
    echo "Error: Failed to create file" >&2
    exit 1 #exits with error code
fi

#overwrites file
echo "$WRITESTR" > "$WRITEFILE" 

echo "$2 written in $1"