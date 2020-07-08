#pragma once
#include <stdint.h>

typedef struct  {
  uint8_t *buffer;
  int buffer_size;
  int num_bits; 
} BitMap;

// returns the number of bytes to store bits booleans
int BitMap_getBytes(int bits);

// initializes a bitmap on an external array
// ma: [1=available, 0=unavailableORreleased]
void BitMap_init(BitMap* bit_map, int num_bits, uint8_t* buffer);

// sets the bit bit_num in the bitmap
// status= 0 or 1
void BitMap_setBit(BitMap* bit_map, int bit_num, int status);

// inspects the status of the bit bit_num
int BitMap_bit(const BitMap* bit_map, int bit_num);

//ma: check if there is a Buddy in the given level
char isThereABuddy(BitMap* map, int level);

//ma: takes the first free index on that level
char takeFirstIdx(BitMap* map, int level);