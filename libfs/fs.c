 
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"




#define FS_MAX_BLOCK 8192 
#define FS_FILE_MAX_SIZE 32768
#define FS_MAX_FAT 4 
#define FAT_EOC 0xFFFF
/**
 ========   TODO: Phase 0 , preparation ===============
  It is important to observe that the file system must provide persistent storage. Let’s assume that you have created a file system on a virtual disk and mounted it. 
 
$ ./fs_make.x disk.fs 8192
Creating virtual disk 'disk.fs' with '8192' data blocks
$ ./fs_ref.x info disk.fs > ref_output
$ ./test_fs.x info disk.fs > my_output
$ diff ref_output my_output

*/


/** ========   TODO: Phase 1  ===============*/

/**
For this phase, you should probably start by defining the data structures corresponding to the blocks containing the meta-information about the file system 

	superblock : block 0
	Offset	Length (bytes)	Description
	0x00	8	Signature (must be equal to “ECS150FS”)
	0x08	2	Total amount of blocks of virtual disk
	0x0A	2	Root directory block index
	0x0C	2	Data block start index
	0x0E	2	Amount of data blocks
	0x10	1	Number of blocks for FAT
	0x11	<=8192	Unused/Padding
	
	FAT : block 1~4 

	root directory, block 5
	The root directory is an array of 128 entries stored in the block following the FAT. Each entry is 32-byte wide and describes a file, according to the following format:
        Offset	Length (bytes)	Description
        0x00	16	Filename (including NULL character)
        0x10	4	Size of the file (in bytes)
        0x14	2	Index of the first data block
        0x16	10	Unused/Padding
    An empty entry is defined by the first character of the entry’s filename being equal to the NULL character.
*/


 
struct _superblock {
	char signature[8];				//"ECS150FS";
	int16_t amountVD;				//FS_MAX_BLOCK+ 1+FS_MAX_FAT+1;
	int16_t indexRootDirectory; 		//FS_MAX_FAT +1;
	int16_t indexDataBlock; 			//FS_MAX_FAT+1+1;
	int16_t amountDataBlock; 			//FS_MAX_BLOCK;
	int8_t amountFAT;					//4;
 	//int8_t padding[BLOCK_SIZE-17];
};
struct _superblock superblock;
struct _directory {
	char filename[FS_FILENAME_LEN];
	uint32_t fileSize;
	int16_t indexFirstDataBlock;
	//int8_t padding [10];
};

struct _directory directory [FS_FILE_MAX_COUNT];
uint16_t FAT[FS_MAX_BLOCK];

struct fd {
		uint32_t fileSize;  //打开标志,初始值0，打开以后为了表示打开标志变为: 100+fd
		int16_t indexFirstDataBlock;
		int32_t offset;
	};

struct fd FD[FS_OPEN_MAX_COUNT];

int8_t mount=-1;

int fs_mount(const char *diskname)
{

	//  1 :  Open the virtual disk

	/*Return: -1 if no FS is currently mounted, or if the virtual disk cannot be closed, or if there are still open file descriptors.*/
	if (block_disk_open(diskname)!=0){   
			fprintf(stderr, "fs_mount:virtual disk file %s cannot be opened \n", diskname);
			return -1;
		}

	//  2-1: Read  superblock

	if (block_read(0, &superblock)){   
			fprintf(stderr, "fs_mount:read superBlock error\n");
			return -1;
		}

	//  error checking signature 
	if (strcmp(superblock.signature,"ECS150FS")){
			fprintf(stderr, "fs_mount:signature error: \n" );
			return -1;
		}

	// error checking total amount of block = block_disk_count() returns.
	if (superblock.amountVD <= 3 ){
			fprintf(stderr, "fs_mount: amountVD %d too small \n", superblock.amountVD);
			return -1;
		}

	if (superblock.amountVD != block_disk_count()){
			perror("fs_mount:amountVD != block_disk_count \n");
			return -1;
		}


	//  2-2: Read  FAT

	for (int i=0; i< superblock.amountFAT;i++){
		if (block_read(i+1, &FAT[i*BLOCK_SIZE])){   // virtual disk file @diskname cannot be opened or if no valid * file system can be located. 
			perror("fs_mount:read error\n");
			return -1;
			}	
	}


	//  2-3: Read  root directory
	if (block_read(superblock.amountFAT+1, directory)){   // virtual disk file @diskname cannot be opened or if no valid * file system can be located. 
		perror("fs_mount:read error\n");
		return -1;
		}	

	mount=0;
	return 0;
}

