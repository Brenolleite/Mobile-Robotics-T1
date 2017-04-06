#include "Robot.h"

Robot::Robot(Simulator *sim, std::string name) {
  this->sim = sim;
  this->name = name;

  this->velocity[0] = 10;
  this->velocity[1] = 10;

  handle = sim->getHandle(name);

  if (LOG) {
    FILE *data =  fopen("gt.txt", "wt");
    if (data!=NULL)
      fclose(data);
    data =  fopen("sonar.txt", "wt");
    if (data!=NULL)
      fclose(data);
    data =  fopen("points.txt", "wt");
    if (data!=NULL)
      fclose(data);
  }

  /* Get handles of sensors and actuators */
  encoderHandle[0] = sim->getHandle("Pioneer_p3dx_leftWheel");
  encoderHandle[1] = sim->getHandle("Pioneer_p3dx_rightWheel");
  std::cout << "Left Encoder: "<< encoderHandle[0] << std::endl;
  std::cout << "Right Encoder: "<< encoderHandle[1] << std::endl;

  /* Get handles of sensors and actuators */
  motorHandle[0] = sim->getHandle("Pioneer_p3dx_leftMotor");
  motorHandle[1] = sim->getHandle("Pioneer_p3dx_rightMotor");
  std::cout << "Left motor: "<<  motorHandle[0] << std::endl;
  std::cout << "Right motor: "<<  motorHandle[1] << std::endl;

  simxChar sensorName[31];
  /* Connect to sonar sensors. Requires a handle per sensor. Sensor name: Pioneer_p3dx_ultrasonicSensorX, where
   * X is the sensor number, from 1 - 16 */
  for(int i = 0; i < 16; i++)
  {
    sprintf(sensorName,"%s%d","Pioneer_p3dx_ultrasonicSensor",i+1);
    sonarHandle[i] = sim->getHandle(sensorName);
    if (sonarHandle[i] == -1)
      std::cout <<  "Error on connecting to sensor " + i+1 << std::endl;
    else
    {
      std::cout << "Connected to sensor\n";
    }

    //init sensors
    sim->readProximitySensor(sonarHandle[i],NULL,NULL,0);
  }

  /* Get the robot current absolute position */
  sim->getObjectPosition(handle,robotPosition);
  sim->getObjectOrientation(handle,robotOrientation);

  initialPose[0]=robotPosition[0];
  initialPose[1]=robotPosition[1];
  initialPose[2]=robotOrientation[2];
  for(int i = 0; i < 3; i++)
  {
    robotLastPosition[i] = robotPosition[i];
  }

  /* Get the encoder data */
  sim->getJointPosition(motorHandle[0],&encoder[0]);
  sim->getJointPosition(motorHandle[1],&encoder[1]);
  //std::cout << encoder[0] << std::endl;
  //std::cout << encoder[1] << std::endl;

  // Initialization of average filter
  initAvrFilter();
}

void Robot::update() {
  drive(this->velocity[0],this->velocity[1]);
  updateSensors();
  updatePose();
  check();
}

void:: Robot::check()
{
    this->velocity[0] = 40;
    this->velocity[1] = 0;
    float angle[8] = {-30.0,-30.0,-30.0,-30.0,30.0,30.0,30.0,50.0};
    float minDist[8] = {0.1,0.2,0.3,0.3,0.3,0.3,0.2,0.1};
    for(int i = 0; i < 8; i++)
    {
        if(sonarReadings[i] > 0 && sonarReadings[i] < minDist[i]){
            this->velocity[0] = 0;
            this->velocity[1] += angle[i];
        }
    }

    if(this->velocity[0] == 0 && this->velocity[1] == 0){
        this->velocity[1] = 30;
    }
}

void Robot::updateSensors()
{

  /* Update sonars */
  for(int i = 0; i < NUM_SONARS; i++)
  {
    simxUChar state;       // sensor state
    simxFloat coord[3];    // detected point coordinates [only z matters]

    /* simx_opmode_streaming -> Non-blocking mode */

    /* read the proximity sensor
     * detectionState: pointer to the state of detection (0=nothing detected)
     * detectedPoint: pointer to the coordinates of the detected point (relative to the frame of reference of the sensor) */
    if (sim->readProximitySensor(sonarHandle[i],&state,coord,1)==1)
    {
      if(state > 0)
        sonarReadings[i] = coord[2];
      else
        sonarReadings[i] = -1;
    }
  }

  /* Update encoder data */
  lastEncoder[0] = encoder[0];
  lastEncoder[1] = encoder[1];

  /* Get the encoder data */
  if (sim->getJointPosition(motorHandle[0], &encoder[0]) == 1);
  //      std::cout << "ok left enconder"<< encoder[0] << std::endl;  // left
  if (sim->getJointPosition(motorHandle[1], &encoder[1]) == 1);
  //  std::cout << "ok right enconder"<< encoder[1] << std::endl;  // right

}

void Robot::updatePose()
{
  for(int i = 0; i < 3; i++)
  {
    robotLastPosition[i] = robotPosition[i];
  }

  /* Get the robot current position and orientation */
  sim->getObjectPosition(handle,robotPosition);
  sim->getObjectOrientation(handle,robotOrientation);
}

