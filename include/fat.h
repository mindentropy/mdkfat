#ifndef FAT_H_

#define FAT_H_

#include <stdint.h>
#include "diskio.h"

#define EPOCH_YEAR  1980

#define FS_UNKNOWN 	0U
#define FS_FAT12 	1U
#define FS_FAT16 	2U
#define FS_FAT32 	3U

#define BOOT_SIGNATURE 			0xAA55

#define ERROR  					3
#define INVALID_BOOT_SIGNATURE  2
#define VALID_FAT_FMT 			1

#define DIRECTORY_ENTRY_SIZE  	32

#define DIRECTORY_ENTRY_FREE 	0x00
#define DIRECTORY_ENTRY_FREE1   0xE5


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

#define FSI_LEAD_SIG_OFF 			0
#define FSI_RESERVED1_OFF 			(FSI_LEAD_SIG_OFF + 4)
#define FSI_STRUCSIG_OFF			(FSI_RESERVED1_OFF + 480)
#define FSI_FREE_COUNT_OFF 			(FSI_STRUCSIG_OFF + 4)
#define FSI_NXT_FREE_OFF 			(FSI_FREE_COUNT_OFF + 4)
#define FSI_RESERVED2_OFF 			(FSI_NXT_FREE_OFF + 4)
#define FSI_TRAILSIG_OFF 			(FSI_RESERVED2_OFF + 12)


#define DIR_NAME_OFF 				0
#define DIR_ATTR_OFF 				(DIR_NAME_OFF + 11)
#define DIR_NTRES_OFF 				(DIR_ATTR_OFF + 1)
#define DIR_CRT_TIMETENTH_OFF 		(DIR_NTRES_OFF + 1)
#define DIR_CRT_TIME_OFF 			(DIR_CRT_TIMETENTH_OFF + 1)
#define DIR_CRT_DATE_OFF 			(DIR_CRT_TIME_OFF + 2)
#define DIR_LST_ACC_DATE_OFF 		(DIR_CRT_DATE_OFF + 2)
#define DIR_FST_CLUS_HI_OFF 		(DIR_LST_ACC_DATE_OFF + 2)
#define DIR_WRT_TIME_OFF 			(DIR_FST_CLUS_HI_OFF + 2)
#define DIR_WRT_DATE_OFF 			(DIR_WRT_TIME_OFF + 2)
#define DIR_FST_CLUS_LO_OFF 		(DIR_WRT_DATE_OFF + 2)
#define DIR_FILESIZE 				(DIR_FST_CLUS_LO_OFF + 2)


#define ATTR_READ_ONLY  		0x01U
#define ATTR_HIDDEN 			0x02U
#define ATTR_SYSTEM 			0x04U
#define ATTR_VOLUME_ID 			0x08U
#define ATTR_DIRECTORY 			0x10U
#define ATTR_ARCHIVE 			0x20U
#define ATTR_LONG_NAME 			(ATTR_READ_ONLY| \
								ATTR_HIDDEN| 	\
								ATTR_SYSTEM| 	\
								ATTR_VOLUME_ID)


#define LDIR_ORD_OFF			0
#define LDIR_NAME1_OFF 			(LDIR_ORD_OFF + 1)
#define LDIR_ATTR_OFF 			(LDIR_NAME1_OFF + 10)
#define LDIR_TYPE_OFF 			(LDIR_ATTR_OFF + 1)
#define LDIR_CHKSUM_OFF			(LDIR_TYPE_OFF + 1)
#define LDIR_NAME2_OFF 			(LDIR_CHKSUM_OFF + 1)
#define LDIR_FST_CLUS_LO_OFF 	(LDIR_NAME2_OFF + 12)
#define LDIR_NAME3_OFF 			(LDIR_FST_CLUS_LO_OFF + 2)


struct fatfs_info {
	uint32_t 	fsi_lead_sig;
	uint32_t 	fsi_struc_sig;
	uint32_t  	fsi_free_count;
	uint32_t 	fsi_nxt_free;
	uint32_t 	fsi_trail_sig;
};

struct dir_entry {
	char dir_name[11];
	uint8_t dir_attr;
	uint8_t dir_crt_time_tenth;
	uint32_t dir_crt_time;
	uint32_t dir_crt_date;
	uint16_t dir_last_acc_date;
	uint16_t dir_write_time;
	uint16_t dir_write_date;
	uint32_t dir_first_cluster;
	uint32_t dir_file_size;
};

struct long_dir_entry {
	uint8_t 	ldir_ord;
	uint8_t 	ldir_name1[10];
	uint8_t 	ldir_attr;
	uint8_t 	ldir_type;
	uint8_t 	ldir_chksum;
	uint8_t 	ldir_name2[12];
	uint16_t 	ldir_fst_clus_lo;
	uint8_t  	ldir_name3[4];
};

struct fatfs {
	uint8_t 	fatfs_type;
	uint8_t  	bpb_sectors_per_cluster;
	uint8_t 	bpb_num_of_fats;
	uint8_t 	bpb_media_type;
	uint16_t 	bpb_bytes_per_sector;
	uint16_t 	bpb_resvd_sector_cnt;
	uint16_t 	bpb_root_ent;
	uint16_t 	bpb_bk_boot_sect;
	uint16_t 	bpb_fs_info;
	uint32_t 	bpb_fat_start_sector;
	uint32_t 	bpb_total_sectors;
	uint32_t 	bpb_root_cluster;
	uint32_t 	bpb_sectors_per_fat;

	uint32_t 	first_data_sector;
	uint32_t 	bytes_per_cluster;

	uint32_t 	EOC;

	uint32_t 	bs_vol_id;
	char 		bs_vol_label[11];

	struct fatfs_info fatfs_info;
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

void dump_fatfs_fsinfo(struct fatfs *fatfs);
void dump_fatfs_info(struct fatfs *fatfs);


// TODO: Below should be moved to a util header file 

#define DWORD(ptr_val) (((uint32_t) *((uint32_t *)(ptr_val))) & 0xFFFFFFFFU)

#define WORD(ptr_val) (((uint16_t) *((uint16_t *)(ptr_val))) & 0xFFFFU)
#define BYTE(ptr_val) (((uint8_t) *((uint8_t *)(ptr_val))) & 0xFFU)


#define get_first_data_sector(resvd_cnt,num_of_FATS,FAT_size,root_dir_sectors) \
	((resvd_cnt) + ((num_of_FATS) * (FAT_size)) + (root_dir_sectors))

#define get_first_sector_of_cluster(cluster,sector_per_cluster,firstdatasector) \
	(((cluster - 2) * (sector_per_cluster)) + (firstdatasector))

#define bytes_per_cluster(bytes_per_sector,sectors_per_cluster) \
	((bytes_per_sector) * (sectors_per_cluster))


#define is_directory_entry_free(char_val) \
	( ((char_val) == DIRECTORY_ENTRY_FREE) || \
		((char_val) == DIRECTORY_ENTRY_FREE1) )

#endif
