#include "buddy_allocator.h"
#include <stdio.h>


#define BUDDY_LEVELS 3
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS))

//ma: maximum number of bits, used to size the bitmap
#define BUFFER_SIZE ((1<<(BUDDY_LEVELS+1))-1) 

char memory[MEMORY_SIZE];
uint8_t buffer[BUFFER_SIZE];

BuddyAllocator alloc;

int main(int argc, char** argv) {

  // we initialize the allocator
  printf("init... ");
  BuddyAllocator_init(&alloc,
  					  BUDDY_LEVELS,
                      buffer,
                      BUFFER_SIZE,
                      memory,
                      MIN_BUCKET_SIZE);
  printf("DONE\n");

  
  void* p1=BuddyAllocator_malloc(&alloc, 100);
  void* p2=BuddyAllocator_malloc(&alloc, 100);
  void* p3=BuddyAllocator_malloc(&alloc, 100000);
  /*
	  BuddyAllocator_free(&alloc, p1);
	  BuddyAllocator_free(&alloc, p2);
	  BuddyAllocator_free(&alloc, p3);
  */
  
}
