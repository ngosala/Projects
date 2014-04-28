#include "storage_mgr.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "dt.h"

struct buffer_LRU {
    int pos;
    int pageNum;
    int fixCount;
    bool dirty;
    int age;
    char *data;
    struct buffer_LRU *next;
} *start_lru = NULL;

struct buffer_Queue {
    int pos;
    int pageNum;
    int fixCount;
    bool dirty;
    char *data;
    struct buffer_Queue *next;
} *strt_fifo = NULL, *rear_fifo = NULL, *handler = NULL;

struct buffer_Clock {
    int pos;
    int pageNum;
    int fixCount;
    bool dirty;
    char *data;
    int ref;
    struct buffer_Clock *next;
} *strt_clock = NULL, *clock_hand = NULL;

typedef struct buffer_pool_stat {
    int readIO;
    int writeIO;
    PageNumber *pageNumber_stat;
    int *fixCounts_stat;
    bool *dirtyPages_stat;
} buffer_pool_stat;

typedef struct buffer_stat {
    void *buf;
    struct buffer_pool_stat *pool_Stat;
    SM_FileHandle *fh;
} buffer_stat;


// convenience macros
#define MAKE_LRU_BUFFER()					\
  ((struct buffer_LRU *) malloc (sizeof(struct buffer_LRU)))

#define MAKE_QUEUE_BUFFER()				\
  ((struct buffer_Queue *) malloc (sizeof(struct buffer_Queue)))

/*
#define MAKE_CLOCK_BUFFER()				\
  ((struct buffer_Clock *) malloc (sizeof(struct buffer_Clock)))
 */

#define MAKE_BUFFER_STAT()				\
  ((buffer_stat *) malloc (sizeof(buffer_stat)))

#define MAKE_BUFFER_POOL_STAT()				\
  ((buffer_pool_stat *) malloc (sizeof(buffer_pool_stat)))


//LRU Page replacement methods

/*
 * AUTHOR: Nikhil
 * METHOD: Calculate Length of Buffer
 * INPUT: NONE
 * OUTPUT: Length of buffer
 */
int lengthofBuffer() {
    struct buffer_LRU *root;
    root = start_lru;
    int count = 0;
    while (root != NULL) {
        count++;
        root = root->next;
    }
    return count;
}

/*
 * AUTHOR: Nikhil
 * METHOD: Search for Frame
 * INPUT: PageNUmber
 * OUTPUT: Page Frame Position
 */
int searchForFrame_LRU(PageNumber pNum) {
    int flag = -1;
    struct buffer_LRU *temp;
    temp = start_lru;
    while (temp != NULL) {
        if (temp->pageNum == pNum) {
            flag = temp->pos;
            break;
        }
        temp = temp->next;
    }
    return flag;
}

/*
 * AUTHOR: Nikhil
 * METHOD: LRU algorithm
 * INPUT: NONE
 * OUTPUT: position
 */
int LRU() {
    struct buffer_LRU *temp;
    temp = start_lru;
    int pos;
    int max;
    max = temp->age; //Sets max to first value
    pos = temp->pos;
    while (temp != NULL) {
        if (temp->age > max) {
            max = temp->age;
            pos = temp->pos;
        }
        temp = temp->next;
    }
    return pos;
}

/*
 * AUTHOR: Nikhil
 * METHOD: increment age
 * INPUT: NONE
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC increment_age() {
    struct buffer_LRU *root;
    root = start_lru;
    while (root != NULL) {
        root->age = root->age + 1;
        root = root->next;
    }
    return RC_OK;
}

/*
 * AUTHOR: Nikhil
 * METHOD: max position
 * INPUT: NONE
 * OUTPUT: position of the file
 */
int maxpos() {
    struct buffer_LRU *temp;
    temp = start_lru;
    int pos;
    int max;
    max = temp->pos; //Sets max to first value
    while (temp != NULL) {
        if (temp->pos > max) {
            max = temp->pos;
        }
        temp = temp->next;
    }
    return max;

}

