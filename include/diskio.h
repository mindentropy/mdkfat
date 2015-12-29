#ifndef DISKIO_H_

#define DISKIO_H_

#include <stdint.h>
#include <stdio.h>

/*
 * Interface design inspired by ELMChan FATFS.
 *
 */


int disk_io_init(void);

int disk_io_read(
				uint8_t *buff,
				uint32_t sector, /* Sector to read */
				uint32_t offset, /* Offset in the sector */
				uint32_t count
				); /* Number of bytes to read*/

int disk_io_write(
				char *buff,
				uint32_t sector, /* Sector to write */
				uint32_t offset, /* Offset in the sector */
				uint32_t count
				); /* Number of bytes to write */

#endif
