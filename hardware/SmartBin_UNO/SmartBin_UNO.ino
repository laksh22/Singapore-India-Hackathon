//TODO: Remove all the serial.prints for the final communication

#include <stdlib.h>
#include <math.h>
#include <Servo.h>
#include "HX711.h" // have to install this library in Tools>Manage Libraries

#define BIN_WIDTH 30
#define BIN_HEIGHT 30
#define BIN_LENGTH 30

struct GeoFence
{
  float x;      //Center latitude
  float y;      //Center longitude
  float radius; //Radius in degrees from the center
};

GeoFence validLoc = {0.000000, 0.000000, 200.000000};

bool isLocked = false;

float lat = 0.000000;
float lon = 0.000000;

int ownerID = 0;
int transporterID = 0; //Set this to who's allowed to open it next
int nextTransporterID = 0;
int facilityID = 0; //Set this to who's allowed to open/close it next
int nextFacilityID = 0;
int scannerID = 0;
int IdType = 0;

bool transporterScanned = false;
bool facilityScanned = false;

float weight = 0.000000;
float volume = 0.000000;

const int servoPin = 8;
Servo Servo1; // Creating a servo obj
const int trigPinLeft = 9;
const int echoPinLeft = 10;
const int trigPinRight = 11;
const int echoPinRight = 12;

#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT 5
#define CLK 6
HX711 scale;

void setup()
{
  pinMode(trigPinLeft, OUTPUT);
  pinMode(echoPinLeft, INPUT);
  pinMode(trigPinRight, OUTPUT);
  pinMode(echoPinRight, INPUT);

  //servo setup
  Servo1.attach(servoPin);
  Servo1.write(0); // initially unlocked

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();                        //Assuming there is no weight on the scale at start up, reset the scale to 0
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() > 0)
  {
    String inputText = Serial.readString();
    Serial.println(inputText); //DEBUG
    parseInput(inputText.c_str());
    if (isLocked)
    {
      if (IdType == 1 && !facilityScanned && scannerID == facilityID && isWithinLoc(lat, lon) && calculateVolume() == volume)
      {
        facilityScanned = true;
        Serial.println("F Scan"); //DEBUG
      }
      if (IdType == 2 && !transporterScanned && scannerID == transporterID && isWithinLoc(lat, lon) && calculateVolume() == volume)
      {
        transporterScanned = true;
        Serial.println("T Scan"); //DEBUG
      }
      if (transporterScanned && facilityScanned)
      {
        unlockBin(); // Open lock
        ownerID = facilityID;
        transporterScanned = facilityScanned = false;
        transporterID = nextTransporterID;
        volume = calculateVolume();
        weight = readWeightSensor();
        sendUnlockOutput();
      }
    }
    else
    { // if bin is unlocked,
      if (IdType == 1 && !facilityScanned && scannerID == facilityID && isWithinLoc(lat, lon))
      {
        facilityScanned = true;
        Serial.println("F Scan"); //DEBUG
      }
      if (IdType == 2 && !transporterScanned && scannerID == transporterID && isWithinLoc(lat, lon))
      {
        transporterScanned = true;
        Serial.println("T Scan"); //DEBUG
      }
      if (transporterScanned && facilityScanned)
      {
        Serial.println("LOCKING"); //DEBUG
        lockBin();                 // Close lock
        ownerID = transporterID;
        transporterScanned = facilityScanned = false;
        facilityID = nextFacilityID;
        volume = calculateVolume();
        weight = readWeightSensor();
        sendLockOutput();
      }
    }
  }
}

void lockBin()
{
  Servo1.write(90);
  isLocked = true;
}

void unlockBin()
{
  Servo1.write(0);
  isLocked = false;
}

