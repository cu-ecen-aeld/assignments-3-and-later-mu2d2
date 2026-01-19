#!/bin/sh
#finder script for assignment 1
#Author: Muthuu SVS

#exits if command fails or unset variable referenced
set -e
set -u

FILESDIR=$1 # first argument directory path
SEARCHSTR=$2 #2nd argument text string to search for

if [[ "$#" -ne 2 ]]
then
    echo "Error: 2 arguments not supplied." >&2
    echo "Usage: $0 <filedirectory> <searchstring>"
    exit 1
fi

echo "Two arguments passed in."

