#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

int end = 0;


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
    pthread_mutex_t mutex; //EM
    pthread_mutex_t notfull_mutex;
    pthread_mutex_t full_mutex;
    int end;

} monitor;

void partialRealAverage(monitor * disc);

void partialImaginaryAverage(monitor * disc);

void partialPotency(monitor * disc);

void partialNoise(monitor * disc);

visibility_s * buildVisibility(char * readedData);

float distance(visibility_s * visibility);

void * consume(void * disc);

void * readData(int radio, int width, int flag, char * nameFileIn, monitor ** discs);

void partialRealAverage(monitor * disc)
{
    if (disc->in != 0)
    {
        float newAverage = disc->pReal * (disc->quantityPReal);

	    for(int i = 0; i < disc->in; i++)
		    newAverage = newAverage + disc->buffer[i]->r; 

	    disc->quantityPReal += disc->in;
        disc->pReal = newAverage/disc->quantityPReal;
    }
    
}

void partialImaginaryAverage(monitor * disc)
{
    if (disc->in != 0)
	{
        float newAverage = disc->pImaginary * (disc->quantityPImaginary);

	    for(int i = 0; i < disc->in; i++)
		    newAverage = newAverage + disc->buffer[i]->i; 
	
        disc->quantityPImaginary += disc->in;
        disc->pImaginary = newAverage/disc->quantityPImaginary;
    }
}

void partialPotency(monitor * disc)
{

	for(int i = 0; i < disc->in; i++)
		disc->pPotency = disc->pPotency + sqrt(pow(disc->buffer[i]->r, 2) + pow(disc->buffer[i]->i, 2));
}


void partialNoise(monitor * disc)
{
	for(int i = 0; i < disc->in; i++)
		disc->pPotency = disc->pPotency + disc->buffer[i]->w;
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
    
    printf("Soy monitor ID: %d\n", disc_consumer->id);

    do{
        pthread_mutex_lock(&(disc_consumer->mutex));
        //printf("Soy disc con in: %d\n", disc_consumer->in);
        if (disc_consumer->in != disc_consumer->bufferSize)
        {
            pthread_mutex_unlock(&(disc_consumer->mutex));
            pthread_cond_wait(&(disc_consumer->notfull_cond), &(disc_consumer->notfull_mutex));
        }

        if (end == 0)
        {
            partialRealAverage(disc_consumer);
            partialImaginaryAverage(disc_consumer);
            partialNoise(disc_consumer);
            partialPotency(disc_consumer);

            disc_consumer->in = 0;
            pthread_cond_signal(&(disc_consumer->full_cond));
            pthread_mutex_unlock(&(disc_consumer->mutex));

            printf("preal: %f\n",disc_consumer->pReal);
            printf("pimaginary: %f\n",disc_consumer->pImaginary);
            printf("pnoise: %f\n",disc_consumer->pNoise);
            printf("ppotency: %f\n",disc_consumer->pPotency);
        }

        else
        {
            partialRealAverage(disc_consumer);
            partialImaginaryAverage(disc_consumer);
            partialNoise(disc_consumer);
            partialPotency(disc_consumer);
            
            break;
        }

    }while(!(end == 0));
}

void * readData(int radio, int width, int flag, char * nameFileIn, monitor ** discs)
{
    FILE* fp;
    char buf[1024];
    int i;
    float origin_distance;

    if ((fp = fopen(nameFileIn, "r")) == NULL)
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
        //printf("Entré a lock de productor?\n %f", visibility->u);
        printf("Entré a lock de productor?\n %f\n", origin_distance);

        i = 0;

        while(i < radio)
        {
            if(radioList[i] <= origin_distance && origin_distance < radioList[i+1])
            {
                printf("Radio: %d\n",i);
                pthread_mutex_lock(&(discs[i]->mutex));
                if (discs[i]->bufferSize == discs[i]->in)
                {
                    printf("Entreeeeeeeeeeeee\n");
                    pthread_mutex_unlock(&(discs[i]->mutex));
                    pthread_cond_signal(&(discs[i]->notfull_cond));
                    pthread_cond_wait(&(discs[i]->full_cond),&(discs[i]->full_mutex));
                }
                discs[i]->buffer[discs[i]->in] = visibility;
                discs[i]->in = discs[i]->in + 1;
                pthread_mutex_unlock(&(discs[i]->mutex));
            }

            else if(i == radio -1 && radioList[i] <= origin_distance) //Este if es necesario para tomar al último hijo.
            { 
                printf("Radio: %d\n",i+1);
                pthread_mutex_lock(&(discs[i+1]->mutex));
                if (discs[i+1]->bufferSize == discs[i+1]->in)
                {
                    pthread_mutex_unlock(&(discs[i+1]->mutex));
                    pthread_cond_signal(&(discs[i+1]->notfull_cond))    ;
                    pthread_cond_wait(&(discs[i+1]->full_cond),&(discs[i+1]->full_mutex));
                }
                discs[i+1]->buffer[discs[i+1]->in] = visibility;
                discs[i+1]->in = discs[i+1]->in + 1;
                pthread_mutex_unlock(&(discs[i+1]->mutex));           
                break;
            }
            i++;
        }


         /* End  childs */
    }

    fclose(fp);
}

monitor **initializeMonitors(int radio, int width, int flag, int bufferSize, char *nameFileIn)
{
    monitor ** monitors = malloc(sizeof(monitor)*(radio+1));
    monitor * mon;

    for (int i = 0; i < (radio+1); i++)
    {
        mon = malloc(sizeof(monitor));
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
        mon->id = i;
        pthread_cond_init(&mon->notfull_cond, NULL);
        pthread_cond_init(&mon->full_cond, NULL);
        pthread_mutex_init(&mon->mutex, NULL);
        pthread_mutex_init(&mon->notfull_mutex, NULL);
        pthread_mutex_init(&mon->full_mutex, NULL);
        monitors[i] = mon;
    }

    return monitors;
}




int main(int argc, char *argv[])
{    
    int otp = 0, radio = 0, width = 0, flag = 0, bufferSize = 0;
    char *nameFileIn = NULL, *nameFileOut = NULL;

    while((otp = getopt(argc, argv, ":i:o:n:d:s:b")) != -1)
    {
        if(otp == 'i') nameFileIn = optarg;
        else if(otp == 'o') nameFileOut = optarg;
        else if(otp == 'n') radio = atoi(optarg)-1;
        else if(otp == 'd') width = atoi(optarg);
        else if(otp == 's') bufferSize = 1;
        else if(otp == 'b') flag = 1;
        else
        {
            perror("Invalid Sintaxis");
            exit(-1);
        }
    }

    if((otp == -1 && argc == 1) || radio <= 0 || width <= 0 || bufferSize <= 0)
    {
       perror("Invalid Sintaxis");
        exit(-1); 
    }

    monitor ** monitors = initializeMonitors(radio, width, flag, bufferSize, nameFileIn);
    
    pthread_t * disc_threads;
    disc_threads = malloc(sizeof(pthread_t)*(radio+1));
    for (int i = 0; i < (radio+1); i++)
    {
        pthread_create(&disc_threads[i], NULL, consume, (void *)monitors[i]);
    }

    readData(radio,width,flag, nameFileIn, monitors);
    end = 1;
    for (int i = 0; i < (radio + 1); i++)
    {
        pthread_cond_signal(&(monitors[i]->notfull_cond));
        pthread_join(disc_threads[i], NULL);
    }

    return 0;
}