#include "function.h"

int main(int argc, char *argv[])
{
    printf("holaaa");   
    int radio = 3, width = 100, flag, bufferSize = 2;
    char* fp_source_name = "text.csv";
    monitor ** monitors = malloc(sizeof(monitor)*(radio+1));

    printf("holaaa");   


    for (int i = 0; i <= radio; i++)
    {
        monitor * mon = malloc(sizeof(monitor));
        mon->buffer = malloc(sizeof(visibility_s)*bufferSize);
        mon->in = 0;
        mon->bufferSize = bufferSize;
        mon->quantityPImaginary = 0;
        mon->quantityPReal = 0;
        mon->pNoise = 0;
        mon->pPotency = 0;
        mon->pReal = 0;
        mon->pImaginary = 0;
        mon->end = 0;
        pthread_cond_init(&mon->notfull_cond, NULL);
        pthread_cond_init(&mon->full_cond, NULL);
        pthread_mutex_init(&mon->mutex, NULL);
        pthread_mutex_init(&mon->notfull_mutex, NULL);
        pthread_mutex_init(&mon->full_mutex, NULL);
        monitors[i] = mon;
    }

    printf("holaaa");   

    pthread_t * disc_threads;
    disc_threads = malloc(sizeof(pthread_t)*(radio+1));
    for (int i = 0; i < (radio+1); i++)
    {
        pthread_create(&disc_threads[i], NULL, consume, (void *)monitors[i]);
    }

    readData(radio,width,flag, fp_source_name, monitors);

    for (size_t i = 0; i < (radio + 1); i++)
    {
        monitors[i]->end = 1;
        pthread_join(disc_threads[i], NULL);
    }
    
    return 0;
}