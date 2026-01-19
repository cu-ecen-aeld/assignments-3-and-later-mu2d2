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
echo "searching "$FILESDIR" for "$SEARCHSTR""
#the flags were found with reference to https://man7.org/linux/man-pages/man1/ls.1.html
# https://man7.org/linux/man-pages/man1/grep.1.html
#counting found from https://stackoverflow.com/questions/20895290/count-number-of-files-within-a-directory-in-linux
# https://man7.org/linux/man-pages/man1/wc.1.html
# used wordcount cmd for linecount as well
FILECOUNT=$(ls -1q "$FILESDIR" | wc -l) #gets file & subdirectory count in current directory
LINECOUNT=$(grep -r "$SEARCHSTR" "$FILESDIR" | wc -l) #gets mathcing line count in current directory
echo "The number of files are $FILECOUNT and the number of matching lines are $LINECOUNT" 
