#!/bin/bash
rm -rf subdir-small

for i in $(seq 5); do
    mkdir -p subdir-small/dir_${i};
done
for i in $(seq 7); do
    echo $i > subdir-small/file_${i}.txt;
done
for i in $(seq 3); do
    echo $i $i > subdir-small/dir_1/file_1${i}.txt;
done
for i in $(seq 5); do
    echo $i $i $i > subdir-small/dir_3/file_3${i}.txt;
done
for i in $(seq 4); do
    echo $i $i $i $i > subdir-small/dir_4/file_4${i}.txt;
done
ln -s ../file_3.txt subdir-small/dir_4/link_to_file_3.txt
ln -s ..//dir_2     subdir-small/dir_4/link_to_dir_2
ln -s ../dir_4      subdir-small/dir_4/link_to_dir_4