int fs_umount(void)
{
	/**
	1-2  fs_umount() makes sure that the virtual disk is properly closed and that all the internal data structures of the FS layer are properly cleaned.
	*/

	/* if there are still open file descriptors.*/
	for (int i=0; i<FS_OPEN_MAX_COUNT;i++){
		if (FD[i].indexFirstDataBlock!=0){  
			fprintf(stderr, "fs_stat: file Descriptor %d still not closed. \n",i);
			return -1;
		}
	}
	/**
	At this point, all data must be written onto the virtual disk. Another application that mounts the file system at a later point in time must see the previously created files and the data that was written. This means that whenever fs_umount() is called, all meta-information and file data must have been written out to disk.
	*/		

	/** CAN NOT Clean FAT/directory when unmount, format seems need erase FAT and Directory */
	/**
	for (int i=0; i< amountFAT;i++)
			FAT[i*BLOCK_SIZE]=0;

	for (int i=0; i< amountFAT;i++){
		if (block_write(i+1, &FAT[i*BLOCK_SIZE])){   
			perror("fs_mount:write error\n");
			return -1;
			}	
	FAT[0]= FAT_EOC;
	if (block_write(1, &FAT[0])){   
			perror("fs_mount:write error\n");
			return -1;
			}

	for (int i=0; i< FS_OPEN_MAX_COUNT;i++){
			for (int j=0; j <FS_FILENAME_LEN; j++) directory[i].filename[j]= 0;
			directory[i].sizeOfFile=0;
			directory[i].indexFirstDataBlock=0;
		}
	if (block_write(amountFAT+1, &directory[0])){   
			perror("fs_mount:write error\n");
			return -1;
			}
	*/
	
	if (block_disk_close()==-1 ){  
			perror("fs_umount: Close disk error\n");
			return -1;
		}
	return 0;
}



int fs_info(void)
{
/** Once you’re able to mount a file system, implement the function fs_info() prints some information about the mounted file system 
make sure that the output corresponds exactly to the reference program.
按照 $ ./fs_ref.x info disk.fs > ref_output 格式打印
*/ 
// 打印全局变量 meta

	return 0;
}


/* ========   TODO: Phase 2  ===============*/


int fs_create(const char *filename)
{
	/** if @filename is invalid */ 
	if (!filename) {
		fprintf(stderr, "invalid file diskname: %s", filename);
		return -1;
	}

	if (strlen(filename) >= FS_FILENAME_LEN ){  
			perror("fs_create: file name too long! (1~16 character)\n");
			return -1;
			}

	/**   file already exist! */
	for (int i=0; i <FS_FILE_MAX_COUNT; i++)
		{
			if (strcmp(filename,directory[i].filename)==0 ){  
				perror("fs_create: file already exist!\n");
				return -1;
				}
		}

	/**  Find an find an empty entry  */
	for (int i=0; i <FS_FILE_MAX_COUNT; i++)
		{
			if (directory[i].filename[0]=='\0')
			{  
				strcpy(directory[i].filename,filename);
				directory[i].fileSize =0;
				directory[i].indexFirstDataBlock= FAT_EOC;
				return 0;
			}
			
			if (i>= FS_FILE_MAX_COUNT){  
				perror("fs_create: directory full! (max 128 file)\n");
				return -1;
			}
		}
	return 0;
}



int fs_delete(const char *filename)
{
	if (strlen(filename) >= FS_FILENAME_LEN || strlen(filename)==0 ){  
		perror("fs_create: file name too long! (1~16 character)\n");
		return -1;
	}

	for (uint32_t i=0; i <FS_FILE_MAX_COUNT; i++){
		if (strcmp(filename,directory[i].filename)==0 )  {
			/** and all the data blocks containing the file’s contents must be freed in the FAT.*/
			int indexCurrentBlock=directory[i].indexFirstDataBlock; 		
			for(;indexCurrentBlock < FS_MAX_BLOCK; ){
				if (FAT[directory[i].indexFirstDataBlock]== FAT_EOC){
					FAT[directory[i].indexFirstDataBlock]=0;
					break;
					}
				else {
					int indexOldBlock= indexCurrentBlock;
					indexCurrentBlock= FAT[indexCurrentBlock];
					FAT[indexOldBlock]=0;
					}
				}
			/** the file’s entry must be emptied */			
			for (int j=0; j <FS_FILENAME_LEN; j++) {
				directory[i].filename[j]= 0;
				directory[i].fileSize = 0;
				directory[i].indexFirstDataBlock= 0;			
			}
		}

		if (i>= FS_FILE_MAX_COUNT){  
			perror("fs_create: no such file! \n");
			return -1;
		}
	}
	return 0;
}


/**
 * fs_ls - List files on file system
 *
 * List information about the files located in the root directory.
 *
 * Return: -1 if no FS is currently mounted. 0 otherwise.
 */

