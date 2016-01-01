#include "fat.h"

#undef HEX_OP

#define CHAR_OP

static void dump_buff(const uint8_t *buff,int length)
{
	int i = 0;

	printf("Dump:\n");
	for(i = 0; i<length; i++) {

#ifdef HEX_OP
		if((i!=0) && (i%16) == 0) {
			printf("\n");
		}
#endif

#ifdef HEX_OP
		printf("0x%02x ",(uint32_t)(buff[i] & 0xFF));
#endif

#ifdef CHAR_OP
		printf("%c",(uint8_t)(buff[i] & 0xFF));
#endif

	}

	printf("\n");
}


static uint8_t checkfs(uint8_t *buff,
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
	uint8_t buff[8];

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
								
	dir_entry->dir_file_size = DWORD(buff + DIR_FILESIZE);
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
						uint32_t cluster_val)
{
	uint8_t fat_buff[512];
	uint32_t fat_sec_num = fatfs->bpb_resvd_sector_cnt + 
				((cluster_val << 2)/fatfs->bpb_bytes_per_sector);
	uint32_t fat_offset = (cluster_val<<2) % fatfs->bpb_bytes_per_sector;

	disk_io_read(fat_buff,fat_sec_num,0,512);

	
	return ((*((uint32_t *)(&fat_buff[fat_offset]))) & 0x0FFFFFFF);
}

uint8_t cmp_name(char *str1, char *str2)
{

	while(*str1 != '\0') {
		if(*str1 != *str2)
			return 0;

		str1++;
		str2++;
	}

	return 1;
}

uint8_t read_dir(struct fatfs *fatfs, struct dir_entry *dir_entry, char *filename)
{
	uint8_t buff[32];
	uint32_t dir_entry_cnt = 0;

	uint32_t sector = 
					get_first_sector_of_cluster(
					fatfs->bpb_root_cluster,
					fatfs->bpb_sectors_per_cluster,
					fatfs->first_data_sector);

	uint32_t current_cluster = fatfs->bpb_root_cluster;

	while(1) {

		disk_io_read(
				buff,
				sector,
				dir_entry_cnt,
				DIRECTORY_ENTRY_SIZE
				);

		if(is_directory_entry_free(buff[0])) //Empty entry. Quit!
			break;

		if(buff[DIR_ATTR_OFF] != ATTR_LONG_NAME) {
			parse_dir_entry(buff, dir_entry);
			dump_dir(dir_entry);

			if(cmp_name(filename,dir_entry->dir_name)) {
				return 1;
			}
		} 

		dir_entry_cnt += DIRECTORY_ENTRY_SIZE;

		//If dir_entry_cnt == sector_size
		if(dir_entry_cnt >= (fatfs->bpb_bytes_per_sector) ) { 
			if(dir_entry_cnt >= (fatfs->bytes_per_cluster)) {
				//If the FAT entry reaches the end of the cluster follow the cluster
				//chain until a EndOfCluster is found.

				dir_entry_cnt = 0;
				current_cluster = get_fat_entry(fatfs,current_cluster);

				if(current_cluster == fatfs->EOC)  //End of cluster then Quit!
					break;

				sector = get_first_sector_of_cluster(current_cluster,
								fatfs->bpb_sectors_per_cluster,
								fatfs->first_data_sector);
			} else {
				//FAT Entry spans multiple sectors but not multiple clusters.
				sector++;
			}
		}
	}

	return 0;
}


