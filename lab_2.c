#include "structs.h"
#include "global_variable.h"
#include "function.h"

int end;
int flag;

int main(int argc, char *argv[])
{    
    int otp = 0, disco = 0, width = 0, bufferSize = 0;
    flag = 0;
    char *nameFileIn = NULL, *nameFileOut = NULL;

    while((otp = getopt(argc, argv, "i:o:n:d:s:b")) != -1)
    {
        if(otp == 'i') nameFileIn = optarg;
        else if(otp == 'o') nameFileOut = optarg;
        else if(otp == 'n') disco = atoi(optarg);
        else if(otp == 'd') width = atoi(optarg);
        else if(otp == 's') bufferSize = atoi(optarg);
        else if(otp == 'b') flag = 1;
        else
        {
            printf("Invalid Sintaxis\n");
            exit(-1);
        }
    }
    if((otp == -1 && argc == 1) || disco <= 0 || width <= 0 || bufferSize <= 0)
    {
       printf("Invalid Sintaxis\n");
        exit(-1); 
    }

    monitor ** monitors = initializeMonitors(disco, bufferSize);
    
    discs_properties = malloc(sizeof(float*)*disco);

    pthread_t * disc_threads;
    disc_threads = malloc(sizeof(pthread_t)*disco);

    for (int i = 0; i < disco; i++)
    {
        pthread_create(&disc_threads[i], NULL, consume, (void *)monitors[i]);
    }

    readData(disco,width, nameFileIn, monitors);
    for (int i = 0; i < disco; i++)
    {
        pthread_join(disc_threads[i], NULL);
    }
    createOutFile(nameFileOut);

    for (int i = 0; i < disco; i++)
    {
        writeData(i+1, discs_properties[i],nameFileOut);
    }
    
    return 0;
}