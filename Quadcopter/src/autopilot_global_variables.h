#include "shared_librairies.h"

// Some defines :

// Objective code defines :

#define LAND_TAKEOFF 1
#define GOTO_STANDARD 2
#define GOTO_HOVERING 3
#define POSITION_HOLD 4

// Fifo defines :




// Global variables and structures declarations :


typedef struct autopilotObjective_t
{
    char name[64]; // the objective name that will be displayed
    int priority; // Priority from 0 to 100

    int code; // Objective code : for a complete list, refer to documentation

    // Those elements may not be necessary ( depending from the objective code )

    double destinationX;
    double destinationY;
    double destinationZ;
    double directionBearing;//direction de l'objectif

    double destinationLat;
    double destinationLong;
    double destinationAlt;

    double destinationDistXY; // Distance in m on the X-Y plan
    double maxSpeedXY; // Max speed in m*s-1 on the X-Y plan

    double destinationDist;
    double maxSpeed; // Max speed in m*s-1

    struct autopilotObjective_t* previousObjective; // For FIFO management
    struct autopilotObjective_t* nextObjective; // For FIFO management

}autopilotObjective_t;





typedef struct autopilotSharedState
{

    pthread_mutex_t readWrite;

}autopilotSharedState_t;


// Global variables declarations


extern float landTakeOffCoeff[4][3]; // PID coefficients for Land/Takeoff
extern float gotoStandardCoeff[2][3]; // PID coefficients for goto_standard
extern float gotoHoverCoeff[4][3]; // PID coefficients for goto_hovering


extern autopilotSharedState_t autopilotSharedState;


