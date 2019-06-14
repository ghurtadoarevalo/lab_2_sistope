#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

int end = 0;
float ** discs_properties;
int flag;
pthread_mutex_t mutex; //EM

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
    //printf("\n\nEntró ID: %d en partialRealAverage con disc->in: %d\n\n", disc->id, disc->in);
    if (disc->in != 0)
    {
        float newAverage = disc->pReal * (disc->quantityPReal);

	    for(int i = 0; i < disc->in; i++)
        {
            //printf("Estoy procesando disc->buffer[i]->r: %f\n",disc->buffer[i]->r );
		    newAverage = newAverage + disc->buffer[i]->r; 
            //printf("Soy newAverage: %f\n", newAverage);
        }
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
	for(int i = 0; i < disc->in; i++){
		disc->pPotency =  sqrt(pow(disc->pPotency,2) + pow(disc->buffer[i]->r, 2) + pow(disc->buffer[i]->i, 2));
    }
}


void partialNoise(monitor * disc)
{
	for(int i = 0; i < disc->in; i++)
		disc->pNoise = disc->pNoise + disc->buffer[i]->w;
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
    pthread_mutex_lock(&(mutex));
    monitor * disc_consumer = (monitor *)disc;
    //El hijo se mantiene escuchando hasta que llega una visibilidad con solo ceros, la cual es la manera de salir del ciclo.
    
    //printf("Soy monitor ID: %d\n", disc_consumer->id);
    //printf("Soy disc con id: %d, con in: %d y bufferSize: %d\n",disc_consumer->id, disc_consumer->in, disc_consumer->bufferSize);

    do{

        if(disc_consumer->in != disc_consumer->bufferSize)
        {
            //printf("a1\n\n");
            disc_consumer->blocked = 1;
            pthread_mutex_unlock(&(mutex));
            //printf("a2\n\n");
            pthread_cond_wait(&(disc_consumer->notfull_cond), &(disc_consumer->notfull_mutex));
            //printf("a3\n\n");

            disc_consumer->blocked = 0;
        }

        //printf("a4\n\n");


        if(end == 1)
        {
            disc_consumer->quantityProcessed += disc_consumer->in;
            partialRealAverage(disc_consumer);
            partialImaginaryAverage(disc_consumer);
            partialNoise(disc_consumer);
            partialPotency(disc_consumer);
            
            //printf("\n\nSoy disc con id: %d y calculé: \n",disc_consumer->id);
            //printf("preal: %f\n",disc_consumer->pReal);
            //printf("pimaginary: %f\n",disc_consumer->pImaginary);
            //printf("pnoise: %f\n",disc_consumer->pNoise);
            //printf("ppotency: %f\n",disc_consumer->pPotency);


            //printf("\n\nSoy disc ID: %d, terminando\n", disc_consumer->id);
            
            float * properties = malloc(sizeof(float)*4);
            properties[0] = disc_consumer->pReal;
            properties[1] = disc_consumer->pImaginary;
            properties[2] = disc_consumer->pPotency;
            properties[3] = disc_consumer->pNoise;
            discs_properties[disc_consumer->id] = properties;

            if (flag == 1)
            {
                printf("Soy el hijo de pid %d, procese %d visibilidades\n", disc_consumer->id, disc_consumer->quantityProcessed);
            }

            break;
        }

            //printf("Soy disc con id: %d y consumí\n",disc_consumer->id);


            disc_consumer->quantityProcessed += disc_consumer->in;
            partialRealAverage(disc_consumer);
            partialImaginaryAverage(disc_consumer);
            partialNoise(disc_consumer);
            partialPotency(disc_consumer);
            //printf("\n\nAAAAAAAAAAAAAAn ID: %d\n", disc_consumer->id);

            disc_consumer->in = 0;

            //printf("\n\nSoy disc con id: %d y calculé: \n",disc_consumer->id);
            //printf("preal: %f\n",disc_consumer->pReal);
            //printf("pimaginary: %f\n",disc_consumer->pImaginary);
            //printf("pnoise: %f\n",disc_consumer->pNoise);
            //printf("ppotency: %f\n",disc_consumer->pPotency);
        

            pthread_cond_signal(&(disc_consumer->full_cond));
            pthread_mutex_unlock(&(mutex));
    }while(end == 0);


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

    int * radioList = malloc(sizeof(int)*(radio + 1)); // radioList = [0, R1, R2, R3,..., Rn] -> n = numero de radios

    for (int i = 0; i <= radio; ++i)
        radioList[i] = i*width;
   
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {   
        pthread_mutex_lock(&(mutex));
        visibility_s * visibility = malloc(sizeof(visibility_s)); 
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        visibility = buildVisibility(buf);

        origin_distance = distance(visibility);
        //printf("Entré a lock de productor? %f\n", visibility->u);
        i = 0;
        while(i < radio)
        {

            if(radioList[i] <= origin_distance && origin_distance < radioList[i+1])
            {
                //printf("Produje para el disco: %d\n",i);
                if ((discs[i]->bufferSize - 1) == discs[i]->in)
                {
                    //printf("b1\n\n");
                    pthread_mutex_unlock(&(mutex));
                    //printf("b2\n\n");
                    pthread_cond_signal(&(discs[i]->notfull_cond));
                    //printf("b3\n\n");
                    pthread_cond_wait(&(discs[i]->full_cond),&(discs[i]->full_mutex));
                    //printf("b4\n\n");
                }
                printf("******** disc[%d]->in = %d ********\n", i, discs[i]->in);
                discs[i]->buffer[discs[i]->in] = visibility;
                discs[i]->in += 1;
            }

            else if(i == (radio - 1) && radioList[i] <= origin_distance) //Este if es necesario para tomar al último hijo.
            {   
                //printf("Produje con origin_distance: %f, para el disco: %d\n", origin_distance, i+1);
                if ((discs[i+1]->bufferSize - 1) == discs[i+1]->in)
                {
                    pthread_mutex_unlock(&(mutex));
                    pthread_cond_signal(&(discs[i+1]->notfull_cond))    ;
                    pthread_cond_wait(&(discs[i+1]->full_cond),&(discs[i+1]->full_mutex));
                }
                discs[i+1]->buffer[discs[i+1]->in] = visibility;
                //printf("Soy visibility->u: %f, visibility->v: %f\n\n", visibility->u, visibility->v);
                //printf("In: %d\n", discs[i+1]->in );
                discs[i+1]->in += 1;
                //printf("\n\naaaaaaaaaaaaaaaaaaAAAAAAAAAAAAAAAAAa: %d\n\n",discs[i+1]->in);
                break;
            }
            i++;
        }

        pthread_mutex_unlock(&(mutex));           
    }

    fclose(fp);
}

