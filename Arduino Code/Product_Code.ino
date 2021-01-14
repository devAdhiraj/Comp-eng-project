#include "WiFiEsp.h"
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
WiFiEspClient  client;

#include "SoftwareSerial.h"
#include <LiquidCrystal.h>
SoftwareSerial Serial1(6, 7); // RX, TX
#define ESP_BAUDRATE  19200

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int touchSensor = A4;
int piezo = 10;
int sensor1 = 8;
int sensor2 = 9;
int redPin = A2;
int greenPin = A3;
int xJoy = A1;
int yJoy = A0;
int tripWire1, tripWire2, touchState, initialState1, initialState2;
int count, capacity, trackTime, firstTripped;


void setup() {

  lcd.begin(16, 2);
  pinMode(touchSensor, INPUT);
  pinMode(xJoy, INPUT);
  pinMode(yJoy, INPUT);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(piezo, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  // initialize serial for ESP module
  setEspBaudRate(ESP_BAUDRATE);
  lcdPrint(" Searching for ", 0, 0, true);
  lcdPrint("   ESP8266...   ", 0, 1, false);


  // initialize ESP module
  WiFi.init(&Serial1);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    lcdPrint(" Error! Module ", 0, 0, true);
    lcdPrint("    Not Found   ", 0, 1, false);
    // don't continue - reboot required
    while (true);
  }
  lcdPrint(" Module Located ", 0, 0, true);

  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  setupCount();
}


/*
   The setupCount function runs when the device starts and whenever
   the touchSensor is pressed. This function is used to set the number of
   people in the room manually. For example, to start with if there are already 10
   people in the room, then this function allows the user to set the count to 10.
   This function is also used to set the maximum capacity of the room which is how
   many people can be in the room while still maintaining physical distancing.
*/
void setupCount() {
  lcdPrint("Setup Count and", 0, 0, true);
  lcdPrint("Maximum Capacity", 0, 1, false);
  delay(2500);

  lcdPrint("Live Count:" + String(count), 0, 0, true);
  lcdPrint("Max Capacity:" + String(capacity), 0, 1, false);

  count = changeCount(count, 0, 11, 0);
  capacity = changeCount(capacity, count, 13, 1);
  lcdPrint("     Setup     ", 0, 0, true);
  lcdPrint("    Complete   ", 0, 1, false);
  for (int i = 0; i < 2; i++) {
    tone(piezo, 700);
    delay(125);
    noTone(piezo);
    delay(125);
  }
  delay(1500);
  uploadData();
  laserSetup();
}


/*
   The laserSetup function runs after the setupCount function.
   This function basically makes sure that the lasers are set up
   correctly.
*/
void laserSetup() {
  for (int i = 0; i < 2; i++) {
    tone(piezo, 700);
    delay(125);
    noTone(piezo);
    delay(125);
  }

  lcdPrint("Set Lasers", 3, 0, true);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  delay(500);

  while (tripWire1 == 0 || tripWire2 == 0) {
    tripWire1 = digitalRead(sensor1);
    tripWire2 = digitalRead(sensor2);

    if (tripWire1 == 1 || tripWire2 == 1) {
      tone(piezo, 500);
      digitalWrite(redPin, HIGH);
      delay(500);
      digitalWrite(redPin, LOW);
      noTone(piezo);
      delay(500);
    }
    else {
      delay(1500);
    }
  }

  while (tripWire1 == 1 && tripWire2 == 1 && touchState == 0) {
    tripWire1 = digitalRead(sensor1);
    tripWire2 = digitalRead(sensor2);
    touchState = digitalRead(touchSensor);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    delay(150);
  }

  for (int i = 0; i < 2; i++) {
    tone(piezo, 700);
    delay(125);
    noTone(piezo);
    delay(125);
  }

  int x = 0;
  lcdPrint("Both Lasers", 3, 0, true);
  lcdPrint("Set", 7, 1, false);

  while (tripWire1 == 1 && tripWire2 == 1 && x < 20) {
    tripWire1 = digitalRead(sensor1);
    tripWire2 = digitalRead(sensor2);
    delay(100);
    x++;
  }

  if (x < 20) {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    lcdPrint("Laser Setup", 1, 0, true);
    lcdPrint("Failed", 5, 1, false);
    delay(2000);
    laserSetup();
  }

  else {
    lcdPrint("Laser Setup", 2, 0, true);
    lcdPrint("Successful", 3, 2, false);
    delay(2000);
    trackCount();
  }

}


/*
   The lcdPrint function runs whenever something needs to be printed on the lcd.
   Since printing on the lcd was a common thing that needed to be done several
   times during the code, instead of writing all the commands seperately like
   lcd.clear(), lcd.setCursor, etc. now you can just make a function call in
   one line that which makes the code more organized and efficient.
*/
void lcdPrint(String value, int cx, int cy, bool lcdClear) {
  if (lcdClear == true) {
    lcd.clear();
  }
  lcd.setCursor(cx, cy);
  lcd.print(value);
}


