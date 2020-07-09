#include "buddy_allocator.h"
#include <stdlib.h>
#include <stdio.h>

#define BUDDY_LEVELS 7
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS))

// ma: maximum number of bits, used to size the bitmap
#define BUFFER_SIZE ((1<<(BUDDY_LEVELS+1))-1)

char memory[MEMORY_SIZE];
uint8_t buffer[BUFFER_SIZE];

BuddyAllocator alloc;

/* ma: to see how the bitmap changes over time you should uncomment below
   but for better clarity reduce the number of levels (e.g. 4 levels) */
// void print_test(BitMap* map); 

int main(int argc, char** argv) {

  printf("Allocator initialization...\n");
  BuddyAllocator_init(&alloc, BUDDY_LEVELS, buffer, BUFFER_SIZE, memory, MIN_BUCKET_SIZE);
  printf("DONE\n\n");
  
  // print_test(&alloc.map);
  printf("|FIRST ALLOCATION:\n");
  char* p1=(char*)BuddyAllocator_malloc(&alloc, 500800);
  printf("\n");
  //print_test(&alloc.map);

  printf("|SECOND ALLOCATION:\n");
  int* p2=(int*)BuddyAllocator_malloc(&alloc, 100);
  printf("\n");
  //print_test(&alloc.map);

  printf("|THIRD ALLOCATION:\n");
  int* p3=(int*)BuddyAllocator_malloc(&alloc, 70001);
  printf("\n");
  //print_test(&alloc.map);

  printf("|FOURTH ALLOCATION:\n");
  void* p4=BuddyAllocator_malloc(&alloc, 5000);
  printf("\n");
  //print_test(&alloc.map);

  printf("|FIFTH ALLOCATION:\n");
  long* p5=(long*)BuddyAllocator_malloc(&alloc, 60670);
  printf("\n");
  //print_test(&alloc.map);

  printf("|SIXTH ALLOCATION:\n");
  char* p6=(char*)BuddyAllocator_malloc(&alloc, 50);
  printf("\n");
  //print_test(&alloc.map);
  
  printf("|SEVENTH ALLOCATION:\n");
  double* p7=(double*)BuddyAllocator_malloc(&alloc, 333);
  //print_test(&alloc.map);

  printf("_____________________________________________________________________\n\n");

  printf("|FIRST DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p1);
  printf("\n");
  //print_test(&alloc.map);

  printf("|SECOND DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p2);
  printf("\n");
  //print_test(&alloc.map);

  printf("|THIRD DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p3);
  printf("\n");
  //print_test(&alloc.map);

  printf("|FOURTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p4);
  printf("\n");
  //print_test(&alloc.map);;

  printf("|FIFTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p5);
  printf("\n");
  //print_test(&alloc.map);

  printf("|SIXTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p6);
  printf("\n");
  //print_test(&alloc.map);

  printf("|SEVENTH DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p7);
  //print_test(&alloc.map);

  printf("_____________________________________________________________________\n\n");

  printf("|ANOTHER ALLOCATION:\n");
  p1=BuddyAllocator_malloc(&alloc, 55);
  printf("\n");
  //print_test(&alloc.map);

  printf("|ANOTHER DEALLOCATION:\n");
  BuddyAllocator_free(&alloc, p1);
  //print_test(&alloc.map);

  printf("_____________________________________________________________________\n\n");
	
  // ma: now we release backwards 
  printf("|ALLOCATION #1:\n");
  p2=BuddyAllocator_malloc(&alloc, 87554);
  printf("\n");
  //print_test(&alloc.map);

  printf("|ALLOCATION #2:\n");
  p3=BuddyAllocator_malloc(&alloc, 342);
  printf("\n");
  //print_test(&alloc.map);

  printf("|ALLOCATION #3:\n");
  p4=BuddyAllocator_malloc(&alloc, 500400);
  printf("\n");
  //print_test(&alloc.map);

  printf("|DEALLOCATION #1:\n");
  BuddyAllocator_free(&alloc, p4);
  printf("\n");
  //print_test(&alloc.map);

  printf("|DEALLOCATION #2:\n");
  BuddyAllocator_free(&alloc, p3);
  printf("\n");
  //print_test(&alloc.map);

  printf("|DEALLOCATION #3:\n");
  BuddyAllocator_free(&alloc, p2);
  //print_test(&alloc.map);

  return EXIT_SUCCESS;
}