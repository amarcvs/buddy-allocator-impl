#include <stdlib.h>
#include <assert.h>
#include "bit_map.h"

int BitMap_getBytes(int bits) {
  return bits/8 + ((bits%8)!=0);
}

void BitMap_init(BitMap* bit_map, int num_bits, uint8_t* buffer) {
  bit_map->buffer=buffer;
  bit_map->num_bits=num_bits;
  bit_map->buffer_size=BitMap_getBytes(num_bits);
}

void BitMap_setBit(BitMap* bit_map, int bit_num, int status) {
  int byte_num=bit_num>>3;
  assert(byte_num<bit_map->buffer_size);
  // or bit_num%8 with compiler optimization
  int bit_in_byte=bit_num&0x07; 
  if (status) {
    bit_map->buffer[byte_num] |= (1<<(7-bit_in_byte));
  } else {
    bit_map->buffer[byte_num] &= ~(1<<(7-bit_in_byte));
  }
}  

int BitMap_bit(const BitMap* bit_map, int bit_num) {
  int byte_num=bit_num>>3; 
  assert(byte_num<bit_map->buffer_size);
  int bit_in_byte=bit_num&0x07;
  return (bit_map->buffer[byte_num] & (1<<(7-bit_in_byte)))!=0;
}

//ma: bits start always from bit_num=0
char isThereABuddy(BitMap* map, int level) {
  // STEP #1: we are in the root
  if(!level) return map->buffer[level]>>7;

  // STEP #2:
  int levelItems=1<<level;
  int prevLevelsItems=levelItems-1;
  int bit_num=levelItems-1;
  int byte_num=bit_num>>3;
  assert(byte_num<map->buffer_size);
  int bit_in_byte=bit_num&0x07;
  
  // STEP #2.1: we are in the first byte
  if(bit_in_byte < 7) {
  	//ma: to clean the byte from bits of useless levels
    int tmp = map->buffer[byte_num] >> (7-(bit_in_byte+prevLevelsItems));
    return tmp<<(7-bit_in_byte);
  }

  // STEP #3: if the level starts from the last bit of a byte -> check that bit and if it is not 1, go to the next byte
  if(bit_in_byte == 7) if((map->buffer[byte_num])&1) return 1;

  // STEP #3.1 
  //ma: next-level info and byte where the next level starts
  int next_levelItems=1<<(level+1);
  int next_bit_num= next_levelItems-1;     
  int next_byte_num=next_bit_num>>3;
  assert(next_byte_num<map->buffer_size);
  int next_bit_in_byte = next_bit_num&0x07;

  // STEP #3.2
  for(int i=byte_num+1; i<next_byte_num; ++i) {
    if(map->buffer[i]) return 1;
  }

  // STEP #3.3 
  //ma: we are at the end
  return map->buffer[next_byte_num]>>(8-next_bit_in_byte);
}

int takeFirstIdx(BitMap* map, int level) {
  int levelItems=1<<level;

  //ma: bit_num -> num of the first element of the level
  int bit_num=levelItems-1;  
  int byte_num=bit_num>>3;
  int bit_in_byte=bit_num&0x07;

  uint8_t block = map->buffer[byte_num];
  block = block<<bit_in_byte;

  int idx = bit_num-bit_in_byte;
  uint8_t mask = 1<<7;
  char count=1;
  for(int i=0; i<levelItems+bit_in_byte; ++i) {
    if(block & mask) {
      if(((i+1)/8) == 0) return idx+bit_in_byte;
      return idx;
    }
    
    if(count==8) {
      mask = 1<<7; //ma: we reset the mask
      block = map->buffer[byte_num + ((i+1)/8)];
      count = 0;
      idx++;
      count++;
      continue;
    }

    mask = mask >> 1;
    idx++;
    count++;
    }

  //ma: a problem happened
  exit(1);
}