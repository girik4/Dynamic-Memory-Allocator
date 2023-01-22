///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2022 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission Fall 2022, CS354-deppeler
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Main File:        p3Heap.c
// This File:        p3Heap.c
// Other Files:      None 
// Semester:         CS 354 Fall 2022
// Instructor:       deppeler
//
// Author:           Girik Soni
// Email:            gsoni@wisc.edu
// CS Login:         girik
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   Fully acknowledge and credit all sources of help,
//                   including Peer Mentors, Instructors, and TAs.
//
// Persons:         Rahul Chakwate (TA) 
//                  
//
// Online sources:  None 
//                 
////////////////////////////////////////////////////////////////////////////////


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "p3Heap.h"

/*
 * This structure serves as the header or each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {           

    int size_status;

    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 */

 /*
 Size of the heap allocated. This changes with every balloc() and bfree() call
 */
int allocated_size;

 
/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 * - If a BEST-FIT block found is NOT found, return NULL
 *   Return NULL unable to find and allocate block for desired size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* balloc(int size) {     
    //TODO: Your code goes in here.
    if (size < 0 )
    {
        return NULL;
    }
    /*
    Masking and checking if everything is valid
    */
    int req_size = sizeof(blockHeader) + size;
    int diff = 8 - (req_size % 8);
    if (req_size % 8 == 0)
    {
        diff = 0;
    }
    
    req_size = req_size + diff;
    //printf("%i  %i", req_size, diff);

    int t_size = alloc_size - allocated_size;
    if (t_size & 1)
    {
        t_size = t_size - 1;
    }
    if (t_size & 2)
    {
        t_size = t_size - 2;
    }

    if (req_size > t_size)
    {
        return NULL;
    }

    blockHeader *current = heap_start;
    blockHeader *alloc = NULL;      //variable to keep track of the smallest available size bigger than the given size. Will be none if we find a perfect fit or if ther is no such fit
    int first = 0;
   // while (*current < ((char*)heap_start + alloc_size)) {
    while (current->size_status != 1)
    {
    
        t_size = current->size_status;
        if (t_size & 1)
        {
        t_size = t_size - 1;
        }
        if (t_size & 2)
        {
        t_size = t_size - 2;
        }

        if (!(current->size_status&1))
        {
            if (t_size == req_size)
            {
                current->size_status += 1;
                allocated_size += req_size;
                int size = current->size_status;
                if (size&1)
                {
                    size -= 1;
                }
                if (size&2)
                {
                    size -=2;
                }
                blockHeader *temp = (blockHeader*)((char*)current + size);
                if (temp->size_status != 1)
                {
                    temp->size_status +=2;
                }
                return (blockHeader*)((char*)current +sizeof(blockHeader));
            }
            if (t_size > req_size)
            {
                if (first == 0)
                {
                    alloc = current;
                    first = 1;
                }
                if (first == 1)
                {
                    int temp_size = alloc->size_status;
                    if (temp_size & 2)
                    {
                        temp_size -= 2;
                    }
                    if (t_size < temp_size)
                    {
                        alloc = current;
                    }                                      
                }               
            }                       
        }
        current = (blockHeader*)((char*)current + t_size);
    }

    //No match found
    if (alloc == NULL)
    {
       return NULL;
    }
    
    int prev = 0;
    int new_size = alloc->size_status;
    if (new_size&2)
    {
        prev = 1;
        new_size = new_size - 2;
    }

    int new_block = new_size - req_size; //for splitting
    if (new_block >= 8)
    {   
        alloc->size_status = req_size;
        if(prev == 1){
            alloc ->size_status += 2;
        }  
        alloc->size_status += 1;
        blockHeader *temp = (blockHeader*)((char*)alloc + req_size);
        temp->size_status = new_block+2;
        temp = (blockHeader*)((char*)temp + new_block);
        temp = (blockHeader*)((char*)temp - 4);
        temp->size_status = new_block;
        /*
        blockHeader *temp = (blockHeader*)((char*)alloc + req_size);
        temp->size_status = (new_block) + 2;
        temp = (blockHeader*)((char*)temp + new_block-2- 4);
        temp->size_status = new_block;
        alloc->size_status = req_size + 1;
        if (prev == 1)
        {
            alloc->size_status += 2;
        }
        //alloc->size_status += 1;
        */
    }else
    {
        blockHeader *temp = (blockHeader*)((char*)alloc + new_size);
        if (temp->size_status != 1)
        {
            temp->size_status += 2;
        }
        alloc->size_status += 1;
    }
    allocated_size += req_size;
    
    return (blockHeader*)((char*)alloc +sizeof(blockHeader));
} 
 
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */                    
int bfree(void *ptr) {    
    //TODO: Your code goes in here.
    blockHeader *current = (blockHeader*)(ptr - sizeof(blockHeader));
    if (ptr == NULL)
    {
      //  printf("a");
        return -1;
    }
    if (!(((unsigned int)ptr) % 8 == 0))
    {
       // printf("H");
        return -1;
    }
    if ((blockHeader*)ptr<heap_start || (blockHeader*)ptr > (blockHeader*)(char*)heap_start+alloc_size)
    {
       // printf("b");
        return -1;
    }
    if ((current->size_status & 1) == 0)
    {
      //  printf("Not free");
        return -1;
    }
    current->size_status -= 1;
    int size = current->size_status;
    if (size & 2)
    {
        size -= 2;
    }
    blockHeader *temp = (blockHeader*)((char*)current + size);
    temp->size_status -=2;
    temp = (blockHeader*)((char*)temp - sizeof(blockHeader));
    temp->size_status = size;
    allocated_size -= size;
    return 0;
} 

