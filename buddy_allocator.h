#pragma once
//ma #include "pool_allocator.h"
//ma #include "linked_list.h"
#include <stdint.h>
#include "bit_map.h"

#define MAX_LEVELS 16

//ma // one entry of the buddy list
/*typedef struct BuddyListItem {
  ListItem list;
  int idx;   // tree index
  int level; // level for the buddy
  char* start; // start of memory
  int size;
  struct BuddyListItem* buddy_ptr;
  struct BuddyListItem* parent_ptr;
} BuddyListItem;*/


typedef struct  {
  //ma ListHead free[MAX_LEVELS];
  //ma PoolAllocator list_allocator;
  BitMap map;

  int num_levels;
  char* memory; // the memory area to be managed
  int min_bucket_size; // the minimum page of RAM that can be returned
} BuddyAllocator;


//ma // computes the size in bytes for the buffer of the allocator
//ma int BuddyAllocator_calcSize(int num_levels);


//ma // initializes the buddy allocator, and checks that the buffer is large enough
void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         uint8_t *buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size);
// returns (allocates) a buddy at a given level.
// side effect on the internal structures
// 0 id no memory available
//BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level);


// releases an allocated buddy, performing the necessary joins
// side effect on the internal structures
//void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, BuddyListItem* item);

//allocates memory
//void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

//releases allocated memory
//void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);
