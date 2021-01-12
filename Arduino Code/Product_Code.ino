#include "WiFiEsp.h"
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiEspClient  client;

#include "SoftwareSerial.h"
#include <LiquidCrystal.h>
SoftwareSerial Serial1(6, 7); // RX, TX
#define ESP_BAUDRATE  19200

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int count;
int capacity;
int touchSensor = A4;
int piezo = 10;
int sensor1 = 9;
int sensor2 = 8;
int redPin = A3;
int greenPin = A2;
int xJoy = A1;
int yJoy = A0;
int tripWire1, tripWire2, touchState;
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
    // don't continue
    while (true);
  }
  lcdPrint(" Module Located ", 0, 0, true);

  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  //
  setupCount();
  delay(20000); // Wait 20 seconds to update the channel again
}

void setupCount() {
  lcdPrint("Setup Count and", 0, 0, true);
  lcdPrint("Maximum Capacity", 0, 1, false);
  delay(2500);

  lcdPrint("Live Count:" + String(count), 0, 0, true);
  lcdPrint("Max Capacity:" + String(capacity), 0, 1, false);

  count = changeCount(count, 0, 11, 0);
  delay(500);
  capacity = changeCount(capacity, count, 13, 1);

  lcdPrint("     Setup     ", 0, 0, true);
  lcdPrint("    Complete   ", 0, 1, false);
  delay(1500);

  laserSetup();
}

void laserSetup() {
  lcdPrint("Set Laser", 3, 0, true);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  int sensorState1 = 0;
  int sensorState2 = 0;
  delay(500);
  while (sensorState1 == 0 || sensorState2 == 0) {
    sensorState1 = digitalRead(sensor1);
    sensorState2 = digitalRead(sensor2);

    if (sensorState1 == 1 || sensorState2 == 1) {
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
  int touchState = 0;
  while (sensorState1 == 1 && sensorState2 == 1 && touchState == 0) {
    sensorState1 = digitalRead(sensor1);
    sensorState2 = digitalRead(sensor2);
    touchState = digitalRead(touchSensor);
    digitalWrite(redPin, LOW);
    delay(150);
    digitalWrite(greenPin, HIGH);
  }

  int x = 0;
  while (sensorState1 == 1 && sensorState2 == 1 && x < 20) {
    sensorState1 = digitalRead(sensor1);
    sensorState2 = digitalRead(sensor2);
    lcdPrint("Both Lasers", 2, 0, true);
    lcdPrint("Set", 7, 1, false);
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
    lcdPrint("Laser Setup", 1, 0, true);
    lcdPrint("Successful", 3, 2, false);
    delay(2000);
    trackCount();
  }
}

void lcdPrint(String value, int cx, int cy, bool lcdClear) {
  if (lcdClear == true) {
    lcd.clear();
  }
  lcd.setCursor(cx, cy);
  lcd.print(value);
}

int changeCount(int temp, int minVal, int cx, int cy) {

  if (temp < minVal) {
    temp = minVal;
  }
  while (digitalRead(touchSensor) != 1) {
    int xState = analogRead(xJoy);
    int yState = analogRead(yJoy);
    if (300 < yState < 700 && xState < 430) {
      if (temp < 1000) {
        temp++;
      }
    }
    else if (300 < yState < 700 && xState > 700) {
      if (temp > minVal) {
        temp--;
      }
    }
    lcdPrint(String(temp) + "   ", cx, cy, false);
    delay(175);
  }

  return temp;

}


void trackCount() {
  tripWire1 = 1;
  tripWire2 = 1;
  touchState = 0;
  lcdPrint("Live Count:" + String(count), 0, 0, true);
  lcdPrint("Max Capacity:" + String(capacity), 0, 1, false);
  
  while (tripWire1 == 1 && tripWire2 == 1 && touchState == 0) {
    tripWire1 = digitalRead(sensor1);
    tripWire2 = digitalRead(sensor2);
    touchState = digitalRead(touchSensor);
    delay(100);
  }

  if (touchState == 1) {
    setupCount();
  }

  else {
    int firstTripped;
    if (tripWire1 == 0) {
      firstTripped = 1;
    }
    else {
      firstTripped = 2;
    }
    int temp = trackCount1(tripWire1, tripWire2);
    count += (firstTripped - temp);
  }
  trackCount();
}

int trackCount1(int t1, int t2) {
  int initialState1 = t1;
  int initialState2 = t2;

  while (t1 == initialState1 && t2 == initialState2 && touchState == 0) {
    t1 = digitalRead(sensor1);
    t2 = digitalRead(sensor2);
    touchState = digitalRead(touchSensor);
    delay(100);
    if(t1 == 0 && t2 == 0){
      trackCount2();
      initialState1 = digitalRead(sensor1);
      initialState2 = digitalRead(sensor2);
      t1 = initialState1;
      t2 = initialState2;
    }
  }

  if (touchState == 1) {
    setupCount();
  }
  else if (t1 - initialState1 == 1) {
    return 1;
  }
  else {
    return 2;
  }

}


void trackCount2() {
  tripWire1 = 0;
  tripWire2 = 0;

  while (tripWire1 == 0 && tripWire2 == 0 && touchState == 0) {
    touchState = digitalRead(touchSensor);
    tripWire1 = digitalRead(sensor1);
    tripWire2 = digitalRead(sensor2);
    delay(100);
  }

  if (touchState == 1) {
    setupCount();
  }
}



void uploadData() {
  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    //    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network.
      delay(5000); // MAKE MY OWN DELAY

    }
  }

  // set the fields with the values
  ThingSpeak.setField(1, count);
  ThingSpeak.setField(2, capacity);

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
}





/* This function attempts to set the ESP8266 baudrate to 19200 because software
  serial ports are limited to 19200 baudrate */

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
