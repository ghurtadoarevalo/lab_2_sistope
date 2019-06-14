#ifndef FUNCTION
#define FUNCTION

// Prototipos de las Funciones. 
//------------------------------------------------------------

void partialRealAverage(monitor * disc);

void partialImaginaryAverage(monitor * disc);

void partialPotency(monitor * disc);

void partialNoise(monitor * disc);

visibility_s * buildVisibility(char * readedData);

float distance(visibility_s * visibility);

void * consume(void * disc);

void * readData(int radio, int width, int flag, char * nameFileIn, monitor ** discs);

monitor **initializeMonitors(int radio, int bufferSize);

void writeData(int number, float *results, char *outFileName);

void createOutFile(char *outFileName);

#endif