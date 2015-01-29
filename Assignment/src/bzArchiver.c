#include <bz2lib.h>

int main(int argc, char *argv[])
{
	bz_struct *bzStruct;
	bz_stream *stream;

	char *sourceFile;
	char *destFile;

	stream = malloc(sizeof(bz_stream));
	bzStruct = malloc(sizeof(bz_struct));

	switch(argc)
	{
		case 3:
			sourceFile = malloc(FILENAME_LENGTH);
			strcpy(sourceFile, argv[2]);
			if(!strcmp(argv[1],"-c") || !strcmp(argv[1],"-C"))
			{
				if(hasbz2Extention(sourceFile)==YES)
				{
					free(sourceFile);
					free(bzStruct);
					free(stream);
					printf("file already compressed..\n");
					return 0;
				}
				else
				{
					destFile = malloc(FILENAME_LENGTH);
					strcpy(destFile,sourceFile);
					strcat(destFile,".bz2");
				}
			}
			else if(!strcmp(argv[1],"-d") || !strcmp(argv[1],"-D"))
			{
				if(hasbz2Extention(sourceFile)==NO)
				{
					free(sourceFile);
					free(bzStruct);
					free(stream);
					printf("please choose another file..\n");
					return 0;
				}
				else
				{
					destFile = malloc(FILENAME_LENGTH);
					strncpy(destFile,sourceFile,strlen(sourceFile)-4);
					break;
				}
			}
			else
			{
				printf("please choose a valid option \n"
						"-c or -C for compression\n"
						"-d or -D for decompression\n");
			}
			break;

		case 4:
			sourceFile = malloc(FILENAME_LENGTH);
			strcpy(sourceFile, argv[2]);

			destFile = malloc(FILENAME_LENGTH);
			strcpy(destFile, argv[3]);

			if(!strcmp(argv[1],"-c") || !strcmp(argv[1],"-C"))
			{
				if(hasbz2Extention(sourceFile)==YES ||hasbz2Extention(destFile)==NO)
				{
					free(sourceFile);
					free(bzStruct);
					free(stream);
					printf("please check your source and destination..\n");
					return 0;
				}
			}
			else if(!strcmp(argv[1],"-d") || !strcmp(argv[1],"-D"))
			{
				if(hasbz2Extention(sourceFile)==NO ||hasbz2Extention(destFile)==YES)
				{
					free(sourceFile);
					free(bzStruct);
					free(stream);
					printf("please check your source and destination....\n");
					return 0;
				}
			}
			else
			{
				printf("please choose a valid option \n"
						"-c or -C for compression\n"
						"-d or -D for decompression\n");
			}
			break;

		default:
			free(bzStruct);
			free(stream);
			printf("usage\n(compress) => ./output <option> <source> <destination>\n");
			return 0;
	}

	if (stream == NULL || bzStruct == NULL)
	{
		return NOT_OK;
	}

	if (!strcmp(argv[1], "-c") || !strcmp(argv[1], "-C"))
	{
		if (initcompress(stream, bzStruct) != OK)
		{

			printf("initcompress  not successful...\n");
			return 0;
		}

		if(compressAndWrite(sourceFile, destFile, stream, bzStruct)!=OK)
		{
			free(destFile);
			free(sourceFile);
			free(bzStruct);
			free(stream);
			return 0;
		}

		if(finilizeCompression(stream, bzStruct)!=OK)
		{
			free(destFile);
			free(sourceFile);
			free(bzStruct);
			free(stream);
			return 0;
		}
	}

	else if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "-D"))
	{
		if (initializeDecompress(stream, bzStruct) != OK)
		{
			printf("initdecompress  not successful...\n");
			return 0;
		}

		if(DecompressAndWrite(sourceFile, destFile, stream, bzStruct)!=OK)
		{
			free(destFile);
			free(sourceFile);
			free(bzStruct);
			free(stream);
			return 0;
		}

		if(decompressEnd(stream, bzStruct) != OK)
		{
			free(destFile);
			free(sourceFile);
			free(bzStruct);
			free(stream);
			return 0;
		}
	}
	return 0;
}

int initcompress(bz_stream *stream, bz_struct *bzStruct)
{
	memset(stream, 0, sizeof(bz_stream));
	memset(bzStruct, 0, sizeof(bz_struct));

	stream->bzalloc = NULL;
	stream->bzfree = NULL;
	stream->opaque = NULL;

	stream->avail_out = BUFFER_SIZE;
	stream->next_out = bzStruct->outputBuffer;

	if (BZ2_bzCompressInit(stream, BLOCK_SIZE, VERBOSITY, WORKFACTOR) != BZ_OK)
	{
		return COMP_INIT_ERROR;
	}

	return OK;
}