//Funcion que crea un archivo con el nombre del string entrante.
//Entrada: Nombre del archivo de salida.
//Salida: Ninguna.
void createOutFile(char *outFileName)
{
    FILE *file = fopen(outFileName, "w");
    fclose(file);
}

//Función que escribe los resultados de los discos en el archivo de salida.
//Entrada: numero de disco, lista con los resultados del disco, nombre del archivo de salida.
//Salida: Ninguna.
void writeData(int number, float *results, char *outFileName)
{
	FILE *file = fopen(outFileName, "a");
	
	fprintf(file, "Disco %d:\nMedia real: %f\nMedia imaginaria: %f\nPotencia: %f\nRuido total: %f\n",
		number, results[0], results[1], results[2], results[3]);

	fclose(file);
}

monitor **initializeMonitors(int radio, int width, int flag, int bufferSize, char *nameFileIn)
{
    monitor ** monitors = malloc(sizeof(monitor)*(radio + 1));
    monitor * mon;

    for (int i = 0; i <= radio; i++)
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
        mon->id = i;
        pthread_cond_init(&mon->notfull_cond, NULL);
        pthread_cond_init(&mon->full_cond, NULL);
        pthread_mutex_init(&mon->notfull_mutex, NULL);
        pthread_mutex_init(&mon->full_mutex, NULL);
        monitors[i] = mon;
    }

    return monitors;
}


int main(int argc, char *argv[])
{    
    int otp = 0, radio = 0, width = 0, bufferSize = 0;
    flag = 0;
    char *nameFileIn = NULL, *nameFileOut = NULL;

    while((otp = getopt(argc, argv, ":i:o:n:d:s:b")) != -1)
    {
        if(otp == 'i') nameFileIn = optarg;
        else if(otp == 'o') nameFileOut = optarg;
        else if(otp == 'n') radio = atoi(optarg);
        else if(otp == 'd') width = atoi(optarg);
        else if(otp == 's') bufferSize = atoi(optarg);
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

    pthread_mutex_init(&mutex, NULL);
    monitor ** monitors = initializeMonitors(radio, width, flag, bufferSize, nameFileIn);
    discs_properties = malloc(sizeof(float*)*(radio + 1));

    pthread_t * disc_threads;
    disc_threads = malloc(sizeof(pthread_t)*(radio + 1));
    for (int i = 0; i <= radio; i++)
    {
        pthread_create(&disc_threads[i], NULL, consume, (void *)monitors[i]);
    }

    readData(radio,width,flag, nameFileIn, monitors);
    end = 1;
    printf("terminé\n");
    exit(1);
    int blockeds;
    while(1)
    {
        blockeds = 0;
        for (int i = 0; i <= radio; i++)
        {

            if (monitors[i]->blocked == 1)
            {
                blockeds++;
            }
        }
            
        if (blockeds == (radio + 1))
        {
            for (int i = 0; i <= radio; i++)
            {
                pthread_cond_signal(&(monitors[i]->notfull_cond));
                pthread_join(disc_threads[i], NULL);
            }
            break;
        }
    }

    createOutFile(nameFileOut);

    //discs_properties[0];
    //printf("aaaaaaaa: %f", discs_properties[0][0]);

    for (int i = 0; i <= radio; i++)
    {
        writeData(i+1, discs_properties[i],nameFileOut);
    }
    
    return 0;
}