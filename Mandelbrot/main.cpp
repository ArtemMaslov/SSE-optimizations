#include <stdio.h>

#include "Mandelbrot.h"

#include "AlphaBlending.h"


int main(int argc, char* argv[])
{
	// DrawMandelbrot();
	// DrawSSEMandelbrot();
	// DrawFloatSSEMandelbrot();

	// DrawAlphaBlending();
	DrawSSEAlphaBlending();

	return 0;
}