
#include "storage_mgr.h"
#include "dberror.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include<sys/stat.h>

/*
 * AUTHOR: Sindhu 
 * METHOD: Create the file
 * INPUT: File Name
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC createPageFile(char *fileName) {
    FILE *fptr = fopen(fileName, "wb");
    if (fptr == NULL) {
        return RC_FILE_NOT_FOUND;
    } else {
        return RC_OK;
    }
}

void initStorageManager() {
}

/*
 * AUTHOR: Sindhu 
 * METHOD: Open the file
 * INPUT: File Name,File Structure
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    FILE *fptr = NULL;
    fHandle->mgmtInfo = (void *) malloc(PAGE_SIZE);
    if (fHandle->mgmtInfo == NULL) {
        return RC_NOMEM;
    }
    struct stat buffer;
    if (stat(fileName, &buffer) != 0) {
        if (errno == ENOENT) {
            return RC_FILE_NOT_FOUND;
        }
    }
    fptr = fopen(fileName, "wb");
    if (fptr == NULL) {
        return RC_FILE_NOT_OPENED;
    } else {
        printf("File $%s  exist\n", fileName);
        fHandle->fileName = fileName;
        fHandle->mgmtInfo = fptr;
        fHandle->totalNumPages = 1;
        fHandle->curPagePos = 0;
        fseek(fptr, 0L, SEEK_SET);
        fwrite(fHandle, PAGE_SIZE, 1, fptr);
        return RC_OK;
    }
}

/*
 * AUTHOR: Sindhu 
 * METHOD: Close the file
 * INPUT: File Structure
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC closePageFile(SM_FileHandle *fHandle) {

    FILE *fptr = fHandle->mgmtInfo;
    if (fptr == NULL) {
        return RC_FILE_NOT_OPENED;
    } else {
        if (fclose(fptr) == 0) {
            fptr = 0;
            fHandle->mgmtInfo = NULL;
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Sindhu 
 * METHOD: Destroy the file
 * INPUT: File Name
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC destroyPageFile(char *fileName) {
    int deleteStatus;
    deleteStatus = remove(fileName);
    if (deleteStatus == 0) {
        return RC_OK;
    } else {
        return RC_FILE_NOT_DELETED;
    }
}

/*
 * AUTHOR: Arpita 
 * METHOD: Get the current block position
 * INPUT: File Structure
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
int getBlockPos(SM_FileHandle *fHandle) {
    int pos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            pos = ftell(fptr);
            if (pos == -1L) {
                return RC_FILE_CUR_POS_ERROR;
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Arpita 
 * METHOD: Read the current Block from the File
 * INPUT: File Structure, Content of the data to be read
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC readCurrentBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            curPos = fHandle->curPagePos;
            if (curPos == -1L) {
                return RC_FILE_CUR_POS_ERROR;
            } else {
                //Seeking to the current block of the file
                if ((fseek(fptr, (curPos + 1) * PAGE_SIZE, SEEK_SET)) < 0) {
                    return RC_FILE_SEEK_ERROR;
                } else {
                    if ((fread(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                        return RC_READ_FAILED;
                    } else {
                        curPos = (ftell(fptr) / PAGE_SIZE) - 1;
                        fHandle->curPagePos = curPos;
                    }
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Arpita 
 * METHOD: Read the previous Block from the current block in the File
 * INPUT: File Structure, Content of the data to be read
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC readPreviousBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            curPos = fHandle->curPagePos;
            if (curPos == -1L) {
                return RC_FILE_CUR_POS_ERROR;
            } else {
                //Seeking to the previous block in the file
                if ((fseek(fptr, curPos * PAGE_SIZE, SEEK_SET)) < 0) {
                    return RC_FILE_SEEK_ERROR;
                } else {
                    if ((fread(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                        return RC_READ_FAILED;
                    } else {
                        curPos = (ftell(fptr) / PAGE_SIZE) - 1;
                        fHandle->curPagePos = curPos;
                    }
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Arpita 
 * METHOD: Read the next Block from the current block in the File
 * INPUT: File Structure, Content of the data to be read
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC readNextBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            curPos = fHandle->curPagePos;
            if (curPos == -1L) {
                return RC_FILE_CUR_POS_ERROR;
            } else {
                //seeking to the next block in the file
                if ((fseek(fptr, (curPos + 2) * PAGE_SIZE, SEEK_SET)) < 0) {
                    return RC_FILE_SEEK_ERROR;
                } else {
                    if ((fread(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                        return RC_READ_FAILED;
                    } else {
                        curPos = (ftell(fptr) / PAGE_SIZE) - 1;
                        fHandle->curPagePos = curPos;
                    }
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Arpita 
 * METHOD: Read Last Block from File
 * INPUT: Page Number,File Structure, Content of the data to be read
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC readLastBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {

            //Seeking to the last block of the file
            if ((fseek(fptr, 0l, SEEK_END)) < 0) {
                return RC_FILE_SEEK_ERROR;
            } else {
                if ((fread(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                    return RC_READ_FAILED;
                } else {
                    curPos = (ftell(fptr) / PAGE_SIZE);
                    fHandle->curPagePos = curPos - 1;
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Nikhil 
 * METHOD:Read First Block from File
 * INPUT: File Structure, Content of the data to be read
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC readFirstBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            //Seeking to the first block of the file
            if ((fseek(fptr, PAGE_SIZE, SEEK_SET)) < 0) {
                return RC_FILE_SEEK_ERROR;
            } else {
                if ((fread(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                    return RC_READ_FAILED;
                } else {
                    curPos = (ftell(fptr) / PAGE_SIZE) - 1;
                    fHandle->curPagePos = curPos;
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Arpita 
 * METHOD:Read Block from File
 * INPUT: Page Number,File Structure, Content of the data to be read
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC readBlock(int pageNum, SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            if ((fseek(fptr, (pageNum + 1) * PAGE_SIZE, SEEK_SET)) < 0) {
                return RC_FILE_SEEK_ERROR;
            } else {
                if ((fread(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                    return RC_READ_FAILED;
                } else {
                    curPos = (ftell(fptr) / PAGE_SIZE) - 1;
                    fHandle->curPagePos = curPos;
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Nikhil 
 * METHOD: To write the given data to the given page block in the file
 * INPUT: Page Number,File Structure, Content of the data to be written
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC writeBlock(int pageNum, SM_FileHandle* fHandle, SM_PageHandle memPage) {
    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            //Seeking to the given block 
            if ((fseek(fptr, (pageNum + 1) * PAGE_SIZE, SEEK_SET)) < 0) {
                return RC_FILE_SEEK_ERROR;
            } else {
                if ((fwrite(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                    return RC_WRITE_FAILED;
                } else {
                    curPos = (ftell(fptr) / PAGE_SIZE) - 1;
                    fHandle->curPagePos = curPos;
                    fHandle->totalNumPages = fHandle->totalNumPages + 1;
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Nikhil 
 * METHOD: To write the given data to the current page block in the file
 * INPUT: File Structure, Content of the data to be written
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC writeCurrentBlock(SM_FileHandle* fHandle, SM_PageHandle memPage) {

    int curPos = -1;
    FILE *fptr = NULL;
    if (fHandle->fileName == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    } else {
        fptr = fHandle->mgmtInfo;
        if (fptr == NULL) {
            return RC_FILE_NOT_OPENED;
        } else {
            curPos = fHandle->curPagePos;
            if (curPos == -1) {
                return RC_FILE_CUR_POS_ERROR;
            } else {
                //Seeking to the current block in the file
                if ((fseek(fptr, (curPos + 1) * PAGE_SIZE, SEEK_SET)) < 0) {
                    return RC_FILE_SEEK_ERROR;
                } else {
                    if ((fwrite(memPage, PAGE_SIZE, 1, fptr)) == -1) {
                        return RC_WRITE_FAILED;
                    } else {
                        curPos = (ftell(fptr) / PAGE_SIZE) - 1;
                        fHandle->curPagePos = curPos;
                        fHandle->totalNumPages = fHandle->totalNumPages + 1;
                    }
                }
            }
        }
    }
    return RC_OK;
}

/*
 * AUTHOR: Sindhu
 * METHOD: Ensure the capacity.
 * INPUT: File Structure,Number of pages
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    char buf[PAGE_SIZE];
    FILE *fptr = NULL;
    int i;
    fptr = fHandle->mgmtInfo;
    if (fptr == NULL) {
        return RC_FILE_NOT_OPENED;
    } else {
        int totalNumberPages = fHandle->totalNumPages;
        if (totalNumberPages < numberOfPages) {
            fseek(fptr, 0L, SEEK_END);
            for (i = 0; i < (numberOfPages - totalNumberPages); i++) {
                fwrite(buf, PAGE_SIZE, 1, fptr);
            }
            fHandle->totalNumPages = numberOfPages;
        }
    }
    return RC_OK;
}

/*
 * @author : sindhu
 * METHOD: To add an empty block to the file.
 * INPUT: File Structure
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC appendEmptyBlock(SM_FileHandle *fHandle) {
    char buf[PAGE_SIZE];
    FILE *fptr = NULL;
    fptr = fHandle->mgmtInfo;
    if (fptr == NULL) {
        return RC_FILE_NOT_OPENED;
    } else {
        int totalNumberPages = fHandle->totalNumPages;
        if (totalNumberPages == 0) {
            return RC_NO_PAGES_ERROR;
        } else {
            if ((fseek(fptr, 0L, SEEK_END)) < 0) {
                return RC_FILE_SEEK_ERROR;
            } else {
                if ((fwrite(buf, PAGE_SIZE, 1, fptr)) == -1) {
                    return RC_WRITE_FAILED;
                } else {
                    int curPos = (ftell(fptr) / PAGE_SIZE);
                    fHandle->curPagePos = curPos;
                    fHandle->totalNumPages = fHandle->totalNumPages + 1;
                }
            }
        }
    }
    return RC_OK;
}
