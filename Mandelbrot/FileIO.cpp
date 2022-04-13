#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "FileIO.h"

#include <Windows.h>
#include <WinGDI.h>

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

size_t GetFileSize(FILE* file)
{
	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	return fileSize;
}

/**
 * @brief Читает файл в формате bmp.
 *            Поддерживается:
 *            1) Только 3 версия BMP. (BITMAPINFOHEADER)
 *            2) Только 24, 32 бита на пиксель.
 * 
 * @param fileName Имя входного файла.
 * @param bmp      Выходная структура данных.
 * 
 * @return false в случае ошибки.
*/
bool ReadBitMap(const char* fileName, BmpImage* bmp)
{
	assert(fileName);
	assert(bmp);

 	FILE* file = fopen(fileName, "r");

	if (!file)
	{
		printf("Файл \"%s\" не найден", fileName);
		return false;
	}

	const size_t bitmapMinSize = sizeof(tagBITMAPFILEHEADER) + sizeof(tagBITMAPINFOHEADER);

	size_t fileSize = GetFileSize(file);

	char* buffer = (char*)calloc(fileSize, sizeof(char));

	if (!buffer)
	{
		puts("Не достаточно памяти.");
		fclose(file);
		return false;
	}

	if (fread(buffer, sizeof(char), fileSize, file) < bitmapMinSize)
	{
		puts("Файл повреждён.");
		free(buffer);
		fclose(file);
		return false;
	}

	fclose(file);

	tagBITMAPFILEHEADER fileHeader = {};

	memmove(&fileHeader, buffer, sizeof(fileHeader));

	tagBITMAPINFOHEADER infoHeader = {};

	const size_t headerSize = *(DWORD*)(buffer + sizeof(fileHeader));

	memmove(&infoHeader, buffer + sizeof(fileHeader), sizeof(infoHeader));

	bmp->width = infoHeader.biWidth;
	bmp->height = infoHeader.biHeight;

	if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32)
	{
		puts("Данная версия .bmp не поддерживается. Поддерживается только кодирование 24 и 32 бита на пиксель.");
		return false;
	}

	bmp->bytePerPixel = 4;

	bmp->dataSize = infoHeader.biWidth * infoHeader.biHeight * bmp->bytePerPixel;

	bmp->data = (char*)calloc(bmp->dataSize, sizeof(char));

	if (!bmp->data)
	{
		puts("Недостаточно памяти.");
		free(buffer);
		return false;
	}

	if (infoHeader.biBitCount == 32)
	{
		memmove(bmp->data, buffer + fileHeader.bfOffBits, bmp->dataSize);
	}
	else
	{
		char* srcData = buffer + fileHeader.bfOffBits;

		for (size_t dataIndex = 0; dataIndex < bmp->dataSize; dataIndex += 4)
		{
			*(DWORD*)(bmp->data + dataIndex) = *(DWORD*)srcData;
			bmp->data[dataIndex + 3] = 255;

			srcData += 3;
		}
	}

	free(buffer);

	return true;
}

void BmpImageDestructor(BmpImage* bmp)
{
	if (bmp)
		free(bmp->data);
}