#!/bin/env bash

# matrix
# Assignment 1 CS 344
# Author: Gregory D Stula


# dims function
#
# desc: takes single argument, which is the name of a file
# output: rows cols
#
# Assumptions: array format is valid
dims() {

    # Test that only a singular argument was passed
    # >&2 redirects output to stderr
    # >/dev/null is added to avoid also outputting to stdout
    if [ $# -ne 1 ]; then
        >&2 echo "Too many aruments" >/dev/null
        exit 1
    fi

    # Check if file name is readable or exists
    if [ ! -r $1 ]; then
        >&2 echo "file error" >/dev/null
        exit 1
    fi

    # memory map the file to an array of lines
    mapfile dim_matrix < $1

    # this gives the number of elements in a bash array
    # number of lines is equal to number of rows
    rows=${#dim_matrix[@]}

    # creat array from first line in the the file we read
    row_array=(${dim_matrix[0]})

    # count the elements of the first row to get the numer of cols
    cols=${#row_array[@]}

    echo "$rows $cols"
}

# Call the function named by argument 1, with the remaining arguments passed to it
$1 "${@:2}"


