#ifndef FUNCTION_H
#define FUNCTION_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "structs.h"

/*
Structs 
*/

void partialRealAverage(monitor * disc);

void partialImaginaryAverage(monitor * disc);

void partialPotency(monitor * disc);

void partialNoise(monitor * disc);

visibility_s * buildVisibility(char * readedData);

float distance(visibility_s * visibility);

void * consume(void * disc);

void * readData(int radio, int width, int flag, char * fp_source_name, monitor ** discs);




#endif