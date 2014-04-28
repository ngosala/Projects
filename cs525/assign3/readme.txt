Assignment 3 implements a Record Manager which helps in creating, inserting, updating an deleting records from the Page File, as well as scanning the records with the eval expressions.

1> Recordmgr.c
Implements record creation, insertion, updation and deletion. Along with managing the table information, block header information and creating and updating the slot directory in the data blocks.

Scan funtions for scaning the records with eval expressions.

Additional Helper methods added to enhance the modularity of code.
Record format is Fixed length. Records are inserted at the end of the block file and slot directory starts after block header. Tombstone concept implemented by making the record offset 0 in the record directory

Table Management info has no. of blocks and schema of the file
Data block header has block no, no of slots and no of records


2> Parser.c 
Implements the shell for executing the SQL stmts. Support execution of Create, Insert ,Select , Update and Deletion stmts.

Delete and upate are done based on the primary key. 
Test cases are provided in the testparser.txt

3> db.h
Implements scan data structs required for Scan operations.

>Additional test cases are also added to test support for bool and float data types.