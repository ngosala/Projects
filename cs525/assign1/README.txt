First Assignment includes implementation of Storage Manager which is capable creating and opening a file, writing at specified location, 

reading from specified location and keeping track of current position using a custom data structure.


>Implemented createPageFile method for creating a file with initial page size 1 and size of 0B.

>Implemented openPageFile method for opening the created file and store the details of file using custom data structur SM_FileHanlde.
 
>Implemeted closePageFile and destroyPageFile methods for closing and delete files.

>Implemnted getBlockPos for keeping track of current position of the file.

>Read and write functionalities readCurrentBlock,readPreviousBlock,readNextBlock,readLastBlock,readFirstBlock,readBlock,writeBlock,writeCurrentBlock are implemented
 in such a way that any block in the file can be read and/or written. Each function implements definite architecture to keep track of ongoing events like changing the 
position parameter after writing the file. 

>Additional functionalities ensureCapacity for ensuring required number of pages are available and appendEmptyBlock to append empty blocks when required are implemented.

>Additional test cases are also added to test the complete functionality of program. 