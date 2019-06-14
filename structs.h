#ifndef STRUCTS
#define STRUCTS
//Estructura que se utiliza para almacenar una visibilidad

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>


typedef struct {
	float u;
	float v;
	float r;
	float i;
	float w;
	int status;
} visibility_s;

typedef struct
{
    visibility_s **buffer;
    int id;
    int in;
    int bufferSize;
    int quantityPReal;
    int quantityPImaginary;
    float pReal;
    float pImaginary;
    float pPotency;
    float pNoise;
    pthread_cond_t notfull_cond;
    pthread_cond_t full_cond;
    pthread_mutex_t notfull_mutex;
    pthread_mutex_t full_mutex;
    int quantityProcessed;
    int blocked;
} monitor;

#endif