int fs_ls(void)
{
	if(mount==-1) return -1;
	printf ("file name       size ");
	for (int i=0; i <FS_FILE_MAX_COUNT; i++)
	{
		if (directory[i].filename[0]!='\0'){  
			printf ( "%s \t %d Bytes\n",directory[i].filename,directory[i].fileSize);
			
		}
	}
	return 0;
}



/**========================== phase  3=============================================-*/
int fs_open(const char *filename)
{
	int positionDir=0; 	
	int positionFD=0; 	

	/** Return: -1 if no FS is currently mounted,  */

	/** if @filename is invalid */ 
	if (!filename) {
		perror("invalid file diskname");
		return -1;
	}
	
	/** if string @filename is too long, */
	if (strlen(filename) >= FS_FILENAME_LEN ){  
			perror("fs_open: file name too long! (1~16 character)\n");
			return -1;
			}	

	/*  search directory */
	for (int positionDir=0; positionDir <FS_FILE_MAX_COUNT; positionDir++){
		if (strcmp(filename,directory[positionDir].filename)==0) 
			break;
	}
	/**if there is no file named @filename to open*/
	if (positionDir>= FS_FILE_MAX_COUNT){  
			fprintf(stderr, "fs_open: there is no file named %s to open\n",filename);
			return -1;
		}		
			
	/** find a empty item from FD array */
	for (int positionFD=0; positionFD <FS_OPEN_MAX_COUNT; positionFD++) {  
		if (FD[positionFD].indexFirstDataBlock== 0){
			FD[positionFD].fileSize =directory[positionDir].fileSize;
			FD[positionFD].indexFirstDataBlock= directory[positionDir].indexFirstDataBlock;
			FD[positionFD].offset= 0;
			return positionFD;
		}	
	}
	if (positionFD>= FS_OPEN_MAX_COUNT){  
		fprintf(stderr, "fs_open: root directory already contains %d files. \n",FS_FILE_MAX_COUNT);
		return -1;
	}
	return 0;
}


int fs_close(int fd)
{
	/**  Return: -1 if no FS is currently mounted */
	/** if file descriptor @fd is invalid (out of bounds or not currently open) */ 
	if (fd >= FS_OPEN_MAX_COUNT || fd <=0){  
			fprintf(stderr, "fs_close: file Descriptor should between 0~31\n");
			return -1;
			}
	if (FD[fd].indexFirstDataBlock == 0 ){  
			fprintf(stderr, "fs_stat: fd NOT opened\n" );
			return -1;
			}
	/** * Close file descriptor @fd. */		
	FD[fd].fileSize=0;
	FD[fd].indexFirstDataBlock= 0;
	FD[fd].offset= 0;

	return 0;
}



int fs_stat(int fd)
{
	/**  Return: -1 if no FS is currently mounted */
	
	/** if file descriptor @fd is invalid (i.e., out of bounds, or not currently open)*/
	if (fd >= FS_OPEN_MAX_COUNT || fd <=0){  
		fprintf(stderr, "fs_stat: file descriptor %d is invalid:out of bounds \n",fd);
		return -1;
		}
	if (FD[fd].indexFirstDataBlock == 0 ){  
		fprintf(stderr, "fs_stat: %d is invalid ：not currently open)", fd );
		return -1;
		}

	return FD[fd].fileSize;
}



int fs_lseek(int fd, size_t offset)
{
	/** Return: -1 if no FS is currently mounted, */
	
	/** if file descriptor @fd is invalid (i.e., out of bounds, or not currently open)*/
	if (fd >= FS_OPEN_MAX_COUNT || fd <=0){  
		fprintf(stderr, "fs_lseek:file descriptor %d is invalid:out of bounds\n", fd);
		return -1;
		}

	if (FD[fd].indexFirstDataBlock == 0 ){  
		fprintf(stderr, "fs_stat: file descriptor %d is invalid:not currently open\n", fd );
		return -1;
		}
			
	/**if @offset is larger than the current file size*/
	if (offset >  FD[fd].fileSize ){  
		fprintf(stderr, "fs_lseek: offset is larger than the current file size:\n");
		return -1;
		}
			
	FD[fd].offset = offset; 

	return 0;
}

/**========================== phase  4 =============================================-*/

