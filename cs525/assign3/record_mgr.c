#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <time.h>

#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "dt.h"
#include "db.h"
#include "tables.h"
#include "record_mgr.h"

/*
 * AUTHOR: Arpita 
 * METHOD: Initialzing the record manager
 * INPUT: mgmtData
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC initRecordManager(void* mgmtData) {
    return RC_OK;
}

/*
 * AUTHOR: Arpita 
 * METHOD: Shut down the record manager
 * INPUT: mgmtData
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC shutdownRecordManager() {
    return RC_OK;
}

/*
 * AUTHOR: Nikhil & Arpita 
 * METHOD: Create the table 
 * INPUT: Table Name, Schema
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC createTable(char* name, Schema* schema) {
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    //create the page file
    createPageFile(name);
    initBufferPool(bm, name, 3, RS_FIFO, NULL);

    //Insert the table mgmt info into TABLE INFO BLOCK
    pinPage(bm, bh, TABLE_INFO_BLOCK);
    update_tbl_mgmt_info(schema, bh->data, 0, TBL_NEW);
    markDirty(bm, bh);
    unpinPage(bm, bh);


    //Initialize the data block with the block Id 
    pinPage(bm, bh, DATA_INFO_BLOCK);
    update_data_block_header(bh->data, BLK_NEW, 0, 0, 0);
    markDirty(bm, bh);
    unpinPage(bm, bh);

    //Write the content to the file
    forceFlushPool(bm);

    //Shut down the buffer pool
    shutdownBufferPool(bm);
    free(bm);
    return RC_OK;
}

/*
 * AUTHOR: Nikhil  
 * METHOD: Open the table 
 * INPUT: Table Struct, Table Name
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC openTable(RM_TableData* rel, char* name) {

    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    int i = 0;
    rel->name = (char *) malloc(sizeof (name));
    rel->mgmtData = malloc(sizeof (bm));
    initBufferPool(bm, name, 3, RS_FIFO, NULL);

    pinPage(bm, bh, TABLE_INFO_BLOCK);
    //read schema from  the file
    ReadSchema(rel, bh->data);
    unpinPage(bm, bh);

    //copy the name and buffer pool
    memcpy(rel->name, name, sizeof (name));
    rel->mgmtData = bm;
    free(bh);
    return RC_OK;
}

/*
 * AUTHOR: Arpita  
 * METHOD: Close the table 
 * INPUT: Table Struct
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC closeTable(RM_TableData *rel) {
    free(rel->schema->dataTypes);
    free(rel->schema->attrNames);
    free(rel->schema->keyAttrs);
    free(rel->schema->typeLength);
    free(rel->schema);
    free(rel->name);
    shutdownBufferPool(rel->mgmtData);
    return RC_OK;
}

/*
 * AUTHOR: Arpita  
 * METHOD: Delete the table 
 * INPUT:  Table Name
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC deleteTable(char *name) {
    destroyPageFile(name);
    return RC_OK;
}

/*
 * AUTHOR: Arpita  
 * METHOD: Get the number of tuples 
 * INPUT:  Table Struct
 * OUTPUT: int
 */
int getNumTuples(RM_TableData *rel) {
    //variable declaation
    unsigned int no_blocks = 0, i = 1;
    unsigned int no_records = 0;
    unsigned int total_records = 0;
    unsigned int offset = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    //pin the Table info block to get the no of blocks
    pinPage(rel->mgmtData, bh, TABLE_INFO_BLOCK);
    memcpy(bh->data, &no_blocks, sizeof (int));
    unpinPage(rel->mgmtData, bh);

    for (i = 1; i <= no_blocks; i++) {
        pinPage(rel->mgmtData, bh, i);
        offset = offset + (2 * (sizeof (int)));
        memcpy(bh->data + offset, &no_records, sizeof (int));
        unpinPage(rel->mgmtData, bh);
        total_records = total_records + no_records;
    }
    free(bh);
    return total_records;
}

// handling records in a table

