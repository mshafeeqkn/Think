CC = gcc

TARGET = output
# SOURCE = src/compress.c
SOURCE = src/bzArchiver.c
SOURCEOBJ = $(addsuffix .o,$(basename $(wildcard $(SOURCE))))
LIBPATH = lib
OPTTAR = -lbz2 
OPTOBJ = -c -Wall -Iinclude -g3
HEADERS = include/bz2lib.h include/bzlib.h

all: $(TARGET)

$(TARGET) : $(SOURCEOBJ) $(HEADERS)
	$(CC) -o $@ $< -L$(LIBPATH) $(OPTTAR) 

$(SOURCEOBJ): $(SOURCE) $(HEADERS)
	$(CC) $(OPTOBJ) -o $(SOURCEOBJ) $(SOURCE)

clean:
	rm -f $(TARGET) \
	$(SOURCEOBJ)