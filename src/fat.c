#include "fat.h"


static void dump_buff(const char *buff,int length)
{
	int i = 0;

	for(i = 0; i<length; i++) {
		if((i!=0) && (i%16) == 0) {
			printf("\n");
		}
		printf("0x%02x ",(uint32_t)(buff[i] & 0xFF));
	}
}

static uint8_t checkfs(char *buff,
						uint32_t sector,
						uint8_t *fatfstype)
{
	uint32_t root_dir_sectors = 0;
	uint16_t root_ent_cnt = 0;
	uint16_t bytes_per_sec = 0;
	uint32_t fat_sz = 0;
	uint32_t totseccnt = 0;
	uint32_t datasectors = 0;
	uint16_t reservedcnt = 0;
	uint16_t num_of_fats = 0;
	uint8_t sector_per_cluster = 0;
	uint32_t count_of_clusters = 0;


	if(disk_io_read(buff,
				sector,
				510,
				2) < -1) {
		return ERROR;
				}

	if(WORD(buff) != BOOT_SIGNATURE) {
		return INVALID_BOOT_SIGNATURE;
	}

	disk_io_read(buff,
					sector,
					BPB_ROOT_ENT_CNT,
					2);
	root_ent_cnt  = WORD(buff);


	disk_io_read(buff,sector,
					BPB_BYTES_PER_SEC_OFF,
					2);
	bytes_per_sec = WORD(buff);

	root_dir_sectors = ((root_ent_cnt << 5) + (bytes_per_sec - 1)) / bytes_per_sec;

	disk_io_read(buff,sector,
					BPB_FATSZ16,
					2);
	fat_sz = WORD(buff);

	if(fat_sz == 0) {
		disk_io_read(buff,sector,
						BPB_FATSZ32,
						4);
		fat_sz = DWORD(buff);
	}
	
	disk_io_read(buff,sector,
					BPB_TOTSEC16,
					2);

	totseccnt = WORD(buff);

	if(totseccnt == 0) {
		disk_io_read(buff,sector,
						BPB_TOTSEC32,
						4);

		totseccnt = DWORD(buff);
	}

	disk_io_read(buff,
					sector,
					BPB_RSVD_SECTOR_CNT,
					2);
	reservedcnt = WORD(buff);

	disk_io_read(buff,
					sector,
					BPB_NUM_FATS,
					1);
	num_of_fats = BYTE(buff);

	datasectors = totseccnt - (reservedcnt + (num_of_fats * fat_sz) + root_dir_sectors);

	disk_io_read(buff,
					sector,
					BPB_SECTOR_PER_CLUSTER_OFF,
					1);
	sector_per_cluster = BYTE(buff);

	count_of_clusters = datasectors/sector_per_cluster;

	
	if(count_of_clusters < 4085)
		*fatfstype = FS_FAT12;
	else if(count_of_clusters < 65525)
		*fatfstype = FS_FAT16;
	else 
		*fatfstype = FS_FAT32;

	printf("Count of clusters %u\n",count_of_clusters);
	return VALID_FAT_FMT;
}

void read_fs_info(struct fatfs *fatfs, struct fatfs_info *fatfs_info)
{
	char buff[8];

	disk_io_read(buff,fatfs->bpb_fs_info,FSI_LEAD_SIG_OFF,4);
	fatfs_info->fsi_lead_sig = DWORD(buff);
	printf("Lead Sig : 0x%x\n",fatfs_info->fsi_lead_sig);


	disk_io_read(buff,fatfs->bpb_fs_info,FSI_STRUCSIG_OFF,4);
	fatfs_info->fsi_struc_sig = DWORD(buff);
	printf("Struc Sig : 0x%x\n",fatfs_info->fsi_struc_sig);

	disk_io_read(buff,fatfs->bpb_fs_info,FSI_FREE_COUNT_OFF,4);
	fatfs_info->fsi_free_count = DWORD(buff);
	printf("Free clusters : %u\n",fatfs_info->fsi_free_count);
	
	disk_io_read(buff,fatfs->bpb_fs_info,FSI_NXT_FREE_OFF,4);
	fatfs_info->fsi_nxt_free = DWORD(buff);
	printf("Next free cluster : %u\n",fatfs_info->fsi_nxt_free);

	disk_io_read(buff,fatfs->bpb_fs_info,FSI_TRAILSIG_OFF,4);
	fatfs_info->fsi_trail_sig = DWORD(buff);
	printf("Trail sig : 0x%x\n",fatfs_info->fsi_trail_sig);
}

