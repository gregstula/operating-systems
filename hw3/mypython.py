#!/bin/env python3

# Gregory D Stula
# 2020-02-16
# CS344 Winter 2020
import random

# open three files
for i in range(0,3):
    f=open(f"file{i}","w+")
    for i in range(0,10):
        # write random lowecase ascii letter (97-122)
        x = chr(random.randint(97,122))
        f.write(x)
    # last letter must be newline
    f.write('\n')
    # close the file
    f.close()

# print the contents of each file
for i in range(0,3):
    #open each one
    f=open(f"file{i}","r+")
    # read the 10 random chars and print them
    text=f.read(10)
    print(text)


# genrate 2 random nums to print
num1=random.randint(1,42)
num2=random.randint(1,42)


#print them
print(num1)
print(num2)

#get the product
prod = num1 * num2

#print it
print(prod)
