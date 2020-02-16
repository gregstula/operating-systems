#!/bin/env python3
import random

for i in range(0,3):
    # open three files
    f=open(f"file{i}","w+")
    for i in range(0,11):
        # write random lowecase ascii letter (97-122)
        x = chr(random.randint(97,122))
        f.write(x)
# close the file
    f.close()