int fat_mount(struct fatfs *fatfs)
{
	int result = 0;
	char buff[36];
	uint8_t fatfs_type = 0;
	uint32_t fsize = 0;

	if((result = disk_io_init()) == -1) {
		return result;
	}

	
	result = checkfs(buff,0,&(fatfs->fatfs_type));
	if(result != VALID_FAT_FMT) {
		return -1;
	}

	/* Initialize fs info */
	 disk_io_read(buff,
					0,
					BPB_SECTOR_PER_CLUSTER_OFF,
					1);
	fatfs->bpb_sectors_per_cluster = BYTE(buff);

	printf("Sectors per cluster : %u\n",
					fatfs->bpb_sectors_per_cluster);

	disk_io_read(buff,0,
					BPB_BYTES_PER_SEC_OFF,
					2);
	fatfs->bpb_bytes_per_sector = WORD(buff);
	printf("Bytes per sector :%u\n",
				fatfs->bpb_bytes_per_sector
				);

	
	disk_io_read(buff,
					0,
					BPB_FATSZ32,
					4);
	fatfs->bpb_sectors_per_fat = DWORD(buff);
	

	printf("Sectors per fat: %u\n",fatfs->bpb_sectors_per_fat);

	disk_io_read(buff,
					0,
					BPB_NUM_FATS,
					1);
	fatfs->bpb_num_of_fats = BYTE(buff);
	
	printf("Num of fats: %d\n",fatfs->bpb_num_of_fats);

	fsize = (fatfs->bpb_sectors_per_fat) * (fatfs->bpb_num_of_fats);

	disk_io_read(buff,
					0,
					BPB_RSVD_SECTOR_CNT,
					2);
	fatfs->bpb_resvd_sector_cnt =  WORD(buff);

	fatfs->bpb_fat_start_sector = fatfs->bpb_resvd_sector_cnt;
					
	printf("No.of reserved sectors : %u\n",fatfs->bpb_resvd_sector_cnt);

	disk_io_read(buff,
					0,
					BPB_ROOT_ENT_CNT,
					2);
	fatfs->bpb_root_ent = WORD(buff);

	disk_io_read(buff,
					0,
					BPB_TOTSEC32,
					4);

	fatfs->bpb_total_sectors = DWORD(buff);

	printf("Total sectors : %u\n",fatfs->bpb_total_sectors);

	fatfs->bpb_first_data_sector = fatfs->bpb_resvd_sector_cnt + 
			(fatfs->bpb_num_of_fats * fatfs->bpb_sectors_per_fat);
			

	disk_io_read(buff,0,
			BPB_ROOTCLUSTER,
			4);

	fatfs->bpb_root_cluster = DWORD(buff);
	printf("Root cluster : %d\n",fatfs->bpb_root_cluster); 


	printf("bkup sector offset : %u\n",BPB_BK_BOOT_SECT);
	disk_io_read(buff,0,
				BPB_BK_BOOT_SECT,
				2);
	fatfs->bpb_bk_boot_sect = WORD(buff);
	printf("Backup boot sector at : %d\n",fatfs->bpb_bk_boot_sect);

	disk_io_read(buff,0,BPB_FS_INFO,2);
	fatfs->bpb_fs_info = WORD(buff);
	printf("FS Info sector : %u\n",fatfs->bpb_fs_info);

	return result;
}


int fat_open(
				struct fatfs *fatfs,
				const char *filename
				)
{
	int result = 0;

	unsigned long fat_begin_lba = 0;
	struct fatfs_info fatfs_info;

	fatfs->fat_begin_lba = fatfs->bpb_resvd_sector_cnt;
	fatfs->cluster_begin_lba = fatfs->bpb_resvd_sector_cnt + 
		(fatfs->bpb_num_of_fats * fatfs->bpb_sectors_per_fat);

	printf("FAT cluster begin : %u sectors\n",fatfs->cluster_begin_lba);

	read_fs_info(fatfs,&fatfs_info);

	return result;
}


int fat_read(void *buff,
			int count,
			int *bytes_read)
{
	int result = 0;



	return result;
}

int fat_write(void *buff,
			int count,
			int *bytes_written)
{
	int result = 0;



	return result;
}

int fat_lseek(int offset)
{
	int result = 0;



	return result;
}
