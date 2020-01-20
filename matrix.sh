#!/bin/env bash

# matrix
# Assignment 1 CS 344
# Author: Gregory D Stula


# dims function
#
# desc: gives dimensions row, cols for tab seperated matrix
# input takes single argument or piped input, which is the name of a file
# output: rows cols
#
# Assumptions: array format is valid

dims() {

    # Test that only a singular argument was passed
    # >&2 redirects output to stderr
    # >/dev/null is added to avoid also outputting to stdout
    if [ $# -gt 1 ]; then
        >&2 echo "Too many arguments"
        exit 1
    fi

    # Check if file name is readable or exists
    if [ ! -r $1 ]; then
        >&2 echo "file error"
        exit 1
    fi

    # memory map the file to an array of lines
    # otherwise mapfile defaults to stdin
    if [ $# -eq 0 ]; then
        mapfile dim_matrix
    else
       mapfile dim_matrix < $1
    fi

    # this gives the number of elements in a bash array
    # number of lines is equal to number of rows
    rows=${#dim_matrix[@]}

    # creat array from first line in the the file we read
    row_array=(${dim_matrix[0]})

    # count the elements of the first row to get the numer of cols
    cols=${#row_array[@]}

    echo "$rows $cols"

}

# add function
#
# desc: adds two tab seperated matrices

add() {

    # Test that two arguments was passed
    # >&2 redirects output to stderr
    # >/dev/null is added to avoid also outputting to stdout
    if [ $# -ne 2 ]; then
        >&2 echo "Wrong number of arguments"
        exit 1
    fi

    # Check if files are readable or exist
    if [ ! -r $1 ] || [ ! -r $2 ]; then
        >&2 echo "file error"
        exit 1
    fi

    # reuse dims function to test dimensions
    dims1=$(dims $1)
    dims2=$(dims $2)

    # Matrix addition requires rows and cols to be exactly the same
    # reuse the dims function and check for equal output
    if [ "$dims1" != "$dims2" ]; then
        >&2 echo "Addition not possible."
        exit 1;
    fi

    # mapfile reads the file as an array of lines
    # each line is a row in the tab seperated matrix
    mapfile matrix1 < $1
    mapfile matrix2 < $2

    # iterate over the indeces of the first matrix
    # the ! sign means give us a indeces the @ sign means of every element
    # each index refers to a line (aka row)
    for i in "${!matrix1[@]}"; do

        # Now operate on each row
        # We create new arrays from the line at the index
        row1=(${matrix1[i]})
        row2=(${matrix2[i]})

        sum_row=()

        # Subtract 1 from the size so we can know when a tab is not needed
        eol=$(( ${#row1[@]} - 1 ))

        for j in "${!row1[@]}"; do
            # append new row with the sum of the numbers in each column
            col=$(( ${row1[j]} + ${row2[j]} ))
            sum_row+=$col

            if [ $j -ne $eol ]; then
                sum_row+=$'\t' #matrix is tab seperated
            fi
        done
        # IFS controls how the values of the array are delimited
        # the default vaulues remove tabs but we want to print those literally
        # we use sub process here to avoid changing it for the other loops
        (IFS='\n'; echo ${sum_row[@]})
    done
}

transpose() {
# Test that only a singular argument was passed
    # >&2 redirects output to stderr
    # >/dev/null is added to avoid also outputting to stdout
    if [ $# -gt 1 ]; then
        >&2 echo "Too many arguments"
        exit 1
    fi

    # Check if file name is readable or exists
    if [ ! -r $1 ]; then
        >&2 echo "file error"
        exit 1
    fi

    # memory map the file to an array of lines if files passed
    # otherwise mapfile defaults to stdin
    if [ $# -eq 0 ]; then
        mapfile dim_matrix
    else
       mapfile dim_matrix < $1
    fi

    # turn output of dims function into array with dims and set dimensions
    dim_array=($(dims $1))

    rows=${dim_array[0]}
    cols=${dim_array[1]}

    # Subtract 1 from the size so we can know when a tab is not needed
    eol=$(( rows - 1 ))

    # create a new row from the all the elements in col i
    for ((i=0; i < cols; i++)); do
        new_row=()

        # get and iterate over all ideces j for the array of rows (matrix)
        for j in "${!dim_matrix[@]}"; do
            line_array=(${dim_matrix[j]})

            # append new row and add tab if not last element of new row
            new_row+=${line_array[i]}
            if [ $j -ne $eol ]; then
                new_row+=$'\t' #matrix is tab seperated
            fi
        done
        # same as addition printing, set IFS in a subprocess to keep tabs
        (IFS=''; echo ${new_row[@]})
    done
}


multiply() {

    # Test that two arguments was passed
    # >&2 redirects output to stderr
    # >/dev/null is added to avoid also outputting to stdout
    if [ $# -ne 2 ]; then
        >&2 echo "Wrong number of arguments"
        exit 1
    fi

    # Check if files are readable or exist
    if [ ! -r $1 ] || [ ! -r $2 ]; then
        >&2 echo "file error"
        exit 1
    fi

    # reuse dims function to test dimensions
    # put dimensions in array to easily index them
    dims_array1=($(dims $1))
    dims_array2=($(dims $2))


    rows1=${dims_array1[0]} # product row size M
    cols1=${dims_array1[1]} # N

    rows2=${dims_array2[0]} # N
    cols2=${dims_array2[1]} # product cols size P

    product_row=$rows1
    product_col=$cols2

    # Check that matrices are MxN and NxP
    # we do this by checking that N = N
    if [ "$cols1" != "$rows2" ]; then
        >&2 echo "Multiplication not possible."
        exit 1;
    fi

    # transpose the second matrix so we can treat it's cols as lines
    # we use process subsitution to let mapfile grab the sunshell output and give us an array of lines
    mapfile matrix_2 < <(transpose $2)
    echo $matrix_2
    #for loop over both and multiply each index accumulate to a sum add to sum
    #append final sum to new array product_col times
    #print
    # do everything again product_row times
}


# Call the function named by argument 1, with the remaining arguments passed to it
$1 "${@:2}"

