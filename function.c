#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

float partialRealAverage(visibility_s *buffer, int lenBuffer, int *quantityPReal, double realAverage)
{
	float newAverage = realAverage * (*quantityPReal);

	for(int i = 0; i < lenBuffer; i++)
		newAverage = newAverage + buffer[i]->r; 

	*quantityPReal = *quantityPReal + lenBuffer;
 	return newAverage/quantityPReal; 
}


float partialImaginaryAverage(visibility_s *buffer, int lenBuffer, int *quantityPImaginary, double imaginaryAverage)
{
	float newAverage = imaginaryAverage * (*quantityPImaginary);

	for(int i = 0; i < lenBuffer; i++)
		newAverage = newAverage + buffer[i]->i; 
	
	*quantityPImaginary = *quantityPImaginary + lenBuffer;
 	return newAverage/quantityPReal; 
}


float partialPotency(visibility_s *buffer, int lenBuffer, double potencyAverage)
{
	float newPotency = potencyAverage;

	for(int i = 0; i < lenBuffer; i++)
		newPotency = newPotency + sqr(pow(buffer[i]->r, 2) + pow(buffer[i]->i, 2));

	return newPotency;
}


float partialNoise(visibility_s *buffer, int lenBuffer, double noiseAverage)
{
	float newNoise = noiseAverage;

	for(int i = 0; i < lenBuffer; i++)
		newNoise = newNoise + buffer[i]->w;

	return newNoise;
}

