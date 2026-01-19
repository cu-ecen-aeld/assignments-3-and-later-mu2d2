#!/bin/bash
#finder script for assignment 1
#Author: Muthuu SVS

#exits if command fails or unset variable referenced
set -e
set -u

FILESDIR=$1 # first argument directory path
SEARCHSTR=$2 #2nd argument text string to search for

#checks if exactly 2 arguments were passed

#this if check was created with help from https://stackoverflow.com/questions/7989486/check-that-there-are-at-least-two-arguments-given-in-a-bash-script
if [[ "$#" -ne 2 ]]
then
    echo "Error: 2 arguments not supplied." >&2
    echo "Usage: $0 <filedirectory> <searchstring>"
    exit 1
fi

echo "Two arguments passed in."

#checks if filedirectory is a valid directory
if [ -d "$FILESDIR" ]
then
    echo "Directory is valid"
else
    echo "Error: Directory not valid" >&2
    exit 1
fi

FILECOUNT=$(find "$FILESDIR" -type -d | wc -l) #gets file count in current directory(including itself)
LINECOUNT=$(grep -r "$SEARCHSTR" "$FILESDIR" | wc -l) #gets mathcing line count in current directory(including itself)
echo "Number of files are $FILECOUNT and the number of matching lines are $LINECOUNT" 
