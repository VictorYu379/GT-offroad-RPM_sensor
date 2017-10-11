/**
 * @author Jingkai Yu
 * @version 1.0
 */

int digital_read = 0;
uint32_t timer1;
uint32_t timer2;
uint32_t interval1;
uint32_t interval2;
uint32_t interval3;
uint32_t interval4;
uint32_t averagedInterval;
uint32_t intervals[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int arrayIndex = 0;
int arraySum = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13, INPUT);

  // Initialize the timers
  timer1 = millis();
  timer2 = millis();
}

void loop() {
  // Print to the serial monitor to signal the start
  Serial.println("started");
  
  // put your main code here, to run repeatedly
  // Use a infinite loop to keep the number assigned to "arrayIndex" and "intervals"
  while (true) {

    // Once approaching of the magnet is detected, go into this logic.
    if (digitalRead(13) == LOW) {

      // While loop until the magnet leaves the sensors, and that counts as one time
      while (digitalRead(13) == LOW) {
      }

      // Make the array circular
      arrayIndex = (++arrayIndex) % 10;

      // Record the time spot of this time and calculate the time difference
      timer1 = millis();
      intervals[arrayIndex] = timer1 - timer2;

      // Calculate the average of the array to reduce the noise and error
      for (int i = 0; i < 10; i++) {
        arraySum += intervals[i];
      }
      averagedInterval = arraySum / 10;

      // Transfer the time difference to RPM
      averagedInterval = 1000000 / (16 * averagedInterval);
      Serial.println(averagedInterval);
      Serial.println();

      // Record the time spot when this time ends
      timer2 = millis();

      // Reset the summation of the array.
      arraySum = 0;
    }        

    // Make a command to reset the whole system
    if (Serial.available()) {
      Serial.readString();
      break;
    }
  }
}