void parseInput(char str[])
{

  //UNLOCKING
  if (str[1] == 'U')
  {
    // TUNLOCK#ID_LAT_LON OR FUNLOCK#ID_LAT_LON_TRANSPORTERID

    if (str[0] == 'T')
    {
      IdType = 2;
    }
    if (str[0] == 'F')
    {
      IdType = 1;
    }

    char *token;
    token = strtok(str, "#");

    //Get scanner ID
    token = strtok(NULL, "_");
    scannerID = atoi(token);
    Serial.println(scannerID);
    //Get current latitude
    token = strtok(NULL, "_");
    lat = atof(token);
    Serial.println(lat);
    //Get current longitude
    token = strtok(NULL, "_");
    lon = atof(token);
    Serial.println(lon);

    if (IdType == 2)
    {
      //Get ID of who can collect the bin for transportation
      token = strtok(NULL, "_");
      nextTransporterID = atoi(token);
      Serial.println(nextTransporterID);
    }
  }

  //LOCKING
  if (str[1] == 'L')
  {
    //TLOCK#ID_LAT_LON OR FLOCK#ID_LAT_LON_X_Y_RADIUS_FACILITYID

    //Transporter is scanning
    if (str[0] == 'T')
    {

      IdType = 2;

      char *token;
      token = strtok(str, "#");
      //Get scanner ID
      token = strtok(NULL, "_");
      scannerID = atoi(token);
      Serial.println(scannerID);
      //Get current latitude
      token = strtok(NULL, "_");
      lat = atof(token);
      Serial.println(lat);
      //Get current longitude
      token = strtok(NULL, "_");
      lon = atof(token);
      Serial.println(lat);
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
      Serial.println(scannerID);
      //Get current latitude
      token = strtok(NULL, "_");
      lat = atof(token);
      Serial.println(lat);
      //Get current longitude
      token = strtok(NULL, "_");
      lon = atof(token);
      Serial.println(lon);
      //Get geo-fence for destination
      token = strtok(NULL, "_");
      float x = atoi(token);
      token = strtok(NULL, "_");
      Serial.println(x);
      float y = atoi(token);
      Serial.println(y);
      token = strtok(NULL, "_");
      float radius = atoi(token);
      Serial.println(radius);
      //Get ID of destination facility
      token = strtok(NULL, "_");
      nextFacilityID = atoi(token);
      Serial.println(nextFacilityID);
      changeValidLoc(x, y, radius);
    }
  }
}

float readWeightSensor()
{
  float weightInLbs = scale.get_units();
  float weightInKg = weightInLbs * 0.453592;
  weight = weightInKg;
  Serial.print("weight:"); //DEBUG
  Serial.println(weight);  //DEBUG
  return weight
}

void sendLockOutput()
{
  //LOCK#OWNERID_VOL_WEIGHT
  char output_str[50];
  sprintf(output_str, "LOCK#%d_%d_%d_%f_%f_%f_%f", ownerID, volume, weight, validLoc.lat1, validLoc.lat2, validLoc.lon1, validLoc.lon2);
  Serial.println(output_str);

  Serial.println(volume);        //DEBUG
  Serial.println(weight);        //DEBUG
  Serial.println(validLoc.lat1); //DEBUG
  Serial.println(output_str);    //DEBUG
}

void sendUnlockOutput()
{
  //UNLOCK#OWNERID_VOL_WEIGHT
  char output_str[50];
  sprintf(output_str, "UNLOCK#%d_%f_%f", ownerID, volume, weight);
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
  return distance;
}

float calculateVolume()
{
  int leftPinDistance = getSensorDistance(trigPinLeft, echoPinLeft);
  int rightPinDistance = getSensorDistance(trigPinRight, echoPinRight);
  float currentVol = BIN_LENGTH * BIN_WIDTH * (BIN_HEIGHT - ((leftPinDistance + rightPinDistance) / 2));
  return currentVol;
}

//Change destination where bin can be locked or unlocked
void changeValidLoc(float x, float y, float radius)
{
  validLoc.x = x;
  validLoc.y = y;
  validLoc.radius = radius;
}

//Check if the scanning location is within geo-fence that is allowed
bool isWithinLoc(float lat, float lon)
{
  float d1 = lat - validLoc.x;
  float d2 = lon - validLoc.y;
  float distance = sqrt((d1 * d1) + (d2 * d2));
  if (distance < validLoc.radius)
  {
    return true;
  }
  else
  {
    return false;
  }
}
