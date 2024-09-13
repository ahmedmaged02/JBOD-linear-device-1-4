/* Author:Ahmed Ibrahim
   Date:2/18/2024
    */

/***
 *      ______ .___  ___. .______     _______.  ______        ____    __   __
 *     /      ||   \/   | |   _  \   /       | /      |      |___ \  /_ | /_ |
 *    |  ,----'|  \  /  | |  |_)  | |   (----`|  ,----'        __) |  | |  | |
 *    |  |     |  |\/|  | |   ___/   \   \    |  |            |__ <   | |  | |
 *    |  `----.|  |  |  | |  |   .----)   |   |  `----.       ___) |  | |  | |
 *     \______||__|  |__| | _|   |_______/     \______|      |____/   |_|  |_|
 *
 */

#include "mdadm.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "jbod.h"
#include "util.h"

int mountFlag = 0;



int bitPacker(uint32_t Reserved, uint32_t Command, uint32_t BlockID, uint32_t DiskID){// packs operation code for jbod operation

  return Reserved | (Command<<14) | (BlockID<<20) | (DiskID<<28);//offset command, block id and diskid in correct bits.

}

int mdadm_mount(void) {
  /* YOUR CODE */
  jbod_cmd_t Command = JBOD_MOUNT;

  if(mountFlag ==  1){
    return -1; // it is already mounted so mounting again would cause a failure
   
  }

  uint32_t input = bitPacker(0,Command,0,0);// pack mount command

  if(jbod_operation(input,0) == -1){// if mount failes return -1
    return -1;
  }
  mountFlag = 1;// set global variable to mounted

  return 1;
}

int mdadm_unmount(void) {
  /* YOUR CODE */

  jbod_cmd_t Command = JBOD_UNMOUNT;

  if(mountFlag == 0){
    return -1; // it is already unmounted so mounting again would cause a failure
   
  }

  uint32_t input = bitPacker(0,Command,0,0);// pack umount command

  if(jbod_operation(input,0) == -1){// if unmount fails return fail
    return -1;
  }
  mountFlag = 0;// set global variable to unmounted

  return 1;
}


 void startPositions(uint32_t addr,int *currDisk, int *currBlock, int * currByte){// helper function to calculate starting disk, block, byte

  if(currDisk){
    *currDisk = addr/ JBOD_DISK_SIZE;
  }
  if(currBlock){
    *currBlock = (addr%JBOD_DISK_SIZE)/JBOD_BLOCK_SIZE;
  }
  if(currByte){
    *currByte = addr%JBOD_BLOCK_SIZE;
  }
}

void readBlock (uint8_t **buf, int start_offset, int read_len, int *currBlock, int* bRead){
  /*Helper function to read a block given starting offset and read len
  it also updates buf for more data, currblock, bytes read */
  uint8_t BP[256];//temp buff to hold data
  jbod_operation(bitPacker(0,JBOD_READ_BLOCK,0,0),BP);// store block in temp buf
  memcpy(*buf,BP+start_offset,read_len);// copy from temp buf to actual buf
  *buf=*buf+read_len;// increment buf for more reads
  *currBlock=*currBlock+1;// increment curr block
  *bRead = *bRead + read_len;// increment bytes read
}

void check_disk(int *currBlock, int* currDisk){// checks if disk needs to be changed to next
  if(*currBlock>= JBOD_BLOCK_SIZE){// if curr block is more than 255 than disk needs to be changed
    *currDisk=*currDisk+1;// increment disk
    *currBlock=0;// set block to 0th
    jbod_operation(bitPacker(0,JBOD_SEEK_TO_DISK,0,*currDisk),0);// seek to new disk
  }
  return;
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  /* YOUR CODE */


  
  int currDisk,currBlock,currByte,bRead = 0;// initilizes variables for starting disk, block, start byte and bytes read

  

  startPositions(addr,&currDisk,&currBlock,&currByte);// figure out starting disk, block, start byte

  if(mountFlag == 0){
    return -1;
  }
  if(len > 1024){
    return -1;
  }
  if(len == 0 && buf == NULL){
    return 0;
  }
  if(len <= 0){
    return -1;
  }
  if(buf == NULL){
    return -1;
  }
  if(addr > 1048576 || addr + len > 1048576){
    return -1;
  }
  if(len > 0 && buf == NULL){
    return -1;
  }

  uint32_t SEEK_TO_DISK = bitPacker(0,JBOD_SEEK_TO_DISK,0,currDisk);// op code for seek to disk
  uint32_t SEEK_TO_BLOCK = bitPacker(0,JBOD_SEEK_TO_BLOCK,currBlock,0);// op code for seek to blcik

  jbod_operation(SEEK_TO_DISK,0);
  jbod_operation(SEEK_TO_BLOCK,0);

  
  int read_len=0;

  while(bRead < len){

    if(bRead == 0 && (len+currByte) <= JBOD_BLOCK_SIZE){//reading a single block 
      read_len=len;
      readBlock (&buf,currByte, read_len, &currBlock, &bRead);
      return bRead;
    }
    else if (bRead == 0){// first block read  for reading multiple case
      read_len=JBOD_BLOCK_SIZE-currByte;
      readBlock (&buf,currByte, read_len, &currBlock, &bRead);
    }

    else if ((len-bRead) <= JBOD_BLOCK_SIZE){// final block reads where read len might not be the whole block
      //Less than JBOD Block size is left to be read
      check_disk(&currBlock, &currDisk);
      read_len= (len-bRead);
      readBlock (&buf,0,read_len,&currBlock, &bRead);
      return bRead;
    }
    else if ((len-bRead) > JBOD_BLOCK_SIZE){// middle block reads where bytes to read is more than JBOD Block size
      check_disk(&currBlock, &currDisk);
      read_len= JBOD_BLOCK_SIZE;
      readBlock (&buf,0,read_len,&currBlock, &bRead);
    }
  }
  return bRead;
}
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
/*CWD /home/ahmed/311/sp24-lab2-ahmedmaged02 */