uint32_t get_sector_from_offset(struct fatfs *fatfs,
								struct dir_entry *dir_entry)
{
	uint32_t sector,cluster_count,current_cluster;
	uint32_t sector_offset = 0;
	//If the offset is less than a cluster and is at the start
	//then get the cluster from the dir_entry and move to
	//the particular sector based on the offset.
	if(dir_entry->file_rw_info.byte_offset < bytes_per_cluster(
									fatfs->bpb_bytes_per_sector,
									fatfs->bpb_sectors_per_cluster)) 
	{
		sector = get_first_sector_of_cluster(
						dir_entry->dir_first_cluster,
						fatfs->bpb_sectors_per_cluster,
						fatfs->first_data_sector
						);

		//Based on the offset find the particular sector in the cluster.
		/*sector_offset = ((dir_entry->file_rw_info.byte_offset) % 
					(fatfs->bytes_per_cluster))/(fatfs->bpb_bytes_per_sector);*/

		sector_offset = get_sector_offset_from_cluster(
						dir_entry->file_rw_info.byte_offset,
						fatfs->bytes_per_cluster,
						fatfs->bpb_bytes_per_sector
						);

		sector += sector_offset;
	} else {

		/* Using the offset follow the cluster chain to find the particular 
		 * cluster.  
		 * Get how many cluster chains to follow.
		 */

		cluster_count = get_cluster_count(
						dir_entry->file_rw_info.byte_offset,
						fatfs->bpb_bytes_per_sector,
						fatfs->bpb_sectors_per_cluster
						);

		current_cluster = dir_entry->dir_first_cluster;
	
		sector_offset = get_sector_offset_from_cluster(
						dir_entry->file_rw_info.byte_offset,
						fatfs->bytes_per_cluster,
						fatfs->bpb_bytes_per_sector
						);

		while(cluster_count--) {
			current_cluster = get_fat_entry(fatfs,current_cluster);
		}

		sector = get_first_sector_of_cluster(
								current_cluster,
								fatfs->bpb_sectors_per_cluster,
								fatfs->first_data_sector
								);

		sector += sector_offset;
	}

	return sector;
}

uint32_t read_file (
					struct fatfs *fatfs,
					struct dir_entry *dir_entry,
					uint8_t *buff,
					uint32_t len
					)
{
	uint8_t ch;
	uint32_t bytecnt = 0;
	uint32_t i = 0;
	uint32_t curr_cluster = dir_entry->dir_first_cluster;

	uint32_t sector = get_first_sector_of_cluster(
			curr_cluster,
			fatfs->bpb_sectors_per_cluster,
			fatfs->first_data_sector);

/*
 * Read sector boundaries.
 */
	while(bytecnt < len) {
		disk_io_read(&ch,sector,i++,sizeof(ch));
		buff[bytecnt++] = ch;

		dir_entry->file_rw_info.byte_offset++;

		if(((bytecnt) % fatfs->bpb_bytes_per_sector) == 0) {
			i = 0;
			if(((bytecnt) % fatfs->bytes_per_cluster) == 0) {
				//Find the next cluster.
				curr_cluster = get_fat_entry(fatfs,curr_cluster);
				//Find the first sector in that cluster.
				sector = get_first_sector_of_cluster
							(
								curr_cluster,
								fatfs->bpb_sectors_per_cluster,
								fatfs->first_data_sector
							);
			} else {
				sector++;
			}
		}
	}

	dump_buff(buff,bytecnt);

	return bytecnt;
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

void init_fs_info(struct fatfs *fatfs)
{
	uint8_t buff[20], i = 0;
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

	//fsize = (fatfs->bpb_sectors_per_fat) * (fatfs->bpb_num_of_fats);

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

	fatfs->bytes_per_cluster = bytes_per_cluster(
								fatfs->bpb_bytes_per_sector,
								fatfs->bpb_sectors_per_cluster
								);
}

int fat_mount(struct fatfs *fatfs)
{
	int result = 0;
	uint8_t buff[36];
	//uint32_t fsize = 0;

	if((result = disk_io_init()) == -1) {
		return result;
	}
	
	result = checkfs(buff,0,&(fatfs->fatfs_type));
	if(result != VALID_FAT_FMT) {
		return -1;
	}

	init_fs_info(fatfs);

	return result;
}


int fat_open(
				struct fatfs *fatfs,
				const char *filename
				)
{
	int result = 0;
	uint8_t buff[2048];

	struct fatfs_info fatfs_info;
	struct dir_entry dir_entry;

	read_fs_info(fatfs,&fatfs_info);
	printf("ldir_name3 off :%u\n",LDIR_NAME3_OFF);

	if(read_dir(fatfs,&dir_entry,"MB") == MATCH) {
		read_file(fatfs,&dir_entry,buff,1024);
	}

	dir_entry.file_rw_info.byte_offset = 1079;
	printf("Sector from offset : %u\n",get_sector_from_offset(fatfs,&dir_entry));
	printf("Byte offset : %u\n",
			(get_sector_from_offset(fatfs,&dir_entry) * 512) + get_byte_offset_of_sector(dir_entry.file_rw_info.byte_offset,fatfs->bpb_bytes_per_sector));

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
