#include "structs.h"
#include "function.h"
#include "global_variable.h"

//Función que calcula la media real de todas las visibilidades obtenidas hasta el momento del llamado de la función.
//Entrada: Lista de estructuras monitores.
//Salida: Ninguna.
void partialRealAverage(monitor * disc)
{
    if (disc->in != 0)
    {
        float newAverage = disc->pReal * (disc->quantityPReal);

	    for(int i = 0; i < disc->in; i++)
        {
		    newAverage = newAverage + disc->buffer[i]->r; 
        }
	    disc->quantityPReal += disc->in;
        disc->pReal = newAverage/disc->quantityPReal;
    }
    
}

//Función que calcula la media imaginaria de todas las visibilidades obtenidas hasta el momento del llamado de la función.
//Entrada: Lista de estructuras monitores.
//Salida: Ninguna.
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

//Función que calcula la potencia de todas las visibilidades obtenidas hasta el momento del llamado de la función
//Entrada: Lista de estructuras monitores.
//Salida: Ninguna.
void partialPotency(monitor * disc)
{
	for(int i = 0; i < disc->in; i++){
		disc->pPotency =  sqrt(pow(disc->pPotency,2) + pow(disc->buffer[i]->r, 2) + pow(disc->buffer[i]->i, 2));
    }
}

//Función que calcula el ruido de todas las visibilidades obtenidas hasta el momento del llamado de la función.
//Entrada: Lista de estructuras monitores.
//Salida: Ninguna.
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

//Función que procesa los datos ingresados al buffer por readData(), juntas simulan el algoritmo del productor/consumidor además
//de proveer la exclusión mutua para la ejecución del programa.
//Entrada: Estructura monitor casteada a void.
//Salida: Ninguna.
void * consume(void * disc)
{
    monitor * disc_consumer = (monitor *)disc;
    do
    {
        pthread_mutex_lock(&(disc_consumer->full_mutex));

        while(disc_consumer->in != (disc_consumer->bufferSize)-1)
        {       
            pthread_cond_wait(&(disc_consumer->notfull_cond), &(disc_consumer->full_mutex));
        }

        if(end == 1)
        {
            disc_consumer->quantityProcessed += disc_consumer->inBackUp;
            disc_consumer->in = disc_consumer->inBackUp;
            partialRealAverage(disc_consumer);
            partialImaginaryAverage(disc_consumer);
            partialNoise(disc_consumer);
            partialPotency(disc_consumer);
            
            float * properties = malloc(sizeof(float)*4);
            properties[0] = disc_consumer->pReal;
            properties[1] = disc_consumer->pImaginary;
            properties[2] = disc_consumer->pPotency;
            properties[3] = disc_consumer->pNoise;
            discs_properties[disc_consumer->id] = properties;

            if (flag == 1)
            {
                printf("Soy la hebra de pid %d, procese %d visibilidades\n", disc_consumer->id, disc_consumer->quantityProcessed);
            }

            break;
        }

        disc_consumer->quantityProcessed += disc_consumer->in;
        partialRealAverage(disc_consumer);
        partialImaginaryAverage(disc_consumer);
        partialNoise(disc_consumer);
        partialPotency(disc_consumer);

        disc_consumer->in = 0;

        pthread_cond_signal(&(disc_consumer->full_cond));
        pthread_mutex_unlock(&(disc_consumer->full_mutex));

    }while(end == 0);
}

//Función que  lee línea por línea el archivo .csv y envía cada una de estas líneas a la hebra que corresponda según su disco.
//Entrada: cantidad de radios, ancho del disco, flag para indicar si se muestra o no la información final de los hijos, nombre del
//archivo, información de los monitores.
//Salida: Ninguna.
int * readData(int disco, int width, char * nameFileIn, monitor ** discs)
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

    int * radioList = malloc(sizeof(int)*(disco)); // radioList = [0, R1, R2, R3,..., Rn] -> n = numero de radios

    for (int i = 0; i < disco; ++i)
        radioList[i] = i*width;
   
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {   
        visibility_s * visibility = malloc(sizeof(visibility_s)); 
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        visibility = buildVisibility(buf);

        origin_distance = distance(visibility);
        i = 0;
        while(i < disco)
        {
            if(radioList[i] <= origin_distance && origin_distance < radioList[i+1])
            {
                pthread_mutex_lock(&(discs[i]->full_mutex));
                while((discs[i]->bufferSize)-1 == discs[i]->in)
                {
                    pthread_cond_wait(&(discs[i]->full_cond),&(discs[i]->full_mutex));
                }
                discs[i]->buffer[discs[i]->in] = visibility;
                discs[i]->in += 1;
                pthread_cond_signal(&(discs[i]->notfull_cond));
                pthread_mutex_unlock(&(discs[i]->full_mutex));

            }

            else if(i == (disco - 1) && radioList[i] <= origin_distance) //Este if es necesario para tomar al último hijo.
            {   
                pthread_mutex_lock(&(discs[i]->full_mutex));

                while((discs[i]->bufferSize)-1 == discs[i]->in)
                {
                    pthread_cond_wait(&(discs[i]->full_cond),&(discs[i]->full_mutex));
                }
                discs[i]->buffer[discs[i]->in] = visibility;
                discs[i]->in += 1;
                pthread_cond_signal(&(discs[i]->notfull_cond));
                pthread_mutex_unlock(&(discs[i]->full_mutex));
            }
            i++;

        }

    }
    end = 1;
    for (int i = 0; i < disco; i++)
    {
        discs[i]->inBackUp = discs[i]->in; 
        discs[i]->in = (discs[i]->bufferSize)-1;
        pthread_cond_signal(&(discs[i]->notfull_cond));
    }    

    fclose(fp);
    return 0;
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

//Función que genera tantos monitores como discos (radio + 1) e inicializa sus variables en valores convenienes.
//Entrada: numero de disco, tamaño del buffer.
//Salida: Estructura monitor.
monitor ** initializeMonitors(int disco, int bufferSize)
{
    monitor ** monitors = (monitor**)malloc(sizeof(monitor*)*(disco));
    for (int i = 0; i < disco; i++)
    {
        monitor * mon = malloc(sizeof(monitor));
        mon->buffer = (visibility_s**)malloc(sizeof(visibility_s*)*bufferSize);
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
        pthread_mutex_init(&mon->full_mutex, NULL);
        monitors[i] = mon;
    }    
    return monitors;
}
