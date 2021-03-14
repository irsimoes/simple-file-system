#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   64
#define POINTERS_PER_INODE 14

#define FALSE 0
#define TRUE 1

#define VALID 1
#define NON_VALID 0

#define FREE 0
#define NOT_FREE 1

struct fs_superblock {
	unsigned int magic;
	unsigned int nblocks;
	unsigned int ninodeblocks;
	unsigned int ninodes;
};

struct fs_superblock my_super;

struct fs_inode {
	unsigned int isvalid;
	unsigned int size;
	unsigned int direct[POINTERS_PER_INODE];
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};

unsigned char * blockBitMap;

struct fs_inode inode;

int fs_format()
{
  union fs_block block;
  unsigned int i, nblocks;
  int ninodeblocks;

  if(my_super.magic == FS_MAGIC){
    printf("Cannot format a mounted disk!\n");
    return -1;
  }
  nblocks = disk_size();
  block.super.magic = FS_MAGIC;
  block.super.nblocks = nblocks;
  ninodeblocks = (int)ceil((float)nblocks*0.1);
  block.super.ninodeblocks = ninodeblocks;
  block.super.ninodes = block.super.ninodeblocks * INODES_PER_BLOCK;
  
  printf("superblock:\n");
  printf("    %d blocks\n",block.super.nblocks);
  printf("    %d inode blocks\n",block.super.ninodeblocks);
  printf("    %d inodes\n",block.super.ninodes);

  /* escrita do superbloco */
  disk_write(0,block.data);

  /* preparacao da tabela de inodes */
  bzero( block.data, DISK_BLOCK_SIZE);
  for( i = 0; i < INODES_PER_BLOCK; i++ )
    block.inode[i].isvalid = NON_VALID;

  /* escrita da tabela de inodes */
  for( i = 1; i <= ninodeblocks; i++)
    disk_write( i, block.data );

  return 0;
}

void fs_debug()
{
	union fs_block sBlock;
	union fs_block iBlock;
	unsigned int i, j, k;

	disk_read(0,sBlock.data);

	if(sBlock.super.magic != FS_MAGIC){
	  printf("disk unformatted !\n");
	  return;
	}
	printf("superblock:\n");
	printf("    %d blocks\n",sBlock.super.nblocks);
	printf("    %d inode blocks\n",sBlock.super.ninodeblocks);
	printf("    %d inodes\n",sBlock.super.ninodes);

	for( i = 1; i <= sBlock.super.ninodeblocks; i++){
	  disk_read( i, iBlock.data );
	  for( j = 0; j < INODES_PER_BLOCK; j++)
	    if( iBlock.inode[j].isvalid == VALID){
	      printf("-----\n inode: %d\n", (i-1)*INODES_PER_BLOCK + j);
	      printf("size: %d \n",iBlock.inode[j].size);
	      printf("blocks:");
	      for( k = 0; k < POINTERS_PER_INODE; k++)
			    if (iBlock.inode[j].direct[k]!=0)
				    printf("  %d",iBlock.inode[j].direct[k]);
		  printf("\n");
	    }
	}
}

int fs_mount()
{
  union fs_block block;
  int i, j, k;

  if(my_super.magic == FS_MAGIC){
    printf("disc already mounted!\n");
    return -1;
  }

  disk_read(0,block.data);
  if(block.super.magic != FS_MAGIC){
    printf("cannot mount an unformatted disc!\n");
    return -1;
  }
  if(block.super.nblocks != disk_size()){
    printf("file system size and disk size differ!\n");
    return -1;
  }

  /* CODIGO A FAZER */

    my_super=block.super;
    blockBitMap = malloc(my_super.nblocks);
    for (i = 1; i<=my_super.ninodeblocks; i++){
      disk_read(i,block.data);
	  blockBitMap[i] = NOT_FREE;
      for (j = 0; j<INODES_PER_BLOCK; j++){
			if (block.inode[j].isvalid){
				for(k = 0; k<POINTERS_PER_INODE;k++){
				  if (block.inode[j].direct[k] != 0)
					blockBitMap[block.inode[j].direct[k]] = NOT_FREE;
				}
			}
		}
    }
    blockBitMap[0] = NOT_FREE;
  return 0;
}

int fs_create()
{
  int freeInode, inodeBlock;
  union fs_block block;
  int i, j;


  if(my_super.magic != FS_MAGIC){
    printf("disc not mounted\n");
    return -1;
  }
 
  /* CODIGO A FAZER */
  char notFound = 1;
  freeInode=-1;
  for(i = 1; i<= my_super.ninodeblocks && notFound; i++){
    disk_read(i,block.data);
    for(j=0; j<INODES_PER_BLOCK && notFound; j++){
    if (!block.inode[j].isvalid){
      freeInode = j+((i-1)*INODES_PER_BLOCK);
      notFound = 0;
      inodeBlock = i;
	  block.inode[j].size = 0;
	  block.inode[j].isvalid = VALID;
	  for (int k=0; k<POINTERS_PER_INODE; k++){
		block.inode[j].direct[k] = FREE;
			}
    }
    }
  }
if(inodeBlock !=0)
disk_write(inodeBlock, block.data);
  return freeInode;
}
 
void inode_load( int inumber, struct fs_inode *inode ){
  int inodeBlock;
  union fs_block block;

  if( inumber > my_super.ninodes ){
    printf("inode number too big \n");
    abort();
  }
  inodeBlock = 1 + (inumber/INODES_PER_BLOCK);
  disk_read( inodeBlock, block.data );
  *inode = block.inode[inumber % INODES_PER_BLOCK];
}

