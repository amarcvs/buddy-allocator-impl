#include <stdio.h>
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

//----------------------------------------------------------------------------------------------------------------------

char isThereABuddy(BitMap* map, int level) {
  if(!level) return map->buffer[level]>>7;

  int itemsPerLevel=1<<level;               //ma: entries at level "level"
  int itemsPerLevelPrec = itemsPerLevel-1;  //ma: entries at upper levels
  
  //ma: always from bit_num=0
  int bit_num= itemsPerLevel-1;     
  int byte_num=bit_num>>3; assert(byte_num<map->buffer_size);
  int bit_in_byte = bit_num%8;
  
  if(byte_num==0 && bit_in_byte < 7) {
    int tmp = map->buffer[byte_num] >> (7-(bit_in_byte+itemsPerLevelPrec));
    return tmp<<(itemsPerLevelPrec+(8-(bit_in_byte+itemsPerLevel)));
  }
  else if(byte_num==0 && bit_in_byte ==7) if((map->buffer[byte_num])&1) return 1;
  
  //ma: next level info
  int next_itemsPerLevel=1<<(level+1);      //ma: start from bit = 0
  // int next_itemsPerLevelPrec = next_itemsPerLevel-1;
  
  //ma: byte where the next level starts
  int next_bit_num= next_itemsPerLevel-1;     
  int next_byte_num=next_bit_num>>3; 
  //assert(next_byte_num<map->buffer_size); //ma: we should also modify buffer_size
  int next_bit_in_byte = next_bit_num%8;

  //ma: case where we are not within the first two levels and that the level starts with a single bit in a byte
  if ((map->buffer[byte_num])&1) return 1;

  //ma: else: we check in the other bytes next the first one
  for(int i=0, j=byte_num+1; i<byte_num+1+next_byte_num-byte_num; ++i, ++j) {
    if(j == byte_num+1+next_byte_num-byte_num-1) {
      char tmp = map->buffer[j];
      return tmp>>(8-next_bit_in_byte);
    }
    if(map->buffer[j]) return 1;
  }
  return 0;
  
}

char takeFirstIdx(BitMap* map, int level) {
  //ma: TODO: check if you enter a level where there are no buddy, it should not return 0
  
  //ma: entries at level "level"
  int itemsPerLevel=1<<level; 

  int bit_num=(1<<level)-1;   //ma: bit number of the first element of the level
  uint8_t byte_num=bit_num>>3;
  char bit_in_byte=bit_num%8; //bit offset in the byte

  uint8_t block = map->buffer[byte_num];
  block = block<<bit_in_byte;

  char idx = bit_num-bit_in_byte; //ma: to be deleted: redundant information
  uint8_t mask = 1<<7;
  char count=1;
  for(int i=0; i<itemsPerLevel+bit_in_byte; ++i) {
    if(block & mask) {
      if(((i+1)/8) == 0) return idx+bit_in_byte;
      return idx;
    }
    
    if(count==8) {
      mask = 1<<7; //resettiamo la mask
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

  //exit(0);
  return -1; //ma: we should check if work!
}