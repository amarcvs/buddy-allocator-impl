#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "buddy_allocator.h"

//ma: helper functions :) ------------------------------------------------------------------------------------------------------------------------------
int levelIdx(size_t idx){
  return (int)floor(log2(idx + 1));
};

int buddyIdx(int idx){
  if (idx&0x1) return idx+1;
  return idx-1;
}

int parentIdx(int idx){
  return (idx-1)/2;
}

int startIdx(int idx){
  return (idx + 1 - (1<<levelIdx(idx)));
}

//ma: returns the amount of an item in a level "level"
int itemSize(BuddyAllocator * alloc, int level) {
  return (alloc->min_bucket_size*(1<<alloc->num_levels))/(1<<level);
}

char* start_memory(BuddyAllocator* alloc, char level, char buddy) {
  return (alloc->memory+(itemSize(alloc, level)*buddy));
}

void print_test(BitMap* map) {
  printf("[ ");
  for(int i = 0; i <32; i++) { // print first 4 byte
    printf("%u ", BitMap_bit(map, i));
  }
  printf("]\n");
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------

//ma: check if there is a Buddy in level "level"
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

//ma: takes the first free index on that level
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



void BuddyAllocator_createListItem(BuddyAllocator* alloc,
                                                char idx,
                                                char parent_ptr){
  // [1=available, 0=unavailableORreleased]
  BitMap_setBit(&alloc->map, idx, 1);
  printf("Available Item. idx:%d, level:%d, start:%p, size:%d\n", idx,
                                                                  levelIdx(idx),
                                                                  start_memory(alloc, levelIdx(idx), idx),
                                                                  itemSize(alloc, levelIdx(idx)));
}



/*// detaches and destroys an item in the free lists 
void BuddyAllocator_destroyListItem(BuddyAllocator* alloc, char item){
  //int level=levelIdx(item);
  //List_detach(&alloc->free[level], (ListItem*)item);///
  
  printf("Destroying Item. level:%d, idx:%d, start:%p, size:%d\n", level, item, start_memory(alloc, level, item, ((1<<alloc->num_levels)*alloc->min_bucket_size)/(1<<level)));
  BitMap_setBit(&alloc->map,item, 1);
  //PoolAllocatorResult release_result=PoolAllocator_releaseBlock(&alloc->list_allocator, item);///
  //assert(release_result==Success);///

}*/


void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         uint8_t* buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size){

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
  BuddyAllocator_createListItem(alloc, 0, 0);
}



char BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){
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
    BitMap_setBit(&alloc->map, left_idx, 1); 
    BitMap_setBit(&alloc->map, right_idx, 1);
    print_test(&alloc->map);
  }

  //ma: the level is not all busy [there is at least a buddy]
  if(isThereABuddy(&alloc->map, level)) {
    char idx = takeFirstIdx(&alloc->map, level);
    printf("INDICE %d\n", idx);

    //ma: memory area no longer available
    BitMap_setBit(&alloc->map, idx, 0);
    return idx;
  }

  //assert(0); 
  return 0;
}



void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, char item){
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
  BitMap_setBit(&alloc->map, item, 0); 
  BitMap_setBit(&alloc->map, buddy_ptr, 0);

  // ... and we make (release) the parent available
  BuddyAllocator_releaseBuddy(alloc, parent_ptr);

}



//allocates memory
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
  char* buddymemorystart = alloc->memory+(itemSize(alloc, level)*startIdx(buddy));
  buddymemorystart[0] = buddy;

  // we write in the memory region managed the buddy idx
  return buddymemorystart + 1;
}


//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  printf("freeing %p\n", mem); 
  
  // we retrieve the buddy from the system 
  char* buddy_ptr=(char*)mem;
  char buddy = (--buddy_ptr)[0];
  
  printf("level %d\n", levelIdx(buddy));

  //ma: taking buddy idx
  BuddyAllocator_releaseBuddy(alloc, buddy);
}