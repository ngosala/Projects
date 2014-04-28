
#include "dberror.h"
#include "record_mgr.h"


#define DBName "database"

typedef struct BlockDirectory {
    int blocknum;
    int freespace;
    int numberofrecords;
    
} BlockDirectory;

typedef struct BlockHeader {
    int blocknum;
    char *schema;
    int curentoffset;
    
} BlockHeader;

typedef struct ScanData {
    Expr *cond;
    int currRecord;
    int numRecords;
    int no_blocks;
    int currentblock;
    int parsedRecords;
} ScanData;

typedef struct DB_Data
{
  char *name;
  void *mgmtData;
  struct BlockDirectory *BlockDirectory;
  struct BlockHeader *BlockHeader;
  
} DB_Data;

#define MAKE_DBData()					\
  ((DB_Data *) malloc (sizeof(DB_Data)))

#define MAKE_BlockDirectory()					\
  ((BlockDirectory *) malloc (sizeof(BlockDirectory)))

#define MAKE_BlockHeader()					\
  ((BlockHeader *) malloc (sizeof(BlockHeader)))

extern RC createDB();
extern RC insertTable();
extern RC deleteTable();
extern RC updateTable();
extern RC deleteDB();
