#include <assert.h>
#include <emmintrin.h>
#include <smmintrin.h>

#include "Mandelbrot.h"

#include "TXLib.h"

#include "FileIO.h"

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

const size_t SCREEN_WIDTH  = 800;
const size_t SCREEN_HEIGHT = 600;

typedef RGBQUAD video_mem_t[SCREEN_HEIGHT][SCREEN_WIDTH];

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

struct MPixel
{
	int x;
	int y;
};

struct MRect
{
	double minX;
	double maxX;

	double minY;
	double maxY;
};

static MRect screen =
{
	0,
	SCREEN_WIDTH,
	0,
	SCREEN_HEIGHT
};

static MPixel moved = { 0, 0 };

static int moveStepX = 8;
static int moveStepY = 8;

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

static bool ReadKeyboard();

static RGBQUAD BlendColors(RGBQUAD front, RGBQUAD back);

static void DrawTableAndRacket(video_mem_t* video_mem, const BmpImage* table, const BmpImage* racket);

static void DrawSSETableAndRacket(video_mem_t* video_mem, const BmpImage* table, const BmpImage* racket);

static void DrawAlphaBlending(video_mem_t* video_mem, const bool simple);

static inline void DrawSSEPixels(video_mem_t* video_mem, __m128i racketLo, __m128i tableLo,
								 const size_t yTable, const size_t xTable);

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

static bool ReadKeyboard()
{
	if (txGetAsyncKeyState(VK_ESCAPE))
		return false;

	if (txGetAsyncKeyState(VK_RIGHT))
		moved.x += moveStepX * (txGetAsyncKeyState(VK_SHIFT)? 2.5 : 1.0);

	if (txGetAsyncKeyState(VK_LEFT))
		moved.x -= moveStepX * (txGetAsyncKeyState(VK_SHIFT)? 2.5 : 1.0);

	if (txGetAsyncKeyState(VK_DOWN))
		moved.y -= moveStepY * (txGetAsyncKeyState(VK_SHIFT)? 2.5 : 1.0);

	if (txGetAsyncKeyState(VK_UP))
		moved.y += moveStepY * (txGetAsyncKeyState(VK_SHIFT)? 2.5 : 1.0);

	return true;
}

static RGBQUAD BlendColors(RGBQUAD front, RGBQUAD back)
{
	RGBQUAD color =
	{
		back.rgbBlue  * (255 - front.rgbReserved) / 255 + front.rgbBlue  * front.rgbReserved / 255,
		back.rgbGreen * (255 - front.rgbReserved) / 255 + front.rgbGreen * front.rgbReserved / 255,
		back.rgbRed   * (255 - front.rgbReserved) / 255 + front.rgbRed   * front.rgbReserved / 255,
		255
	};

	return color;
}

// DrawTable(); DrawRacket();
/*
static void DrawTable(video_mem_t* video_mem, BmpImage* table)
{
	assert(video_mem);
	assert(table);

	size_t maxWidth  = (SCREEN_WIDTH >= table->width)   ? table->width  : SCREEN_WIDTH;
	size_t maxHeight = (SCREEN_HEIGHT >= table->height) ? table->height : SCREEN_HEIGHT;

	for (size_t yIndex = 0; yIndex < maxHeight; yIndex++)
	{
		for (size_t xIndex = 0; xIndex < maxWidth; xIndex++)
		{
			char* pixel = table->data + (table->width * yIndex + xIndex) * table->bytePerPixel;

			RGBQUAD color = *(RGBQUAD*)pixel;

			color.rgbReserved = 255;

			(*video_mem)[yIndex][xIndex] = color;
		}
	}
}

static void DrawRacket(video_mem_t* video_mem, BmpImage* table, BmpImage* racket)
{
	assert(video_mem);
	assert(table);
	assert(racket);

	size_t maxWidth  = (SCREEN_WIDTH >= table->width)   ? table->width  : SCREEN_WIDTH;
	size_t maxHeight = (SCREEN_HEIGHT >= table->height) ? table->height : SCREEN_HEIGHT;

	size_t racketX0 = moved.x;
	size_t racketY0 = moved.y;


	for (size_t yTable = racketY0, yRacket = 0;
		 yTable < maxHeight && yRacket < racket->height;
		 yTable++, yRacket++)
	{

		for (size_t xTable = racketX0, xRacket = 0;
			 xTable < maxWidth && xRacket < racket->width;
			 xTable++, xRacket++)
		{
			char* tablePixel = table->data + (table->width * yTable + xTable) * table->bytePerPixel;
			char* racketPixel = racket->data + (racket->width * yRacket + xRacket) * racket->bytePerPixel;

			RGBQUAD tableColor = *(RGBQUAD*)tablePixel;
			tableColor.rgbReserved = 255;

			RGBQUAD racketColor = *(RGBQUAD*)racketPixel;

			(*video_mem)[yTable][xTable] = BlendColors(racketColor, tableColor);
		}
	}
}*/