int compressAndWrite(char *sourceFile, char *destFile, bz_stream *stream,
		bz_struct *bzStruct)
{
	int byteRead;
	int compressRet;
	int byteWrote;

	bzStruct->sourceFileDesc = open(sourceFile, O_RDONLY);
	bzStruct->destFileDesc = open(destFile, O_WRONLY | O_CREAT);

	if (bzStruct->sourceFileDesc == -1)
	{
		return FILE_OPEN_ERROR;
	}

	if (bzStruct->destFileDesc == -1)
	{
		return FILE_OPEN_ERROR;
	}
	stream->avail_out = BUFFER_SIZE;
	stream->next_out = bzStruct->outputBuffer;
	do
	{
		if (stream->avail_in == 0)
		{
			byteRead = read(bzStruct->sourceFileDesc, bzStruct->inputBuffer,
			BUFFER_SIZE);
			if (byteRead == -1)
			{
				return FILE_READ_ERROR;
			}

			stream->avail_in = byteRead;
			stream->next_in = bzStruct->inputBuffer;

			if (byteRead < BUFFER_SIZE)
			{
				break;
			}
		}

		compressRet = BZ2_bzCompress(stream, BZ_RUN);
		if (compressRet != BZ_RUN_OK)
		{
			return COMPRESS_ERROR;
		}

		if (stream->avail_out != BUFFER_SIZE)
		{
			byteWrote = write(bzStruct->destFileDesc, bzStruct->outputBuffer,
			BUFFER_SIZE - stream->avail_out);
			if (byteWrote == -1)
			{
				return FILE_WRITE_ERROR;
			}
			stream->avail_out = BUFFER_SIZE;
			stream->next_out = bzStruct->outputBuffer;
		}
	} while (true);

	do
	{
		compressRet = BZ2_bzCompress(stream, BZ_FINISH);
		if (compressRet == BZ_FINISH_OK || compressRet == BZ_STREAM_END)
		{
			byteWrote = write(bzStruct->destFileDesc, bzStruct->outputBuffer,
			BUFFER_SIZE - stream->avail_out);
			if (byteWrote == -1)
			{
				return FILE_WRITE_ERROR;
			}
			stream->avail_out = BUFFER_SIZE;
			stream->next_out = bzStruct->outputBuffer;
		}
		else
			return COMPRESS_ERROR;
	} while (compressRet != BZ_STREAM_END);
	return OK;
}

int finilizeCompression(bz_stream *stream, bz_struct *bzStruct)
{
	if (BZ2_bzCompressEnd(stream) != BZ_OK)
	{
		return COMPRESS_END_ERROR;
	}

	if (close(bzStruct->sourceFileDesc) == -1)
	{
		return FILE_CLOSE_ERROR;
	}

	if (close(bzStruct->destFileDesc) == -1)
	{
		return FILE_CLOSE_ERROR;
	}

	return OK;
}

int initializeDecompress(bz_stream *stream, bz_struct *bzStruct)
{
	memset(stream, 0, sizeof(bz_stream));
	memset(bzStruct, 0, sizeof(bz_struct));

	stream->bzalloc = NULL;
	stream->bzfree = NULL;
	stream->opaque = NULL;

	stream->avail_out = BUFFER_SIZE;
	stream->next_out = bzStruct->outputBuffer;

	if (BZ2_bzDecompressInit(stream, VERBOSITY, SMALL) != BZ_OK)
	{
		return INIT_DECOMPRESS_ERROR;
	}

	return OK;
}

int DecompressAndWrite(char *sourceFile, char *destFile, bz_stream *stream,
		bz_struct *bzStruct)
{
	int byteRead;
	int totalRead = 0;
	int byteWrote;
	int decompressRet;
	int totalWrote = 0;

	bzStruct->sourceFileDesc = open(sourceFile, O_RDONLY);
	bzStruct->destFileDesc = open(destFile, O_WRONLY | O_CREAT);

	if (bzStruct->sourceFileDesc == -1)
	{
		return FILE_OPEN_ERROR;
	}

	if (bzStruct->destFileDesc == -1)
	{
		return FILE_OPEN_ERROR;
	}

	do
	{
		byteRead = read(bzStruct->sourceFileDesc, bzStruct->inputBuffer, BUFFER_SIZE);

		if (byteRead == -1)
		{
			return FILE_READ_ERROR;
		}

		totalRead += byteRead;
		stream->avail_in = byteRead;
		stream->next_in = bzStruct->inputBuffer;
		decompressRet = BZ2_bzDecompress(stream);

		if (decompressRet != BZ_OK && decompressRet != BZ_STREAM_END)
		{
			return DECOMPRESS_ERROR;
		}
		if (stream->avail_out != BUFFER_SIZE)
		{
			do
			{
				byteWrote = write(bzStruct->destFileDesc, bzStruct->outputBuffer,
				BUFFER_SIZE - stream->avail_out);

				if (byteWrote == -1)
				{
					return FILE_WRITE_ERROR;
				}

				stream->next_out = bzStruct->outputBuffer;
				stream->avail_out = BUFFER_SIZE;
				totalWrote += byteWrote;

				decompressRet = BZ2_bzDecompress(stream);
				if (decompressRet != BZ_OK && decompressRet != BZ_STREAM_END)
				{
					return DECOMPRESS_ERROR;
				}
			}while(decompressRet != BZ_STREAM_END && stream->avail_out != BUFFER_SIZE);

			byteWrote = write(bzStruct->destFileDesc, bzStruct->outputBuffer,
			BUFFER_SIZE - stream->avail_out);

			if (byteWrote == -1)
			{
				return FILE_WRITE_ERROR;
			}
		}

	} while (byteRead != 0);


	return OK;
}

int decompressEnd(bz_stream *stream, bz_struct *bzStruct)
{
	if (BZ2_bzDecompressEnd(stream) != BZ_OK)
	{
		return DECOM_END_ERROR;
	}

	if (close(bzStruct->sourceFileDesc) == -1)
	{
		return FILE_CLOSE_ERROR;
	}

	if (close(bzStruct->sourceFileDesc) == -1)
	{
		return FILE_CLOSE_ERROR;
	}

	return OK;
}

int hasbz2Extention(char *FileName)
{
	int length;
	length = strlen(FileName);
	if(strcmp(FileName+length-4, ".bz2"))
	{
		return NO;
	}
	else
	{
		return	YES;
	}
}
