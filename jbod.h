#ifndef JBOD_H_
#define JBOD_H_

#include <stdint.h>

#define JBOD_NUM_DISKS            16  /* Put the correct value here */
#define JBOD_DISK_SIZE            65536 /* Put the correct value here */
#define JBOD_BLOCK_SIZE           256  /* Put the correct value here */
#define JBOD_NUM_BLOCKS_PER_DISK  256/* Calculate the correct value here */

/*example: 
#define JBOD_EXAMPLE 1234 
defines a symbolic constant named JBOD_EXAMPLE with a value of 1234. 
This means that throughout the code, wherever JBOD_EXAMPLE is used, it will be replaced with the value 1234 
during the preprocessing stage of compilation.
You need to do the same for the right values of JBOD_NUM_DISKS,JBOD_DISK_SIZE, JBOD_BLOCK_SIZE  and JBOD_NUM_BLOCKS_PER_DISK  


*/

typedef enum {
  JBOD_MOUNT,
  JBOD_UNMOUNT,
  JBOD_SEEK_TO_DISK,
  JBOD_SEEK_TO_BLOCK,
  JBOD_READ_BLOCK,
  JBOD_WRITE_BLOCK,
  JBOD_SIGN_BLOCK,
  JBOD_NUM_CMDS,
} jbod_cmd_t;

typedef enum {
  JBOD_NO_ERROR,
  JBOD_UNMOUNTED,
  JBOD_ALREADY_MOUNTED,
  JBOD_ALREADY_UNMOUNTED,
  JBOD_CACHELOAD_FAIL,
  JBOD_CACHEWRITE_FAIL,
  JBOD_BAD_CMD,
  JBOD_BAD_DISK_NUM,
  JBOD_BAD_BLOCK_NUM,
  JBOD_BAD_READ,
  JBOD_BAD_WRITE,
  JBOD_NUM_ERRNOS,
} jbod_error_t;

int jbod_operation(uint32_t op, uint8_t *block);

extern jbod_error_t jbod_error;
const char *jbod_error_string(int eno);

#endif  // JBOD_H_
