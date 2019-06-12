#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "function.c"


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
    monitor * disc_consumer = (monitor *) disc;
    //El hijo se mantiene escuchando hasta que llega una visibilidad con solo ceros, la cual es la manera de salir del ciclo.
    
    do{
        pthread_mutex_lock(disc_consumer->mutex);
        partialRealAverage(disc_consumer);
        partialImaginaryAverage(disc_consumer);
        partialNoise(disc_consumer);
        partialPotency(disc_consumer);
        disc_consumer->in = 0;
        pthread_mutex_unlock(disc_consumer->mutex);

    }while(!(visibility->u == 0.f && visibility->v == 0.f && visibility->r == 0.f && visibility->i == 0.f && visibility->w == 0.f));

      
}

void * readData(void * pData)
{
    producerData * pData = (producerData*)data; 
    FILE* fp;
    char buf[1024];
    int i;
    float origin_distance;
    int radio = pData->radio;
    int width = pData->width;
    int flag = pData->flag;
    monitor ** discs = pData->discs;

    if ((fp = fopen(pData->fp_source_name, "r")) == NULL)
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
                pthread_mutex_lock(discs[i]->mutex);
                if (discs[i]->bufferSize == discs[i]->in)
                {
                    pthread_cond_wait(notfull,discs[i]->mutex);
                }
                discs[i]->buffer[discs[i]->in] = visibility;
                (discs[i]->in)++;
                pthread_mutex_unlock(discs[i]->mutex);
            }

            else if(i == radio -1 && radioList[i] <= origin_distance) //Este if es necesario para tomar al último hijo.
            { 
                pthread_mutex_lock(discs[i+1]->mutex);
                if (discs[i+1]->bufferSize == discs[i+1]->in)
                {
                    pthread_cond_wait(notfull,discs[i+1]->mutex);
                }
                discs[i+1]->buffer[discs[i]->in] = visibility;
                (discs[i]->in)++;
                pthread_mutex_unlock(discs[i+1]->mutex);                
                break;
            }

            i++;
        }

         /* End  childs */
    }

    fclose(fb);
}

int main(int argc, char *argv[])
{
    char a = 'a';
    pthread_t * discs;
    discs = malloc(sizeof(pthread_t)*3);
    for (int i = 0; i < 4; i++)
    {
        pthread_create(&discs[i], NULL, hola, &a);
        pthread_join(discs[i], NULL);
    }
    
    return 0;
}