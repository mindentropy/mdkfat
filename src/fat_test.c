#include <stdio.h>
#include <stdlib.h>
#include "fat.h"


struct fatfs fatfs;

int main(int argc, char **argv)
{
	fat_mount(&fatfs);
	fat_open(&fatfs,"test");
	dump_fatfs_info(&fatfs);
	dump_fatfs_fsinfo(&fatfs);
	return 0;
}
