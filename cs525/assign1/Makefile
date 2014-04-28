
SRC := \
	dberror.c \
	storage_mgr.c \
	test_assign1_1.c
OBJ := \
	dberror.o \
	storage_mgr.o \
	test_assign1_1.o

assign1: $(OBJ)
	gcc -o assign1 $?
	

%.o: %.c
	gcc -g -c $<

clean:
	rm -rf assign1 *.o
