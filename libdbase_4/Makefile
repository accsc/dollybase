INCLUDE=
#
# Makefile libdollybase
#
#

CC = gcc
SRCS = index.c low.c recs.c deletes.c seeks.c appends.c \
       creates.c export.c memofields.c locker.c \
       labels.c relations.c
OBJS = .libs/index.o .libs/low.o .libs/recs.o \
       .libs/deletes.o .libs/seeks.o .libs/appends.o \
       .libs/creates.o .libs/export.o .libs/memofields.o \
       .libs/locker.o .libs/labels.o .libs/relations.o

all:
	@echo "*******************************************"
	@echo "*					  *"
	@echo "*  	 Compiling libDollyBASE 4         *"
	@echo "*					  *"
	@echo "*******************************************"
	@mkdir -p .libs
	@$(CC) -w -c -fPIC -o .libs/index.o index.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/low.o low.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/recs.o recs.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/deletes.o deletes.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/seeks.o seeks.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/appends.o appends.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/creates.o creates.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/export.o export.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/memofields.o memofields.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/locker.o locker.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/labels.o labels.c $(INCLUDE)
	@$(CC) -w -c -fPIC -o .libs/relations.o relations.c $(INCLUDE)
	@ar q .libs/libdbase_0.4_s.a $(OBJS)
	@ranlib .libs/libdbase_0.4_s.a

debug:
	@echo "Warning: Heavy debug enabled. Huge info about internal work will be provided"
	@mkdir -p .libs
	@$(CC) -c -DDEBUG -fPIC -o .libs/index.o index.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/low.o low.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/recs.o recs.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/deletes.o deletes.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/seeks.o seeks.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/appends.o appends.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/creates.o creates.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/export.o export.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/memofields.o memofields.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/locker.o locker.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/labels.o labels.c $(INCLUDE)
	@$(CC) -c -DDEBUG -fPIC -o .libs/relations.o relations.c $(INCLUDE)
	@ar q .libs/libdbase_0.4_s.a $(OBJS)
	@ranlib .libs/libdbase_0.4_s.a

clean:
	rm -f .libs/*.o .libs/libdbase_0.4_s.a
test: all
	cd regresion && make