static void DrawTableAndRacket(video_mem_t* video_mem, const BmpImage* table, const BmpImage* racket)
{
	assert(video_mem);
	assert(table);
	assert(racket);

	const size_t maxWidth  = (SCREEN_WIDTH  >= table->width)  ? table->width  : SCREEN_WIDTH;
	const size_t maxHeight = (SCREEN_HEIGHT >= table->height) ? table->height : SCREEN_HEIGHT;

	const int racketX0 = moved.x;
	const int racketY0 = moved.y;

	for (size_t yTable = 0;
		 yTable < maxHeight;
		 yTable++)
	{
		for (size_t xTable = 0;
			 xTable < maxWidth; xTable++)
		{
			if ((int)xTable >= racketX0 && (int)xTable < racketX0 + (int)racket->width &&
				(int)yTable >= racketY0 && (int)yTable < racketY0 + (int)racket->height)
			{
				size_t yRacket = yTable - racketY0;
				size_t xRacket = xTable - racketX0;

				char* tablePixel  = table->data  + (table->width  * yTable  + xTable)  * table->bytePerPixel;
				char* racketPixel = racket->data + (racket->width * yRacket + xRacket) * racket->bytePerPixel;

				RGBQUAD tableColor = *(RGBQUAD*)tablePixel;
				tableColor.rgbReserved = 255;

				RGBQUAD racketColor = *(RGBQUAD*)racketPixel;

				(*video_mem)[yTable][xTable] = BlendColors(racketColor, tableColor);
			}
			else
			{
				char* pixel = table->data + (table->width * yTable + xTable) * table->bytePerPixel;

				RGBQUAD color = *(RGBQUAD*)pixel;

				color.rgbReserved = 255;

				(*video_mem)[yTable][xTable] = color;
			}
		}
	}
}

static void DrawSSETableAndRacket(video_mem_t* video_mem, const BmpImage* table, const BmpImage* racket)
{
	assert(video_mem);
	assert(table);
	assert(racket);

	const size_t maxWidth  = (SCREEN_WIDTH  >= table->width)  ? table->width  : SCREEN_WIDTH;
	const size_t maxHeight = (SCREEN_HEIGHT >= table->height) ? table->height : SCREEN_HEIGHT;

	const int racketX0 = moved.x;
	const int racketY0 = moved.y;

	for (size_t yTable = 0;
		 yTable < maxHeight;
		 yTable++)
	{
		for (size_t xTable = 0;
			 xTable < maxWidth; xTable += 4)
		{
			// Если по оси y расположена накладываемая картинка
			if ((int)yTable >= racketY0 && (int)yTable < racketY0 + (int)racket->height)
			{
				// То если выборка пикселей пересекается с ней, рисуем её.
				// Иначе рисуем задний фон.
				if ((int)xTable + 4 > racketX0 && (int)xTable < racketX0 + (int)racket->width)
				{
					// Может быть отрицательным!
					const int yRacket = yTable - racketY0;
					const int xRacket = xTable - racketX0;

					char* tablePixel  = table->data  + 
						(table->width  * yTable  + xTable)  * table->bytePerPixel;

					char* racketPixel = racket->data + 
						((int)racket->width * yRacket + xRacket) * (int)racket->bytePerPixel;
					
					// Будет проинициализировано ниже в блоке if-else.
					__m128i tableLo  = _mm_load_si128((__m128i*)tablePixel);
					__m128i racketLo = _mm_load_si128((__m128i*)racketPixel);

					// Если выборка содержит и пиксели накладываемой картинки и задний фон.
					// Иначе только накладываемая картинка.
					if ((int)xTable < racketX0)
					{
						int val[4] = {};
						memmove(val, racketPixel, sizeof(int) * 4);

						size_t inserted = (size_t)(racketX0 - (int)xTable);

						for (size_t st = 0; st < inserted; st++)
							((int*)&racketLo)[st] = 0;
					}
					else if ((int)xTable + 4 > racketX0 + (int)racket->width)
					{
						size_t inserted = (size_t)((racketX0 + (int)racket->width) - (int)xTable);

						for (size_t st = inserted; st < 4; st++)
							((int*)&racketLo)[st] = 0;
					}

					const __m128i var0   = _mm_setzero_si128();
					const __m128i var255 = _mm_set_epi8(0, -1, 0, -1,
														0, -1, 0, -1,
														0, -1, 0, -1,
														0, -1, 0, -1);

					const char shuffleZero = (char)-1;

					//-----------------------------------------------------------------------
					//            15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
					// tableLo = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
					// 
					// tableHi = [-- -- -- -- | -- -- -- -- | a3 r3 g3 b3 | a2 r2 g2 b2]
					//-----------------------------------------------------------------------

					__m128i tableHi  = _mm_castps_si128(
						_mm_movehl_ps(_mm_castsi128_ps(var0), _mm_castsi128_ps(tableLo)));

					__m128i racketHi = _mm_castps_si128(
						_mm_movehl_ps(_mm_castsi128_ps(var0), _mm_castsi128_ps(racketLo)));

					//-----------------------------------------------------------------------
					//            15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
					// tableLo = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
					// 
					// tableLo = [-- a1 -- r1 | -- g1 -- b1 | -- a0 -- r0 | -- g0 -- b0]
					//-----------------------------------------------------------------------

					tableLo  = _mm_cvtepu8_epi16(tableLo);
					tableHi  = _mm_cvtepu8_epi16(tableHi);

					racketLo = _mm_cvtepu8_epi16(racketLo);
					racketHi = _mm_cvtepu8_epi16(racketHi);

					//-----------------------------------------------------------------------
					//          15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
					// table = [-- a1 -- r1 | -- g1 -- b1 | -- a0 -- r0 | -- g0 -- b0]
					// 
					// alpha = [-- a1 -- a1 | -- a1 -- a1 | -- a0 -- a0 | -- a0 -- a0]
					//-----------------------------------------------------------------------

					const __m128i shufflePattern = _mm_set_epi8(shuffleZero, 14, shuffleZero, 14,
																shuffleZero, 14, shuffleZero, 14,
																shuffleZero, 6, shuffleZero, 6,
																shuffleZero, 6, shuffleZero, 6);

					__m128i alphaLo = _mm_shuffle_epi8(racketLo, shufflePattern);
					__m128i alphaHi = _mm_shuffle_epi8(racketHi, shufflePattern);

					//-----------------------------------------------------------------------
					// racket.RGB *= racket.alpha
					//-----------------------------------------------------------------------

					racketLo = _mm_mullo_epi16(racketLo, alphaLo);
					racketHi = _mm_mullo_epi16(racketHi, alphaHi);

					//-----------------------------------------------------------------------
					// table.RGB *= 255 - racket.alpha
					//-----------------------------------------------------------------------

					tableLo = _mm_mullo_epi16(tableLo,
											  _mm_sub_epi8(var255, alphaLo));

					tableHi = _mm_mullo_epi16(tableHi,
											  _mm_sub_epi8(var255, alphaHi));

					//-----------------------------------------------------------------------
					// table.RGB = racket.RGB * racket.alpha + table.RGB * (255 - racket.alpha)
					//-----------------------------------------------------------------------

					tableLo = _mm_add_epi16(tableLo, racketLo);
					tableHi = _mm_add_epi16(tableHi, racketHi);

					//-----------------------------------------------------------------------
					//            15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
					// table   = [A1 a1 R1 r1 | G1 g1 B1 b1 | A0 a0 R0 r0 | G0 g0 B0 b0] 
					// 
					// tableHi = [-- -- -- -- | -- -- -- -- | A1 R1 G1 B1 | A0 R0 G0 B0]
					//-----------------------------------------------------------------------

					const __m128i movePattern = _mm_set_epi8(shuffleZero, shuffleZero, shuffleZero, shuffleZero,
															 shuffleZero, shuffleZero, shuffleZero, shuffleZero,
															 15, 13, 11, 9,
															 7, 5, 3, 1);

					tableLo = _mm_shuffle_epi8(tableLo, movePattern);
					tableHi = _mm_shuffle_epi8(tableHi, movePattern);

					__m128i color = _mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(tableLo),
																   _mm_castsi128_ps(tableHi)));

					_mm_storeu_si128((__m128i*)&(*video_mem)[yTable][xTable], color);

					continue;
				}
				// Иначе рисуем задний фон.
			}

			// Рисуем задний фон.
			char* pixel = table->data + (table->width * yTable + xTable) * table->bytePerPixel;

			_mm_storeu_si128((__m128i*)&((*video_mem)[yTable][xTable]), _mm_load_si128((__m128i*)pixel));
		}
	}
}

