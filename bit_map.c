#include <stdlib.h>
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
  if(bit_num < 0) return;
  
  int byte_num=bit_num>>3;
  assert(byte_num<bit_map->buffer_size);
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

char isThereABuddy(BitMap* map, int level) {
  int levelItems=1<<level;

  //ma: first idx of level
  int bit_num=levelItems-1;

  for(int i = bit_num; i < levelItems + bit_num; ++i) {
     if(BitMap_bit(map, i)) return 1;
  }

  return 0;
}

int takeFirstIdx(BitMap* map, int level) {
  int levelItems=1<<level;

  //ma: first idx of level
  int bit_num=levelItems-1;

  for(int i = bit_num; i < levelItems + bit_num; ++i) {
     if(BitMap_bit(map, i)) return i;
  }

  //ma: a problem happened
  return -1;
}

void print_bitmap(BitMap* bitmap) {
  printf("Bitmap state [ ");
  
  for(int i = 0; i <bitmap->num_bits; i++) {
    printf("%u ", BitMap_bit(bitmap, i));
  }

  printf("]\n\n");
}