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
	fatfs->fatfs_info.fsi_lead_sig = DWORD(buff);

	disk_io_read(buff,fatfs->bpb_fs_info,FSI_STRUCSIG_OFF,4);
	fatfs->fatfs_info.fsi_struc_sig = DWORD(buff);

	disk_io_read(buff,fatfs->bpb_fs_info,FSI_FREE_COUNT_OFF,4);
	fatfs->fatfs_info.fsi_free_count = DWORD(buff);
	
	disk_io_read(buff,fatfs->bpb_fs_info,FSI_NXT_FREE_OFF,4);
	fatfs->fatfs_info.fsi_nxt_free = DWORD(buff);

	disk_io_read(buff,fatfs->bpb_fs_info,FSI_TRAILSIG_OFF,4);
	fatfs->fatfs_info.fsi_trail_sig = DWORD(buff);
}

void dump_fatfs_fsinfo(struct fatfs *fatfs)
{
	printf("Lead Sig : 0x%x\n",fatfs->fatfs_info.fsi_lead_sig);
	printf("Struc Sig : 0x%x\n",fatfs->fatfs_info.fsi_struc_sig);
	printf("Free clusters : %u\n",fatfs->fatfs_info.fsi_free_count);
	printf("Next free cluster : %u\n",fatfs->fatfs_info.fsi_nxt_free);
	printf("Trail sig : 0x%x\n",fatfs->fatfs_info.fsi_trail_sig);
}


void dump_dir(struct dir_entry *dir_entry) 
{
	int i = 0;

	printf("0x%x ",dir_entry->dir_attr);

	for(i = 0; i<11; i++) {
		printf("%c",dir_entry->dir_name[i]);
	}
	printf(" ");


	printf("%u:%u:%u ", ((dir_entry->dir_crt_time_tenth)&0xF800)>>11,
						((dir_entry->dir_crt_time_tenth)&0x7E0)>>5,
						((dir_entry->dir_crt_time_tenth)&0x1F)
						);


	printf("%u:%u:%u ",
						((dir_entry->dir_crt_time)&0xF800)>>11,
						((dir_entry->dir_crt_time)&0x7E0)>>5,
						((dir_entry->dir_crt_time)&0x1F)
						);

	printf("%u/%u/%u ",
						(((dir_entry->dir_crt_date)>>9)&0x7F)+EPOCH_YEAR,
						((dir_entry->dir_crt_date)&0x1E0)>>5,
						((dir_entry->dir_crt_date)&0x1F)
						);

	printf("%u/%u/%u ",
						(((dir_entry->dir_last_acc_date)>>9)&0x7F)+EPOCH_YEAR,
						((dir_entry->dir_last_acc_date)&0x1E0)>>5,
						((dir_entry->dir_last_acc_date)&0x1F)
						);

	printf("%u:%u:%u ",
						((dir_entry->dir_write_time)&0xF800)>>11,
						((dir_entry->dir_write_time)&0x7E0)>>5,
						((dir_entry->dir_write_time)&0x1F)
						);

	printf("%u/%u/%u ",
						(((dir_entry->dir_write_date)>>9)&0x7F)+EPOCH_YEAR,
						((dir_entry->dir_write_date)&0x1E0)>>5,
						((dir_entry->dir_write_date)&0x1F)
						);

	printf("%u ",dir_entry->dir_first_cluster);
	printf("%u ",dir_entry->dir_file_size);

	printf("\n");

}

void dump_long_dir_entry(struct long_dir_entry *long_dir_entry)
{
	uint8_t i = 0;
	printf("0x%x ",long_dir_entry->ldir_ord);

	printf("%u ",long_dir_entry->ldir_type);
	printf("%u ",long_dir_entry->ldir_chksum);
	printf("%u ",long_dir_entry->ldir_fst_clus_lo);

	for(i = 0; i<10; i++) 
		printf("%c",long_dir_entry->ldir_name1[i]);

	for(i = 0; i<12; i++)
		printf("%c",long_dir_entry->ldir_name2[i]);
	
	for(i = 0; i<4; i++)
		printf("%c",long_dir_entry->ldir_name3[i]);
	
}


