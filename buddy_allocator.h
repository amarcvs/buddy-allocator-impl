#pragma once
#include <stdint.h>
#include "bit_map.h"

#define MAX_LEVELS 16    // -> min_bucket_size < 32 bytes

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
// On success returns a pointer to the newly allocated memory;
// on failure, it returns NULL
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

// releases allocated memory
// On success frees the memory space  pointed by mem; 
// on failure,  (if it has already been called on mem before, or mem is NULL), no operation is performed
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);


