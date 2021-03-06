#include "SD.h"

/*
 * RPM Data Declarations
 */
const int primaryInputPin = 2;
const int secondaryInputPin = 3;

volatile int primaryState = HIGH;
volatile int secondaryState = HIGH;

volatile uint32_t primaryTimer1;
volatile uint32_t primaryTimer2;
volatile uint32_t secondaryTimer1;
volatile uint32_t secondaryTimer2;
volatile uint32_t averagedPrimaryInterval;
volatile uint32_t averagedSecondaryInterval;

volatile uint32_t primaryIntervals[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile uint32_t secondaryIntervals[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile int primaryArrayIndex = 0;
volatile int primaryArraySum = 0;
volatile int secondaryArrayIndex = 0;
volatile int secondaryArraySum = 0;

const int magnetsOnPrimary = 2;
const int magnetsOnSecondary = 2;

/*
 * SD Card Declarations
 */

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

//line counter
int line;

// the logging file
File logfile;

//slows things down, but helpful for debugging
//not recommended for general usage
#define ECHO_TO_SERIAL      true

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

  // create a new file with a unique name
  char filename[] = "RPM00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[3] = i / 10 + '0';
    filename[4] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }
  
  if (!logfile) {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  //initialize csv file columns

  logfile.print("primary,secondary");
  logfile.println();

  if (ECHO_TO_SERIAL) {
    Serial.println("primary,secondary");
  }

  
}

void loop() {
}

void primaryIncrement() {
  primaryTimer2 = millis();
  primaryArrayIndex = (++primaryArrayIndex) % 8;
  primaryIntervals[primaryArrayIndex] = primaryTimer2 - primaryTimer1;
  primaryTimer1 = primaryTimer2;
  for (int i = 0; i < 8; i++) {
    primaryArraySum += primaryIntervals[i] * 0.0277 * (i + 1);
  }
  averagedPrimaryInterval = 1000000 / (16 * primaryArraySum * magnetsOnPrimary);
  //Serial.println(averagedPrimaryInterval);
  primaryArraySum = 0;

  if (averagedPrimaryInterval < 50000 || averagedSecondaryInterval < 50000) {
    logfile.print(averagedPrimaryInterval);
    logfile.print(",");
    logfile.print(averagedSecondaryInterval);
    logfile.println();
    logfile.flush(); 
  }

  if (ECHO_TO_SERIAL) {
    Serial.print(averagedPrimaryInterval);
    Serial.print(",");
    Serial.print(averagedSecondaryInterval);
    Serial.println();
  }
}


void secondaryIncrement() {
  secondaryTimer2 = millis();
  secondaryArrayIndex = (++secondaryArrayIndex) % 8;
  secondaryIntervals[secondaryArrayIndex] = secondaryTimer2 - secondaryTimer1;
  secondaryTimer1 = secondaryTimer2;
  for (int i = 0; i < 8; i++) {
    secondaryArraySum += secondaryIntervals[i] * 0.0277 * (i + 1);
  }
  averagedSecondaryInterval = 1000000 / (16 * secondaryArraySum * magnetsOnSecondary);
  //Serial.println(averagedSecondaryInterval);
  secondaryArraySum = 0;

  if (averagedPrimaryInterval < 50000 || averagedSecondaryInterval < 50000) {
    logfile.print(averagedPrimaryInterval);
    logfile.print(",");
    logfile.print(averagedSecondaryInterval);
    logfile.println();
    logfile.flush(); 
  }

  if (ECHO_TO_SERIAL) {
    Serial.print(averagedPrimaryInterval);
    Serial.print(",");
    Serial.print(averagedSecondaryInterval);
    Serial.println();
  } 
}

void error(String error) {
  Serial.println(error);
  while(true) delay(1);
}

