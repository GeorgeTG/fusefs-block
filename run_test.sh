#!/bin/bash
root=$1
mount=$2
py=$3

if [[ "$#" -ne 3 ]]; then
    echo "Usage $0 <root> <mount> <python scipt>"
    exit 1
fi

fusermount -u $mount
rm -rf $root $mount
mkdir $root $mount
rm -f bbfs.log 
cfs/src/bbfs $root $mount
python $py $mount