/*
 * AUTHOR: Arpita  
 * METHOD: Insert the record
 * INPUT:  Table Struct,Record Struct
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC insertRecord(RM_TableData* rel, Record* record) {
    //variable declaration
    unsigned int blk_num = 0, rec_offset = PAGE_SIZE;
    unsigned int size_blk_header = 0;
    unsigned int no_slot = 0;
    unsigned int no_records = 0;
    unsigned int size_record = 0, size_slot = 0;
    unsigned int blk_size = 0;

    BM_PageHandle *bh = MAKE_PAGE_HANDLE();
    RID *rid = (RID *) malloc(sizeof (RID));

    //pin the table info block to get the block number to insert the record
    pinPage(rel->mgmtData, bh, TABLE_INFO_BLOCK);
    //memcpy(bh->data, &blk_num, sizeof (int));
    memcpy(&blk_num, bh->data, sizeof (int));
    unpinPage(rel->mgmtData, bh);

    //get record size
    size_record = getRecordSize(rel->schema);
    size_slot = size_slot + sizeof (int);

    //get the offset to insert the record
    pinPage(rel->mgmtData, bh, blk_num + 1);

    unsigned int offset = 0;
    //Get the Block Header information
    memcpy(&blk_num, bh->data, sizeof (int));
    offset = offset + sizeof (int);
    memcpy(&no_slot, bh->data + offset, sizeof (int));
    offset = offset + sizeof (int);
    memcpy(&no_records, bh->data + offset, sizeof (int));
    offset = offset + sizeof (int);

    blk_size = (PAGE_SIZE - offset)-((no_slot)*(size_record + size_slot));
    //check if the block has space to add record
    if ((size_record + size_slot) > blk_size) {
        blk_num = blk_num + 1;
        no_slot = 0;
        no_records = 0;
        unpinPage(rel->mgmtData, bh);

        pinPage(rel->mgmtData, bh, TABLE_INFO_BLOCK);
        update_tbl_mgmt_info(rel->schema, bh->data, blk_num, TBL_ADD);
        markDirty(rel->mgmtData, bh);
        unpinPage(rel->mgmtData, bh);

        pinPage(rel->mgmtData, bh, blk_num + 1);
        update_data_block_header(bh->data, BLK_NEW, blk_num, no_slot, no_records);

        //calculate the record offset
        rec_offset = rec_offset - ((no_slot + 1) * size_record);

        //insert the slot offset
        insert_slot(bh->data, no_slot, rec_offset);

        //insert the record
        memmove(bh->data + rec_offset, record->data, size_record);

        update_data_block_header(bh->data, BLK_UPD_SLOT, blk_num, no_slot + 1, no_records);
        update_data_block_header(bh->data, BLK_UPD_REC, blk_num, no_slot, no_records + 1);
        markDirty(rel->mgmtData, bh);
        unpinPage(rel->mgmtData, bh);
    } else {
        //calcuate the record offset
        rec_offset = rec_offset - ((no_slot + 1) * size_record);

        //insert the slot offset
        insert_slot(bh->data, no_slot, rec_offset);

        //insert the record
        memmove(bh->data + rec_offset, record->data, size_record);
        update_data_block_header(bh->data, BLK_UPD_SLOT, blk_num, no_slot + 1, no_records);
        update_data_block_header(bh->data, BLK_UPD_REC, blk_num, no_slot, no_records + 1);
        markDirty(rel->mgmtData, bh);
        unpinPage(rel->mgmtData, bh);
    }
    rid->page = blk_num;
    rid->slot = no_slot;
    record->id.page = rid->page;
    record->id.slot = rid->slot;

    forceFlushPool(rel->mgmtData);
    free(bh);
    free(rid);
    return RC_OK;
}

/*
 * AUTHOR: Arpita  
 * METHOD: Delete the record
 * INPUT:  Table Struct,RID Struct
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC deleteRecord(RM_TableData *rel, RID id) {
    //variable declaration
    unsigned int offset = 0, rec_offset = 0, size_record = 0;
    unsigned int slot_offset = 0, tombmark = TOMBSTONE;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    //calculating the offsets
    size_record = getRecordSize(rel->schema);
    offset = offset + (3 * sizeof (int));
    slot_offset = id.slot;
    pinPage(rel->mgmtData, bh, id.page + 1);

    //marking the tombstone
    memmove(bh->data + (offset + (slot_offset * sizeof (int))), &tombmark, sizeof (int));

    markDirty(rel->mgmtData, bh);
    unpinPage(rel->mgmtData, bh);

    forceFlushPool(rel->mgmtData);
    free(bh);
    return RC_OK;
}

/*
 * AUTHOR: Arpita  
 * METHOD: update the record
 * INPUT:  Table Struct,Record Struct
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC updateRecord(RM_TableData *rel, Record *record) {
    //variable declaration
    unsigned int offset = 0, rec_offset = 0, size_record = 0;
    unsigned int slot_offset = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    //calculating the offsets
    size_record = getRecordSize(rel->schema);
    offset = offset + (3 * sizeof (int));
    slot_offset = record->id.slot;
    pinPage(rel->mgmtData, bh, record->id.page + 1);

    //updating the record
    memcpy(&rec_offset, bh->data + (offset + (slot_offset * sizeof (int))), sizeof (int));
    memmove(bh->data + rec_offset, record->data, size_record);

    markDirty(rel->mgmtData, bh);
    unpinPage(rel->mgmtData, bh);
    forceFlushPool(rel->mgmtData);
    free(bh);
    return RC_OK;
}

/*
 * AUTHOR: Arpita  
 * METHOD: Get the record
 * INPUT:  Table Struct,RID Struct,Record Struct
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC getRecord(RM_TableData *rel, RID id, Record *record) {
    //variable declaration
    unsigned int offset = 0, rec_offset = 0, size_record = 0;
    unsigned int slot_offset = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();

    //calculating the offsets
    size_record = getRecordSize(rel->schema);
    offset = offset + (3 * sizeof (int));
    slot_offset = id.slot;
    pinPage(rel->mgmtData, bh, id.page + 1);

    //fetching  the record
    memcpy(&rec_offset, bh->data + (offset + (slot_offset * sizeof (int))), sizeof (int));

    if (rec_offset == TOMBSTONE) {
        free(bh);
        return RC_RM_RECORD_DELETED;
    }
    memcpy(record->data, bh->data + rec_offset, size_record);
    record->id.page = id.page;
    record->id.slot = id.slot;
    unpinPage(rel->mgmtData, bh);

    free(bh);
    return RC_OK;
}

//Schema handler methods

/*
 * AUTHOR: Arpita  
 * METHOD: Calculate the record size
 * INPUT:  Schema struct
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
int getRecordSize(Schema *schema) {
    unsigned int i;
    unsigned int sizeofRecord = 0;

    //calculating the size of record
    for (i = 0; i < schema->numAttr; i++) {
        switch (schema->dataTypes[i]) {
            case DT_INT:
                sizeofRecord = sizeofRecord + sizeof (int);
                break;
            case DT_STRING:
                sizeofRecord = sizeofRecord + schema->typeLength[i];
                break;
            case DT_FLOAT:
                sizeofRecord = sizeofRecord + sizeof (float);
                break;
            case DT_BOOL:
                sizeofRecord = sizeofRecord + sizeof (bool);
                break;
        }
    }
    return sizeofRecord;
}

Schema *createSchema(int numAttr, char** attrNames, DataType* dataTypes, int* typeLength, int keySize, int* keys) {

    Schema *schema;

    //create the schema and assign the values
    schema = (Schema *) malloc(sizeof (Schema));
    schema->numAttr = numAttr;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keySize = keySize;
    schema->keyAttrs = keys;

    //return schema
    return schema;
}

RC freeSchema(Schema* schema) {
    free(schema);
    return RC_OK;
}

//Record handler methods

RC createRecord(Record** record, Schema* schema) {

    unsigned int i;
    unsigned int size_Record = 0;

    //calculating the size of record
    size_Record = getRecordSize(schema);

    //allocating memory to record
    *record = (Record *) malloc(sizeof (Record));
    (*record)->data = (char *) malloc(size_Record);

    return RC_OK;
}

RC freeRecord(Record *record) {
    free(record->data);
    free(record);
}

/*
 * AUTHOR: Nikhil
 * METHOD: Set Attribute
 * INPUT:  record , schema, attrNum, value
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC setAttr(Record* record, Schema* schema, int attrNum, Value* value) {
    //variable declaration
    unsigned int i = 0;
    unsigned int offset = 0;

    //calculating the offset for setting attribute
    attrOffset(schema, attrNum, &offset);

    switch (schema->dataTypes[attrNum]) {
        case DT_INT:
        {
            int num = value->v.intV;
            memmove(record->data + offset, &num, sizeof (int));
        }
            break;
        case DT_STRING:
        {
            char *buf;
            int len = schema->typeLength[attrNum];
            buf = (char *) malloc(len);
            buf = value->v.stringV;
            memmove(record->data + offset, buf, len);
        }
            break;
        case DT_FLOAT:
        {
            float num = value->v.floatV;
            memmove(record->data + offset, &num, sizeof (float));
        }
            break;
        case DT_BOOL:
        {
            bool num = value->v.boolV;
            memmove(record->data + offset, &num, sizeof (bool));
        }
            break;
        default:
            return RC_FUNC_INVALIDARG;
    }

    return RC_OK;
}

/*
 * AUTHOR: Sindhu & Nikhil
 * METHOD: Get Attribute
 * INPUT:  record , schema, attrNum, value
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {

    //variable declartion
    unsigned int i = 0;
    unsigned int offset = 0;
    char *attrData;
    Value *val;
    int inum = 0;
    int len = 0;
    float fnum;
    bool bnum;

    if (record == NULL || schema == NULL)
        return RC_FUNC_INVALIDARG;

    val = (Value *) malloc(sizeof (Value));

    attrOffset(schema, attrNum, &offset);
    val->dt = schema->dataTypes[attrNum];
    switch (schema->dataTypes[attrNum]) {

        case DT_INT:
            memcpy(&inum, record->data + offset, sizeof (int));
            val->v.intV = inum;
            break;
        case DT_STRING:

            attrData = record->data + offset;
            len = schema->typeLength[attrNum];
            val->v.stringV = (char *) malloc(len + 1);
            strncpy(val->v.stringV, attrData, len);
            val->v.stringV[len] = '\0';
            break;
        case DT_FLOAT:

            memcpy(&fnum, record->data + offset, sizeof (float));
            val->v.floatV = fnum;
            break;
        case DT_BOOL:

            memcpy(&bnum, record->data + offset, sizeof (bool));
            val->v.boolV = bnum;
            break;
        default:
            free(val);
            return RC_FUNC_INVALIDARG;

    }
    *value = val;
    return RC_OK;
}

/*
 * AUTHOR: Sindhu
 * METHOD: copy of Expr
 *  Parses the Exper structure recursively and
 *  allocates memory and copies data.
 * INPUT:  expr , result_expr
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
static RC copyExpr(Expr *expr, Expr **result_expr) {
    RC err = RC_OK;
    Value *val = NULL;
    Operator *op = NULL;
    Expr *copy_expr = NULL;

    if (result_expr == NULL || expr == NULL)
        return RC_FUNC_INVALIDARG;

    if (expr == NULL)
        goto End;

    copy_expr = (Expr *) malloc(sizeof (Expr));

    if (copy_expr == NULL)
        return RC_NOMEM;

    copy_expr->type = expr->type;

    switch (expr->type) {

        case EXPR_CONST:
            val = (Value *) malloc(sizeof (Value));
            if (val == NULL) {
                err = RC_NOMEM;
                goto End;
            }
            *val = *(expr->expr.cons);
            copy_expr->expr.cons = val;
            break;
        case EXPR_ATTRREF:
            copy_expr->expr.attrRef = expr->expr.attrRef;
            break;
        case EXPR_OP:
            op = (Operator *) malloc(sizeof (Operator));
            if (op == NULL) {
                err = RC_NOMEM;
                goto End;
            }
            op->type = expr->expr.op->type;
            op->args = expr->expr.op->args;
            copyExpr(expr->expr.op->args[0], &op->args[0]);
            if (op->args[0] != NULL && op->type != OP_BOOL_NOT) {
                copyExpr(expr->expr.op->args[1], &op->args[1]);
            }
            if (op->args[0] == NULL || (op->type != OP_BOOL_NOT && op->args[1] == NULL)) {
                err = RC_NOMEM;
                goto End;
            }
            break;
        default:
            err = RC_FUNC_INVALIDARG;
    }
End:
    if (err != RC_OK) {
        freeExpr(copy_expr);
        copy_expr = NULL;
    }
    *result_expr = copy_expr;
    return err;
}

/*
 * AUTHOR: Sindhu
 * METHOD: Start scan
 * INPUT:  table data , scan handle, condition
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {

    ScanData *data = NULL;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();
    if (rel == NULL || scan == NULL)
        return RC_FUNC_INVALIDARG;

    data = (ScanData *) malloc(sizeof (ScanData));
    if (data == NULL)
        return RC_NOMEM;

    pinPage(rel->mgmtData, bh, TABLE_INFO_BLOCK);
    memcpy(&data->no_blocks, bh->data, sizeof (int));
    unpinPage(rel->mgmtData, bh);
    data->no_blocks++;
    data->cond = cond;
    data->currRecord = 0;
    data->currentblock = 1;
    data->numRecords = 0;
    data->parsedRecords = 0;

    scan->mgmtData = (void *) data;
    scan->rel = rel;
    free(bh);
    return RC_OK;
}

/*
 * AUTHOR: Sindhu
 * METHOD: next
 * INPUT:  scan handle , record
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC next(RM_ScanHandle *scan, Record *record) {


    ScanData *data;
    unsigned int offset = 0, blockOffset = 0, slot_offset = 0, size_record = 0;
    BM_PageHandle *bh = MAKE_PAGE_HANDLE();
    BM_BufferPool *bm;
    int no_records = 0;
    Schema *sch;
    RID *rids;


    Value *value;
    sch = scan->rel->schema;
    data = scan->mgmtData;
    Expr *cond = data->cond;
    bm = scan->rel->mgmtData;
    // cond->expr=scan->mgmtData;
    data->numRecords = getNumTuples(scan->rel);
    blockOffset = 3 * (sizeof (int));

    pinPage(bm, bh, data->currentblock);
    offset = offset + (2 * (sizeof (int)));
    memcpy(&no_records, bh->data + offset, sizeof (int));
    unpinPage(bm, bh);
    free(bh);
    rids = (RID *) malloc(sizeof (RID) * no_records);
    if (data->currentblock < data->no_blocks + 1) {
        if (data->currRecord <= no_records) {
            Expr *attrnum, *temp;
            if (cond->expr.op->type == OP_BOOL_NOT) {
                temp = cond->expr.op->args[0];
                attrnum = temp->expr.op->args[0];
            } else {
                attrnum = cond->expr.op->args[1];
            }
            record->id.page = data->currentblock - 1;
            record->id.slot = data->currRecord;
            RID rid = record->id;
            getRecord(scan->rel, rid, record);

            getAttr(record, sch, attrnum->expr.attrRef, &value);

            evalExpr(record, sch, cond, &value);
            data->currRecord = data->currRecord + 1;
            scan->mgmtData = data;
            if (value->v.boolV == 1) {
                return RC_OK;
            } else {
                next(scan, record);
            }
        } else {
            if (data->currentblock + 1 < data->no_blocks) {
                data->currentblock = data->currentblock + 1;
                data->currRecord = 0;
                next(scan, record);
            } else {

                return RC_RM_NO_MORE_TUPLES;
            }
        }
    } else {

        return RC_RM_NO_MORE_TUPLES;

    }

}

/*
 * AUTHOR: Sindhu
 * METHOD: closeScan
 * INPUT:  scan handle
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC closeScan(RM_ScanHandle *scan) {

    free(scan->mgmtData);

    return RC_OK;
}

/*
 * AUTHOR: Arpita
 * METHOD: Update the table mgmt info
 * INPUT:  Schema,Data,TableInfoUpdate
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC update_tbl_mgmt_info(Schema *schema, char *data, int dataBlock, TableInfoUpdate type) {
    unsigned int offset = 0, sizeofSchema = 0;
    char *serSchema = malloc(100);

    switch (type) {
            //when page file is created for the first time
        case 0:
            serSchema = serializeSchema1(schema);
            sizeofSchema = strlen(serSchema);
            memmove(data, &dataBlock, sizeof (int));
            offset = offset + sizeof (int);
            memmove(data + offset, &sizeofSchema, sizeof (int));
            offset = offset + sizeof (int);
            memmove(data + offset, serSchema, sizeofSchema);
            break;
            //when page file has to be updated with the new block
        case 1:
            memmove(data, &dataBlock, sizeof (int));
            break;
            //when page file block directory has to be updated
        case 2:
            //In case needed extension
            break;
    }
    free(serSchema);
}

/*
 * AUTHOR: Arpita
 * METHOD: Update the data block header
 * INPUT:  Data
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC update_data_block_header(char *data, TableInfoUpdate type, int blk_num, int num_slots, int num_records) {
    //variable declaration
    unsigned int offset = 0;

    switch (type) {
            //when page file is created for the first time
        case 0:
            memmove(data, &blk_num, sizeof (int));
            offset = offset + sizeof (int);
            memmove(data + offset, &num_slots, sizeof (int));
            offset = offset + sizeof (int);
            memmove(data + offset, &num_records, sizeof (int));
            break;
        case 1:
            offset = offset + (2 * (sizeof (int)));
            memmove(data + offset, &num_records, sizeof (int));
            break;
        case 2:
            offset = offset + sizeof (int);
            memmove(data + offset, &num_slots, sizeof (int));
            break;
    }
}

/*
 * AUTHOR: Arpita
 * METHOD: Insert the record offset in the slot directory
 * INPUT:  Data,Slots
 * OUTPUT: RC_OK-SUCESSS;RC_OTHERS-ON FAIL;
 */
