#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "function.h"
#include "structs.h"

void partialRealAverage(monitor * disc)
{
    int lenBuffer = disc->bufferSize;
    int quantityPReal = disc->quantityPReal;
    int realAverage = disc->pReal;

	float newAverage = realAverage * (*quantityPReal);

	for(int i = 0; i < lenBuffer; i++)
		newAverage = newAverage + disc->buffer[i]->r; 

	disc->quantityPReal = quantityPReal + lenBuffer;
    disc->pReal = newAverage/quantityPReal;
}

void partialImaginaryAverage(monitor * disc)
{
    int lenBuffer = disc->bufferSize;
    int quantityPImaginaty = disc->quantityPImaginary;
    int imaginaryAverage = disc->pImaginary;

	float newAverage = imaginaryAverage * (*quantityPImaginary);

	for(int i = 0; i < lenBuffer; i++)
		newAverage = newAverage + disc->buffer[i]->i; 
	
    disc->quantityPImaginary = quantityPImaginary + lenBuffer;
    disc->pImaginary = newAverage/quantityPImaginary;
}

void partialPotency(monitor * disc)
{
    
	float newPotency = disc->pPotency;
    int lenBuffer = disc->bufferSize;
	for(int i = 0; i < lenBuffer; i++)
		newPotency = newPotency + sqr(pow(disc->buffer[i]->r, 2) + pow(disc->buffer[i]->i, 2));
    disc->pPotency = newPotency;
}


void partialNoise(monitor * disc)
{
	float newNoise = disc->pPotency;
    int lenBuffer = disc->bufferSize;

	for(int i = 0; i < lenBuffer; i++)
		newNoise = newNoise + disc->buffer[i]->w;
    disc->pNoise = newNoise;
}

