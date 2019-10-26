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

tecnicofs-nosync: lib/bst.o lib/hash.o fs.o threads.o main-nosync.o
	$(LD) $(CFLAGS) $(LDFLAGS) $(PTHREAD) -o tecnicofs-nosync lib/bst.o lib/hash.o fs.o threads.o main-nosync.o
tecnicofs-mutex: lib/bst.o lib/hash.o fs.o threads.o main-mutex.o
	$(LD) $(CFLAGS) $(LDFLAGS) $(PTHREAD) -o tecnicofs-mutex lib/bst.o lib/hash.o fs.o threads.o main-mutex.o
tecnicofs-rwlock: lib/bst.o lib/hash.o fs.o threads.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) $(PTHREAD) -o tecnicofs-rwlock lib/bst.o lib/hash.o fs.o threads.o main-rwlock.o

thread.o: threads.c threads.h
	$(CC) $(CFLAGS) -o threads.o -c threads.c

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

lib/hash.o: lib/hash.c lib/hash.h
	$(CC) $(CFLAGS) -o lib/hash.o -c lib/hash.c

fs.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o fs.o -c fs.c

main-nosync.o: main.c fs.h lib/bst.h lib/hash.h
	$(CC) $(CFLAGS) -o main-nosync.o -c main.c

main-mutex.o: main.c fs.h lib/bst.h lib/hash.h
	$(CC) $(CFLAGS) $(MUTEX) -o main-mutex.o -c main.c

main-rwlock.o: main.c fs.h lib/bst.h lib/hash.h
	$(CC) $(CFLAGS) $(RWLOCK) -o main-rwlock.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs*

run: tecnicofs-nosync
	./tecnicofs-nosync
