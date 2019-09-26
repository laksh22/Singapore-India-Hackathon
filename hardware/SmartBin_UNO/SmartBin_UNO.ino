//TODO: fix precision in sprintf and the actual values
//TODO: code to check if latlong is in a geofence
//TODO: remove all the serial.prints for the final communication

#include <stdlib.h>
#include <Servo.h>
#include "HX711.h" // have to install this library in Tools>Manage Libraries

#define BIN_WIDTH 30
#define BIN_HEIGHT 30
#define BIN_LENGTH 30

struct GeoFence
{
  float lat1;
  float lat2;
  float lon1;
  float lon2;
};

GeoFence validLoc = {0.0, 2.0, 0.0, 200.0};

bool isLocked = false;

float lat = 0.0;
float lon = 0.0;

int ownerID = 0;
int transporterID = 0;//Set this to who's allowed to open it next
int nextTransporterID = 0;
int facilityID = 0;//Set this to who's allowed to open/close it next
int nextFacilityID = 0;
int scannerID = 0;
int IdType = 0;

bool transporterScanned = false;
bool facilityScanned = false;

float weight = 0;
float volume = 0;

const int servoPin = 8;
Servo Servo1; // Creating a servo obj
const int trigPinLeft = 9;
const int echoPinLeft = 10;
const int trigPinRight = 11;
const int echoPinRight = 12;

#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT  5
#define CLK  6
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
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() > 0)
  {
    String inputText = Serial.readString();
    Serial.println(inputText);
    parseInput(inputText.c_str());
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
        ownerID = facilityID;
        transporterScanned = facilityScanned = false;
        transporterID = nextTransporterID;
        volume = calculateVolume();
        readWeightSensor();
        sendUnlockOutput();
      }
    }
    else
    { // if bin is unlocked,
      Serial.println("LOC");
      Serial.println(isWithinLoc(lat, lon));
      if (IdType == 1 && !facilityScanned && scannerID == facilityID && isWithinLoc(lat, lon))
      {
        facilityScanned = true;
        Serial.println("F Scan");
      }
      if (IdType == 2 && !transporterScanned && scannerID == transporterID && isWithinLoc(lat, lon))
      {
        transporterScanned = true;
        Serial.println("T Scan");
      }
      if (transporterScanned && facilityScanned)
      {
        Serial.println("LOCKING");
        lockBin(); // Close lock
        ownerID = transporterID;
        transporterScanned = facilityScanned = false;
        facilityID = nextFacilityID;
        volume = calculateVolume();
        readWeightSensor();
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

//TODO: Parse input from bluetooth to get data
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
    
    if(IdType == 2){
      //Get ID of who can collect the bin for transportation
    token = strtok(NULL, "_");
    nextTransporterID = atoi(token);
    Serial.println(nextTransporterID);
    }
    
  }

  //LOCKING
  if (str[1] == 'L')
  {
    //TLOCK#ID_LAT_LON OR FLOCK#ID_LAT_LON_LAT1_LAT2_LON1_LON2_FACILITYID

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
      float lat1 = atoi(token);
      token = strtok(NULL, "_");
      Serial.println(lat1);
      float lat2 = atoi(token);
      Serial.println(lat2);
      token = strtok(NULL, "_");
      float lon1 = atoi(token);
      Serial.println(lon1);
      token = strtok(NULL, "_");
      float lon2 = atoi(token);
      Serial.println(lon2);
      //Get ID of destination facility
      token = strtok(NULL, "_");
      nextFacilityID = atoi(token);
      Serial.println(nextFacilityID);
      changeValidLoc(lat1, lat2, lon1, lon2);
    }
  }
}

//TODO: Set weight to weight sensor measurement
void readWeightSensor(){
  float weightInLbs = scale.get_units();
  float weightInKg = weightInLbs * 0.453592;
  weight = weightInKg;
  Serial.print("weight:");
  Serial.println(weight);
}

void sendLockOutput()
{
  //TODO: Calculate length of output
  //LOCK#OWNERID_VOL_WEIGHT_LAT1_LAT2_LON1_LON2
  char output_str[50];
//  Serial.sprintf("LOCK#%d_%d_%d_%f_%f_%f_%f", ownerID, volume, weight, validLoc.lat1, validLoc.lat2, validLoc.lon1, validLoc.lon2);
  //TODO: serial.println these bois and the volume too
  sprintf(output_str, "LOCK#%d_%d_%d_%f_%f_%f_%f", ownerID, volume, weight, validLoc.lat1, validLoc.lat2, validLoc.lon1, validLoc.lon2);
  Serial.println(volume);
  Serial.println(weight);
  Serial.println(validLoc.lat1);
  Serial.println(output_str);
}

void sendUnlockOutput()
{
  //TODO: Calculate length of output
  //UNLOCK#OWNERID_VOL_WEIGHT
  char output_str[50];
  sprintf(output_str, "UNLOCK#%d_%d_%d", ownerID, volume, weight);
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

int calculateVolume()
{
  int leftPinDistance = getSensorDistance(trigPinLeft, echoPinLeft);
  int rightPinDistance = getSensorDistance(trigPinRight, echoPinRight);
  int currentVol = BIN_LENGTH*BIN_WIDTH*(BIN_HEIGHT-((leftPinDistance + rightPinDistance)/2));
  return currentVol;
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
  return true;
  if ((validLoc.lat1 <= lat) &&
      (lat <= validLoc.lat2) &&
      (validLoc.lon1 <= lon) &&
      (lon <= validLoc.lon2))
  {
    return true;
  }
  return false;
}
