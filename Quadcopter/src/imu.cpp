#include "imu.h"


flightStateShared_t flightStateShared;
positionShared_t positionShared;
rawPositionShared_t homeRawPosition;
rawPositionShared_t rawPositionShared;
homePosition_t homePosition;
//receivedCommandsShared_t receivedCommands;

void* imuHandler(void* arg)
 {
/*
    bidirectionnalHandler_t* bidirectionnalHandler;
    bidirectionnalHandler = (bidirectionnalHandler_t*)arg;

    handler_t* mainITMHandler;
    handler_t* imuITMHandler;

    mainITMHandler = bidirectionnalHandler->mainITMHandler;
    imuITMHandler = bidirectionnalHandler->componentITMHandler;
    */
    handler_t* mainITMHandler;
    mainITMHandler = (handler_t*)arg;

    // Initialization of shared variables :

    initialize_mutex(&flightStateShared.readWriteMutex);
    initialize_mutex(&rawPositionShared.readWriteMutex);
    initialize_mutex(&positionShared.readWriteMutex);

    // Initialisation of handler variables :

    int waitingGPS = 1;

    // Notifying main thread of init :

    message_t currentMessage;
    strcpy(currentMessage.message, "main_imu_info_init");
    currentMessage.priority = 20;
    sendMessage(mainITMHandler, currentMessage);

    double altitude;

    int sampleCount = 0;
    int sampleRate = 0;
    uint64_t rateTimer;
    uint64_t displayTimer;
    uint64_t now;

    //  using RTIMULib here allows it to use the .ini file generated by RTIMULibDemo.

    RTIMUSettings *settings = new RTIMUSettings("RTIMULib");

    RTIMU *imu = RTIMU::createIMU(settings);
    RTPressure *pressure = RTPressure::createPressure(settings);

    if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU found\n");
        exit(1);
    }

    //  This is an opportunity to manually override any settings before the call IMUInit

    //  set up IMU

    imu->IMUInit();

    //  set up pressure sensor

    if (pressure != NULL)
        pressure->pressureInit();

    //  set up for rate timer

    rateTimer = displayTimer = RTMath::currentUSecsSinceEpoch();

    // Waiting for GPS FIX :

    while(waitingGPS)
    {
        pthread_mutex_lock(&receivedCommands.readWriteMutex);

        if(receivedCommands.gpsStatus == FIX) waitingGPS = 0;
        sleep(1);

        pthread_mutex_lock(&receivedCommands.readWriteMutex);
    }

    // Init IMU



    // BMP085 init

    //bmp085_i2c_Begin(); // TODO : test bmp init
   /* unsigned int pressure, temperature;
    unsigned int up, ut;
    unsigned int bmpAltitude;
*/
    // Ultrasonic init :

    unsigned int ultrasonicAltitude;
    // For now, turning ultrasonic ON on startup :

    strcpy(currentMessage.message ,"reader_imu_order_ultrasonicon");
    currentMessage.priority = 20;
    sendMessage(mainITMHandler, currentMessage);

    // Notify main thread of end of init :

    strcpy(currentMessage.message ,"main_imu_info_endofinit");
    currentMessage.priority = 20;
    sendMessage(mainITMHandler, currentMessage);

    // Entering handler loop Area :

    while(1)
    {
        // Sensor updating Area :

        // Updating IMU :
         //  poll at the rate recommended by the IMU

        usleep(imu->IMUGetPollInterval() * 1000);

        while (imu->IMURead()) {
            RTIMU_DATA imuData = imu->getIMUData();

            //  add the pressure data to the structure

            if (pressure != NULL)
                pressure->pressureRead(imuData);

            sampleCount++;

            now = RTMath::currentUSecsSinceEpoch();

            //  display 10 times per second

            if ((now - displayTimer) > 100000) {
                printf("Sample rate %d: %s\n", sampleRate, RTMath::displayDegrees("", imuData.fusionPose));

                if (pressure != NULL) {
                    printf("Pressure: %4.1f, height above sea level: %4.1f, temperature: %4.1f\n",
                           imuData.pressure, RTMath::convertPressureToHeight(imuData.pressure), imuData.temperature);
                }

                fflush(stdout);
                displayTimer = now;
            }

            //  update rate every second

            if ((now - rateTimer) > 1000000) {
                sampleRate = sampleCount;
                sampleCount = 0;
                rateTimer = now;
            }

        // Updating BMP :

       /* ut = bmp085_ReadUT();
        up = bmp085_ReadUP();

        pressure = bmp085_GetPressure(up);
        temperature = bmp085_GetTemperature(ut);

        // Calculating bmp altitude :

        bmpAltitude = 44330*(1-pow(pow((pressure/101325),(1/5255)),1000));
*/

        // Altitude calculation Area :

        pthread_mutex_lock(&receivedCommands.readWriteMutex);
        ultrasonicAltitude = receivedCommands.ultrasonicTelemeter;

        pthread_mutex_unlock(&receivedCommands.readWriteMutex);

        /*if (receivedCommands.altitude<1 && bmpAltitude<1)//1m du sol???
        {
            altitude = (0,2*ultrasonicAltitude + 0.3*bmpAltitude +0.5*receivedCommands.altitude)/3;//filtre à complémentarité
             if (bmpAltitude>0.05 && bmpAltitude<0.5)// de 10 à 50 cm du sol ???
             {
                 altitude = (0,6*ultrasonicAltitude + 0.4*bmpAltitude)/2; // ou 0.4*receivedCommands.altitude : le capteur le  lus rapide et précis dans ces conditions
             }
        }

        else {altitude = (0.5*bmpAltitude + 0.5*receivedCommands.altitude)/2;}
*/



        // Position calculation Area :

        double x,y;
        double* px = &x;
        double* py = &y;

        convertPlanarToHome(px,py,receivedCommands.latitude, receivedCommands.longitude);

        // Locking the relatives Mutexs :

        pthread_mutex_lock(&flightStateShared.readWriteMutex);
        pthread_mutex_lock(&rawPositionShared.readWriteMutex);
        pthread_mutex_lock(&positionShared.readWriteMutex);

        // Sending the data to the global variables

        flightStateShared.roll = imuData.fusionPose.x()* RTMATH_RAD_TO_DEGREE ;
        flightStateShared.pitch = imuData.fusionPose.y()* RTMATH_RAD_TO_DEGREE;
        flightStateShared.yaw = imuData.fusionPose.z()* RTMATH_RAD_TO_DEGREE;

        rawPositionShared.altitude = altitude;
        rawPositionShared.longitude = receivedCommands.longitude;
        rawPositionShared.latitude = receivedCommands.latitude;

        positionShared.x = x;
        positionShared.y = y;
        positionShared.z = altitude;

        pthread_mutex_unlock(&flightStateShared.readWriteMutex);
        pthread_mutex_unlock(&rawPositionShared.readWriteMutex);
        pthread_mutex_unlock(&positionShared.readWriteMutex);


        }

    }

 }



