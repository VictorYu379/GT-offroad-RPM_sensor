#include "SD.h"
#include "RTClib.h"

/*
 * We try to keep this reset feature for future use.
 * Once the RF receive the reset command, pin 13 will pull
 * reset pin to GND.
 */
#define RESET_PIN 13


//slows things down, but helpful for debugging
//not recommended for general usage
#define ECHO_TO_SERIAL      true


// for the data logging shield, we use digital pin 10 for the SD cs line
#define CHIP_SELECT_PIN 10


// Read data by analog from the four LVDTs
#define RL_LVDT_PIN 0
#define RR_LVDT_PIN 1
#define FL_LVDT_PIN 2
#define FR_LVDT_PIN 3


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
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
