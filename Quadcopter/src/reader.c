#include reader.h

/*
##############################################
#   Reader functionality                     #
#   Nicolas de Maubeuge and Vivien Billaud   #
#   October 2014                             #
#                                            #
##############################################
*/


// Software defines :

#define ARDUINO_ADDRESS 1
#define SAMPLING_PERIOD_MS 10

// Functions :

void* readerHandler(void* arg)
{
    int i;
    int isUltrasonicOn = 0;
    float* ultrasonicSampleList;
    float filteredValue;


    bidirectionalHandler_t* bidirectionalHandler;
    bidirectionalHandler = (bidirectionalHandler_t*)arg;

    handler_t* mainITMHandler;
    handler_t* readerITMHandler;

    mainITMHandler = bidirectionalHandler.mainITMHandler;
    readerITMHandler = bidirectionalHandler.componentITMHandler;

    message_t* receivedMessage;
    message_t currentMessage;

    strcpy(currentMessage.message, "main_reader_init");
    currentMessage.priority=20;

    sendMessage(mainITMHandler, currentMessage);



    long buffer[9];
    int fd;

    receivedCommandsShared_t receivedCommands;

    pthread_mutex_lock(&receivedCommands.readWriteMutex);


    for (i=1; i<=9; i++) receivedCommands.commands[i]=0;

    pthread_mutex_unlock(&receivedCommands.readWriteMutex);


    if (fd = wiringPiI2CSetup (ARDUINO_ADDRESS)) < 0)
    {
        printDebug("[e] Reader : error connecting to Arduino via I2C")
        // TODO : notify main thread
    }

    // Now we're connected, processing incoming data :

    while(1)
{
    // Message processing AREA :
    receivedMessage = retrieveMessage(readerITMHandler);

        //TODO : make message parser
        if(receivedMessage.message == "ultrasonicon")
        {
            if (isUltrasonicOn)
            {
                printDebug("Something strange in reader : asked to turn on an already on ultrasonic")
            }
            ultrasonicSampleList = initUltrasonic();
            isUltrasonicOn = 1;
        }

        if (receivedMessage.message == "ultrasonicoff")
        {
            if(!isUltrasonicOn)
            {
                printDebug("Something strange in reader : asked to turn off an already off ultrasonic")
            }
            shutdownUltrasonic(ultrasonicSampleList);
            isUltrasonicOn = 0;
        }



        read(fd, buffer, 4*9); //TODO implement connection test and ultrasonic reading

        if (isUltrasonicOn)
        {
            addToSampleList(VALUE. ultrasonicSampleList);
            filteredValue = getFilteredUltrasonic(ultrasonicSampleList);
        }

        // TODO : integrate ultrasonic

        pthread_mutex_lock(receivedCommands.readWriteMutex);

        for (int i=0; i<=8, i++)
        {
            receivedCommands.commands[i] = buffer[i];
        }

        receivedCommands.ultrasonicTelemeter = filteredValue;



        pthread_mutex_unlock(userCommands.readWriteMutex);

        sleep(SAMPLING_PERIOD_MS/1000);

    }
    close(fd);

}

float* initUltrasonic()
{
    sampleList_t* list;
    list = (sampleList_t*)malloc(SAMPLESIZE*sizeof(sampleList_t));
    list.lastEntryIndex = -1;
    list.NumberOfSamples = 0;

    return list;
}


void* addToSampleList(float sample, sampleList_t list)
{
    int toBeInsertedIndex;


    if (list.lastEntryIndex + 1 > SAMPLESIZE-1) tobeInsertedIndex = 0;
    else tobeInsertedIndex = list.lastEntryIndex +1;

    list.list[toBeInsertedIndex] = sample;
    list.lastEntryIndex = toBeInsertedIndex;

    if (list.numberOfSamples < SAMPLESIZE) list.numberOfSamples++;
}

float getFilteredUltrasonic(sampleList_t sampleList)
{
    float result;
    float sampleCopy[SAMPLESIZE];

    for (int i; i<SAMPLESIZE; i++ sampleCopy[i] = sampleList.list[i]; // In order to sort the table, we need to copy it
    qsort(sampleCopy, SAMPLESIZE, sizeof(float), comp);

    result = (sampleCopy[SAMPLESIZE/2] + sampleCopy[SAMPLESIZE/2 +1])/2; // As it it as pair number, me take this as a median

    return result;
}

shutdownUltrasonic(sampleList_t* sampleList)
{
    free(sampleList);

}

comp(const void *a, const void *b)
{
    int *x = (int *) a;
    int *y = (int *) b;
    return *x - *y;
}






