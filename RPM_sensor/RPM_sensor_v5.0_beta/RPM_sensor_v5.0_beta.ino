#include <SD.h>
#include "RTClib.h"
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>


/*
 * Macro for Multiplexer
 */
#define TCAADDR 0x70
Adafruit_MMA8451 mma[4] = Adafruit_MMA8451();
float accelers[12];


/*
 * We try to keep this reset feature for future use.
 * Once the RF receive the reset command, pin 13 will pull
 * reset pin to GND.
 */
#define RESET_PIN 13


//slows things down, but helpful for debugging
//not recommended for general usage
#define ECHO_TO_SERIAL      false


// for the data logging shield, we use digital pin 10 for the SD cs line
#define CHIP_SELECT_PIN 10


// Read data by analog from the four LVDTs
#define RL_LVDT_PIN 0
#define RR_LVDT_PIN 1
#define FL_LVDT_PIN 2
#define FR_LVDT_PIN 3

int rl;
int rr;
int fl;
int fr;



/*
 * RPM Data Declarations
 * Since we are using the Interrupt feature of Arduino, 
 * we use the Interrupt pins on Mega, which are pin 2 and pin 3.
 */
#define PRIMARY_INPUT_PIN 2
#define SECONDARY_INPUT_PIN 3
#define ARRAY_LENGTH 20
#define SD_RECORDING_THRESHOLD 1000
#define TEETH_ON_PRIMARY 2
#define TEETH_ON_SECONDARY 2

volatile int primaryTimer1;
volatile int primaryTimer2;
volatile int secondaryTimer1;
volatile int secondaryTimer2;
volatile int averagedPrimaryInterval;
volatile int averagedSecondaryInterval;

volatile int primaryIntervals[ARRAY_LENGTH];
volatile int secondaryIntervals[ARRAY_LENGTH];
volatile int primaryArrayIndex = 0;
volatile int primaryArraySum = 0;
volatile int secondaryArrayIndex = 0;
volatile int secondaryArraySum = 0;


// the instance of real time clock, using RTC library
RTC_PCF8523 rtc;


// the logging file
String folderName;
File logfile;
char filename[] = "RPM00.CSV";
int logfileID = 0;


//waits until rpm goes above 500 before it starts recording data
boolean beganRecording = false;


void setup() {
  Serial.begin(9600);
  initializeRPM();
  initializeAcceler();
  initializeSD();
}

void loop() {
  // put your main code here, to run repeatedly:
  accelerLoop();
  LVDTLoop();
  logLoop();
  delay(10);
}





void initializeRPM() {
  Serial.println("Initializing Interrupts...");
  pinMode(PRIMARY_INPUT_PIN, INPUT_PULLUP);
  pinMode(SECONDARY_INPUT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PRIMARY_INPUT_PIN), primaryIncrement, FALLING);
  attachInterrupt(digitalPinToInterrupt(SECONDARY_INPUT_PIN), secondaryIncrement, FALLING);
  primaryTimer1 = millis();
  secondaryTimer1 = millis();
  Serial.println("Initialized Interrupts!");
}

void initializeAcceler() {
  Wire.begin();
  Serial.println("Initializing accelerators...");

  for(int i = 0; i < 4; i++) {
    tcaselect(i);
    if (! mma[i].begin()) {
      Serial.println("Couldn't start");
      while (1);
    }
    Serial.println("MMA8451 found!");
  
    mma[i].setRange(MMA8451_RANGE_2_G);
  
    Serial.print("Range = "); Serial.print(2 << mma[i].getRange());  
    Serial.println("G");  
  }
  Serial.println("Initialized accelerators!");
}


void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}


