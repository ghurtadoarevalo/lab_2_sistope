#ifndef FUNCTION
#define FUNCTION

// Prototipos de las Funciones. 
//------------------------------------------------------------//

void partialRealAverage(monitor * disc);

void partialImaginaryAverage(monitor * disc);

void partialPotency(monitor * disc);

void partialNoise(monitor * disc);

visibility_s * buildVisibility(char * readedData);

float distance(visibility_s * visibility);

void * consume(void * disc);

int * readData(int radio, int width, char * nameFileIn, monitor ** discs);

monitor ** initializeMonitors(int disco, int bufferSize);

void writeData(int number, float *results, char *outFileName);

void createOutFile(char *outFileName);

#endif