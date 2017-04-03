#ifndef ROBOT_H
#define ROBOT_H

#define NUM_SONARS  16
#define LOG         1
#define RAIO        0.0975
#define PI          3.14159

#include <fstream>
#include <iostream>
#include "Simulator.h"
#include <math.h>

extern "C" {
   #include "extApi.h"
   #include "v_repLib.h"
}

class Robot
{
public:
    Robot(Simulator *sim, std::string name);
    void update();
    void updateSensors();
    void updatePose();
    void printPose();
    void writeGT();
    void writeSonars();
    void move(float vLeft, float vRight);
    double vRToDrive(double vLinear, double vAngular);
    double vLToDrive(double vLinear, double vAngular);
    void drive(double vLinear, double vAngular);
    void stop();
    void check();
    void writePointsPerSonars();
private:
    const float L = 0.381;                                   // distance between wheels
    const float R = 0.0975;                                  // wheel radius
    std::string name;
    Simulator *sim;

    simxInt handle;                                        // robot handle
    simxFloat velocity[2] = {1,1};                         // wheels' speed
    simxInt sonarHandle[16];                               // handle for sonars
    simxInt motorHandle[2] = {0,0};                        // [0]-> leftMotor [1]->rightMotor
    simxInt encoderHandle[2] = {0,0};
    simxFloat encoder[2] = {0,0};
    simxFloat lastEncoder[2] = {0,0};

    /* Robot Position  */
    simxFloat robotPosition[3] = {0,0,0};                    // current robot position
    simxFloat robotOrientation[3] = {0,0,0};                 // current robot orientation
    float initialPose[3] = {0,0,0};
    simxFloat robotLastPosition[3] = {0,0,0};                // last robot position
    float sonarReadings[NUM_SONARS];
    int sonarAngles[8] = {90, 50, 30, 10, -10, -30, -50, -90};
};

#endif // ROBOT_H
