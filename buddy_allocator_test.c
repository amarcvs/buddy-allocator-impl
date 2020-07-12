#include "buddy_allocator.h"
#include <stdlib.h>
#include <stdio.h>
#include "bit_map.h"

#define BUDDY_LEVELS 7
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS))

// ma: maximum number of bits, used to size the bitmap
#define BUFFER_SIZE ((1<<(BUDDY_LEVELS+1))-1)

char memory[MEMORY_SIZE];
uint8_t buffer[BUFFER_SIZE];

BuddyAllocator alloc;

int main(int argc, char** argv) {

  /* ma: to see how the bitmap changes over time you can call print_bitmap function
   	 every allocation/deallocation; for better clarity reduce the number of levels (e.g. 4 levels) */

  printf("Allocator initialization...\n");
  BuddyAllocator_init(&alloc, BUDDY_LEVELS, buffer, BUFFER_SIZE, memory, MIN_BUCKET_SIZE);
  printf("DONE\n\n");
  
  printf("_STAGE_1________________________________________________________\n\n");

  //ma: #1 allocations and deallocations in increasing order of memory required
  printf("|FIRST ALLOCATION:\n");
  char* p1=(char*)BuddyAllocator_malloc(&alloc, 45);
  printf("\n");

  printf("|SECOND ALLOCATION:\n");
  int* p2=(int*)BuddyAllocator_malloc(&alloc, 1024);
  printf("\n");
  
  printf("|THIRD ALLOCATION:\n");
  double* p3=(double*)BuddyAllocator_malloc(&alloc, 90000);
  printf("\n");
  
  printf("|FOURTH ALLOCATION:\n");
  void* p4=BuddyAllocator_malloc(&alloc, 257000);
  printf("\n");
  
  printf("|FIFTH ALLOCATION:\n");
  long* p5=(long*)BuddyAllocator_malloc(&alloc, 502587);
  printf("\n");
  
  printf("|FIRST DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p1);
  printf("\n");
  
  printf("|SECOND DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p2);
  printf("\n");
  
  printf("|THIRD DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p3);
  printf("\n");
  
  printf("|FOURTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p4);
  printf("\n");
  
  printf("|FIFTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p5);
  printf("\n");
  
  printf("_STAGE_2________________________________________________________\n\n");

  //ma: #2 trying to make an invalid allocations/deallocations
  printf("|FIRST INVALID ALLOCATION/DEALLOCATION:\n");
  p1=(char*)BuddyAllocator_malloc(&alloc, 0);
  if (!p1) printf("Error: impossible to allocate this amount of memory!\n");

  BuddyAllocator_free(&alloc, p1);
  printf("\n");

  printf("|SECOND INVALID ALLOCATION/DEALLOCATION:\n");
  p2=(int*)BuddyAllocator_malloc(&alloc, 5000000);
  if (!p2) printf("Error: impossible to allocate this amount of memory!\n");

  BuddyAllocator_free(&alloc, p2);
  printf("\n");

  printf("|THIRD INVALID ALLOCATION/DEALLOCATION:\n");
  p3=(double*)BuddyAllocator_malloc(&alloc, -56);
  if (!p3) printf("Error: impossible to allocate this amount of memory!\n");

  BuddyAllocator_free(&alloc, p3);
  printf("\n");

  printf("_STAGE_3________________________________________________________\n\n");
	
  //ma: #3 and now we also deallocate in reverse
  printf("|FIRST ALLOCATION:\n");
  p1=(char*)BuddyAllocator_malloc(&alloc, 644);
  printf("\n");
  
  printf("|SECOND ALLOCATION:\n");
  p2=(int*)BuddyAllocator_malloc(&alloc, 20546);
  printf("\n");
  
  printf("|THIRD ALLOCATION:\n");
  p3=(double*)BuddyAllocator_malloc(&alloc, 100000);
  printf("\n");
  
  printf("|FOURTH ALLOCATION:\n");
  p4=BuddyAllocator_malloc(&alloc, 250123);
  printf("\n");
  
  printf("|FIFTH ALLOCATION:\n");
  p5=(long*)BuddyAllocator_malloc(&alloc, 502587);
  printf("\n");
  
  printf("|FIRST DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p5);
  printf("\n");
  
  printf("|SECOND DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p4);
  printf("\n");
  
  printf("|THIRD DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p3);
  printf("\n");
  
  printf("|FOURTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p2);
  printf("\n");
  
  printf("|FIFTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p1);
  printf("\n");
  
  printf("_STAGE_4________________________________________________________\n\n");
	
  //ma: #4 scattered allocations and deallocations in scattered order of memory required
  printf("|FIRST ALLOCATION:\n");
  p1=(char*)BuddyAllocator_malloc(&alloc, 500000);
  printf("\n");
  
  printf("|SECOND ALLOCATION:\n");
  p2=(int*)BuddyAllocator_malloc(&alloc, 256);
  printf("\n");
  
  printf("|THIRD ALLOCATION:\n");
  p3=(double*)BuddyAllocator_malloc(&alloc, 100000);
  printf("\n");
  
  printf("|FOURTH ALLOCATION:\n");
  p4=BuddyAllocator_malloc(&alloc, 250000);
  printf("\n");
  
  printf("|FIFTH ALLOCATION:\n");
  p5=(long*)BuddyAllocator_malloc(&alloc, 8000);
  printf("\n");
  
  printf("|FIRST DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p3);
  printf("\n");
  
  printf("|SECOND DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p5);
  printf("\n");
  
  printf("|THIRD DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p1);
  printf("\n");
  
  printf("|FOURTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p4);
  printf("\n");
  
  printf("|FIFTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p2);
  printf("\n");
  
  return EXIT_SUCCESS;
}