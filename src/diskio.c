#include "diskio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


/* 
 * File to be modified based on the underlying interface
 */


int fd = 0;

int disk_io_init(void)
{
	fd = open("fatfs.img",O_RDONLY);

	if(fd == -1) {
		printf("Could not open file\n");
	}

	return fd;
}


int disk_io_read(
				uint8_t *buff,
				uint32_t sector, 
				uint32_t offset,
				uint32_t count
				)
{
	int ret_val = 0;

	ret_val = lseek(fd,(sector<<9)+offset,SEEK_SET);

	if(ret_val == -1)
		return ret_val;

	ret_val = read(fd,buff,count);

	return ret_val;
}

int disk_io_write(
				char *buff,
				uint32_t sector,
				uint32_t offset,
				uint32_t count
				)
{
	return 0;
}