/*
 * Function for traversing heap block list and coalescing all adjacent 
 * free blocks.
 *
 * This function is used for delayed coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
    //TODO: Your code goes in here.
    blockHeader *current = heap_start;
    int flag = 0;           //flag for keeping track of free spaces
    int ret_flag = 0;       //flag for checking if any coalescing is being done this is what is being returned
    while (current->size_status!=1)
    {
        int prev_taken = 0;
        int t_size = current->size_status;
        if (t_size&2)
        {
            t_size -= 2;
        }
        if (t_size&1)
        {
            t_size-=1;
        }
        if ((current->size_status&1) == 0)
        {
            //printf("%08x", (unsigned int)(current));
            blockHeader *coalesce_current = current;
            t_size = coalesce_current->size_status;
            prev_taken = 0;
            if (t_size & 2)
            {
                t_size -= 2;
                prev_taken = 1;
            }
            coalesce_current = (blockHeader*)((char*)coalesce_current + t_size);
            
            while ((coalesce_current->size_status & 1) != 1)
            {
                if (coalesce_current->size_status==1)
                {
                    break;
                }
                
                int temp = coalesce_current->size_status;
                t_size += temp;
                coalesce_current = (blockHeader*)((char*)coalesce_current + temp);
                flag = 1;
                ret_flag = 1;
            }
            
        }
        if (flag == 1)
        {
            current->size_status = t_size;
            if (prev_taken == 1)
            {
                current->size_status+= 2;
            }
            blockHeader *temp = (blockHeader*)((char*)current + t_size - sizeof(blockHeader));
            temp->size_status = t_size;

            flag = 0;
        }
        current = (blockHeader*)((char*)current + t_size);
    }
	return ret_flag; 
}

 
/* 
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    
 
    static int allocated_once = 0; //prevent multiple myInit calls
 
    int pagesize;   // page size
    int padsize;    // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int fd;

    blockHeader* end_mark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
} 
                  
/* 
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     
 
    int counter;
    char status[6];
    char p_status[6];
    char *t_begin = NULL;
    char *t_end   = NULL;
    int t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used   = -1;

    fprintf(stdout, 
	"*********************************** Block List **********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
	"*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
	"*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end of myHeap.c (Spring 2022)                                         


