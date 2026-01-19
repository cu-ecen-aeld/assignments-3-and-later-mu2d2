#!/bin/bash
#writer script for assignment 1
#Author: Muthuu SVS

#exits if command fails or unset variable referenced
set -e
set -u

WRITEFILE=$1 # first argument path to file
WRITESTR=$2 #2nd argument text string to write in file

#checks if exactly 2 arguments were passed
if [ "$#" -ne 2 ]
then
    echo "Error: 2 arguments not supplied." >&2
    echo "Usage: $0 <writefilepath> <writestring>"
    exit 1
fi

echo "Two arguments passed in."

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