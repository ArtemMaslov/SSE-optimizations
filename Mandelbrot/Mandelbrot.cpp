#include <assert.h>
#include <emmintrin.h>

#include "Mandelbrot.h"

#include "TXLib.h"

const size_t MANDELBROT_WIDTH  = 900;
const size_t MANDELBROT_HEIGHT = 600;

typedef RGBQUAD video_mem_t[MANDELBROT_HEIGHT][MANDELBROT_WIDTH];

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

struct MPoint
{
	double x;
	double y;
};

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
	MANDELBROT_WIDTH,
	0,
	MANDELBROT_HEIGHT
};

static MRect map0 =
{
	-2,
	1,
	-1,
	1
};

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

static bool ReadKeyboard();

static double GetWidth(const MRect* rect);

static double GetHeight(const MRect* rect);

static void GetNextPoint(const MPoint* first, const MPoint* cur, MPoint* next);

static size_t CalcPoint(const MPoint* point, const double maxR, const size_t iterations);

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

static MPoint moved ={ 0, 0 };
static double scale = 1;

static double scaleStep = 1.05;
static double moveStepX = (GetWidth(&map0))  / 100.0;
static double moveStepY = (GetHeight(&map0)) / 100.0;

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

static bool ReadKeyboard()
{
	if (txGetAsyncKeyState(VK_ESCAPE))
		return false;

	if (txGetAsyncKeyState(VK_RIGHT))
		moved.x += moveStepX * (txGetAsyncKeyState(VK_SHIFT)? 3.0 : 1.0);

	if (txGetAsyncKeyState(VK_LEFT))
		moved.x -= moveStepX * (txGetAsyncKeyState(VK_SHIFT)? 3.0 : 1.0);

	if (txGetAsyncKeyState(VK_DOWN))
		moved.y += moveStepY * (txGetAsyncKeyState(VK_SHIFT)? 3.0 : 1.0);

	if (txGetAsyncKeyState(VK_UP))
		moved.y -= moveStepY * (txGetAsyncKeyState(VK_SHIFT)? 3.0 : 1.0);

	if (txGetAsyncKeyState(VK_ADD))
		scale /= scaleStep * (txGetAsyncKeyState(VK_SHIFT)? 1.2f : 1.0);

	if (txGetAsyncKeyState(VK_SUBTRACT))
		scale *= scaleStep * (txGetAsyncKeyState(VK_SHIFT)? 1.2f : 1.0);

	return true;
}

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

static double GetWidth(const MRect* rect)
{
	assert(rect);

	return rect->maxX - rect->minX;
}

static double GetHeight(const MRect* rect)
{
	assert(rect);

	return rect->maxY - rect->minY;
}

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\

static inline void GetNextPoint(const MPoint* first, const MPoint* cur, MPoint* next)
{
	assert(first);
	assert(cur);
	assert(next);

	double x = cur->x;
	double y = cur->y;

	next->x = x * x - y * y + first->x;
	next->y = 2 * x * y + first->y;
}

static inline size_t CalcPoint(const MPoint* point, const double maxR, const size_t iterations)
{
	assert(point);

	MPoint cur =
	{
		point->x,
		point->y
	};

	MPoint next = {0, 0};

	for (size_t st = 0; st < iterations; st++)
	{
		GetNextPoint(point, &cur, &next);

		if (next.x * next.x + next.y * next.y > maxR * maxR)
			return st;

		cur.x = next.x;
		cur.y = next.y;
	}

	return iterations - 1;
}

