#ifndef BZ2LIB_H
#define BZ2LIB_H

#define		BUFFER_SIZE			4096
#define		FILENAME_LENGTH		1024
#define 	BLOCK_SIZE 			9
#define 	VERBOSITY			0
#define		WORKFACTOR			30
#define		SMALL				0

#define		OK					1
#define		NOT_OK				0

#define		YES					1
#define		NO					0

#define		COMP_INIT_ERROR			(-1)
#define		FILE_OPEN_ERROR			(-2)
#define		FILE_READ_ERROR			(-3)
#define		FILE_CLOSE_ERROR		(-4)
#define		FILE_WRITE_ERROR		(-5)
#define		COMPRESS_ERROR			(-6)
#define		COMPRESS_END_ERROR		(-7)
#define		INIT_DECOMPRESS_ERROR	(-8)
#define		DECOMPRESS_ERROR		(-9)
#define		DECOM_END_ERROR			(-10)

#include "bzlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>


typedef struct
{
    char inputBuffer[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE];
    int sourceFileDesc;
    int destFileDesc;
}bz_struct;

int initcompress(bz_stream *stream, bz_struct *bzStruct);
int compressAndWrite(char *sourceFile, char *destFile,
                        bz_stream *stream, bz_struct *bzStruct);
int finilizeCompression(bz_stream *stream, bz_struct *bzStruct);
int initializeDecompress(bz_stream *stream, bz_struct *bzStruct);
int DecompressAndWrite(char *sourceFile, char *destFile,
                        bz_stream *stream, bz_struct *bzStruct);
int decompressEnd(bz_stream *stream, bz_struct *bzStruct);
int hasbz2Extention(char *FileName);

#endif