void Robot::writeGT() {
  /* write data to file */
  /* file format: robotPosition[x] robotPosition[y] robotPosition[z] robotLastPosition[x] robotLastPosition[y] robotLastPosition[z]
   *              encoder[0] encoder[1] lastEncoder[0] lastEncoder[1] */

  if (LOG) {

    FILE *data =  fopen("gt.txt", "at");
    if (data!=NULL)
    {
      for (int i=0; i<3; ++i)
        fprintf(data, "%.2f\t",robotPosition[i]);
      for (int i=0; i<3; ++i)
        fprintf(data, "%.2f\t",robotLastPosition[i]);
      for (int i=0; i<3; ++i)
        fprintf(data, "%.2f\t",robotOrientation[i]);
      for (int i=0; i<2; ++i)
        fprintf(data, "%.2f\t",encoder[i]);
      for (int i=0; i<2; ++i)
        fprintf(data, "%.2f\t",lastEncoder[i]);
      fprintf(data, "\n");
      fflush(data);
      fclose(data);
    }
    else
      std::cout << "Unable to open file";
  }
}

void Robot::writeSonars() {
  /* write data to file */
  /* file format: robotPosition[x] robotPosition[y] robotPosition[z] robotLastPosition[x] robotLastPosition[y] robotLastPosition[z]
   *              encoder[0] encoder[1] lastEncoder[0] lastEncoder[1] */
  if (LOG) {
    FILE *data =  fopen("sonar.txt", "at");
    if (data!=NULL)
    {
      if (data!=NULL)
      {
        for (int i=0; i<NUM_SONARS; ++i)
          fprintf(data, "%.2f\t",sonarReadings[i]);
        fprintf(data, "\n");
        fflush(data);
        fclose(data);
      }
    }
  }
}

void Robot::writePointsPerSonars() {
  float x, y;
  int write = 1;
  if (LOG) {
    FILE *data =  fopen("points.txt", "at");

    if (data!=NULL){
      for (int i=0; i<8; ++i){
        if(sonarReadings[i] > 0){
          x = robotPosition[0] + (sonarReadings[i] + RAIO) * cos(robotOrientation[2] + (sonarAngles[i]*PI)/180);
          y = robotPosition[1] + (sonarReadings[i] + RAIO) * sin(robotOrientation[2] + (sonarAngles[i]*PI)/180);
          
          if(AVR_FILTER)
            write = averageFilter(i, &x, &y);

          // verifica se deve escrever no arquivo
          if(write)
            fprintf(data, "%.4f \t %.4f \n", x, y);
        }
      }
      
      fflush(data);
      fclose(data);
    }
  }
}

void Robot::printPose() {
  std::cout << "[" << robotPosition[0] << ", " << robotPosition[1] << ", " << robotOrientation[2] << "]" << std::endl;
}

void Robot::move(float vLeft, float vRight) {
  sim->setJointTargetVelocity(motorHandle[0], vLeft);
  sim->setJointTargetVelocity(motorHandle[1], vRight);
}

void Robot::stop() {
  sim->setJointTargetVelocity(motorHandle[0], 0);
  sim->setJointTargetVelocity(motorHandle[1], 0);
}

void Robot::drive(double vLinear, double vAngular)
{
  sim->setJointTargetVelocity(motorHandle[0], vLToDrive(vLinear,vAngular));
  sim->setJointTargetVelocity(motorHandle[1], vRToDrive(vLinear,vAngular));
}

double Robot::vRToDrive(double vLinear, double vAngular)
{
  return (((2*vLinear)+(L*vAngular))/2*R);
}

double Robot::vLToDrive(double vLinear, double vAngular)
{
  return (((2*vLinear)-(L*vAngular))/2*R);
}

void Robot::initAvrFilter(){
  int i;
  for(i = 0; i < NUM_SONARS; i++){
    sonarAvrFilters[i].sum_y = sonarAvrFilters[i].sum_x = 0;
    sonarAvrFilters[i].count = 0;
  }
}

float Robot::euclideanDistance(float x1, float y1, float x2, float y2){
  return sqrt((x1*x1 - x2*x2) + (y1*y1 - y2*y2));
}

int Robot::checkAverageDistance(int i, float x, float y){
  float avr_x, avr_y;
  
  if(sonarAvrFilters[i].count == 0)
    return 1;
 
  avr_x =  sonarAvrFilters[i].sum_x/sonarAvrFilters[i].count;
  avr_y =  sonarAvrFilters[i].sum_y/sonarAvrFilters[i].count;
  
  // Caso a distancia seja menor que a permitida para media retorna 1
  return (euclideanDistance(avr_x, avr_y, x, y) < MAX_DISTANCE);
}

int Robot::averageFilter(int i, float *x, float *y){
  avr_filter *sonarFilter = &sonarAvrFilters[i];

  // Se ja foi ignorado muitos pontos por esse sonar reseta o mesmo
  if(sonarFilter->avrPointsIgnored == MAX_IGNORED && MAX_DISTANCE){
    *x = sonarFilter->sum_x/sonarFilter->count;
    *y = sonarFilter->sum_y/sonarFilter->count;
    
    sonarFilter->count =  sonarFilter->sum_y = sonarFilter->sum_x = 0;

    sonarFilter->avrPointsIgnored = 0;

    return 1;
  }
  
  /*
    MAX_DISTANCE = 0 desliga ignoramento de pontos
    por distancia euclidiana
  */

  if(!MAX_DISTANCE || checkAverageDistance(i, *x, *y)){
    sonarFilter->sum_x += *x;
    sonarFilter->sum_y += *y;
  
    // Verifica se deve escrever a média desses pontos
    if(sonarFilter->count++ == POINTS_PER_AVR){
      *x = sonarFilter->sum_x/POINTS_PER_AVR;
      *y = sonarFilter->sum_y/POINTS_PER_AVR;
      
      sonarFilter->count =  sonarFilter->sum_y = sonarFilter->sum_x = 0;
      
      return 1;
    }
  } 
  else{
    sonarFilter->avrPointsIgnored++;
  }

 
  return 0;
}
