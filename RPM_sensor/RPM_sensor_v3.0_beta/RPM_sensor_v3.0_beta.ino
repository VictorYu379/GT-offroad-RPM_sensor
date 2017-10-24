const int primaryInputPin = 3;
const int secondaryInputPin = 2;

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

const int resetPin = 10;

void setup() {
  // put your setup code here, to run once:
  pinMode(primaryInputPin, INPUT_PULLUP);
  pinMode(secondaryInputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(primaryInputPin), primaryIncrement, FALLING);
  attachInterrupt(digitalPinToInterrupt(secondaryInputPin), secondaryIncrement, FALLING);
  primaryTimer1 = millis();
  secondaryTimer1 = millis();
  Serial.begin(9600);
  Serial.println("Started");
}

void loop() {
}

void primaryIncrement() {
  Serial.println("haha");
  primaryTimer2 = millis();
  primaryArrayIndex = (++primaryArrayIndex) % 8;
  primaryIntervals[primaryArrayIndex] = primaryTimer2 - primaryTimer1;
  primaryTimer1 = primaryTimer2;
  for (int i = 0; i < 10; i++) {
    primaryArraySum += primaryIntervals[i];
  }
  averagedPrimaryInterval = 1000000 / (16 * (primaryArraySum / 8));
  Serial.println(averagedPrimaryInterval);
  primaryArraySum = 0;
}


void secondaryIncrement() {
  Serial.println("ahah");
  secondaryTimer2 = millis();
  secondaryArrayIndex = (++secondaryArrayIndex) % 8;
  secondaryIntervals[secondaryArrayIndex] = secondaryTimer2 - secondaryTimer1;
  secondaryTimer1 = secondaryTimer2;
  for (int i = 0; i < 10; i++) {
    secondaryArraySum += secondaryIntervals[i];
  }
  averagedSecondaryInterval = 1000000 / (16 * (secondaryArraySum / 8));
  Serial.println(averagedSecondaryInterval);
  secondaryArraySum = 0;
}

