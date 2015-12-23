#ifndef FAT_H_

#define FAT_H_

#include <stdint.h>
#include "diskio.h"


#define FS_UNKNOWN 	0U
#define FS_FAT12 	1U
#define FS_FAT16 	2U
#define FS_FAT32 	3U

#define BOOT_SIGNATURE 			0xAA55

#define ERROR  					3
#define INVALID_BOOT_SIGNATURE  2
#define VALID_FAT_FMT 			1


/* BPB Offsets */

#define JMP_TABLE_OFF 				0
#define BS_OEM_NAME_OFF 			(JMP_TABLE_OFF + 3)
#define BPB_BYTES_PER_SEC_OFF 		(BS_OEM_NAME_OFF + 8)
#define BPB_SECTOR_PER_CLUSTER_OFF 	(BPB_BYTES_PER_SEC_OFF + 2)
#define BPB_RSVD_SECTOR_CNT 		(BPB_SECTOR_PER_CLUSTER_OFF + 1)
#define BPB_NUM_FATS 				(BPB_RSVD_SECTOR_CNT + 2)
#define BPB_ROOT_ENT_CNT 			(BPB_NUM_FATS + 1)
#define BPB_TOTSEC16 				(BPB_ROOT_ENT_CNT + 2)
#define BPB_MEDIA 					(BPB_TOTSEC16 + 2)
#define BPB_FATSZ16 				(BPB_MEDIA + 1)
#define BPB_SECTOR_PER_TRACK 		(BPB_FATSZ16 + 2)
#define BPB_NUM_HEADS 				(BPB_SECTOR_PER_TRACK + 2)
#define BPB_HIDD_SEC 				(BPB_NUM_HEADS + 2)
#define BPB_TOTSEC32 				(BPB_HIDD_SEC + 4)
#define BPB_FATSZ32 				(BPB_TOTSEC32 + 4)
#define BPB_EXTFLAGS 				(BPB_FATSZ32 + 4)
#define BPB_FSVER 					(BPB_EXTFLAGS + 2)
#define BPB_ROOTCLUSTER 			(BPB_FSVER + 2)
#define BPB_FS_INFO 				(BPB_ROOTCLUSTER + 4)
#define BPB_BK_BOOT_SECT 			(BPB_FS_INFO + 2)
#define BPB_RESERVED 				(BPB_BK_BOOT_SECT + 2)
#define BPB_DRV_NUM 				(BPB_RESERVED + 12)
#define BPB_RESERVED1 				(BPB_DRV_NUM + 1)
#define BS_BOOTSIG 					(BPB_RESERVED1 + 1)
#define BS_VOL_ID 					(BS_BOOTSIG  + 1)
#define BS_VOL_LABEL				(BS_VOL_ID  + 4)
#define BS_FSTYPE					(BS_VOL_LABEL  + 11)

struct fatfs {
	uint8_t 	fatfs_type;
	uint8_t  	bpb_sectors_per_cluster;
	uint16_t 	bpb_bytes_per_sector;
	uint32_t 	bpb_sectors_per_fat;
	uint8_t 	bpb_num_of_fats;
	uint32_t 	bpb_fat_start_sector;
	uint16_t 	bpb_resvd_sector_cnt;
	uint16_t 	bpb_root_ent;
	uint32_t 	bpb_total_sectors;
	uint32_t 	bpb_first_data_sector;
	uint32_t 	bpb_root_cluster;
	uint32_t 	fat_begin_lba;
	uint32_t 	cluster_begin_lba;
};

/*
struct fatfs {
	uint16_t 	bpb_bytes_per_sector;
	uint8_t  	bpb_sector_per_cluster;
	uint16_t 	bpb_reserved_sec_count;
	uint8_t  	bpb_num_of_fats;
	uint16_t 	bpb_root_ent_cnt;
	uint16_t 	bpb_tot_sect16;
	uint8_t  	bpb_media;
	uint16_t 	bpb_fatsz16;
	uint16_t 	bpb_sector_per_track;
	uint16_t  	bpb_num_of_heads;
	uint32_t  	bpb_hidden_sectors;
	uint16_t 	bpb_tot_sect32;

	uint32_t 	bpb_fatsz32;
	uint16_t 	bpb_extflags;
	uint16_t 	bpb_fs_ver;
	uint32_t 	bpb_root_cluster;
	uint16_t 	bpb_fs_info;
	uint16_t 	bpb_bk_bootsec;
	uint8_t 	bpb_drivenum;
	uint8_t 	bpb_bootsig;
	uint32_t 	bpb_vol_id;
	uint32_t 	bpb_vol_label;
	uint32_t 	bpb_fs_type;

};
*/

int fat_mount(struct fatfs *fatfs);
int fat_open(struct fatfs *fatfs, const char *filename);
int fat_read(void *buff,
			int count,
			int *bytes_read);
int fat_write(void *buff,
			int count,
			int *bytes_written);

int fat_lseek(int offset);


// TODO: Below should be moved to a util header file 

#define DWORD(ptr_val) (((uint32_t) *((uint32_t *)(ptr_val))) & 0xFFFFFFFFU)

#define WORD(ptr_val) (((uint16_t) *((uint16_t *)(ptr_val))) & 0xFFFFU)
#define BYTE(ptr_val) (((uint8_t) *((uint8_t *)(ptr_val))) & 0xFFU)

#endif