void parse_dir_entry(uint8_t *buff, struct dir_entry *dir_entry)
{
	int i = 0;
	dir_entry->dir_attr = buff[DIR_ATTR_OFF];

	for(i = 0;i<11;i++) {
		dir_entry->dir_name[i] = buff[i];
	}

	dir_entry->dir_crt_time_tenth = buff[DIR_CRT_TIMETENTH_OFF];
	dir_entry->dir_crt_time = WORD(buff + DIR_CRT_TIME_OFF);
	dir_entry->dir_crt_date = WORD(buff + DIR_CRT_DATE_OFF);
	dir_entry->dir_last_acc_date = WORD(buff + DIR_LST_ACC_DATE_OFF);
	dir_entry->dir_write_time = WORD(buff + DIR_WRT_TIME_OFF);
	dir_entry->dir_write_date = WORD(buff + DIR_WRT_DATE_OFF);
	
	dir_entry->dir_first_cluster = WORD(buff + DIR_FST_CLUS_LO_OFF)
									| (WORD(buff + DIR_FST_CLUS_HI_OFF) << 16);
								

	/*dir_entry.dir_first_cluster = buff[DIR_FST_CLUS_LO_OFF]  
									|(buff[DIR_FST_CLUS_LO_OFF+1] << 8)
									|(buff[DIR_FST_CLUS_HI_OFF] << 16)
									|(buff[DIR_FST_CLUS_HI_OFF+1] << 24);*/

	dir_entry->dir_file_size = DWORD(buff + DIR_FILESIZE);
							
							

	/*dir_entry.dir_file_size = buff[DIR_FILESIZE]
								|(buff[DIR_FILESIZE+1] << 8)
								|(buff[DIR_FILESIZE+2] << 16)
								|(buff[DIR_FILESIZE+3] << 24);*/
}

void parse_ldir_entry(
					uint8_t *buff,
					struct long_dir_entry *long_dir_entry
					)
{
	int i = 0;

	long_dir_entry->ldir_attr = buff[LDIR_ATTR_OFF];
	long_dir_entry->ldir_ord = buff[LDIR_ORD_OFF];
	
	for(i = 0; i<10; i++) 
		long_dir_entry->ldir_name1[i] = buff[LDIR_NAME1_OFF + i];

	long_dir_entry->ldir_type = buff[LDIR_TYPE_OFF];
	long_dir_entry->ldir_chksum = buff[LDIR_CHKSUM_OFF];

	for(i = 0; i<10; i++)
		long_dir_entry->ldir_name2[i] = buff[LDIR_NAME2_OFF + i];

	long_dir_entry->ldir_fst_clus_lo = WORD(buff+LDIR_FST_CLUS_LO_OFF);

	for(i = 0; i<10; i++)
		long_dir_entry->ldir_name3[i] = buff[LDIR_NAME3_OFF + i];

}

uint32_t get_fat_entry(struct fatfs *fatfs,
						uint8_t *fat_buff,
						uint32_t cluster_val)
{
	uint32_t fat_sec_num = fatfs->bpb_resvd_sector_cnt + 
				((cluster_val << 2)/fatfs->bpb_bytes_per_sector);
	uint32_t fat_offset = (cluster_val<<2) % fatfs->bpb_bytes_per_sector;

	disk_io_read(fat_buff,fat_sec_num,0,512);
	
	return (*(((uint32_t *) fat_buff) + fat_offset)) & 0x0FFFFFFF;
}

void read_dir(struct fatfs *fatfs)
{
	struct dir_entry dir_entry;
	struct long_dir_entry long_dir_entry;

	uint8_t buff[32];

	uint8_t i = 0;
	uint32_t j = 0;

	while(j < 512) {

		disk_io_read(buff,
					fatfs->first_data_sector,
					j,
					32);
		if(buff[0] ==  0x00)
			break;

		if(buff[DIR_ATTR_OFF] != ATTR_LONG_NAME) {
			parse_dir_entry(buff, &dir_entry);
			dump_dir(&dir_entry);
		} else if(buff[LDIR_ATTR_OFF] == ATTR_LONG_NAME) {
			//parse_ldir_entry(buff, &long_dir_entry);
			//dump_long_dir_entry(&long_dir_entry);
		}
		j += 32;
	}

}

