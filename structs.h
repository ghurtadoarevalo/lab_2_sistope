#ifndef STRUCTS_H
#define STRUCTS_H
#include <pthread.h>

//Estructura que se utiliza para almacenar una visibilidad
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
    int in;
    int bufferSize;
    int quantityPReal;
    int quantityPImaginary;
    double pReal;
    double pImaginary;
    double pPotency;
    double pNoise;
    pthread_cond_t notfull_cond;
    pthread_cond_t full_cond;
    pthread_mutex_t mutex; //EM
    pthread_mutex_t notfull_mutex;
    pthread_mutex_t full_mutex;
    int end;

} monitor;



#endif