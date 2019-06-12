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
    char * fp_source_name;
    int radio;
    int width;
    int flag;
    monitor ** discs; 
}producerData;

typedef struct
{
    visibility_s ** buffer;
    int in, out, bufferSize;
    double pReal, pImaginary, pPotency, pNoise;
    int quantityPReal, quantityPImaginary;
    pthread_cond_t notfull, notempty;
    pthread_mutex_t mutex;
} monitor;