void dump_fatfs_info(struct fatfs *fatfs)
{
	int i  = 0;
	printf("Sectors per cluster : %u\n",
					fatfs->bpb_sectors_per_cluster);

	printf("Bytes per sector :%u\n",
				fatfs->bpb_bytes_per_sector
				);
	printf("Sectors per fat: %u\n",fatfs->bpb_sectors_per_fat);
	printf("Num of fats: %d\n",fatfs->bpb_num_of_fats);
	printf("No.of reserved sectors : %u\n",fatfs->bpb_resvd_sector_cnt);
	printf("Root cluster : %d\n",fatfs->bpb_root_cluster); 
	printf("Backup boot sector at : %d\n",fatfs->bpb_bk_boot_sect);
	printf("FS Info sector : %u\n",fatfs->bpb_fs_info);
	printf("Volume id : %x\n",fatfs->bs_vol_id);
	printf("Start sector : %u\n",fatfs->first_data_sector);

	printf("Volume label : ");
	for(i = 0; i<11; i++) {
		printf("%c",fatfs->bs_vol_label[i]);
	}
	printf("\n");
}

int fat_mount(struct fatfs *fatfs)
{
	int result = 0;
	char buff[36];
	uint8_t fatfs_type = 0;
	uint32_t fsize = 0;
	int i = 0;

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


	disk_io_read(buff,0,
					BPB_BYTES_PER_SEC_OFF,
					2);
	fatfs->bpb_bytes_per_sector = WORD(buff);

	
	disk_io_read(buff,
					0,
					BPB_FATSZ32,
					4);
	fatfs->bpb_sectors_per_fat = DWORD(buff);

	disk_io_read(buff,
					0,
					BPB_NUM_FATS,
					1);
	fatfs->bpb_num_of_fats = BYTE(buff);

	fsize = (fatfs->bpb_sectors_per_fat) * (fatfs->bpb_num_of_fats);

	disk_io_read(buff,
					0,
					BPB_RSVD_SECTOR_CNT,
					2);
	fatfs->bpb_resvd_sector_cnt =  WORD(buff);

	fatfs->bpb_fat_start_sector = fatfs->bpb_resvd_sector_cnt;
					

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


	fatfs->first_data_sector = fatfs->bpb_resvd_sector_cnt + 
			(fatfs->bpb_num_of_fats * fatfs->bpb_sectors_per_fat);
			

	disk_io_read(buff,0,
			BPB_ROOTCLUSTER,
			4);

	fatfs->bpb_root_cluster = DWORD(buff);

	disk_io_read(buff,0,
				BPB_BK_BOOT_SECT,
				2);
	fatfs->bpb_bk_boot_sect = WORD(buff);

	disk_io_read(buff,0,BPB_FS_INFO,2);
	fatfs->bpb_fs_info = WORD(buff);


	disk_io_read(buff,0,BS_VOL_ID,4);
	fatfs->bs_vol_id = DWORD(buff);

	disk_io_read(buff,0,BPB_MEDIA,1);
	fatfs->bpb_media_type = buff[0];

	disk_io_read(buff,0,BS_VOL_LABEL,11);
	for(i = 0;i<11;i++) {
		fatfs->bs_vol_label[i] = buff[i];
	}


	return result;
}

void read_fat(struct fatfs *fatfs)
{
	uint32_t cluster_val = 0;
	uint8_t fat_buff[512];
	printf("bpb media : %x\n",fatfs->bpb_media_type);
	printf("FAT entry at cluster val %u : %x\n",cluster_val,
					get_fat_entry(fatfs,fat_buff,cluster_val));
}

int fat_open(
				struct fatfs *fatfs,
				const char *filename
				)
{
	int result = 0;
	
	struct fatfs_info fatfs_info;

	read_fs_info(fatfs,&fatfs_info);

	printf("ldir_name3 off :%u\n",LDIR_NAME3_OFF);

	read_dir(fatfs);

	read_fat(fatfs);

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
