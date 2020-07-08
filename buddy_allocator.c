#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "buddy_allocator.h"

//ma: helper functions :) ------------------------------------------------------------------------------------------------------------------------------
int levelIdx(size_t idx) {
  return (int)floor(log2(idx + 1));
};

int buddyIdx(int idx) {
  if (idx&0x1) return idx+1;
  return idx-1;
}

int parentIdx(int idx) {
  return (idx-1)/2;
}

int startIdx(int idx) {
  return (idx+1 - (1<<levelIdx(idx)));
}

//ma: returns the size of an item in a given level
int itemSize(BuddyAllocator * alloc, int level) {
  return (alloc->min_bucket_size*(1<<alloc->num_levels))/(1<<level);
}

//ma: returns the memory address of an item
char* start_memory(BuddyAllocator* alloc, char level, char buddy) {
  return alloc->memory+(itemSize(alloc, level)*startIdx(buddy));
}

//ma: print the bitmap
void print_test(BitMap* map) {
  printf("[ ");
  for(int i = 0; i <map->num_bits; i++) {
    printf("%u ", BitMap_bit(map, i));
  }
  printf("]\n");
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------

void BuddyAllocator_provideBuddy(BuddyAllocator* alloc, char idx) {
  // [1=available, 0=unavailableORreleased]
  BitMap_setBit(&alloc->map, idx, 1);
  printf("Available Item. idx:%d, level:%d, start:%p, size:%d\n", idx,
                                                                  levelIdx(idx),
                                                                  start_memory(alloc, levelIdx(idx), idx),
                                                                  itemSize(alloc, levelIdx(idx)));
}

void BuddyAllocator_restoringBuddy(BuddyAllocator* alloc, char idx) {
  BitMap_setBit(&alloc->map, idx, 0);
  printf("Destroying Item. idx:%d, level:%d, start:%p, size:%d\n", idx,
  																                                 levelIdx(idx),
  																                                 start_memory(alloc, levelIdx(idx), idx), 
  																                                 itemSize(alloc, levelIdx(idx)));
}

void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         uint8_t* buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size) {

  alloc->num_levels=num_levels;
  alloc->memory=memory;
  alloc->min_bucket_size=min_bucket_size;
  assert (num_levels<MAX_LEVELS);
  
  int map_items=buffer_size;

  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d\n", num_levels);
  printf("\tmax map entries: %d\n", map_items);
  printf("\tmanaged memory %d bytes\n", (1<<num_levels)*min_bucket_size);
  printf("\tbucket size:%d\n", min_bucket_size);
  
  //ma: we initialize the bitmap
  uint8_t* bufferMap=buffer;
  BitMap_init(&alloc->map, map_items, bufferMap);

  //ma: we set first bit of bitmap to mark that all memory is available
  BuddyAllocator_provideBuddy(alloc, 0);
}

char BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level) {
  if (level<0)
    return 0;

  assert(level <= alloc->num_levels);

  if(!isThereABuddy(&alloc->map, level)) {
    char parent_ptr=BuddyAllocator_getBuddy(alloc, level-1);

    //ma: there is nothing to do: all the memory is occupied
    if (parent_ptr < 0)
      return 0;

    //ma: parent already divided and these are the idx of his children
    int left_idx=(parent_ptr<<1)+1;
    int right_idx=left_idx+1;

    printf("split level:%d, left_idx: %d, right_idx: %d\r\n\n\n", level, left_idx, right_idx);
    BuddyAllocator_provideBuddy(alloc, left_idx);
    BuddyAllocator_provideBuddy(alloc, right_idx);
  }

  //ma: the level is not all busy [there is at least a buddy]
  if(isThereABuddy(&alloc->map, level)) {
    char idx = takeFirstIdx(&alloc->map, level);
    
    //ma: memory area no longer available
    BitMap_setBit(&alloc->map, idx, 0);
    return idx;
  }

  //assert(0);
  return 0;
}

void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, char item) {
  char parent_ptr=parentIdx(item);
  char buddy_ptr=buddyIdx(item);
  
  //ma: buddy becomes available again
  BitMap_setBit(&alloc->map, item, 1);
  
  // if on top of the chain, do nothing
  if (! item)
    return;
  
  // if the buddy of this item is not free, we do nothing 
  if (BitMap_bit(&alloc->map, buddy_ptr)==0 ) 
    return;
  
  //ma: we join the two buddies ...
  printf("merge %d\n", levelIdx(item));

  BuddyAllocator_restoringBuddy(alloc, item);
  BuddyAllocator_restoringBuddy(alloc, buddy_ptr);

  // ... and we make (release) the parent available
  BuddyAllocator_releaseBuddy(alloc, parent_ptr);

}

void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {

  // we determine the level of the page
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
  int  level=floor(log2(mem_size/(size+1)));

  // if the level is too small, we pad it to max level
  if (level>alloc->num_levels)
    level=alloc->num_levels;

  printf("requested: %d bytes, level %d \n", size, level);

  // we get a buddy of that size;
  uint8_t buddy=BuddyAllocator_getBuddy(alloc, level);
  if (! buddy)
    return 0;
  
  //ma: now we return a piece of memory
  char* buddymemorystart = start_memory(alloc, level, buddy);
  buddymemorystart[0] = buddy;

  // we write in the memory region managed the buddy idx
  return buddymemorystart + 1;
}

void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  printf("freeing %p\n", mem);
  
  // we retrieve the buddy from the system
  char* buddy_ptr=(char*)mem;
  char buddy = (--buddy_ptr)[0];
  
  printf("level %d\n", levelIdx(buddy));

  //ma: taking buddy idx
  BuddyAllocator_releaseBuddy(alloc, buddy);
}