/**
 * 
 * DIFFERENCE BETWEEN THIS AND 4.0/4.1 :
 * -Does not record data to the SD card every time it takes a reading, it only
 * does it every time the program loops
 * -Waits until both the primary and secondary CVT RPM is >500 before it start to record data
 * -If the size of the logfile is greater than 10 megabytes, it should create a new file with a 
 * new number (ex. RPM15.CSV is >10MB, RPM16.CSV is created and logging begins there)
 * 
 * NOTE: To use the bluetooth device, you should only have to print to the default serial port
 * (Serial) because of how the SD card shield is soldered
 */
#include "SD.h"
#include "RTClib.h"

/*
 * RPM Data Declarations
 */
const int primaryInputPin = 2;
const int secondaryInputPin = 3;

volatile int primaryState = HIGH;
volatile int secondaryState = HIGH;

volatile int primaryTimer1;
volatile int primaryTimer2;
volatile int secondaryTimer1;
volatile int secondaryTimer2;
volatile int averagedPrimaryInterval;
volatile int averagedSecondaryInterval;

const int arrayLength = 20;
const int SDRecordingThreshold = 1000;

volatile int primaryIntervals[arrayLength];
volatile int secondaryIntervals[arrayLength];
volatile int primaryArrayIndex = 0;
volatile int primaryArraySum = 0;
volatile int secondaryArrayIndex = 0;
volatile int secondaryArraySum = 0;

const int teethOnPrimary = 2;
const int teethOnSecondary = 2;

const int resetPin = 10;

// the instance of real time clock, using RTC library
RTC_PCF8523 rtc;

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

//line counter
int line;

// the logging file
String folderName;
File logfile;
char filename[] = "RPM00.CSV";
int logfileID = 0;

//slows things down, but helpful for debugging
//not recommended for general usage
#define ECHO_TO_SERIAL      true

//waits until rpm goes above 500 before it starts recording data
boolean beganRecording = false;


void setup() {
  // put your setup code here, to run once:
  pinMode(primaryInputPin, INPUT_PULLUP);
  pinMode(secondaryInputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(primaryInputPin), primaryIncrement, FALLING);
  attachInterrupt(digitalPinToInterrupt(secondaryInputPin), secondaryIncrement, FALLING);
  primaryTimer1 = millis();
  secondaryTimer1 = millis();
  Serial.begin(9600);
  Serial.println("Initialized Interrupts");

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
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
  logfile.print("primary,secondary,time");
  logfile.println();

  if (ECHO_TO_SERIAL) {
    Serial.println("primary,secondary,time");
  }

  
}

void loop() {
  if (beganRecording) {
    if (logfile.size() > 10485760) {
      setLogfile(folderName);
    }
        
    logfile.print(averagedPrimaryInterval);
    logfile.print(",");
    logfile.print(averagedSecondaryInterval);
    logfile.print(",");
    logfile.print(millis());
    logfile.println();
    logfile.flush();  
  } else {
    if (averagedPrimaryInterval > SDRecordingThreshold || averagedSecondaryInterval > SDRecordingThreshold) {
      beganRecording = true;
    }
  }

  
  if (ECHO_TO_SERIAL) {
    Serial.print(averagedPrimaryInterval);
//    Serial.print(",");
//    Serial.print(averagedSecondaryInterval);
//    Serial.print(",");
//    Serial.print(millis());
    Serial.println();
  }
  delay(10);
}

void primaryIncrement() {
//  Serial.println("haha");
  primaryTimer2 = millis();
  primaryArrayIndex = (++primaryArrayIndex) % arrayLength;
  primaryIntervals[primaryArrayIndex] = primaryTimer2 - primaryTimer1;
  primaryTimer1 = primaryTimer2;
  for (int i = 0; i < arrayLength ; i++) {
    primaryArraySum += primaryIntervals[i];
  }
  averagedPrimaryInterval = 1000000 / (16 * teethOnPrimary * (primaryArraySum / arrayLength));
  //Serial.println(averagedPrimaryInterval);
  primaryArraySum = 0;
}


void secondaryIncrement() {
//  Serial.println("ahah");
  secondaryTimer2 = millis();
  secondaryArrayIndex = (++secondaryArrayIndex) % arrayLength;
  secondaryIntervals[secondaryArrayIndex] = secondaryTimer2 - secondaryTimer1;
  secondaryTimer1 = secondaryTimer2;
  for (int i = 0; i < arrayLength; i++) {
    secondaryArraySum += secondaryIntervals[i];
  }
  averagedSecondaryInterval = 1000000 / (16 * teethOnSecondary * (secondaryArraySum / arrayLength));
  //Serial.println(averagedSecondaryInterval);
  secondaryArraySum = 0;
}

void error(String error) {
  Serial.println(error);
  while(true) delay(1);
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
