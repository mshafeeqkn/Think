#include <bz2lib.h>

int main(int argc, char *argv[])
{
	bz_struct *bzStruct;
	bz_stream *stream;

	if (argc != 4)
	{
		printf(
				"usage\n(compress) => ./output <option> <source> <destination>\n");
		exit(1);
	}

	stream = malloc(sizeof(bz_stream));
	bzStruct = malloc(sizeof(bz_struct));

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

		compressAndWrite(argv[2], argv[3], stream, bzStruct);

		finilizeCompression(stream, bzStruct);
	}

	else if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "-D"))
	{
		if (initializeDecompress(stream, bzStruct) != OK)
		{
			printf("initdecompress  not successful...\n");
			return 0;
		}

		DecompressAndWrite(argv[2], argv[3], stream, bzStruct);
		decompressEnd(stream, bzStruct);
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
	do
	{
		byteRead = read(bzStruct->sourceFileDesc, bzStruct->inputBuffer,
				BUFFER_SIZE);
		if (byteRead == -1)
		{
			return FILE_READ_ERROR;
		}

		if (byteRead == 0)
		{
			break;
		}

		stream->avail_in = byteRead;
		stream->next_in = bzStruct->inputBuffer;
		stream->avail_out = BUFFER_SIZE;
		stream->next_out = bzStruct->outputBuffer;

		compressRet = BZ2_bzCompress(stream, BZ_RUN);
		if (compressRet != BZ_RUN_OK)
		{
			return COMPRESS_ERROR;
		}

		if (stream->avail_out != BUFFER_SIZE)
		{
			do
			{
				byteWrote = write(bzStruct->destFileDesc,
						bzStruct->outputBuffer,
						BUFFER_SIZE - stream->avail_out);
				if (byteWrote == -1)
				{
					return FILE_WRITE_ERROR;
				}
				stream->avail_out = BUFFER_SIZE;
				stream->next_out = bzStruct->outputBuffer;
				compressRet = BZ2_bzCompress(stream, BZ_FLUSH);
			} while (compressRet != BZ_RUN_OK);
			byteWrote = write(bzStruct->destFileDesc, bzStruct->outputBuffer,
					BUFFER_SIZE - stream->avail_out);
			if (byteWrote == -1)
			{
				return FILE_WRITE_ERROR;
			}
		}
	} while (true);

	do
	{
		stream->avail_out = BUFFER_SIZE;
		stream->next_out = bzStruct->outputBuffer;
		compressRet = BZ2_bzCompress(stream, BZ_FINISH);
		if (compressRet == BZ_STREAM_END)
		{
			break;
		}
	} while (compressRet == BZ_FINISH_OK);

	byteWrote = write(bzStruct->destFileDesc, bzStruct->outputBuffer,
			BUFFER_SIZE - stream->avail_out);

	if (byteWrote == -1)
	{
		return FILE_WRITE_ERROR;
	}

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
	int compressRet;
	int totalWrote = 0;
	int i;

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
		byteRead = read(bzStruct->sourceFileDesc, bzStruct->inputBuffer,
				BUFFER_SIZE);

		if (byteRead == -1)
		{
			return FILE_READ_ERROR;
		}

		if (byteRead == 0)
		{
			break;
		}

		totalRead += byteRead;

		stream->avail_in = byteRead;
		stream->next_in = bzStruct->inputBuffer;

		do
		{
			for (i = 0; i < 800; i++)
			{
				compressRet = BZ2_bzDecompress(stream);

				if (compressRet != BZ_OK && compressRet != BZ_STREAM_END)
				{
					return DECOMPRESS_ERROR;
				}

				byteWrote = write(bzStruct->destFileDesc,
						bzStruct->outputBuffer,
						BUFFER_SIZE - stream->avail_out);

				if (byteWrote == -1)
				{
					return FILE_WRITE_ERROR;
				}
				stream->next_out = bzStruct->outputBuffer;
				stream->avail_out = BUFFER_SIZE;
				totalWrote += byteWrote;

			}

		} while (compressRet == BZ_STREAM_END);

	} while (true);

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

