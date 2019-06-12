#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "function.h"

void partialRealAverage(monitor * disc)
{
    int lenBuffer = disc->bufferSize;
    int quantityPReal = disc->quantityPReal;
    int realAverage = disc->pReal;

	float newAverage = realAverage * (quantityPReal);

	for(int i = 0; i < lenBuffer; i++)
		newAverage = newAverage + disc->buffer[i]->r; 

	disc->quantityPReal = quantityPReal + lenBuffer;
    disc->pReal = newAverage/quantityPReal;
}

void partialImaginaryAverage(monitor * disc)
{
    int lenBuffer = disc->bufferSize;
    int quantityPImaginary = disc->quantityPImaginary;
    int imaginaryAverage = disc->pImaginary;

	float newAverage = imaginaryAverage * (quantityPImaginary);

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
		newPotency = newPotency + sqrt(pow(disc->buffer[i]->r, 2) + pow(disc->buffer[i]->i, 2));
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

//Función que tomando una línea del archivo .csv genera una estructura llamada visiblidad
//la cual contiene U, V , R, I y W, además un status para saber si fue leido o no por algún hijo.
//Entrada: String que contiene U,V,R,I y W separados por comas.
//Salida: Estructura visiblidad.
visibility_s * buildVisibility(char * readedData)
{
    visibility_s * visibility = malloc(sizeof(visibility_s));
    char * temp = malloc(sizeof(char)*(1024));
    strcpy(temp,readedData);
    char* token = NULL;
    token = strtok(temp, ",");
    visibility->u = atof(token);
    token = strtok(NULL, ",");
    visibility->v = atof(token);
    token = strtok(NULL, ",");
    visibility->r = atof(token);
    token = strtok(NULL, ",");
    visibility->i = atof(token);
    token = strtok(NULL, ",");
    visibility->w = atof(token);
    visibility->status = 1;

    return visibility;
}

//Función que calcula una distancia (norma) utilizando U y V del vector.
//Entrada: Estructura visibilidad.
//Salida: La distancia calculada como un flotante.
float distance(visibility_s * visibility)
{
    float distance = 0;
    distance = sqrt(pow(visibility->u,2) + pow(visibility->v,2));
    return distance; 
}

void * consume(void * disc)
{
    monitor * disc_consumer = (monitor *)disc;
    //El hijo se mantiene escuchando hasta que llega una visibilidad con solo ceros, la cual es la manera de salir del ciclo.
    
    do{
        pthread_mutex_lock(&(disc_consumer->mutex));
        if (disc_consumer->in != disc_consumer->bufferSize)
        {
            pthread_mutex_unlock(&(disc_consumer->mutex));
            pthread_cond_wait(&(disc_consumer->notfull_cond), &(disc_consumer->notfull_mutex));
        }
        partialRealAverage(disc_consumer);
        partialImaginaryAverage(disc_consumer);
        partialNoise(disc_consumer);
        partialPotency(disc_consumer);
        disc_consumer->in = 0;
        pthread_cond_signal(&(disc_consumer->full_cond));
        pthread_mutex_unlock(&(disc_consumer->mutex));

    }while(!(disc_consumer->end == 0));

      
}

void * readData(int radio, int width, int flag, char * fp_source_name, monitor ** discs)
{
    FILE* fp;
    char buf[1024];
    int i;
    float origin_distance;

    if ((fp = fopen(fp_source_name, "r")) == NULL)
    { 
        perror("fopen source-file");
        exit(-1);
    }

    int * radioList = malloc(sizeof(int)*radio+1); // radioList = [0, R1, R2, R3,..., Rn] -> n = numero de radios

    for (int i = 0; i < radio+1; ++i)
        radioList[i] = i*width;
   
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {   
        visibility_s * visibility = malloc(sizeof(visibility_s)); 
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        visibility = buildVisibility(buf);

        origin_distance = distance(visibility);
        i = 0;

        while(i < radio)
        {
            if(radioList[i] <= origin_distance && origin_distance < radioList[i+1])
            {
                pthread_mutex_lock(&(discs[i]->mutex));
                if (discs[i]->bufferSize == discs[i]->in)
                {
                    printf("Entré a lock de productor?");
                    pthread_mutex_unlock(&(discs[i]->mutex));
                    pthread_cond_signal(&(discs[i]->notfull_cond));
                    pthread_cond_wait(&(discs[i]->full_cond),&(discs[i]->full_mutex));
                }
                discs[i]->buffer[discs[i]->in] = visibility;
                (discs[i]->in)++;
                pthread_mutex_unlock(&(discs[i]->mutex));
            }

            else if(i == radio -1 && radioList[i] <= origin_distance) //Este if es necesario para tomar al último hijo.
            { 
                pthread_mutex_lock(&(discs[i+1]->mutex));
                if (discs[i+1]->bufferSize == discs[i+1]->in)
                {
                    pthread_mutex_unlock(&(discs[i+1]->mutex));
                    pthread_cond_signal(&(discs[i+1]->notfull_cond))    ;
                    pthread_cond_wait(&(discs[i+1]->full_cond),&(discs[i+1]->full_mutex));
                }
                discs[i+1]->buffer[discs[i+1]->in] = visibility;
                (discs[i+1]->in)++;
                pthread_mutex_unlock(&(discs[i+1]->mutex));           
                break;
            }
            i++;
        }


         /* End  childs */
    }

    fclose(fp);
}