/*
 * AUTHOR: Nikhil
 * METHOD: insert into LRU buffer
 * INPUT: File Structure
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC insert_into_buffer_LRU(BM_BufferPool * const bm, BM_PageHandle * const page) {
    buffer_stat *stat;
    struct buffer_LRU *temp, *temp1;
    buffer_pool_stat *pool_stat;
    int searchRes;
    temp = MAKE_LRU_BUFFER();
    temp1 = MAKE_LRU_BUFFER();
    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;
    int sizeOfBuffer;
    int pos = 0;
    sizeOfBuffer = lengthofBuffer();
    if (start_lru == NULL) {
        temp->pos = 0;
        temp->age = 1;
        temp->dirty = false;
        temp->fixCount = 1;
        temp->data = page->data;
        temp->pageNum = page->pageNum;
        start_lru = temp;
        start_lru->next = NULL;
        pool_stat->pageNumber_stat[0] = temp->pageNum;

    } else {

        searchRes = searchForFrame_LRU(page->pageNum);
        if (searchRes != -1) {
            //element found. page pos returned
            temp = start_lru;
            while (temp != NULL) {
                if (temp->pos == searchRes) {
                    break;
                }
                temp = temp->next;
            }
            increment_age();
            temp->age = 1;
            temp->dirty = false;
            temp->fixCount = temp->fixCount + 1;
            temp->data = page->data;
            pool_stat->pageNumber_stat[searchRes] = temp->pageNum;
        } else {
            //element not found
            if (sizeOfBuffer == bm->numPages) {
                pos = LRU();
                temp = start_lru;
                while (temp != NULL) {
                    if (temp->pos == pos) {
                        break;
                    }
                    temp = temp->next;
                }
                if (temp->dirty == true) {
                    writeBlock(temp->pageNum, stat->fh, temp->data);
                    pool_stat->writeIO = pool_stat->writeIO + 1;
                }
                increment_age();
                temp->age = 1;
                temp->pos = pos;
                temp->dirty = false;
                temp->fixCount = 1;
                temp->data = page->data;
                temp->pageNum = page->pageNum;
                pool_stat->pageNumber_stat[pos] = temp->pageNum;
            } else {
                temp1 = start_lru;
                while (temp1 != NULL) {
                    pos = temp1->pos;
                    temp1 = temp1->next;
                }
                //temp = (struct buffer_LRU *) malloc(sizeof (struct buffer_LRU));
                pos = maxpos();
                temp->pos = pos + 1;
                temp->dirty = false;
                temp->fixCount = 1;
                increment_age();
                temp->age = 1;
                temp->data = page->data;
                temp->pageNum = page->pageNum;
                temp->next = start_lru;
                start_lru = temp;
                pool_stat->pageNumber_stat[temp->pos] = temp->pageNum;
            }
        }
    }
    return RC_OK;

}

//FIFO Page replacement methods

/*
 * AUTHOR: Arpita
 * METHOD: Length of Fifo Buffer
 * INPUT: NONE
 * OUTPUT: length of the buffer
 */
