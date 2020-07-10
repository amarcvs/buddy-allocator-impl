#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "buddy_allocator.h"

//ma: helper functions :) ------------------------------------------------------------------------------------------------------------------------------
int levelIdx(int idx) {
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
void* start_memory(BuddyAllocator* alloc, int level, int buddy) {
  return alloc->memory+(itemSize(alloc, level)*startIdx(buddy));
}

//ma: print the bitmap
void print_test(BitMap* map) {
  printf("Bitmap state [ ");
  for(int i = 0; i <map->num_bits; i++) {
    printf("%u ", BitMap_bit(map, i));
  }
  printf("]\n\n");
}

//ma: change the status of the buddy
void BuddyAllocator_setBuddy(BuddyAllocator* alloc, int idx, char status) {
  // [1=available, 0=unavailableORreleased]
  if(status) BitMap_setBit(&alloc->map, idx, 1);
  else BitMap_setBit(&alloc->map, idx, 0);

  printf("item. idx:%d, level:%d, start:%p, size:%d\n", idx,
                                                        levelIdx(idx),
                                                        start_memory(alloc, levelIdx(idx), idx),
                                                        itemSize(alloc, levelIdx(idx)));
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

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

  //ma: we initialize the bitmap
  uint8_t* bufferMap=buffer;
  BitMap_init(&alloc->map, map_items, bufferMap);

  //printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d\n", num_levels);
  printf("\tmax bitmap entries: %d\n", map_items);
  printf("\tbitmap size: %d bytes\n", alloc->map.buffer_size);
  printf("\tmanaged memory: %d bytes\n", (1<<num_levels)*min_bucket_size);
  printf("\tbucket size: %d bytes\n", min_bucket_size);
  
  //ma: we set first bit of bitmap to mark that all memory is available
  BuddyAllocator_setBuddy(alloc, 0, 1);
}

int BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level) {
  if (level<0)
    return -1;

  assert(level <= alloc->num_levels);

  if(!isThereABuddy(&alloc->map, level)) {
    int parent_ptr=BuddyAllocator_getBuddy(alloc, level-1);

    //ma: there is nothing to do: all the memory is occupied
    if (parent_ptr < 0)
      return -1;

    //ma: parent already divided and these are the idx of his children
    int left_idx=(parent_ptr<<1)+1;
    int right_idx=left_idx+1;

    printf("splitting level %d:\n", level-1);

    BuddyAllocator_setBuddy(alloc, left_idx, 1);
    BuddyAllocator_setBuddy(alloc, right_idx, 1);
  }

  //ma: the level is not all busy [there is at least a buddy]
  if(isThereABuddy(&alloc->map, level)) {
    int idx = takeFirstIdx(&alloc->map, level);
    
    //ma: memory area no longer available
    BitMap_setBit(&alloc->map, idx, 0);
    return idx;
  }

  //ma: a problem happened
  return -1;
}

void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int buddy) {
  int parent_ptr=parentIdx(buddy);
  int buddy_ptr=buddyIdx(buddy);
  
  //ma: buddy becomes available again
  BitMap_setBit(&alloc->map, buddy, 1);
  
  // if on top of the chain, do nothing
  if (! buddy)
    return;
  
  // if the "buddy" of this buddy is not free, we do nothing 
  if (BitMap_bit(&alloc->map, buddy_ptr)==0 ) 
    return;
  
  //ma: we merge the two buddies ...
  printf("merging level %d:\n", levelIdx(buddy));

  BuddyAllocator_setBuddy(alloc, buddy, 0);
  BuddyAllocator_setBuddy(alloc, buddy_ptr, 0);

  // ... and we make (release) the parent available
  BuddyAllocator_releaseBuddy(alloc, parent_ptr);

}

void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {

  // we determine the level of the page
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
  int  level=floor(log2(mem_size/(size+4)));

  // if the level is too small, we pad it to max level
  if (level>alloc->num_levels)
    level=alloc->num_levels;

  printf("requested %d bytes, level %d \n", size, level);

  // we get a buddy of that size;
  int buddy=BuddyAllocator_getBuddy(alloc, level);

  // sanity check;
  assert(buddy >= 0 && "Not enough memory!");
  
  //ma: now we return a piece of memory
  int* buddymemorystart = start_memory(alloc, level, buddy);
  buddymemorystart[0] = buddy;

  // we write in the memory region managed the buddy idx
  return buddymemorystart + 1;
}

void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  // we retrieve the buddy from the system
  int* buddy_ptr=(int*)mem;
  int buddy = (--buddy_ptr)[0];
  int level = levelIdx(buddy);
  int dim = itemSize(alloc, level);
  printf("freeing %p, idx %d, level %d, %d bytes\n", buddy_ptr, buddy, level, dim);
  
  // sanity check;
  assert(buddy_ptr==start_memory(alloc, level, buddy));

  //ma: taking buddy idx
  BuddyAllocator_releaseBuddy(alloc, buddy);
}