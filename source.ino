#define NO_LEDS 11 // Number of LEDs
#define LEDS_ARRAY {A0,A1,A2,A3,A4,A5,9,10,11,12,13} // LEDs pin array
#define LED_ON HIGH // LED on signal
#define LED_OFF LOW // LED off signal
#define TIME_ACCURACY micros // Decide whether to use millis or micros function for timing
#define TIME_SCALE 1000000 // This is used to convert (millis or micros) to and from (second)
#define MODIFIER 0.4 // Magic number, used to fine tune the formula
#define ADD_TIME 33300

/************************* VARIABLES *************************/
// The number between the square brackets does not represent the pin number,
// it's merely the index in the array.
// Hour hand pin(s): [0 -> 3]
// Minute hand pin(s): [0 -> 6]
// Second hand pin(s): [7]
// Hour Indicator pin(s): [8->9]
// Minute Indicator pin(s): [9]
// Frame pin(s): [10]
byte pins[NO_LEDS] = LEDS_ARRAY;

volatile byte flag = 0; // Interrupt flag

//NOTE! UNSIGNED LONG WILL OVERFLOW AFTER 71 MINUTES
unsigned long internalTime = 0; // Current internal time (microseconds).
unsigned long previousInternalTime = 0; // Previous internal time (microseconds).
unsigned long internalTimeInSeconds = 0; // Current internal time (seconds).
unsigned long atZeroTime = 0; // Time mark at which point the object pass through the sensor (microseconds)
unsigned long previousAtZeroTime = 0; // Previous time mark at which point the object pass through the sensor (microseconds)
unsigned int oneCycleTime = 0; // Duration of one rotation cycle (microseconds)
unsigned int oneDivisionTime = 0; // Duration of one division = oneCycleTime / 60 (microseconds). [Can be int]
unsigned int temp = 0; // Temporary variable [Can be int]
byte atDivision = 0;
byte i = 0; // Iterator
byte seconds = 0;
byte minutes = 0;
byte hours = 0;
boolean shouldDisplayMinuteIndicator;
boolean shouldDisplayHourIndicator;
boolean shouldDisplaySecondHand;
boolean shouldDisplayMinuteHand;
boolean shouldDisplayHourHand;

/************************* SETUP *************************/
void setup() {
  for(i = 0; i < NO_LEDS; i++)
    pinMode(pins[i], OUTPUT);
  
  digitalWrite(pins[10], LED_ON); // The LED used for the frame should be ON all the time
  attachInterrupt(0, atZero, RISING);
}

/************************* LOOP *************************/
void loop() {
  // Read in the current HH:MM:SS time
  internalTime = TIME_ACCURACY();
  if(internalTime - previousInternalTime > TIME_SCALE / 2) {
    // Update if half a second has passed since the last update
    internalTimeInSeconds = internalTime / TIME_SCALE + ADD_TIME;
    seconds = internalTimeInSeconds % 60;
    minutes = (internalTimeInSeconds / 60) % 60;
    hours = (internalTimeInSeconds / 3600) % 12;
    previousInternalTime = internalTime;
  }
  
  // Interrupt happens (or the object passes through the sensor)
  if(flag == 1) {
    atZeroTime = internalTime; // Or atZeroTime = TIME_ACCURACY();
    oneCycleTime = atZeroTime - previousAtZeroTime; // Note: Left side is int, right side is long
    oneDivisionTime = oneCycleTime / 60;
    previousAtZeroTime = atZeroTime;
    flag = 0;
  }
  
  temp = internalTime - atZeroTime; // Time passed since the last time the object passed through the sensor
  atDivision = temp / oneDivisionTime;
  
  shouldDisplayMinuteIndicator = ((unsigned int)(oneDivisionTime * atDivision) < temp && temp < (unsigned int)(oneDivisionTime * (atDivision + MODIFIER)));
  shouldDisplayHourIndicator = (atDivision % 5 == 0 && (unsigned int)(oneDivisionTime * atDivision) < temp && temp < (unsigned int)(oneDivisionTime * (atDivision + MODIFIER)));
  shouldDisplaySecondHand = (temp < (unsigned int)(oneDivisionTime * (seconds + MODIFIER)));
  shouldDisplayMinuteHand = ((unsigned int)(oneDivisionTime * minutes) < temp && temp < (unsigned int)(oneDivisionTime * (minutes + MODIFIER)));
  shouldDisplayHourHand = ((unsigned int)(oneDivisionTime * (5 * hours + minutes / 12)) < temp && temp < (unsigned int)(oneDivisionTime * (5 * hours + minutes / 12 + MODIFIER)));
  
  // The clock indicators
  if( shouldDisplayMinuteIndicator )
    digitalWrite(pins[9], LED_ON);
  else digitalWrite(pins[9], LED_OFF);
  
  if( shouldDisplayHourIndicator ) {
    digitalWrite(pins[8], LED_ON);
    digitalWrite(pins[9], LED_ON);
  } else {
    if( shouldDisplayMinuteIndicator ) {
      digitalWrite(pins[8], LED_OFF);
    }
    else {
      digitalWrite(pins[8], LED_OFF);
      digitalWrite(pins[9], LED_OFF);
    }
  }
  
  // The clock hands
  if( shouldDisplaySecondHand )
    digitalWrite(pins[7], LED_ON);
  else digitalWrite(pins[7], LED_OFF);

  if( shouldDisplayHourHand ) {
    for(i = 0; i < 4; i++) digitalWrite(pins[i], LED_ON);
  }
  else {
    for(i = 0; i < 4; i++) digitalWrite(pins[i], LED_OFF);
  }
  
  if( shouldDisplayMinuteHand ) {
    for(i = 0; i < 7; i++) digitalWrite(pins[i], LED_ON);
  }
  else {
    if( shouldDisplayHourHand )
      for(i = 4; i < 7; i++) digitalWrite(pins[i], LED_OFF);
    else
      for(i = 0; i < 7; i++) digitalWrite(pins[i], LED_OFF);
  }
}

/************************* FUNCTION DECLARATIONS *************************/
// Interrupt function. Called every time the object pass through the sensor
void atZero() {
  flag = 1;
}