int len_fifo_buffer() {
    struct buffer_Queue *temp;
    int count = 0;
    temp = MAKE_QUEUE_BUFFER();
    temp = strt_fifo;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

/*
 * AUTHOR: Arpita
 * METHOD: Insert new frame in Fifo Buffer
 * INPUT: BM_BufferPool,BM_PageHandle
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC insert_frame_fifo_buffer(BM_BufferPool * const bm, BM_PageHandle * const page) {
    int cur_buffer_size;
    int cur_free_frame=0;
    buffer_stat *stat;
    char *pagedata;
    struct buffer_Queue *temp, *temp1;
    buffer_pool_stat *pool_stat;
    temp = MAKE_QUEUE_BUFFER();
    temp1=MAKE_QUEUE_BUFFER();
    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;
    if (strt_fifo == NULL && rear_fifo == NULL) {
        temp->pos = 0;
        temp->dirty = false;
        temp->fixCount = 1;
        //memcpy(temp->data, page->data, PAGE_SIZE);
        temp->data=page->data;
        temp->pageNum = page->pageNum;
        rear_fifo = temp;
        strt_fifo = temp;
        strt_fifo->next = rear_fifo;
        rear_fifo->next = NULL;
        pool_stat->pageNumber_stat[temp->pos] = temp->pageNum;
        pool_stat->fixCounts_stat[temp->pos] = temp->fixCount;
        pool_stat->dirtyPages_stat[temp->pos] = temp->dirty;
        handler = strt_fifo;
        pool_stat->readIO++;
    } else {
        //if the page frame is full
        cur_buffer_size = len_fifo_buffer();
        if (bm->numPages == cur_buffer_size) {
            //temp1 = handler;
            while (cur_free_frame != bm->numPages) {
                if (handler->fixCount == 0) {
                    if (handler->dirty==true) {
                      
                        writeBlock(handler->pageNum, stat->fh, handler->data);
                        pool_stat->writeIO++;
                    }
                    handler->dirty = false;
                    handler->fixCount = 1;
                    //memcpy(temp->data, page->data, PAGE_SIZE);
                    handler->data=page->data;
                    handler->pageNum = page->pageNum;
                    pool_stat->pageNumber_stat[handler->pos] = handler->pageNum;
                    pool_stat->fixCounts_stat[handler->pos] = handler->fixCount;
                    pool_stat->dirtyPages_stat[handler->pos] = handler->dirty;
                    pool_stat->readIO++;
                    break;
                } else {
                    if (handler->next == NULL) {
                        handler = strt_fifo;
                    } else {
                        handler = handler->next;
                    }
                    //temp1 = handler;
                    cur_free_frame++;
                }
            }
            if (cur_free_frame == bm->numPages) {
                return RC_BUFFER_EXCEEDED;
            } else {
                if (handler->next == NULL) {
                    handler = strt_fifo;
                } else {
                    handler = handler->next;
                }
            }
        }// else add the new page frame
        else {
            temp->next = NULL;
            temp->pos = rear_fifo->pos + 1;
            temp->dirty = false;
            temp->fixCount = 1;
            //memcpy(temp->data, page->data, PAGE_SIZE);
            temp->data=page->data;
            temp->pageNum = page->pageNum;
            rear_fifo->next = temp;
            rear_fifo = temp;
            pool_stat->pageNumber_stat[temp->pos] = temp->pageNum;
            pool_stat->fixCounts_stat[temp->pos] = temp->fixCount;
            pool_stat->dirtyPages_stat[temp->pos] = temp->dirty;
            pool_stat->readIO++;
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Arpita
 * METHOD: Search frame in Fifo Buffer
 * INPUT: PageNUmber
 * OUTPUT: Position of the Page in Frame
 */
int search_frame_fifo_buffer(PageNumber pNum) {
    struct buffer_Queue *temp;
    int pos = -1;
    temp = MAKE_QUEUE_BUFFER();
    temp = strt_fifo;
    while (temp != NULL) {
        if (temp->pageNum == pNum) {
            pos = temp->pos;
            break;
        }
        temp = temp->next;
    }
    return pos;
}

/*

 * AUTHOR: Arpita
 * METHOD: Update frame status in Fifo Buffer
 * INPUT: buffer_Queue,buffer_stat,UpdateFrameData,Position
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC update_frame_stat_fifo_buffer(struct buffer_Queue *buf, buffer_stat *stat, UpdateFrameData update_value, int pos) {
    buffer_pool_stat *pool_stat;
    pool_stat = stat->pool_Stat;
    while (buf != NULL) {
        if (buf->pos == pos) {
            switch (update_value) {
                case(UP_Dirty):
                    buf->dirty = true;
                    pool_stat->dirtyPages_stat[pos] = buf->dirty;
                    break;
                case(UP_Pin):
                    buf->fixCount = buf->fixCount + 1;
                    pool_stat->fixCounts_stat[pos] = buf->fixCount;
                    break;
                case(UP_Unpin):
                    buf->fixCount = buf->fixCount - 1;
                    pool_stat->fixCounts_stat[pos] = buf->fixCount;
                    break;
                case(UP_NOT_Dirty):
                    buf->dirty = false;
                    pool_stat->dirtyPages_stat[pos] = buf->dirty;
                    break;
            }
        }
        buf = buf->next;
    }
    return RC_OK;
}


//Clock replacement method;

/*
 * AUTHOR: Sindhu
 * METHOD: length of the buffer
 * INPUT: NONE
 * OUTPUT: length of the buffer
 */
int length_Clock_buffer() {
    struct buffer_Clock * tmpClock;
    int count = 0;
    tmpClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));
    tmpClock = strt_clock;
    if (strt_clock != NULL) {
        if (tmpClock->next == strt_clock) {
            return 1;
        } else {
            while (tmpClock-> next != strt_clock) {
                count++;
                tmpClock = tmpClock->next;
            }
        }
    }
    return count;
}

/*
 * AUTHOR: Sindhu
 * METHOD: Search for a frame.
 * INPUT: Page NUmber
 * OUTPUT: position of the Page
 */
int search_frame_clock_buffer(PageNumber pNum) {
    struct buffer_Clock *temp;
    int pos = -1;
    temp = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));

    temp = strt_clock;
    if (strt_clock != NULL) {
        if (temp->next == strt_clock && strt_clock->pageNum==pNum) {
            pos = strt_clock->pos;
        } else {
            while (temp->next != strt_clock) {
                if (temp->pageNum == pNum) {
                    pos = temp->pos;
                    break;
                }
                temp = temp->next;
            }
        }
    }
    return pos;
}