static void DrawAlphaBlending(video_mem_t* video_mem, const bool simple)
{
	assert(video_mem);

	BmpImage table  = {};
	BmpImage racket = {};
	
 	if (!ReadBitMap("Table.bmp", &table))
		return; 

	if (!ReadBitMap("Racket.bmp", &racket))
		return;

	const size_t maxWidth  = (SCREEN_WIDTH >= table.width)   ? table.width  : SCREEN_WIDTH;
	const size_t maxHeight = (SCREEN_HEIGHT >= table.height) ? table.height : SCREEN_HEIGHT;

	moved.x = (table.width  - racket.width)  / 2;
	moved.y = (table.height - racket.height) / 2;

	txGetFPS();

	while (true)
	{
		if (!ReadKeyboard())
		{
			BmpImageDestructor(&table);
			BmpImageDestructor(&racket);
			return;
		}

		//for (size_t st = 0; st < 1000; st++)
		{
			if (simple)
			{
				DrawTableAndRacket(video_mem, &table, &racket);
			}
			else
			{
				DrawSSETableAndRacket(video_mem, &table, &racket);
			}
		}

		printf("\r%.2lf", txGetFPS() * 1000);
		txUpdateWindow();
	}
}

void DrawAlphaBlending()
{
	txCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
	Win32::_fpreset();
	txBegin();

	video_mem_t* video_mem = (video_mem_t*)txVideoMemory();

	DrawAlphaBlending(video_mem, true);
}

void DrawSSEAlphaBlending()
{
	txCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
	Win32::_fpreset();
	txBegin();

	video_mem_t* video_mem = (video_mem_t*)txVideoMemory();

	DrawAlphaBlending(video_mem, false);
}

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\