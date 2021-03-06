
#ifndef PILOT_GLOBAL_V
#define PILOT_GLOBAL_V

#include "shared_librairies.h"


// Some defines for pilot mode :

#define MANUAL 1
#define AUTO 2
#define TEST 3


// Global variables structures declaration

typedef struct pilotCommandsShared_t
{
    double chan1; // 0-100%
    double chan2; // 0-100%
    double chan3; // 0-100%
    double chan4; // 0-100%
    double chan5; // 0-100%
    double chan6; // 0-100%
    double chan7; // 0-100%
    double chan8; // 0-100%
    double chan9; // 0-100%

    int refreshingPeriod;

    pthread_mutex_t readWrite;

} pilotCommandsShared_t;

typedef struct pilotStateShared_t // Pilot state for datalogging
{
    int pilotMode;
    pthread_mutex_t readWriteMutex;

}pilotStateShared_t;


// Global variables definitions

extern pilotCommandsShared_t pilotCommandsShared;
extern pilotStateShared_t pilotStateShared;

#endif