/*
 * AUTHOR: Sindhu
 * METHOD:insert frame clock buffer
 * INPUT: BM_BufferPool,BM_PageHandle
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC insert_frame_clock_buffer(BM_BufferPool * const bm, BM_PageHandle * const page) {
    buffer_stat *stat;
    struct buffer_Clock *temp, *temp1;
    buffer_pool_stat *pool_stat;

    temp = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));
    temp1 = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    int pageFound;
    int cur_buffer_size;
    int cur_free_frame = 0;
    temp->data = (char *) malloc(PAGE_SIZE);
    if (strt_clock == NULL) {
        temp->pos = 0;
        temp->dirty = false;
        //memcpy(temp->data, page->data, PAGE_SIZE);
        temp->data = page->data;
        temp->pageNum = page->pageNum;
        temp->ref = 1;
        temp->fixCount = 1;
        strt_clock = temp;
        strt_clock->next = strt_clock;
        pool_stat->pageNumber_stat[temp->pos] = temp->pageNum;
        clock_hand = strt_clock;
    } else {//search for the requested page
        cur_buffer_size = length_Clock_buffer();
        if (bm->numPages == cur_buffer_size) {
            temp = clock_hand;
            while (cur_free_frame != bm->numPages) {
                if (temp->ref == 0 && temp->fixCount == 0) {
                    if (temp->dirty) {
                        writeBlock(temp->pageNum, stat->fh, temp->data);
                        pool_stat->writeIO++;
                    }

                    temp->dirty = false;
                    temp->fixCount = 1;
                    //memcpy(temp->data, page->data, PAGE_SIZE);
                    temp->data=page->data;
                    temp->pageNum = page->pageNum;
                    pool_stat->pageNumber_stat[temp->pos] = temp->pageNum;
                    pool_stat->fixCounts_stat[temp->pos] = temp->fixCount;
                    pool_stat->dirtyPages_stat[temp->pos] = temp->dirty;
                    break;
                } else if (temp->ref == 0 && temp->fixCount == 1) {
                    temp = temp->next;
                } else if (temp->ref == 1) {
                    temp->ref = 0;
                    temp = temp->next;
                }
                cur_free_frame++;
            }
            if (cur_free_frame == bm->numPages) {
                if (temp == strt_clock) {
                    if (temp->ref == 0 && temp->fixCount == 0) {
                        if (temp->dirty) {
                            writeBlock(temp->pageNum, stat->fh, temp->data);
                            pool_stat->writeIO++;
                        }

                        temp->dirty = false;
                        temp->fixCount = 1;
                        //memcpy(temp->data, page->data, PAGE_SIZE);
                        temp->data=page->data;
                        temp->pageNum = page->pageNum;
                        pool_stat->pageNumber_stat[temp->pos] = temp->pageNum;
                        pool_stat->fixCounts_stat[temp->pos] = temp->fixCount;
                        pool_stat->dirtyPages_stat[temp->pos] = temp->dirty;
                        clock_hand = temp->next;
                    } else {
                        return RC_BUFFER_EXCEEDED;
                    }
                } else {
                    return RC_BUFFER_EXCEEDED;
                }
            } else {
                clock_hand = temp->next;
            }
        } else {
            temp1 = strt_clock;
            while (temp1->next != strt_clock) {
                temp1 = temp1->next;
            }
            temp->pos = temp1->pos + 1;
            temp->dirty = false;
            //memcpy(temp->data, page->data, PAGE_SIZE);
            temp->data=page->data;
            temp->pageNum = page->pageNum;
            temp->ref = 1;
            temp->fixCount = 1;
            temp->next = strt_clock;
            temp1->next = temp;
            pool_stat->pageNumber_stat[temp->pos] = temp->pageNum;
            pool_stat->fixCounts_stat[temp->pos] = temp->fixCount;
            pool_stat->dirtyPages_stat[temp->pos] = temp->dirty;
        }
    }
    return RC_OK;
}

// Buffer Manager Interface Pool Handling

/*
 * AUTHOR: Nikhil & Arpita & Sindhu
 * METHOD: Initialise Buffer Pool
 * INPUT: Buffer POOL;PAGE FRAME;BUFFER SIZE;REPLACEMENT STRATEGY;STRAT DATA
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC initBufferPool(BM_BufferPool * const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void* stratData) {

    SM_FileHandle *fh = (SM_FileHandle *) malloc(PAGE_SIZE);
    memset(fh,0,PAGE_SIZE);
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;
    struct buffer_Queue *temp;
    struct buffer_Clock *tmpClock;
    PageNumber* pageNumArray;
    int* fixCountsArray;
    bool* dirtyMarked;
    int i = 0;
    stat = MAKE_BUFFER_STAT();
    temp = MAKE_QUEUE_BUFFER();
    tmpClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));
    pool_stat = MAKE_BUFFER_POOL_STAT();

    bm->pageFile = (char *) pageFileName;
    bm->numPages = numPages;
    bm->strategy = strategy;

    //initialize the pool_stat members
    pageNumArray = (PageNumber *) malloc(sizeof (PageNumber) * bm->numPages);
    fixCountsArray = (int *) malloc(sizeof (int)*numPages);
    dirtyMarked = (bool *) malloc(sizeof (bool) * numPages);
    for (i = 0; i < bm->numPages; i++) {
        pageNumArray[i] = NO_PAGE;
        fixCountsArray[i] = 0;
        dirtyMarked[i] = false;
    }
    pool_stat->readIO = 0;
    pool_stat->writeIO = 0;
    pool_stat->pageNumber_stat = pageNumArray;
    pool_stat->dirtyPages_stat = dirtyMarked;
    pool_stat->fixCounts_stat = fixCountsArray;

    //Open the Page File 
    openPageFile(bm->pageFile, fh);
    switch (strategy) {
        case RS_FIFO:
            stat->fh = fh;
            stat->buf = strt_fifo;
            stat->pool_Stat = pool_stat;
            break;
        case RS_LRU:
            stat->fh = fh;
            stat->buf = start_lru;
            stat->pool_Stat = pool_stat;
            break;
        case RS_CLOCK:
            stat->fh = fh;
            stat->buf = strt_clock;
            stat->pool_Stat = pool_stat;
            break;
        case RS_LFU:
            //printf("LFU");
            break;
        case RS_LRU_K:
            //printf("LRU-K");
            break;
        default:
            return RC_BUFFER_UNDEFINED_STRATEGY;
    }
    bm->mgmtData = stat;
    return RC_OK;
}

/*
 * AUTHOR: Nikhil & Arpita & Sindhu
 * METHOD: shutdownBufferPool
 * INPUT: BM_BufferPool
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC shutdownBufferPool(BM_BufferPool * const bm) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;
    struct buffer_Queue *temp;
    struct buffer_LRU *temp1;
    struct buffer_Clock *tmpClock;
    temp = MAKE_QUEUE_BUFFER();
    temp1 = MAKE_LRU_BUFFER();
    tmpClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    int pos = 0;
    int numofPages = bm->numPages;

    switch (bm->strategy) {
        case RS_FIFO:
            temp = strt_fifo;
            while (temp != NULL) {
                if (temp->dirty) {
                    writeBlock(temp->pageNum, stat->fh, temp->data);
                    pool_stat->writeIO = pool_stat->writeIO + 1;
                }
                temp = temp->next;
            }
            break;

        case RS_LRU:
            temp1 = start_lru;
            while (temp1 != NULL) {
                if (temp1->dirty) {
                    writeBlock(temp1->pageNum, stat->fh, temp1->data);
                    pool_stat->writeIO = pool_stat->writeIO + 1;
                }
                temp1 = temp1->next;
            }
            break;
        case RS_CLOCK:
            tmpClock = strt_clock;
            if (tmpClock->next == strt_clock) {
                writeBlock(strt_clock->pageNum, stat->fh, strt_clock->data);
                pool_stat->writeIO = pool_stat->writeIO + 1;
            } else {
                while (tmpClock->next != strt_clock) {  
                    if (tmpClock->dirty) {
                        writeBlock(tmpClock->pageNum, stat->fh, tmpClock->data);
                        pool_stat->writeIO = pool_stat->writeIO + 1;
                    }
                    tmpClock = tmpClock->next;
                }
            }
            break;
        case RS_LFU:
            //printf("LFU");
            break;
        case RS_LRU_K:
            //printf("LRU-K");
            break;
        default:
            return RC_BUFFER_UNDEFINED_STRATEGY;
    }

    strt_fifo = NULL;
    rear_fifo = NULL;
    handler = NULL;
    start_lru = NULL;
    //strt_clock = NULL;
    closePageFile(stat->fh);
    free(temp);
    free(temp1);
    free(pool_stat->dirtyPages_stat);
    free(pool_stat->fixCounts_stat);
    free(pool_stat->pageNumber_stat);
    free(stat->fh);
    free(stat->buf);
    free(stat->pool_Stat);
    free(bm->mgmtData);
  
    //free(stat);
    
    return RC_OK;
}

/*
 * AUTHOR: Sindhu
 * METHOD: forceFlushPool
 * INPUT: BM_BufferPool
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */

RC forceFlushPool(BM_BufferPool * const bm) {

    buffer_stat *stat;
    buffer_pool_stat *pool_stat;
    struct buffer_LRU *temp1;
    struct buffer_Clock *tmpClock;
    struct buffer_Queue *temp;
    int numberofPages = bm->numPages;
    int pos;
    temp = MAKE_QUEUE_BUFFER();
    temp1 = MAKE_LRU_BUFFER();
    tmpClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    switch (bm->strategy) {
        case RS_FIFO:
            temp = strt_fifo;
            while (temp != NULL) {
                if (temp->dirty) {
                    writeBlock(temp->pageNum, stat->fh, temp->data);
                    pool_stat->writeIO++;
                    temp->dirty = false;
                    pool_stat->dirtyPages_stat[temp->pos] = temp->dirty;
                }
                temp = temp->next;
            }
            break;
        case RS_LRU:
            temp1 = start_lru;
            while (temp1 != NULL) {
                if (temp1->dirty) {
                    writeBlock(temp1->pageNum, stat->fh, temp1->data);
                    pool_stat->writeIO = pool_stat->writeIO + 1;
                }
                temp1 = temp1->next;
            }
            break;
        case RS_CLOCK:
            tmpClock = strt_clock;
            if (tmpClock->next == strt_clock) {
                writeBlock(strt_clock->pageNum, stat->fh, strt_clock->data);
                pool_stat->writeIO = pool_stat->writeIO + 1;
            } else {
                while (tmpClock->next != strt_clock) {
                    if (tmpClock->dirty) {
                        writeBlock(tmpClock->pageNum, stat->fh, tmpClock->data);
                        pool_stat->writeIO = pool_stat->writeIO + 1;
                    }
                    tmpClock = tmpClock->next;
                }
            }
            break;
        case RS_LFU:
            //printf("LFU");
            break;
        case RS_LRU_K:
            //printf("LRU-K");
            break;
        default:
            return RC_BUFFER_UNDEFINED_STRATEGY;
    }
    free(temp);
    free(temp1);
    return RC_OK;
}

