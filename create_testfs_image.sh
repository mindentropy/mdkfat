#!/bin/bash

IMG_FILE=fatfs.img
TMP_MOUNT_DIR=tmp_mnt

TEST_FILENAME="test"
TEST_FILE_EXT=".txt"

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
echo "Creating an image file \"$OUTIMGFILE\" of size $SIZE"
dd if=/dev/zero of=$IMG_FILE bs=$BS count=$COUNT

echo "---------------------"
echo "Creating FAT32 fs"
mkfs.fat -F 32 -v $IMG_FILE


if [ -d "$TMP_MOUNT_DIR" ]
then
	echo "Cannot proceed! Directory $TMP_MOUNT_DIR already present. Please change the tmp mount directory or delete the existing directory"
	exit
fi

mkdir $TMP_MOUNT_DIR

echo "Attempting to mount the image to $TMP_MOUNT_DIR"
mount -tvfat $IMG_FILE $TMP_MOUNT_DIR


echo "Attempting to unmount $TMP_MOUNT_DIR"
umount $TMP_MOUNT_DIR
