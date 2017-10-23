/** @author Jingkai Yu
 *  @version 2.0 beta2
 */


int primaryRead = 13;
int secondaryRead = 8;
uint32_t primaryTimer1;
uint32_t primaryTimer2;
uint32_t secondaryTimer1;
uint32_t secondaryTimer2;
uint32_t averagedPrimaryInterval;
uint32_t averagedSecondaryInterval;
uint32_t primaryIntervals[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t secondaryIntervals[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int primaryArrayIndex = 0;
int primaryArraySum = 0;
int secondaryArrayIndex = 0;
int secondaryArraySum = 0;
boolean primaryLow = false;
boolean secondaryLow = false;
int magnetsOnPrimary = 2;
int magnetsOnSecondary = 2;
int resetPin = 10;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(primaryRead, INPUT);
  pinMode(secondaryRead, INPUT_PULLUP);
  pinMode(resetPin, OUTPUT);

  // Initialize the timers
  primaryTimer1 = millis();
  primaryTimer2 = millis();
  secondaryTimer1 = millis();
  secondaryTimer2 = millis();
  
  // Print to the serial monitor to signal the start
  Serial.println("started");
}


void loop() {

  // Once approaching of the magnet is detected by primary, go into this logic.
  if (primaryLow) {

    // Once the magnet is leaving the primary, go into the logic.
    primaryLow = (digitalRead(primaryRead) == LOW);
    if (!primaryLow) {

      // Make the array circular
      primaryArrayIndex = (++primaryArrayIndex) % 10;
      
      // Record the time spot of this time and calculate the time difference
      primaryTimer1 = millis();
      primaryIntervals[primaryArrayIndex] = primaryTimer1 - primaryTimer2;
      primaryTimer2 = primaryTimer1;
      
      // Calculate the average of the array to reduce the noise and error
      for (int i = 0; i < 10; i++) {
        primaryArraySum += primaryIntervals[i];
      }
      averagedPrimaryInterval = primaryArraySum / 10;
      
      // Transfer the time difference to RPM (here we putting 2 magnets on the primary CVT)
      averagedPrimaryInterval = 1000000 / (16 * averagedPrimaryInterval);
      Serial.print(averagedPrimaryInterval);
      Serial.println(" ");

      // Reset the summation of the array.
      primaryArraySum = 0;
    }
  }

  // Code for Secondary is the same as Primary
  if (secondaryLow) {
    secondaryLow = (digitalRead(secondaryRead) == LOW);
    if (!secondaryLow) {
      secondaryArrayIndex = (++secondaryArrayIndex) % 10;
      secondaryTimer1 = millis();
      secondaryIntervals[secondaryArrayIndex] = secondaryTimer1 - secondaryTimer2;
      secondaryTimer2 = secondaryTimer1;
      for (int i = 0; i < 10; i++) {
        secondaryArraySum += secondaryIntervals[i];
      }
      averagedSecondaryInterval = secondaryArraySum / 10;
      averagedSecondaryInterval = 2000000 / (16 * averagedSecondaryInterval);
      Serial.print(averagedSecondaryInterval);
      Serial.println();
      secondaryArraySum = 0;
    }
  }

  primaryLow = (digitalRead(primaryRead) == LOW);
  secondaryLow = (digitalRead(secondaryRead) == LOW);
//  Serial.print(primaryLow);
//  Serial.println(secondaryLow);

  // Make a command to reset the whole system
  if (Serial.available()) {
    Serial.readString();
    reset();
  }
}

// this function resets the whole program.
void reset() {
  digitalWrite(resetPin, LOW);
}
