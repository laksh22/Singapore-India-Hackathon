#include <stdlib.h>
#include <Servo.h>

#define BIN_WIDTH 30

struct GeoFence
{
  float lat1;
  float lat2;
  float lon1;
  float lon2;
};

GeoFence validLoc = {0.0, 0.0, 0.0, 0.0};

bool isLocked = false;

float lat = 0.0;
float lon = 0.0;

int ownerID = 0;
int transporterID = 0;
int facilityID = 0;
int scannerID = 0;
int IdType = 0;

bool transporterScanned = false;
bool facilityScanned = false;

int weight = 0;
int volume = 0;

const int servoPin = 8;
Servo Servo1; // Creating a servo obj
const int trigPinBottom = 9;
const int echoPinBottom = 10;
const int trigPinTop = 11;
const int echoPinTop = 12;

void setup()
{
  pinMode(trigPinBottom, OUTPUT);
  pinMode(echoPinBottom, INPUT);
  pinMode(trigPinTop, OUTPUT);
  pinMode(echoPinTop, INPUT);

  Servo1.attach(servoPin);
  Servo1.write(0); // initially unlocked
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() > 0)
  {
    //TODO: Make function to get full input before parsing
    parseInput("Sample Text");
    if (isLocked)
    {
      if (IdType == 1 && !facilityScanned && scannerID == facilityID && isWithinLoc(lat, lon) && calculateVolume() == volume)
      {
        facilityScanned = true;
      }
      if (IdType == 2 && !transporterScanned && scannerID == transporterID && isWithinLoc(lat, lon) && calculateVolume() == volume)
      {
        transporterScanned = true;
      }
      if (transporterScanned && facilityScanned)
      {
        unlockBin(); // Open lock
        isLocked = false;
        ownerID = facilityID;
        transporterScanned = facilityScanned = false;
        readWeightSensor();
        sendUnlockOutput();
      }
    }
    else
    { // if bin is unlocked,
      if (IdType == 1 && !facilityScanned && scannerID == facilityID && isWithinLoc(lat, lon))
      {
        facilityScanned = true;
      }
      if (IdType == 2 && !transporterScanned && scannerID == transporterID && isWithinLoc(lat, lon))
      {
        transporterScanned = true;
      }
      if (transporterScanned && facilityScanned && weight1 == weight2)
      {
        lockBin(); // Close lock
        isLocked = true;
        ownerID = transporterID;
        transporterScanned = facilityScanned = false;
        volume = calculateVolume();
        readWeightSensor();
        sendUnlockOutput();
      }
    }
  }
}

void lockBin()
{
  Servo1.write(90);
}

void unlockBin()
{
  Servo1.write(0);
}

//TODO: Parse input from bluetooth to get data
void parseInput(char str[])
{

  //UNLOCKING
  if (str[1] == 'U')
  {
    // TUNLOCK#ID_LAT_LON OR FUNLOCK#ID_LAT_LON

    char *token;
    token = strtok(str, "#");

    //Get scanner ID
    token = strtok(NULL, "_");
    scannerID = atoi(token);
    //Get current latitude
    token = strtok(NULL, "_");
    lat = atof(token);
    //Get current longitude
    token = strtok(NULL, "_");
    lon = atof(token);
  }

  //LOCKING
  if (str[1] == 'L')
  {
    //TLOCK#ID_LAT_LON OR FLOCK#ID_LAT_LON_LAT1_LAT2_LON1_LON2

    //Transporter is scanning
    if (str[0] == 'T')
    {

      IdType = 2;

      char *token;
      token = strtok(str, "#");
      //Get scanner ID
      token = strtok(NULL, "_");
      scannerID = atoi(token);
      //Get current latitude
      token = strtok(NULL, "_");
      lat = atof(token);
      //Get current longitude
      token = strtok(NULL, "_");
      lon = atof(token);
    }

    //Facility is scanning
    if (str[0] == 'F')
    {

      IdType = 1;

      char *token;
      token = strtok(str, "#");
      //Get scanner ID
      token = strtok(NULL, "_");
      scannerID = atoi(token);
      //Get current latitude
      token = strtok(NULL, "_");
      lat = atof(token);
      //Get current longitude
      token = strtok(NULL, "_");
      lon = atof(token);
      //Get geo-fence for destination
      token = strtok(NULL, "_");
      float lat1 = atoi(token);
      token = strtok(NULL, "_");
      float lat2 = atoi(token);
      token = strtok(NULL, "_");
      float lon1 = atoi(token);
      token = strtok(NULL, "_");
      float lon2 = atoi(token);
      changeValidLoc(lat1, lat2, lon1, lon2);
    }
  }
}

//TODO: Set weight to weight sensor measurement
void readWeightSensor()
{
}

void sendLockOutput()
{
  //TODO: Calculate length of output
  //LOCK#OWNERID_VOL_WEIGHT_LAT1_LAT2_LON1_LON2
  char output_str[50];
  sprintf(output_str, "LOCK#%d_%d_%d_%f_%f_%f_%f", ownerID, volume, weight1, validLoc.lat1, validLoc.lat2, validLoc.lon1, validLoc.lon2);
  Serial.println(output_str);
}

//TODO: Serial output upon unlocking
void sendUnlockOutput()
{
  //TODO: Calculate length of output
  //UNLOCK#OWNERID_VOL_WEIGHT
  char output_str[50];
  sprintf(output_str, "UNLOCK#%d_%d_%d", ownerID, volume, weight1);
  Serial.println(output_str);
}

int getSensorDistance(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  int duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  int distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  return distance;
}

int calculateVolume()
{
  int bottomPin = getSensorDistance(trigPinBottom, echoPinBottom);
  int topPin = getSensorDistance(trigPinTop, echoPinTop);
  if (bottomPin < BIN_WIDTH)
  {
    if (topPin < BIN_WIDTH)
    {
      return 2;
    }
    else
    {
      return 1;
    }
  }
  else
  {
    return 0;
  }
}

//Change destination where bin can be locked or unlocked
void changeValidLoc(float lat1, float lat2, float lon1, float lon2)
{
  validLoc.lat1 = lat1;
  validLoc.lat2 = lat2;
  validLoc.lon1 = lon1;
  validLoc.lon2 = lon2;
}

//Check if the scanning location is within geo-fence that is allowed
bool isWithinLoc(float lat, float lon)
{
  if ((validLoc.lat1 <= lat) &&
      (lat <= validLoc.lat2) &&
      (validLoc.lon1 <= lon) &&
      (lon <= validLoc.lon2))
  {
    return true;
  }
  else
  {
    return false;
  }
}