/*
   The changeCount function is called when the live count or max capacity
   needs to be changed manually (using the joystick). Since changing max
   capacity and live count involves very similar code, this function was
   made in a generic way so it can be used for both tasks. By generic,
   I mean it uses general variables which can be initialized in the function
   call depending on whether the live count needs to be changed or the max capacity.
*/
int changeCount(int tempVar, int minVal, int cx, int cy) {
  for (int i = 0; i < 2; i++) {
    tone(piezo, 700);
    delay(125);
    noTone(piezo);
    delay(125);
  }
  if (tempVar < minVal) {
    tempVar = minVal;
  }
  while (digitalRead(touchSensor) != 1) {
    int xState = analogRead(xJoy);
    int yState = analogRead(yJoy);
    if (300 < yState < 700 && xState < 430) {
      if (tempVar < 1000) {
        tempVar++;
      }
    }
    else if (300 < yState < 700 && xState > 700) {
      if (tempVar > minVal) {
        tempVar--;
      }
    }
    lcdPrint(String(tempVar) + "   ", cx, cy, false);
    delay(175);
  }

  return tempVar;

}


/*
   The trackCount function is the main function that runs for most of the time.
   The goal of this is to keep track of the number of people in a room by increasing
   or decreasing the count whenever someone walks in or out of the room.
*/
void trackCount() {
  tripWire1 = 1;
  tripWire2 = 1;
  touchState = 0;
  noTone(piezo);

  if (count > capacity) {
    tone(piezo, 700);
  }
  else if (count == capacity) {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    noTone(piezo);
  }
  else {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    noTone(piezo);
  }

  if (trackTime >= 500) {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    uploadData();
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
  }

  while (tripWire1 == 1 && tripWire2 == 1 && touchState == 0) {
    readSensors();
  }
  tone(piezo, 650);

  if (touchState == 1) {
    setupCount();
  }

  else {

    if (tripWire1 == 0) {
      firstTripped = 1;
    }
    else {
      firstTripped = 2;
    }
    count += (firstTripped - trackCount1());
    if (count < 0) {
      count = 0;
    }
  }
  trackCount();
}


/*
   The trackCount1 function runs when one of the wires are tripped.
   This function's main goal is to return which wire got untripped last.
   As that would allow the trackCount function to determine how the count will
   change. This function records the state of both the tripWires initially,
   and then if both the wires get tripped then it goes to trackCount2, but if
   a wire gets untripped, then it returns which wire got untripped last.
*/
int trackCount1() {
  initialState1 = tripWire1;
  initialState2 = tripWire2;

  while (tripWire1 == initialState1 && tripWire2 == initialState2 && touchState == 0) {
    readSensors();

    if (tripWire1 == 0 && tripWire2 == 0) {
      trackCount2();
      initialState1 = tripWire1;
      initialState2 = tripWire2;
    }
  }

  if (touchState == 1) {
    setupCount();
  }
  else if (tripWire1 - initialState1 == 1) {
    return 1;
  }
  else {
    return 2;
  }

}


/*
   trackCount2 function runs when both the wires are tripped.
   This function just waits for one of the wires to get untripped
   and then returns back to trackCount1.
*/
void trackCount2() {
  tripWire1 = 0;
  tripWire2 = 0;

  while (tripWire1 == 0 && tripWire2 == 0 && touchState == 0) {
    readSensors();
  }

  if (touchState == 1) {
    setupCount();
  }
}


/*
   The readSensors function is used to make the code more efficient,
   instead of writing the same piece of code in multiple places, this
   function can be called to do the same job. It essentially reads
   the states of the two Laser Sensors and the touch Sensor and it also
   updates the values of count and capacity on the LCD every 455 milliseconds.
*/
void readSensors() {
  tripWire1 = digitalRead(sensor1);
  tripWire2 = digitalRead(sensor2);
  touchState = digitalRead(touchSensor);
  delay(65);
  trackTime++;
  if (trackTime % 7 == 0) {
    lcdPrint("Live Count:" + String(count) + "  ", 0, 0, true);
    lcdPrint("Max Capacity:" + String(capacity) + "  ", 0, 1, false);
  }
}


/*
   The uploadData function is where the wifi module is connected to wifi
   and then it makes an HTTP call to the Thingspeak server using the
   provided API key. The HTTP call is to write or POST data to the server
   which is how it becomes visible on the Thingspeak website.
*/

void uploadData() {
  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    lcdPrint(" Connecting....", 0, 0, true);
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      delay(3000);
    }
  }

  // set the fields with the values
  ThingSpeak.setField(1, count);
  ThingSpeak.setField(2, capacity);

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) {
    trackTime = 0;
  }
  else {
    trackTime = 400;
  }
  //  trackCount();
}


/*
  This function attempts to set the ESP8266 baudrate to 19200 because software
  serial ports are limited to 19200 baudrate.
*/
void setEspBaudRate(unsigned long baudrate) {
  long rates[6] = {115200, 74880, 57600, 38400, 19200, 9600};

  lcdPrint("Initializing...", 0, 0, true);

  for (int i = 0; i < 6; i++) {
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);
  }

  Serial1.begin(baudrate);
}
