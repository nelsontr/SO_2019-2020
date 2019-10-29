# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20
#	Modified by Matheus França and Nelson Trindade, group 22 

CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../
LDFLAGS=-lm
PTHREAD=-pthread
MUTEX=-DMUTEX	#Flag for the program to use Mutex
RWLOCK=-DRWLOCK #Flag for the program to use Rwlock


# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs-nosync tecnicofs-mutex tecnicofs-rwlock

tecnicofs-nosync: lib/bst.o   fs.o threads.o main-nosync.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-nosync lib/bst.o   fs.o threads.o main-nosync.o
tecnicofs-mutex: lib/bst.o   fs.o threads-mutex.o main-mutex.o
	$(LD) $(CFLAGS) $(LDFLAGS) $(PTHREAD) -o tecnicofs-mutex lib/bst.o   fs.o threads-mutex.o main-mutex.o
tecnicofs-rwlock: lib/bst.o   fs.o threads-rwlock.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) $(PTHREAD) -o tecnicofs-rwlock lib/bst.o   fs.o threads-rwlock.o main-rwlock.o

threads-mutex.o: threads.c threads.h
	$(CC) $(CFLAGS) $(MUTEX) -o threads-mutex.o -c threads.c

threads-rwlock.o: threads.c threads.h
	$(CC) $(CFLAGS) $(RWLOCK) -o threads-rwlock.o -c threads.c

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

fs.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o fs.o -c fs.c

main-nosync.o: main.c fs.h lib/bst.h  
	$(CC) $(CFLAGS) $(PTHREAD)  -o main-nosync.o -c main.c

main-mutex.o: main.c fs.h lib/bst.h  
	$(CC) $(CFLAGS) $(PTHREAD) $(MUTEX) -o main-mutex.o -c main.c

main-rwlock.o: main.c fs.h lib/bst.h  
	$(CC) $(CFLAGS) $(PTHREAD) $(RWLOCK) -o main-rwlock.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs*

run: tecnicofs-nosync
	./tecnicofs-nosync