int fs_write(int fd, void *buf, size_t count)
{
char buffer[BLOCK_SIZE*FS_MAX_BLOCK];
int indexCurrentBlock =FD[fd].indexFirstDataBlock;
	
/** 1 ==========  no need expand =====================*/
	if (count <= FD[fd].fileSize - FD[fd].offset ){ 
		/** read total file blocks into buffer  */ 
		for (uint32_t i=0 ;i< (FD[fd].fileSize/BLOCK_SIZE+1); i++ )	{
			if (block_read(indexCurrentBlock, &buffer[i*BLOCK_SIZE])){ 
				perror("fs_mount:read error\n");
				return -1;
				}	
			indexCurrentBlock =FAT[indexCurrentBlock];
			buf += BLOCK_SIZE;
			}
			
		/** replace suitable part */	
		memcpy (buffer + FD[fd].offset,buf,count);
		
		/** write back */
		indexCurrentBlock =FD[fd].indexFirstDataBlock;
		for (uint32_t i=0 ;i< (FD[fd].fileSize/BLOCK_SIZE+1); i++ ){
			if (block_write(indexCurrentBlock, &buffer[i*BLOCK_SIZE])){ 
				perror("fs_mount:read error\n");
				return -1;
				}	
			indexCurrentBlock =FAT[indexCurrentBlock];
			buf += BLOCK_SIZE;
			}
		return count;
	}

/**  2 ==========  NEED Expand file =====================*/
else {
	/** expand file, assum disk has enough free blocks */
	/** move to last file block */
	for (uint32_t i=0 ;i< FD[fd].fileSize/BLOCK_SIZE; i++ )
		indexCurrentBlock =FAT[indexCurrentBlock];

	/** Find a empty Block and add in link : first-fit strategy */
	for (uint32_t i=0 ;i< (count/BLOCK_SIZE-(FD[fd].fileSize-FD[fd].offset)/BLOCK_SIZE); i++ ){
		for (int j=0 ;j< FS_MAX_BLOCK; j++ ){
			if (FAT[j]==0) {
				FAT[indexCurrentBlock]= j;
				indexCurrentBlock=j;
			} 
		}
		FAT[indexCurrentBlock]= FAT_EOC;
	}
	
	
	FD[fd].fileSize = FD[fd].fileSize - FD[fd].offset + count;
	indexCurrentBlock =FD[fd].indexFirstDataBlock;
	/* read total file blocks into buffer  */ 
	for (uint32_t i=0 ;i< (FD[fd].fileSize/BLOCK_SIZE+1); i++ )	{
		if (block_read(indexCurrentBlock, &buffer[i*BLOCK_SIZE])){ 
			perror("fs_mount:read error\n");
			return -1;
			}	
		indexCurrentBlock =FAT[indexCurrentBlock];
		buf += BLOCK_SIZE;
		}
		
	/** replace suitable part */	
	memcpy (buffer + FD[fd].offset,buf,count);
	
	/** write back */
	indexCurrentBlock =FD[fd].indexFirstDataBlock;
	for (uint32_t i=0 ;i< (FD[fd].fileSize/BLOCK_SIZE+1); i++ )	{
		if (block_write(indexCurrentBlock, &buffer[i*BLOCK_SIZE])){ 
			perror("fs_mount:read error\n");
			return -1;
			}	
		indexCurrentBlock =FAT[indexCurrentBlock];
		buf += BLOCK_SIZE;
		}
	return count;
	}
}


int fs_read(int fd, void *buf, size_t count)

{
char buffer[BLOCK_SIZE*FS_MAX_BLOCK];
int indexCurrentBlock =FD[fd].indexFirstDataBlock;

/** if file descriptor @fd is invalid (i.e., out of bounds, or not currently open)*/
if (fd >= FS_OPEN_MAX_COUNT || fd <=0){  
	fprintf(stderr,"fs_read: file descriptor %d is invalid:out of bounds \n",fd);
	return -1;
	}
if (FD[fd].indexFirstDataBlock == 0 ){  
	fprintf(stderr, "fs_read: %d is invalid ：not currently open)", fd );
	return -1;
	}
/** if @buf is NULL*/
if ( buf == NULL ){  
	perror("fs_read: buf is NULL)" );
	return -1;
	}

/*  read all file into buffer */
for (uint32_t i=0 ;i< (FD[fd].fileSize/BLOCK_SIZE+1); i++ )	{
	if (block_read(indexCurrentBlock, &buffer[i*BLOCK_SIZE])){ 
		perror("fs_mount:read error\n");
		return -1;
		}	
	indexCurrentBlock =FAT[indexCurrentBlock];
	buf += BLOCK_SIZE;
	}
	
if (count <= FD[fd].fileSize-FD[fd].offset){
	memcpy (buf, buffer+FD[fd].offset,count);
	return count;
	}
else {
	memcpy (buf, buffer+FD[fd].offset,FD[fd].fileSize-FD[fd].offset);
	return FD[fd].fileSize-FD[fd].offset;
	}
}
