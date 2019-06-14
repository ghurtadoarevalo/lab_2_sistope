#include "structs.h"
#include "global_variables.h"
#include "function.h"

int end;

int main(int argc, char *argv[])
{    
    end = 0;
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