RC insert_slot(char *data, int num_slots, int rec_offset) {
    unsigned int slot_offset = 3 * (sizeof (int));

    //calcualte the offset to insert the record offset 
    slot_offset = slot_offset + ((num_slots)*(sizeof (int)));

    memmove(data + slot_offset, &rec_offset, sizeof (int));
    return RC_OK;
}

RC ReadSchema(RM_TableData* rel, char *schemadata) {
    Schema *schema;
    char *tempschema, *temp1, *temp2, *temp3;
    char* columns[50];
    int i;
    unsigned int sizeOfSchema = 0, offset = 0;


    rel->schema = malloc(sizeof (Schema));



    offset = offset + sizeof (int);
    memcpy(&sizeOfSchema, schemadata + offset, sizeof (int));
    tempschema = (char *) malloc(sizeOfSchema);
    offset = offset + sizeof (int);
    memcpy(tempschema, schemadata + offset, sizeOfSchema);

    rel->schema->numAttr = atoi(strtok(tempschema, ";"));

    rel->schema->typeLength = malloc((rel->schema->numAttr) * sizeof (int));

    temp1 = strtok(NULL, ";");

    rel->schema->keySize = atoi(strtok(NULL, ";"));

    rel->schema->keyAttrs = malloc((rel->schema->keySize) * sizeof (int));

    temp2 = strtok(NULL, ";");
    i = 0;
    temp2 = strtok(temp2, ",");

    while (temp2 != NULL) {

        rel->schema->keyAttrs[i] = atoi(temp2);
        temp2 = strtok(NULL, ",");
        i++;

    }

    temp2 = strtok(temp1, ",");
    i = 0;
    while (temp2 != NULL) {

        columns[i] = temp2;
        temp2 = strtok(NULL, ",");
        i++;

    }

    rel->schema->attrNames = (char **) malloc(rel->schema->numAttr * sizeof (char*));
    (rel->schema)->dataTypes = (DataType *) malloc(rel->schema->numAttr * sizeof (DataType));
    for (i = 0; i < rel->schema->numAttr; i++) {

        rel->schema->attrNames[i] = strtok(columns[i], ":");

        temp2 = strtok(NULL, ":");
        if (strstr(temp2, "INT")) {

            rel->schema->dataTypes[i] = DT_INT;
        } else if (strstr(temp2, "STR")) {
            rel->schema->dataTypes[i] = DT_STRING;
            temp3 = strstr(temp2, "[");
            temp3 = strtok(temp3, "]");
            temp3++;
            rel->schema->typeLength[i] = atoi(temp3);

        } else if (strstr(temp2, "FLOAT")) {
            rel->schema->dataTypes[i] = DT_FLOAT;
        } else if (strstr(temp2, "BOOL")) {
            rel->schema->dataTypes[i] = DT_BOOL;
        }

    }
    free(tempschema);
    return RC_OK;
}
