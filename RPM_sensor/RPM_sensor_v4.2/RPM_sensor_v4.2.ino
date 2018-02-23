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

/*
 * RPM Data Declarations
 */
const int primaryInputPin = 3;
const int secondaryInputPin = 2;

volatile int primaryState = HIGH;
volatile int secondaryState = HIGH;

volatile uint32_t primaryTimer1;
volatile uint32_t primaryTimer2;
volatile uint32_t secondaryTimer1;
volatile uint32_t secondaryTimer2;
uint32_t averagedPrimaryInterval;
uint32_t averagedSecondaryInterval;

volatile uint32_t primaryIntervals[50];
volatile uint32_t secondaryIntervals[50];
volatile int primaryArrayIndex = 0;
volatile int primaryArraySum = 0;
volatile int secondaryArrayIndex = 0;
volatile int secondaryArraySum = 0;

const int magnetsOnPrimary = 1;
const int magnetsOnSecondary = 1;

const int resetPin = 10;

/*
 * SD Card Declarations
 */
// the digital pins that connect to the LEDs
#define redLEDpin 3
#define greenLEDpin 4

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

//line counter
int line;

// the logging file
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

  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

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

  setLogfile();
  
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
      setLogfile();
    }
        
    logfile.print(averagedPrimaryInterval);
    logfile.print(",");
    logfile.print(averagedSecondaryInterval);
    logfile.print(",");
    logfile.print(millis());
    logfile.println();
    logfile.flush();  
  } else {
    if (averagedPrimaryInterval > 500 && averagedSecondaryInterval > 500) {
      beganRecording = true;
    }
  }

  
  if (ECHO_TO_SERIAL) {
    Serial.print(averagedPrimaryInterval);
    Serial.print(",");
    Serial.print(averagedSecondaryInterval);
    Serial.print(",");
    Serial.print(millis());
    Serial.println();
  }
  
}

void primaryIncrement() {
//  Serial.println("haha");/
  primaryTimer2 = millis();
  primaryArrayIndex = (++primaryArrayIndex) % 50;
  primaryIntervals[primaryArrayIndex] = primaryTimer2 - primaryTimer1;
  primaryTimer1 = primaryTimer2;
  for (int i = 0; i < 50; i++) {
    primaryArraySum += primaryIntervals[i];
  }
  averagedPrimaryInterval = 1000000 / (16 * (primaryArraySum / 8));
  //Serial.println(averagedPrimaryInterval);
  primaryArraySum = 0;
}


void secondaryIncrement() {
//  Serial.println("ahah");/
  secondaryTimer2 = millis();
  secondaryArrayIndex = (++secondaryArrayIndex) % 50;
  secondaryIntervals[secondaryArrayIndex] = secondaryTimer2 - secondaryTimer1;
  secondaryTimer1 = secondaryTimer2;
  for (int i = 0; i < 50; i++) {
    secondaryArraySum += secondaryIntervals[i];
  }
  averagedSecondaryInterval = 1000000 / (16 * (secondaryArraySum / 8));
  //Serial.println(averagedSecondaryInterval);
  secondaryArraySum = 0;
}

void error(String error) {
  Serial.println(error);
  while(true) delay(1);
}

void setLogfile() {
  
  // create a new file
  for (uint8_t i = logfileID; i < 100; i++) {
    filename[3] = i / 10 + '0';
    filename[4] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      logfileID = i;
      break;  // leave the loop!
    }
  }

  if (!logfile) {
    error("couldnt create file");
  }
}