void DrawSimpleMandelbrot(video_mem_t* video_mem)
{
	assert(video_mem);

	const size_t imageHeight = GetHeight(&screen);
	const size_t imageWidth  = GetWidth(&screen);

	while(true)
	{
		if (!ReadKeyboard())
			return;

		for (size_t st = 0; st < 100; st++)
		{
			MRect map =
			{
				(map0.minX + moved.x) * scale,
				(map0.maxX + moved.x) * scale,

				(map0.minY + moved.y) * scale,
				(map0.maxY + moved.y) * scale
			};

			double xMapStep = GetWidth(&map)  / imageWidth;
			double yMapStep = GetHeight(&map) / imageHeight;

			MPoint point = { map.minX, map.maxY };

			for (size_t yIndex = 0; yIndex < imageHeight; yIndex++)
			{
				point.x = map.minX;

				for (size_t xIndex = 0; xIndex < imageWidth; xIndex++)
				{
					size_t iterNum = CalcPoint(&point, 100, 256);

					RGBQUAD color = RGBQUAD
					{
						(BYTE)(48  + 11.6341   * (iterNum)), // B
						(BYTE)(134 + 13.1257   * (iterNum)), // G
						(BYTE)(243 + 15.231257 * (iterNum))  // R
					};

					(*video_mem)[yIndex][xIndex] = color;

					point.x += xMapStep;
				}

				point.y -= yMapStep;
			}
		}

		printf("\r%.2lf", txGetFPS() * 100);
		txUpdateWindow();
	}
}

void DrawOptimizedMandelbrot(video_mem_t* video_mem)
{
	assert(video_mem);

	const size_t imageHeight = GetHeight(&screen);
	const size_t imageWidth  = GetWidth(&screen);

	while (true)
	{
		if (!ReadKeyboard())
			return;

		for (size_t st = 0; st < 100; st++)
		{
			double minX     = (map0.minX + moved.x) * scale;
			double maxX     = (map0.maxX + moved.x) * scale;

			double minY     = (map0.minY + moved.y) * scale;
			double maxY     = (map0.maxY + moved.y) * scale;

			double xMapStep = (maxX - minX) / imageWidth;
			double x2MapStep = 2 * xMapStep;
			double yMapStep = (maxY - minY) / imageHeight;

			__m128d pointX = _mm_setzero_pd();
			__m128d pointY = _mm_set_pd(maxY, maxY);
			__m128d maxR   = _mm_set_pd(100, 100);

			for (size_t yIndex = 0; yIndex < imageHeight; yIndex++)
			{
				pointX = _mm_set_pd(minX, minX + xMapStep);

				for (size_t xIndex = 0; xIndex < imageWidth; xIndex += 2)
				{
					__m128d curX    = pointX;
					__m128d curY    = pointY;

					__m128i iterNum = _mm_set_epi64x(0, 0);

					for (size_t st = 0; st < 255; st++)
					{
						__m128d nextX =
							_mm_add_pd(
								_mm_sub_pd(
									_mm_mul_pd(curX, curX),
									_mm_mul_pd(curY, curY)),
								pointX);

						__m128d nextY =
							_mm_add_pd(
								_mm_mul_pd(
									_mm_set_pd(2, 2),
									_mm_mul_pd(curX, curY)),
								pointY);

						__m128d r2 =
							_mm_add_pd(
								_mm_mul_pd(nextX, nextX),
								_mm_mul_pd(nextY, nextY));

						__m128d cmpRes = _mm_cmple_pd(r2, maxR);

						if (_mm_movemask_pd(cmpRes) == 0)
							break; // Обе точки ушли на бесконечность

						iterNum = _mm_sub_epi64(iterNum, _mm_castpd_si128(cmpRes));

						curX = nextX;
						curY = nextY;
					}

					long long* ptr_iterNum = (long long*)&iterNum;

					for (size_t st = 0; st < 2; st++)
					{
						BYTE iters = (BYTE)ptr_iterNum[1 - st];

						RGBQUAD color = RGBQUAD
						{
							(BYTE)(48  + 11.6341   * (iters)), // B
							(BYTE)(134 + 13.1257   * (iters)), // G
							(BYTE)(243 + 15.2312   * (iters))  // R
						};

						(*video_mem)[yIndex][xIndex + st] = color;
					}

					pointX = _mm_add_pd(pointX, _mm_set_pd(x2MapStep, x2MapStep));
				}

				pointY = _mm_sub_pd(pointY, _mm_set_pd(yMapStep, yMapStep));
			}
		}

		printf("\r%.2lf", txGetFPS() * 100);
		txUpdateWindow();
	}
}