// Buffer Manager Interface Access Pages

/*
 * AUTHOR: Nikhil & Arpita & Sindhu
 * METHOD: Pin Pages
 * INPUT: BM_BufferPool,BM_PageHandle,PageNumber
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum) {
    SM_PageHandle ph;

    buffer_stat *stat;
    struct buffer_LRU *temp;
    struct buffer_Clock *bufClock;
    struct buffer_LRU *node;
    buffer_pool_stat *pool_stat;
    int pos = -1;

    ph = (SM_PageHandle) malloc(PAGE_SIZE);
    memset(ph,0,PAGE_SIZE);
    temp = MAKE_LRU_BUFFER();
    bufClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));
    node = MAKE_LRU_BUFFER();


    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;
    node = stat->buf;

    page->pageNum = pageNum;
    switch (bm->strategy) {
        case RS_FIFO:
            pos = search_frame_fifo_buffer(page->pageNum);
             //if(searchForFrame_LRU(page->pageNum)==-1){
            if(searchForFrame_LRU(page->pageNum)==-1){
             readBlock(pageNum, stat->fh, ph);
             page->data=ph;
           
            }
            if (pos != -1) {
                update_frame_stat_fifo_buffer(strt_fifo, stat, UP_Pin, pos);
            } else {
                insert_frame_fifo_buffer(bm, page);
            }
            break;
        case RS_LRU:
             if(searchForFrame_LRU(page->pageNum)==-1){
             readBlock(pageNum, stat->fh, ph);
             pool_stat->readIO = pool_stat->readIO+1;
             page->data=ph;
            }
            insert_into_buffer_LRU(bm,page);
            break;
        case RS_CLOCK:
            pos = search_frame_clock_buffer(page->pageNum);
            bufClock = strt_clock;
            if (pos != -1) {
                if (pos == strt_clock->pos) {
                    strt_clock->fixCount++;
                    pool_stat->dirtyPages_stat[strt_clock->pos] = strt_clock->fixCount;
                } else {
                    while (bufClock->next != strt_clock) {
                        if (bufClock->pos == pos) {
                            bufClock->fixCount++;
                            pool_stat->fixCounts_stat[bufClock->pos] = bufClock->fixCount;
                            pool_stat->readIO++;
                        }
                        bufClock = bufClock->next;
                    }
                }
            } else {
                insert_frame_clock_buffer(bm, page);
                pool_stat->readIO++;
            }
            break;
        case RS_LFU:
            //printf("LFU");
            break;
        case RS_LRU_K:
            //printf("LRU-K");
            break;
        default:
            return RC_BUFFER_UNDEFINED_STRATEGY;
    }
    free(temp);
    free(node);
    return RC_OK;
}

/*
 * AUTHOR: Nikhil & Arpita & Sindhu
 * METHOD: markDirty
 * INPUT: BM_BufferPool,BM_PageHandle
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;
    struct buffer_LRU *node;
    struct buffer_Queue *buf;
    struct buffer_Clock *bufClock;
    int pos = -1;
    buf = MAKE_QUEUE_BUFFER();
    bufClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));
    node = MAKE_LRU_BUFFER();

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    switch (bm->strategy) {
        case RS_FIFO:
            pos = search_frame_fifo_buffer(page->pageNum);
            buf = strt_fifo;
            if (pos != -1) {
                while (buf != NULL) {
                    if (buf->pos == pos) {
                        //memcpy(buf->data, page->data, PAGE_SIZE);
                        buf->data=page->data;
                        update_frame_stat_fifo_buffer(strt_fifo, stat, UP_Dirty, pos);
                    }
                    buf = buf->next;
                }
            }
            break;
        case RS_LRU:
            pos = LRU(bm, page->pageNum);
            node = start_lru;
            while (node != NULL) {
                if (node->pos == pos) {
                    node->dirty = true;
                    pool_stat->dirtyPages_stat[pos] = node->dirty;
                }
                node = node->next;
            }
            break;
        case RS_CLOCK:
            pos = search_frame_clock_buffer(page->pageNum);
            bufClock = strt_clock;
            if (pos != -1) {
                if (pos == strt_clock->pos) {
                    //memcpy(strt_clock->data, page->data, PAGE_SIZE);
                    strt_clock->data=page->data;
                    strt_clock->dirty = true;
                    pool_stat->dirtyPages_stat[pos] = bufClock->dirty;
                } else {
                    while (bufClock->next != strt_clock) {
                        if (bufClock->pos == pos) {
                            //memcpy(buf->data, page->data, PAGE_SIZE);
                            buf->data=page->data;
                            bufClock->dirty = true;
                            pool_stat->dirtyPages_stat[pos] = bufClock->dirty;
                        }
                        bufClock = bufClock->next;
                    }
                }
            }
            break;
        case RS_LFU:
            //printf("LFU");
            break;
        case RS_LRU_K:
            //printf("LRU-K");
            break;
        default:
            return RC_BUFFER_UNDEFINED_STRATEGY;
    }

    return RC_OK;
}

/*
 * AUTHOR: Nikhil & Arpita & Sindhu
 * METHOD: Unpin page 
 * INPUT: BM_BufferPool,BM_PageHandle
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) {
    buffer_stat *stat;
    struct buffer_Queue *buf;
    struct buffer_LRU *bufLRU;
    struct buffer_Clock *bufClock;
    buffer_pool_stat *pool_stat;
    int pos = -1;
    int pinCount = 0;
    buf = MAKE_QUEUE_BUFFER();
    bufLRU = MAKE_LRU_BUFFER();
    bufClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;
    switch (bm->strategy) {
        case RS_FIFO:
            pos = search_frame_fifo_buffer(page->pageNum);
            buf = strt_fifo;
            while (buf != NULL) {
                if (buf->pos == pos) {
                    update_frame_stat_fifo_buffer(strt_fifo, stat, UP_Unpin, pos);
                }
                buf = buf->next;
            }
            break;
        case RS_LRU:
            pos = searchForFrame_LRU(page->pageNum);
            bufLRU = start_lru;
            while (bufLRU != NULL) {
                if (bufLRU->pos == pos) {
                    bufLRU->fixCount = bufLRU->fixCount - 1;
                    pool_stat->fixCounts_stat[bufLRU->pos] = bufLRU->fixCount;
                }
                bufLRU = bufLRU->next;
            }
            break;
        case RS_CLOCK:
            pos = search_frame_clock_buffer(page->pageNum);
            bufClock = strt_clock;
            if (pos != -1) {
                if (pos == strt_clock->pos) {
                    strt_clock->fixCount = strt_clock->fixCount - 1;
                    pool_stat->fixCounts_stat[strt_clock->pos] = strt_clock->fixCount;
                } else {
                    while (bufClock->next != strt_clock) {
                        if (bufClock->pos == pos) {
                            bufClock->fixCount = bufClock->fixCount - 1;
                            pool_stat->fixCounts_stat[bufClock->pos] = bufClock->fixCount;
                        }
                        bufClock = bufClock->next;
                    }
                }
            }
            break;
        case RS_LFU:
            //printf("LFU");
            break;
        case RS_LRU_K:
            //printf("LRU-K");
            break;
        default:
            return RC_BUFFER_UNDEFINED_STRATEGY;
    }
    free(bufLRU);
    free(buf);
    free(bufClock);
    return RC_OK;
}

/*
 * AUTHOR: Arpita and Sindhu
 * METHOD: write date to disk
 * INPUT: BM_BufferPool,BM_PageHandle
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;
    struct buffer_Queue *temp;
    struct buffer_LRU *bufLru;
    struct buffer_Clock *bufClock;

    bufClock = (struct buffer_Clock *) malloc(sizeof (struct buffer_Clock));
    bufLru = MAKE_LRU_BUFFER();
    temp = MAKE_QUEUE_BUFFER();

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    int pos = -1;
    writeBlock(page->pageNum, stat->fh, page->data);

    switch (bm->strategy) {
        case RS_FIFO:
            pos = search_frame_fifo_buffer(page->pageNum);
            temp = strt_fifo;
            if (pos != -1) {
                while (temp != NULL) {
                    if (temp->pos == pos) {
                        //memcpy(temp->data, page->data, PAGE_SIZE);
                        temp->data=page->data;
                        writeBlock(temp->pageNum, stat->fh, temp->data);
                        pool_stat->writeIO++;
                        update_frame_stat_fifo_buffer(stat->buf, stat, UP_NOT_Dirty, pos);
                    }
                    temp = temp->next;
                }
            }
            break;
        case RS_LRU:
            pos = searchForFrame_LRU(page->pageNum);
            bufLru = start_lru;
            if (pos != -1) {
                while (bufLru != NULL) {
                    if (bufLru->pos == pos) {
                        //memcpy(bufLru->data, page->data, PAGE_SIZE);
                        bufLru->data=page->data;
                        writeBlock(bufLru->pageNum, stat->fh, bufLru->data);
                        bufLru->dirty = false;
                        pool_stat->dirtyPages_stat[pos] = bufLru->dirty;
                        pool_stat->writeIO++;
                    }
                    bufLru = bufLru->next;
                }
            }
            break;
        case RS_CLOCK:

            pos = search_frame_clock_buffer(page->pageNum);
            bufClock = strt_clock;
            if (pos != -1) {
                if (pos == strt_clock->pos) {
                    strt_clock->fixCount = strt_clock->fixCount - 1;
                    pool_stat->fixCounts_stat[strt_clock->pos] = strt_clock->fixCount;
                } else {

                    while (bufClock->next != strt_clock) {
                        if (bufClock->pos == pos) {
                            //memcpy(bufClock->data, page->data, PAGE_SIZE);
                            bufClock->data=page->data;
                            writeBlock(bufClock->pageNum, stat->fh, bufClock->data);
                            bufClock->dirty = false;
                            pool_stat->dirtyPages_stat[pos] = bufClock->dirty;
                            pool_stat->writeIO++;
                        }
                        bufClock = bufClock->next;
                    }
                }
            }

            break;
        case RS_LFU:
            //printf("LFU");
            break;
        case RS_LRU_K:
            //printf("LRU-K");
            break;
        default:
            return RC_BUFFER_UNDEFINED_STRATEGY;
    }
    free(bufLru);
    return RC_OK;
}

// Statistics Interface

/*
 * AUTHOR: Nikhil
 * METHOD: get Frame Counts
 * INPUT: BM_BufferPool 
 * OUTPUT: PageNumber *
 */
