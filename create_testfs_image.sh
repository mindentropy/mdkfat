#!/bin/bash

IMG_FILE=fatfs.img
TMP_MOUNT_DIR=tmp_mnt

TEST_FILENAME="test"
TEST_FILE_EXT=".txt"
VOLUME_LABEL="TestVolume"

let BS=1024
let COUNT=2048

let SIZE=$BS*$COUNT

echo "Test Image creation"
echo "==================="

if [ -e "$IMG_FILE" ]
then
	echo "$IMG_FILE file present. Removing to create a new image"
	rm -v "$IMG_FILE"
fi

echo "---------------------"
echo "Creating an image file \"$IMG_FILE\" of size $SIZE"
dd if=/dev/zero of=$IMG_FILE bs=$BS count=$COUNT

echo "---------------------"
echo "Creating FAT32 fs"
mkfs.fat -n $VOLUME_LABEL -F32 -v $IMG_FILE


if [ -d "$TMP_MOUNT_DIR" ]
then
	echo "Cannot proceed! Directory $TMP_MOUNT_DIR already present. Please change the tmp mount directory or delete the existing directory"
	exit
fi

mkdir $TMP_MOUNT_DIR

echo "Attempting to mount the image to $TMP_MOUNT_DIR"
mount -tvfat $IMG_FILE $TMP_MOUNT_DIR


echo "Populating the directory with files:"
for((i=1;i<10;i++));
do 
	echo "$TEST_FILENAME$i$TEST_FILE_EXT"
	echo "val$i">>"$TMP_MOUNT_DIR/$TEST_FILENAME$i$TEST_FILE_EXT"
done 

#Download William Shakespeare's Macbeth from Project Gutenberg.

echo "Downloading Macbeth from Project Gutenberg"
wget http://www.gutenberg.org/cache/epub/2264/pg2264.txt -O $TMP_MOUNT_DIR/mb.txt

sync

echo "Attempting to unmount $TMP_MOUNT_DIR"
umount $TMP_MOUNT_DIR

rmdir -v $TMP_MOUNT_DIR
