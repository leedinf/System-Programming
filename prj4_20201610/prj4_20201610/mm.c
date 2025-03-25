/*
 * mm-efree.c - Implementation of malloc using an explicit free list
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
    /* Your student ID */
    "20201610",
    /* Your full name*/
    "Doan Lee",
    /* Your email address */
    "dhdksdl@sogang.ac.kr",
};

// /* single word (4) or double word (8) alignment */
// #define ALIGNMENT 8

// /* rounds up to the nearest multiple of ALIGNMENT */
// #define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define NEXT_FIT

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ //line:vm:mm:beginconst
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  //line:vm:mm:endconst 

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) //line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            //line:vm:mm:get
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    //line:vm:mm:put

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   //line:vm:mm:getsize
#define GET_ALLOC(p) (GET(p) & 0x1)                    //line:vm:mm:getalloc

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      //line:vm:mm:hdrp
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) //line:vm:mm:ftrp

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) //line:vm:mm:nextblkp
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) //line:vm:mm:prevblkp
/* $end mallocmacros */
// #define NEXT_FREE(bp) ((char *)(bp) + WSIZE);
#define SET_P(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))
#define NEXT_P(ptr) ((char *)(ptr))
#define PREV_P(ptr) ((char *)(ptr) + WSIZE)
#define NEXT_NODE(ptr) (*(char **)(ptr))
#define PREV_NODE(ptr) (*(char **)(PREV_P(ptr)))

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */  

//Free SegList
#define LISTNUM 10
static void *seglist[LISTNUM];
static void removeFree(void *bp);
static void insertFree(void *bp, size_t size);
static int getClass(size_t size);

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void *place(void *bp, size_t asize);
/* 
 * mm_init - Initialize the memory manager 
 */
/* $begin mminit */
int mm_init(void) 
{
    /* Create the initial empty heap */


    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) //line:vm:mm:begininit
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */ 
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE);                     //line:vm:mm:endinit  

    for(int i=0;i<LISTNUM; i++){
        seglist[i] = NULL;
    }
    if(extend_heap(4)==NULL)return -1;
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) 
        return -1;
    return 0;
}

void *mm_malloc(size_t size) 
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;      

    // if (heap_listp == 0){
    //     mm_init();
    // }
    // if (size == 0)
    //     return NULL;

    if (size <= DSIZE)                                          //line:vm:mm:sizeadjust1
        asize = 2*DSIZE;                                        //line:vm:mm:sizeadjust2
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE); //line:vm:mm:sizeadjust3

    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {  //line:vm:mm:findfitcall
        place(bp, asize);                  //line:vm:mm:findfitplace
        return bp;
    }
    
    
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);                 //line:vm:mm:growheap1
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)  
        return NULL;       


    place(bp, asize);    
                                 //line:vm:mm:growheap3
    return bp;
} 

static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; //line:vm:mm:beginextend
    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        //line:vm:mm:endextend

    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   //line:vm:mm:freeblockhdr
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   //line:vm:mm:freeblockftr
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ //line:vm:mm:newepihdr

    insertFree(bp,size);

    /* Coalesce if the previous block was free */
    return coalesce(bp);                                          //line:vm:mm:returnblock
}
static void *coalesce(void *bp) 
{
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;//이미 insert 함.
    }


    else if (prev_alloc && !next_alloc) {      /* Case 2 뒤지우고하나로*/
        removeFree(bp);
        removeFree(NEXT_BLKP(bp)); 
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
        insertFree(bp,size);
        return bp;
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 앞지우고하나로*/
        removeFree(bp);
        removeFree(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insertFree(bp,size);
        return bp;
    }

    else {                                     /* Case 4 앞뒤지우고하나로*/
        removeFree(bp);
        removeFree(PREV_BLKP(bp));
        removeFree(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insertFree(bp,size);
        return bp;
    }

}

void mm_free(void *bp)
{
    if (bp == 0) 
        return;

    size_t size = GET_SIZE(HDRP(bp));
    if (heap_listp == 0){
        mm_init();
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    insertFree(bp,size);


    coalesce(bp);

}

static void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));   
    removeFree(bp);
    if ((csize - asize) >= (2*DSIZE)) { 
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(csize-asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(csize-asize, 0)); // 할당하고 남은 블록
        insertFree(NEXT_BLKP(bp), csize - asize);
    }
    else { 
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }

}
static void insertFree(void *bp, size_t size) {


    void *findp = bp;
    int c = getClass(size);
    findp = seglist[c];
    void *insertp = NULL;

    int t =0;
    if(findp != NULL){
        t=GET_SIZE(HDRP(findp));
    }
    while ((findp != NULL) && (size > t) ) {
        insertp = findp;        
        findp = NEXT_NODE(findp); 
        if(findp != NULL) t=GET_SIZE(HDRP(findp)); 
    }

    if (insertp != NULL) {       
        SET_P(NEXT_P(bp), findp);     
        SET_P(NEXT_P(insertp), bp);
        SET_P(PREV_P(bp), insertp);     
    } else {                        
        SET_P(PREV_P(bp), NULL);
        SET_P(NEXT_P(bp), findp);     
        seglist[c] = bp;

    }

    if (findp != NULL) {
        SET_P(PREV_P(findp), bp);     
    }
    return;
}

