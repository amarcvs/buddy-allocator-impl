#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "buddy_allocator.h"

//ma: helper functions :)
int levelIdx(size_t idx){
  return (int)floor(log2(idx));
};

int buddyIdx(int idx){
  if (idx&0x1){
    return idx-1;
  }
  return idx+1;
}

int parentIdx(int idx){
  return idx/2;
}

int startIdx(int idx){
  return (idx-(1<<levelIdx(idx)));
}


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
  int next_itemsPerLevelPrec = next_itemsPerLevel-1;
  
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
      return map->buffer[j]>>(8-next_bit_in_byte);
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
  char byte_num=bit_num>>3;
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
  return 0;
}

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

  //ma: maybe we can add some other interesting information
  printf("BUDDY INITIALIZING\n");
  printf("\tlevels: %d\n", num_levels);
  printf("\tbucket size:%d\n", min_bucket_size);
  printf("\tmanaged memory %d bytes\n", (1<<num_levels)*min_bucket_size);
  
  //ma: we initialize the bitmap
  uint8_t* bufferMap=buffer;
  BitMap_init(&alloc->map, buffer_size, bufferMap); // with &?

  //ma: we set first bit of bitmap to mark that all memory is available
  BitMap_setBit(&alloc->map, 0, 1); // [1=available, 0=unavailableORreleased]

};


BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level){
  if (level<0)
    return 0;
  assert(level <= alloc->num_levels);

  if (! alloc->free[level].size ) { // no buddies on this level
    BuddyListItem* parent_ptr=BuddyAllocator_getBuddy(alloc, level-1);
    if (! parent_ptr)
      return 0;

    // parent already detached from free list
    int left_idx=parent_ptr->idx<<1;
    int right_idx=left_idx+1;
    
    printf("split l:%d, left_idx: %d, right_idx: %d\r", level, left_idx, right_idx);
    BuddyListItem* left_ptr=BuddyAllocator_createListItem(alloc,left_idx, parent_ptr);
    BuddyListItem* right_ptr=BuddyAllocator_createListItem(alloc,right_idx, parent_ptr);
    // we need to update the buddy ptrs
    left_ptr->buddy_ptr=right_ptr;
    right_ptr->buddy_ptr=left_ptr;
  }
  // we detach the first
  if(alloc->free[level].size) {
    BuddyListItem* item=(BuddyListItem*)List_popFront(alloc->free+level);
    return item;
  }
  /*if (map->buffer[level]) { //ma: the level is not all busy [there is at least a buddy]
    // restituita la memoria:
    // 1) funzioni su arrray che prende un intervallo di memoria 
    // 2) buddy sx o destro osservando se sono di idx pari o dispari
  }*/

  if(isThereABuddy(&alloc->map, level)) {
    char idx = takeFirstIdx(&alloc->map, level); // prendi il primo bit libero del livello // setta a 0 uno dei bit 
    // consegna memoria
  }

  assert(0);
  return 0;
}
/*
void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, BuddyListItem* item){

  BuddyListItem* parent_ptr=item->parent_ptr;
  BuddyListItem *buddy_ptr=item->buddy_ptr;
  
  // buddy back in the free list of its level
  List_pushFront(&alloc->free[item->level],(ListItem*)item);

  // if on top of the chain, do nothing
  if (! parent_ptr)
    return;
  
  // if the buddy of this item is not free, we do nothing
  if (buddy_ptr->list.prev==0 && buddy_ptr->list.next==0) 
    return;
  
  //join
  //1. we destroy the two buddies in the free list;
  printf("merge %d\n", item->level);
  BuddyAllocator_destroyListItem(alloc, item);
  BuddyAllocator_destroyListItem(alloc, buddy_ptr);
  //2. we release the parent
  BuddyAllocator_releaseBuddy(alloc, parent_ptr);

}
*/

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size) {
  // we determine the level of the page
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
  int  level=floor(log2(mem_size/(size+8))); //ma: should be seize and not -8 in this case

  // if the level is too small, we pad it to max
  if (level>alloc->num_levels)
    level=alloc->num_levels;

  printf("requested: %d bytes, level %d \n", size, level);

  // we get a buddy of that size;
  BuddyListItem* buddy=BuddyAllocator_getBuddy(alloc, level);
  if (! buddy)
    return 0;

  // we write in the memory region managed the buddy address
  BuddyListItem** target= (BuddyListItem**)(buddy->start);
  *target=buddy;
  return buddy->start+8;
}

/*
//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem) {
  printf("freeing %p", mem);
  // we retrieve the buddy from the system
  char* p=(char*) mem;
  p=p-8;
  BuddyListItem** buddy_ptr=(BuddyListItem**)p;
  BuddyListItem* buddy=*buddy_ptr;
  //printf("level %d", buddy->level);
  // sanity check;
  assert(buddy->start==p);
  BuddyAllocator_releaseBuddy(alloc, buddy);
  
}*/
