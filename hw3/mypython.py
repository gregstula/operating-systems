#!/bin/env python3
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