void DrawOptimizedFloatMandelbrot(video_mem_t* video_mem)
{
	assert(video_mem);

	const size_t imageHeight = GetHeight(&screen);
	const size_t imageWidth  = GetWidth(&screen);

	while (true)
	{
		if (!ReadKeyboard())
			return;

		//for (size_t st = 0; st < 100; st++)
		{
			double minX     = (map0.minX + moved.x) * scale;
			double maxX     = (map0.maxX + moved.x) * scale;
			
			double minY     = (map0.minY + moved.y) * scale;
			double maxY     = (map0.maxY + moved.y) * scale;

			double xMapStep = (maxX - minX) / imageWidth;
			double x4MapStep = 4 * xMapStep;
			double yMapStep = (maxY - minY) / imageHeight;

			__m128 pointX = _mm_setzero_ps();
			__m128 pointY = _mm_set_ps1(maxY);
			__m128 maxR   = _mm_set_ps1(100);

			for (size_t yIndex = 0; yIndex < imageHeight; yIndex++)
			{
				pointX = _mm_set_ps(minX, minX + xMapStep, minX + 2 * xMapStep, minX + 3 * xMapStep);

				for (size_t xIndex = 0; xIndex < imageWidth; xIndex += 4)
				{
					__m128 curX     = pointX;
					__m128 curY     = pointY;

					__m128i iterNum = _mm_set1_epi32(0);

					for (size_t st = 0; st < 255; st++)
					{
						__m128 nextX =
							_mm_add_ps(
								_mm_sub_ps(
									_mm_mul_ps(curX, curX),
									_mm_mul_ps(curY, curY)),
								pointX);

						__m128 nextY =
							_mm_add_ps(
								_mm_mul_ps(
									_mm_set_ps1(2),
									_mm_mul_ps(curX, curY)),
								pointY);

						__m128 r2 =
							_mm_add_ps(
								_mm_mul_ps(nextX, nextX),
								_mm_mul_ps(nextY, nextY));

						__m128 cmpRes = _mm_cmple_ps(r2, maxR);

						if (_mm_movemask_ps(cmpRes) == 0)
							break; // Обе точки ушли на бесконечность

						iterNum = _mm_sub_epi32(iterNum, _mm_castps_si128(cmpRes));

						curX = nextX;
						curY = nextY;
					}

					int* ptr_iterNum = (int*)&iterNum;

					for (size_t st = 0; st < 4; st++)
					{
						BYTE iters = (BYTE)ptr_iterNum[3 - st];

						RGBQUAD color = RGBQUAD
						{
							(BYTE)(48  + 11.6341   * (iters)), // B
							(BYTE)(134 + 13.1257   * (iters)), // G
							(BYTE)(243 + 15.2312   * (iters))  // R
						};

						(*video_mem)[yIndex][xIndex + st] = color;
					}

					pointX = _mm_add_ps(pointX, _mm_set_ps1(x4MapStep));
				}

				pointY = _mm_sub_ps(pointY, _mm_set_ps1(yMapStep));
			}
		}

		printf("\r%.2lf", txGetFPS() * 100);
		txUpdateWindow();
	}
}

void DrawFloatSSEMandelbrot()
{
	txCreateWindow(MANDELBROT_WIDTH, MANDELBROT_HEIGHT);
	Win32::_fpreset();
	txBegin();

	video_mem_t* video_mem = (video_mem_t*)txVideoMemory();

	DrawOptimizedFloatMandelbrot(video_mem);
}

void DrawSSEMandelbrot()
{
	txCreateWindow(MANDELBROT_WIDTH, MANDELBROT_HEIGHT);
	Win32::_fpreset();
	txBegin();

	video_mem_t* video_mem = (video_mem_t*)txVideoMemory();

	DrawOptimizedMandelbrot(video_mem);
}

void DrawMandelbrot()
{
	txCreateWindow(MANDELBROT_WIDTH, MANDELBROT_HEIGHT);
	Win32::_fpreset();
	txBegin();

	video_mem_t* video_mem = (video_mem_t*)txVideoMemory();

	DrawSimpleMandelbrot(video_mem);
}

///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\
///***///***///---\\\***\\\***\\\___///***___***\\\___///***///***///---\\\***\\\***\\\