static void removeFree(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int c = getClass(size);

    void *next_bp = NEXT_NODE(bp);
    void *prev_bp = PREV_NODE(bp);

    // 다음 노드가 있는 경우
    if (next_bp != NULL) {
        SET_P(PREV_P(next_bp), prev_bp);
    }

    // 이전 노드가 있는 경우
    if (prev_bp != NULL) {
        SET_P(NEXT_P(prev_bp), next_bp);
    } else {
        seglist[c] = next_bp; // 리스트 헤드를 업데이트
    }

    return;
}

static void *find_fit(size_t asize)
{
    char *bp;
    size_t limitsize = asize;

    int idx = 0;
    while (idx < LISTNUM) {
        if ( (seglist[idx] != NULL) && (limitsize <= 1) || (idx >= LISTNUM - 1)  ) {
            bp = seglist[idx];
            while (bp != NULL) {
                if (asize <= GET_SIZE(HDRP(bp))) {
                    return bp; 
                }
                bp = NEXT_NODE(bp);
            }
        }
        limitsize >>= 1;
        idx++;
    }
    return NULL; 
}
void *mm_realloc(void *ptr, size_t size) {
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return mm_malloc(size);
    }

    size_t asize = (size <= DSIZE) ? 2 * DSIZE : DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
    size_t oldsize = GET_SIZE(HDRP(ptr));

    if (asize <= oldsize) {
        return ptr;
    }

    void *prev = PREV_BLKP(ptr);
    size_t prev_free = !GET_ALLOC(HDRP(prev));
    size_t prev_size = GET_SIZE(HDRP(prev));
    size_t total_p = oldsize + prev_size;
    void *next = NEXT_BLKP(ptr);
    size_t next_free = !GET_ALLOC(HDRP(next));
    size_t next_size = GET_SIZE(HDRP(next));
    size_t total_n = oldsize + next_size;

    if (prev_free && (total_p >= asize)) {
        removeFree(prev);
        PUT(HDRP(prev), PACK(total_p, 1));
        PUT(FTRP(ptr), PACK(total_p, 1));
        memmove(prev, ptr, oldsize);
        return prev;
    }
    if (next_free && (total_n >= asize)) {
        removeFree(next);
        PUT(HDRP(ptr), PACK(total_n, 1));
        PUT(FTRP(ptr), PACK(total_n, 1));
        return ptr;
    }



    if (prev_free && next_free && (oldsize + prev_size + next_size >= asize)) {
        total_n = oldsize + prev_size + next_size;
        removeFree(prev);
        removeFree(next);
        PUT(HDRP(prev), PACK(total_n, 1));
        PUT(FTRP(next), PACK(total_n, 1));
        memmove(prev, ptr, oldsize);
        return prev;
    }

    void *newptr = mm_malloc(size);
    if (newptr == NULL) {
        return NULL;
    }

    size_t copy_size = (oldsize < size) ? oldsize : size;
    memcpy(newptr, ptr, copy_size);

    mm_free(ptr);

    return newptr;
}


static int getClass(size_t size) {
    int idx = 0;
    size_t s = size;

    while ((idx < LISTNUM - 1) && (size > 1)) {
        size = size / 2;
        idx++;
    }

    return idx;
}
