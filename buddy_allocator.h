#pragma once
#include <stdint.h>
#include "bit_map.h"

#define MAX_LEVELS 16

typedef struct  {
  BitMap map;
  int num_levels;
  char* memory;  	     // the memory area to be managed
  int min_bucket_size;   // the minimum page of RAM that can be returned
} BuddyAllocator;


// initializes the buddy allocator
void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         uint8_t *buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size);

// returns a buddy index at a given level.
// 0 if no memory available
int BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level);


// releases a buddy (set to 0 the proper index), performing the necessary "joins"
void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int item);

// allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

// releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);


