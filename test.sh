#!/bin/bash
rm -f bbfs.log 
fusermount -u mount
rm -rf root
mkdir root
cfs/src/bbfs root mount
python test.py mount
