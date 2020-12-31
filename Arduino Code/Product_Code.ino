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
int xJoy = A1;
int yJoy = A0;
int button = 9;
int sensor1 = A3;
int sensor2 = 8;
int tripWire1, tripWire2, buttonState;
void setup() {
  lcd.begin(16, 2);
  pinMode(button, INPUT);
  pinMode(xJoy, INPUT);
  pinMode(yJoy, INPUT);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
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

  lcdPrint("Live Count:" + String(count), 0, 0, true);
  lcdPrint("Max Capacity:" + String(capacity), 0, 1, false);
  delay(500);
  trackCount();
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
  while (digitalRead(button) != 1) {
    int xState = analogRead(xJoy);
    int yState = analogRead(yJoy);
    if (300 < xState < 700 && yState > 700) {
      if (temp < 1000) {
        temp++;
      }
    }
    else if (300 < xState < 700 && yState < 430) {
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
  tripWire1 = 0;
  tripWire2 = 0;
  buttonState = 0;

  while (tripWire1 == 0 && tripWire2 == 0 && buttonState == 0) {
    tripWire1 = digitalRead(sensor1);
    tripWire2 = digitalRead(sensor2);
    buttonState = digitalRead(button);
    delay(50);
  }

  if (buttonState == 1) {
    setupCount();
  }

  else if (tripWire1 == 1) {
    int lastTripped = helperCount(1);
    int countChange = (lastTripped - 1);
    count += countChange;
  }
  else {
    int lastTripped = helperCount(2);
    int countChange = (lastTripped - 2);
    count += countChange;
  }
  
  trackCount();
}

int helperCount(int num) {
  int tripped;
  int trippedWire = 1;
  int checkWire = 0;
  int checkTrip;
  
  if (num == 1) {
    checkTrip = sensor2;
    tripped = sensor1;
  }
  else {
    checkTrip = sensor1;
    tripped = sensor2;
  }
  while (trippedWire == 1 && checkWire == 0 && buttonState == 0){
    trippedWire = digitalRead(tripped);
    checkTrip = digitalRead(checkWire);
    buttonState = digitalRead(button);
    delay(50);
  }

  if(trippedWire == 0){
    if(tripped == sensor1){
      return 2;
    }
    else{
      return 1;
    }
  }
  else if(checkTrip == 1){
    return helperCount(untripCheck()); 
  }

  else{
    setupCount();
  }
}

int untripCheck(){
  int trip1 = 1;
  int trip2 = 1;
  buttonState = 0;
  while (trip1 == 1 && trip2 == 1 && buttonState == 0){
    trip1 = digitalRead(sensor1);
    trip2 = digitalRead(sensor2);
    buttonState = digitalRead(button);
    delay(50);
  }

  if(buttonState == 1){
    setupCount();
  }

  else if(trip1 == 0){
    return 1;
  }
  else{
    return 2;
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
