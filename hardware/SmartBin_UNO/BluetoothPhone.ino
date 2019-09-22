/* References:
> https://www.youtube.com/watch?v=E-1w7dL3Cps
> https://howtomechatronics.com/tutorials/arduino/arduino-and-hc-05-bluetooth-module-tutorial/
> https://www.arduino.cc/en/Tutorial/SoftwareSerialExample
*/

const int BUFFER = 50;
int inChar = -1; //initially, value comes from phone
//char data[BUFFER];
int dataIndex = 0;
String data = "";

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
}


void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) { // Checks whether data is comming from the serial port
    inChar = Serial.read(); // Reads the data from the serial port
    char tempStr[2]={inChar, '\0'};
    data.concat(tempStr);
    Serial.println(data);
    
  }
  
  if (inChar=='0') { // number 48 => '0' in ascii. Phone gives characters only
    digitalWrite(LED_BUILTIN, LOW); // Turn LED OFF
    Serial.println("LED: OFF"); // Send back, to the phone, the String "LED: ON"
    inChar = 0;
    data = "";
  }
  else if (inChar=='1') {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("LED: ON");
    inChar = 0;
    data = "";
  }
  else if (data=="volume") {
    Serial.println("10cm3");
    data = "";
  }
}
