struct GeoFence {
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

bool transporterScanned = false;
bool facilityScanned = false;

int weight1 = 0;
int weight2 = 0;
int volume = 0;

void setup() {
}

void loop() {
  if(isLocked){
    if(isFacility(scannerID) && !facilityScanned && scannerID == facilityID && isWithinLoc(lat, lon) && calculateVolume() == volume){
      facilityScanned = true;
    }
    if(isTransporter(scannerID) && !transporterScanned && scannerID == transporterID && isWithinLoc(lat, lon) && calculateVolume() == volume){
      transporterScanned = true;
    }
    if(transporterScanned && facilityScanned){
      //TODO: Open lock
      isLocked = true;
      ownerID = facilityID;
      transporterScanned = facilityScanned = false;
    }
  } else {
    if(isFacility(scannerID) && !facilityScanned && scannerID == facilityID && isWithinLoc(lat, lon)){
      facilityScanned = true;
    }
    if(isTransporter(scannerID) && !transporterScanned && scannerID == transporterID && isWithinLoc(lat, lon)){
      transporterScanned = true;
    }
    if(transporterScanned && facilityScanned && weight1==weight2){
      //TODO: Open lock
      isLocked = false;
      ownerID = transporterID;
      transporterScanned = facilityScanned = false;
      volume = calculateVolume();
      //TODO: Change validLoc to destination
      changeValidLoc(0.0, 0.0, 0.0, 0.0);
    }
  }
  //IF LOCKED - Trying to unlock
    //Check if scanner is authorised and location is valid
    //Check if volume matches original volume
    //Do the same for 2nd user
    //Transfer ownership to 2nd user
  //IF OPEN - Trying to lock
    //Check if scanner is authorised and location is valid
    //Check if entered weight and vol matches those entered by 2nd user
    //Repeat with 2nd user
    //Set validLoc as destination
    //Set owner as 2nd user
}

// TODO: Find if scanner ID is that of a transporter
bool isTransporter(int ID) {
  return false;
}

// TODO: Find if scanner ID is that of a facility
bool isFacility(int ID) {
  return false;
}

// TODO: Use ultrasonic sensor data to calculate volume
int calculateVolume() {
  return 0;
}

//Change who owns the bin currently
void changeOwnerID(int ID){
  ownerID = ID;
}

//Change destination where bin can be locked or unlocked
void changeValidLoc(float lat1, float lat2, float lon1, float lon2){
  validLoc.lat1 = lat1;
  validLoc.lat2 = lat2;
  validLoc.lon1 = lon1;
  validLoc.lon2 = lon2;
}

//Check if the scanning location is within ge-fence that is allowed
bool isWithinLoc(float lat, float lon){
  if((validLoc.lat1 <= lat) && 
      (lat <= validLoc.lat2) &&
      (validLoc.lon1 <= lon) &&
      (lon <= validLoc.lon2)){
    return true;
  } else {
    return false;
  }
}
