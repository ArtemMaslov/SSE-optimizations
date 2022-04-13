#ifndef FILE_IO_H_
#define FILE_IO_H_

struct BmpImage
{
	size_t width;
	size_t height;
	size_t bytePerPixel;

	size_t dataSize;
	char   *data;
};

size_t GetFileSize(FILE* file);

bool ReadBitMap(const char* fileName, BmpImage* bmp);

void BmpImageDestructor(BmpImage* bmp);

#endif