void initializeSD() {
  
  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(CHIP_SELECT_PIN, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  folderName = setFolder();
  setLogfile(folderName);
  
  if (!logfile) {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  //initialize csv file columns
  logfile.print("primary,secondary,RL,RR,FL,FR,rl_x,rl_y,rl_z,rr_x,rr_y,rr_z,fl_x,fl_y,fl_z,fr_x,fr_y,fr_z,time");
  logfile.println();

  if (ECHO_TO_SERIAL) {
    Serial.println("primary,secondary,RL,RR,FL,FR,rl_x,rl_y,rl_z,rr_x,rr_y,rr_z,fl_x,fl_y,fl_z,fr_x,fr_y,fr_z,time");
  }
}


String setFolder() {
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  DateTime now = rtc.now();
  String nian = String(now.year());
  String yue = String(now.month());
  String ri = String(now.day());
  Serial.println(nian + yue + ri);
  int start = 1;
  while (SD.exists(nian + yue + ri + "-" + String(start))) {
    start++;
  }
  SD.mkdir(nian + yue + ri + "-" + String(start));
  return nian + yue + ri + "-" + String(start);
}


void setLogfile(String folderName) {
  // create a new file
  for (uint8_t i = logfileID; i < 100; i++) {
    filename[3] = i / 10 + '0';
    filename[4] = i % 10 + '0';
    if (! SD.exists(String(folderName) + "/" + filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(String(folderName) + "/" + filename, FILE_WRITE);
      logfileID = i;
      break;  // leave the loop!
    }
  }

  if (!logfile) {
    error("couldnt create file");
  }
}


void error(String error) {
  Serial.println(error);
  while(true) delay(1);
}






void primaryIncrement() {
  primaryTimer2 = millis();
  primaryArrayIndex = (++primaryArrayIndex) % ARRAY_LENGTH;
  primaryIntervals[primaryArrayIndex] = primaryTimer2 - primaryTimer1;
  primaryTimer1 = primaryTimer2;
  for (int i = 0; i < ARRAY_LENGTH; i++) {
    primaryArraySum += primaryIntervals[i];
  }
  averagedPrimaryInterval = 1000000 / (16 * TEETH_ON_PRIMARY * (primaryArraySum / ARRAY_LENGTH));
  //Serial.println(averagedPrimaryInterval);
  primaryArraySum = 0;
}


void secondaryIncrement() {
  secondaryTimer2 = millis();
  secondaryArrayIndex = (++secondaryArrayIndex) % ARRAY_LENGTH;
  secondaryIntervals[secondaryArrayIndex] = secondaryTimer2 - secondaryTimer1;
  secondaryTimer1 = secondaryTimer2;
  for (int i = 0; i < ARRAY_LENGTH; i++) {
    secondaryArraySum += secondaryIntervals[i];
  }
  averagedSecondaryInterval = 1000000 / (16 * TEETH_ON_SECONDARY * (secondaryArraySum / ARRAY_LENGTH));
  //Serial.println(averagedSecondaryInterval);
  secondaryArraySum = 0;
}


void accelerLoop() {
  int loopnum = 0;
  while (loopnum < 4) {
    mma[loopnum].read();
    sensors_event_t event; 
    mma[loopnum].getEvent(&event);
    accelers[loopnum * 3] = event.acceleration.x;
    accelers[loopnum * 3 + 1] = event.acceleration.y;
    accelers[loopnum * 3 + 2] = event.acceleration.z;
    loopnum++;
  }
}


void LVDTLoop() {
  rl = analogRead(RL_LVDT_PIN);
  rr = analogRead(RR_LVDT_PIN);
  fl = analogRead(FL_LVDT_PIN);
  fr = analogRead(FR_LVDT_PIN);
}


void logLoop() {
  if (beganRecording) {
    if (logfile.size() > 52428800) {
      setLogfile(folderName);
    }
    logfile.print(averagedPrimaryInterval);
    logfile.print(",");
    logfile.print(averagedSecondaryInterval);
    logfile.print(",");
    logfile.print(rl);
    logfile.print(",");
    logfile.print(rr);
    logfile.print(",");
    logfile.print(fl);
    logfile.print(",");
    logfile.print(fr);
    logfile.print(",");
    int accelersIndex = 0;
    while (accelersIndex < 12) {
      logfile.print(accelers[accelersIndex]);
      logfile.print(",");
      accelersIndex++;
    }
    logfile.print(millis());
    logfile.println();
    logfile.flush();  
  } else {
    if (averagedPrimaryInterval > SD_RECORDING_THRESHOLD || averagedSecondaryInterval > SD_RECORDING_THRESHOLD) {
      beganRecording = true;
    }
  }

  
  if (ECHO_TO_SERIAL) {
    Serial.print(averagedPrimaryInterval);
    Serial.print(",");
    Serial.print(averagedSecondaryInterval);
    Serial.print(",");
    Serial.print(rl);
    Serial.print(",");
    Serial.print(rr);
    Serial.print(",");
    Serial.print(fl);
    Serial.print(",");
    Serial.print(fr);
    Serial.print(",");
    int accelersIndex = 0;
    while (accelersIndex < 12) {
      Serial.print(accelers[accelersIndex]);
      Serial.print(",");
      accelersIndex++;
    }
    Serial.print(millis());
    Serial.println();
  }
}

