##########################################################
#
# $Id:$
#
# $Log:$
#
#

CC=i960-elf-gcc
#CFLAGS=-O -DNORMALUNIX -DLINUX
CFLAGS=-O -DNORMALUNIX -DBARE_METAL -msb -mnumerics
LDFLAGS=-msb -mnumerics --as-needed
LIBS=-lm -lc -lnosys

O=i960

all:	 $(O)/sndserver

clean:
	rm -f *.o *~ *.flc
	rm -f i960/*

# Target
$(O)/sndserver: \
	$(O)/soundsrv.o \
	$(O)/sounds.o \
	$(O)/wadread.o \
	$(O)/impl.o
	$(CC) $(CFLAGS) $(LDFLAGS) \
	$(O)/soundsrv.o \
	$(O)/sounds.o \
	$(O)/wadread.o \
	$(O)/impl.o -o $(O)/sndserver $(LIBS)
	echo make complete.

# Rule
$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


