#include <Arduino_APDS9960.h>
#include <Arduino_HS300x.h>

enum State { IDLE,
             MONITOR,
             ALERT };                 // Enumeration holding all states
State currentState = IDLE;            // Default state starts off in IDLE

unsigned long lastFlash = 0;         // Timer for blinking of alert light
unsigned long lastTempRead = 0;      // Timer for moderating temnperature readings
bool redState = HIGH;                // State of LED, used when blinking in ALERT state

const float deadband = 0.25
const float Threshold = 22.5;       // Added deadband to address temperautre at threshold


void setup() {
  Serial.begin(9600);
  HS300x.begin();
  APDS.begin();

  // LED setup
  pinMode(LEDR, OUTPUT); 
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  // Making sure all LEDs are off when booted
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}

void loop() {
  int gesture = -1;         // Initilaization of variable used to store gestures
  float temp;               // Initilaization of variable used to store temperatures


  if (APDS.gestureAvailable()) {               // Reads gesture only if the channel is open and available
    gesture = APDS.readGesture();
  }
  // Swithc case for FSM, with each state as a case
  switch (currentState) {                      
    case IDLE:
      analogWrite(LEDB, 200);          // Dimmed  blue LED
      digitalWrite(LEDR, HIGH);
      digitalWrite(LEDG, HIGH);

      if (gesture == GESTURE_UP || gesture == GESTURE_RIGHT) {       // Transition statement and condition fgor IDLE -> MONITOR
        currentState = MONITOR;
      }

      break;
    case MONITOR:
      analogWrite(LEDB,255);            // Switching off blue LED
      digitalWrite(LEDB, HIGH);
      digitalWrite(LEDR, HIGH);
      digitalWrite(LEDG, LOW);         //Switching on green LED 


      if (millis() - lastTempRead >= 1000) {            // Reads temperature every second. Periodic readings prevent sensor from heating up
        temp = HS300x.readTemperature();
        Serial.println(temp);
        lastTempRead = millis();
        if (temp > (Threshold - deadband) || temp < (Threshold + deadband) {                        // Transition statement and condition for MONITOR -> ALERT
          currentState = ALERT;
        }
         
      }

      if (gesture == GESTURE_DOWN || gesture == GESTURE_LEFT) {   // Transition statement and condition for MONITOR -> IDLE
          currentState = IDLE;
        }

      
      break;

    case ALERT:
      analogWrite(LEDB,255);   // Switching off blue LED
      digitalWrite(LEDG, HIGH);
      digitalWrite(LEDB, HIGH);

      if (millis() - lastFlash >= 500) {      //Blinking red LED every 500 ms. Use of millis() so as not to interrupt other processes
        redState = !redState;
        digitalWrite(LEDR, redState);
        lastFlash = millis();           //Timer reset
      }

      if (gesture == GESTURE_UP || gesture == GESTURE_DOWN || gesture == GESTURE_RIGHT || gesture == GESTURE_LEFT) {    // Transition statement and condition for ALERT -> IDLE
        currentState = IDLE;
      }

      break;
  }
}
