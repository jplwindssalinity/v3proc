/* Header file for Yahya Beam Pattern */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "YahyaAntenna.h"
#include "sizes.h"

void fft2d(complex b[MaxArraySize][MaxArraySize], int n, int inv);
float antnna(float theta, float phi, float bw1, float bw2);