void inode_save( int inumber, struct fs_inode *inode ){
  int inodeBlock;
  union fs_block block;

  if( inumber > my_super.ninodes ){
    printf("inode number too big \n");
    abort();
  }
  inodeBlock = 1 + (inumber/INODES_PER_BLOCK);
  disk_read(inodeBlock, block.data );
  block.inode[inumber % INODES_PER_BLOCK] = *inode;
  disk_write( inodeBlock, block.data );
}

int fs_delete( int inumber )
{
  int i;
  
  if(my_super.magic != FS_MAGIC){
    printf("disc not mounted\n");
    return -1;
  }

  /* CODIGO A FAZER */
  
  inode_load(inumber,&inode);
  if (!inode.isvalid) return -1;
  inode.size = 0;
  inode.isvalid = NON_VALID;
  for (i = 0; i<POINTERS_PER_INODE; i++){
    inode.direct[i]=FREE;
    blockBitMap[inode.direct[i]] = FREE;
  }
  inode_save(inumber,&inode);
  return 0;
}

int fs_getsize( int inumber )
{

  if(my_super.magic != FS_MAGIC){
    printf("disc not mounted\n");
    return -1;
  }

  /* CODIGO A FAZER */
  inode_load(inumber,&inode);
  return inode.size;
}


/**************************************************************/
int fs_read( int inumber, char *data, int length, int offset )
{
  int currentBlock, offsetCurrent, offsetInBlock;
  int bytesLeft, /*nCopy,*/ bytesToRead;
  //char *dst;
  union fs_block buff;

  if(my_super.magic != FS_MAGIC){
    printf("disc not mounted\n");
    return -1;
  }
  inode_load( inumber, &inode );
  if( inode.isvalid == NON_VALID ){
    printf("inode is not valid\n");
    return -1;
  }
  
  if( offset > inode.size ){
    printf("offset bigger that file size !\n");
    return -1;
  }

  /* CODIGO A FAZER */
  
  if (!inode.isvalid) return -1;
  if (length < 0 ) return -1;
  if (offset < 0) return -1;
  if (data == NULL) return -1;
  
  if (length <= (inode.size - offset))
  bytesLeft = length;
  else 
  bytesLeft = inode.size - offset;

  bytesToRead = 0;
  
  if(offset == inode.size) return -1;

  offsetInBlock = offset % DISK_BLOCK_SIZE;
  offsetCurrent = offsetInBlock;
  
  for (currentBlock = (offset / DISK_BLOCK_SIZE); (currentBlock < POINTERS_PER_INODE) && bytesLeft > 0; currentBlock++){
  disk_read(inode.direct[currentBlock], buff.data);
  for (int i = offsetCurrent; bytesLeft > 0 && i < DISK_BLOCK_SIZE; i++){
          data[bytesToRead++] = buff.data[i];
		  bytesLeft--;
    }
  offsetCurrent = 0;
  }
  return bytesToRead;
}

/******************************************************************/

int getFreeBlock() {
  int i, found;

  i = 0;
  found = FALSE;
  do{
    if(blockBitMap[i] == FREE){
      found = TRUE;
      blockBitMap[i] = NOT_FREE;
    }
    else i++;
  }while((!found) && (i < my_super.nblocks));

  if(i == my_super.nblocks) return -1; /* nao ha blocos livres */
  else return i;
}


int fs_write( int inumber, char *data, int length, int offset )
{
  int currentBlock, offsetCurrent, offsetInBlock;
  int bytesLeft, /*nCopy,*/ bytesToWrite/*, newEntry*/;
  //char *src;
  union fs_block buff;

  if(my_super.magic != FS_MAGIC){
    printf("disc not mounted\n");
    return -1;
  }
  inode_load( inumber, &inode );
  if( inode.isvalid == NON_VALID ){
    printf("inode is not valid\n");
    return -1;
  }

  if( offset > inode.size ){
    printf("starting to write after end of file\n");
    return -1;
  }

  /* CODIGO A FAZER */


  bytesLeft = length;
  int nblock = 0;
  bytesToWrite = 0;
  offsetInBlock = offset % DISK_BLOCK_SIZE;
  offsetCurrent = offsetInBlock;
  for (currentBlock = (offset / DISK_BLOCK_SIZE); (currentBlock < POINTERS_PER_INODE) && bytesLeft > 0; currentBlock++){
  if (!inode.direct[currentBlock]){
    if ((nblock = getFreeBlock()) == -1 ) return -1;
    for(int j = 0; j < DISK_BLOCK_SIZE; j++)
		buff.data[j]=0;
    inode.direct[currentBlock] = nblock;
  }else
  {
    // ja la tinha um
    nblock = inode.direct[currentBlock];
  }
  
  disk_read(nblock, buff.data);
  for (int i = offsetCurrent; i< bytesLeft + offsetCurrent && i < DISK_BLOCK_SIZE; i++){
          buff.data[i] = data[bytesToWrite++];
    }
  bytesLeft-=bytesToWrite;
  offsetCurrent = 0;
  disk_write(nblock,buff.data);
  
  
  }
		inode.size = offset+bytesToWrite;

//acabou o codigo a fazer

  inode_save( inumber, &inode );
  return bytesToWrite;
}