PageNumber *getFrameContents(BM_BufferPool * const bm) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;
    return pool_stat->pageNumber_stat;
}

/*
 * AUTHOR: Nikhil
 * METHOD: get Fix Counts
 * INPUT: BM_BufferPool
 * OUTPUT: int *
 */
int *getFixCounts(BM_BufferPool * const bm) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    return pool_stat->fixCounts_stat;

}

/*
 * AUTHOR: Arpita
 * METHOD: dirty marked statistics
 * INPUT: BM_BufferPool
 * OUTPUT: bool *
 */
bool *getDirtyFlags(BM_BufferPool * const bm) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    return pool_stat->dirtyPages_stat;
}

/*
 * AUTHOR: Arpita
 * METHOD: get Number of read from Disk
 * INPUT: BM_BufferPool
 * OUTPUT: int
 */
int getNumReadIO(BM_BufferPool * const bm) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;
    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    return pool_stat->readIO;
}

/*
 * AUTHOR: Arpita
 * METHOD: get Number of writes to disk
 * INPUT: BM_BufferPool
 * OUTPUT: int
 */
int getNumWriteIO(BM_BufferPool * const bm) {
    buffer_stat *stat;
    buffer_pool_stat *pool_stat;

    stat = bm->mgmtData;
    pool_stat = stat->pool_Stat;

    return pool_stat->writeIO;
}

