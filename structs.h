#ifndef STRUCTS
#define STRUCTS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

//Estructura que se utiliza para almacenar una visibilidad
typedef struct {
	float u;
	float v;
	float r;
	float i;
	float w;
	int status;
} visibility_s;

//Estructura que se utiliza almacenar, crear y representar un monitor
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