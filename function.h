#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

/*
Structs 
*/




float partialRealAverage(visibility_s *buffer, int lenBuffer, int *quantityPReal, double realAverage);

float partialImaginaryAverage(visibility_s *buffer, int lenBuffer, int *quantityPImaginary, double imaginaryAverage);

float partialPotency(visibility_s *buffer, int lenBuffer, double potencyAverage);

float partialNoise(visibility_s *buffer, int lenBuffer, double noiseAverage);

