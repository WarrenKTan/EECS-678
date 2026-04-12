#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAXSTR 1000

int main(int argc, char *argv[])
{
  char line[MAXSTR];
  int *page_table, *mem_map;
  unsigned int log_size, phy_size, page_size, d;
  unsigned int num_pages, num_frames;
  unsigned int offset, logical_addr, physical_addr, page_num, frame_num;

  /* Get the memory characteristics from the input file */
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Logical address space size: %d^%d", &d, &log_size)) != 2){
    fprintf(stderr, "Unexpected line 1. Abort.\n");
    exit(-1);
  }
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Physical address space size: %d^%d", &d, &phy_size)) != 2){
    fprintf(stderr, "Unexpected line 2. Abort.\n");
    exit(-1);
  }
  fgets(line, MAXSTR, stdin);
  if((sscanf(line, "Page size: %d^%d", &d, &page_size)) != 2){
    fprintf(stderr, "Unexpected line 3. Abort.\n");
    exit(-1);
  }

  /* Allocate arrays to hold the page table and memory frames map */
  num_pages = pow(d, log_size - page_size);
  num_frames = pow(d, phy_size - page_size);

  
  fprintf(stdout, "Number of Pages: %x, Number of Frames: %x\n\n", num_pages, num_frames);

  // int table with 'num_pages' possible indices
  page_table = (int *)malloc(num_pages * sizeof(int));

  // int map with 'num_frames' possible indices
  mem_map = (int *)malloc(num_frames * sizeof(int));

  /* Initialize page table to indicate that no pages are currently mapped to physical memory */
  for(int i = 0; i < num_pages; i++){
    page_table[i] = -1;
  }

  /* Initialize memory map table to indicate no valid frames */
  for(int i = 0; i < num_frames; i++){
    mem_map[i] = -1;
  }

  /* Read each accessed address from input file. Map the logical address to
     corresponding physical address */
  fgets(line, MAXSTR, stdin);
  while(!(feof(stdin))){
    sscanf(line, "0x%x", &logical_addr);
    fprintf(stdout, "Logical Address: 0x%x\n", logical_addr);
    
	  /* Calculate page number and offset from the logical address */
    // lower bits
    offset = logical_addr & ((1 << page_size) - 1);
    
    // upper bits ('page_size' length)
    page_num = logical_addr >> page_size;
    fprintf(stdout, "Page Number: %x\n", page_num);

    /* Form corresponding physical address */
    if(page_table[page_num] == -1){
      // page fault
      fprintf(stdout, "Page Fault!\n");

      // find the next empty location to map the address
      for(unsigned int i = 0; i < num_frames; i++){
        if (mem_map[i] == -1) {
          frame_num = i;
          mem_map[i] = page_num;
          page_table[page_num] = frame_num;
          break;
        }
      }
    }else{
      // map the address to the found location
      frame_num = page_table[page_num];
    }
    fprintf(stdout, "Frame Number: %x\n", frame_num);

    physical_addr = (frame_num << page_size) | offset;
    fprintf(stdout, "Physical Address: 0x%x\n\n", physical_addr);

    /* Read next line */
    fgets(line, MAXSTR, stdin);
  }

  return 0;
}
