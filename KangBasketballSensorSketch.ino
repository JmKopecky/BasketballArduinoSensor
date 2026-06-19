/*
Arduino software to detect a ball within range and start a countdown timer when it is removed, 
buzzing once the timer elapses. Said timer can be configured via the device's buttons, adding 
one second or one hundredth of a second depending on which is pressed. Using the reset button
(or power cycling the device) resets this timer to 0. 

Programmed by Joseph Kopecky
*/

#include "TM1637Display.h" //library to drive the four digit display
#include "Ultrasonic.h" //library to drive the ultrasonic distance measurer

//define constants for later use
#define DEFAULT_BUTTON_COOLDOWN 100 //how long between button press before it can be re-pressed, preventing accidental double-clicks
#define SECONDS_TO_MILLIS 1000
#define HUNDREDS_TO_MILLIS 10
#define MAX_TIMER 99990 //the maximum value on the timer before it overflows to zero. 99 seconds, 99 hundredths of a second
#define BALL_DISTANCE_THRESHOLD_CM 20 //the distance from the UV distance sensor that the ball must be to trigger as taken
#define BUZZER_DURATION 1000 //how long the buzzer plays, in milliseconds

//define pins
#define DISTANCE_SENSOR_PIN 19
#define BUZZER_PIN 6
#define SECOND_BUTTON_PIN 5
#define HUNDREDTH_BUTTON_PIN 4
#define DISPLAY_CLOCK_PIN 2
#define DISPLAY_DATAIO_PIN 3

//prep the ultrasonic distance sensor
Ultrasonic ultrasonic(DISTANCE_SENSOR_PIN);

//prep the four digit display
TM1637Display display = TM1637Display(DISPLAY_CLOCK_PIN, DISPLAY_DATAIO_PIN);

unsigned long systemUptime = 0;
byte buttonCooldown = 0;
unsigned int buzzerRemainingTime = 0;
unsigned long setTimerLength = 0;
unsigned long remainingTimerLength = 0;
bool ballInPlace = false;

/*
  Arduino method called when the device starts or when the reset button is pressed.
*/
void setup() {
  //setup display
  display.setBrightness(7); //7 is max brightness for the display
  display.clear();
  
  //setup buttons
  pinMode(SECOND_BUTTON_PIN, INPUT);
  pinMode(HUNDREDTH_BUTTON_PIN, INPUT);

  //setup buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  //get initial time to compare against.
  systemUptime = millis();
}

/*
  Arduino method called repeatedly after setup. 
*/
void loop() {
  //record change in time since last loop
  unsigned short deltaTime = millis() - systemUptime;
  systemUptime += deltaTime;

  listenForButtons(deltaTime);
  updateMainTimer(deltaTime);
  updateBuzzerTimer(deltaTime);
  trackBallPos();
  renderTimer();
}

/*
  Detect when the ball is removed and when it is put back.
  post: if ball has been put back, ballInPlace = true. 
        If ball is removed while the timer isn't counting down, ballInPlace = false and timer starts.
*/
void trackBallPos() {
  //if the timer hasn't finished, don't detect position changes lest we detect something weird while the ball is being moved.
  if (remainingTimerLength != 0) {
    return;
  }

  long measuredDistanceCM = ultrasonic.MeasureInCentimeters();

  //record the ball being put back
  if (!ballInPlace && measuredDistanceCM < BALL_DISTANCE_THRESHOLD_CM) {
    ballInPlace = true;
  //record the ball being removed
  } else if (ballInPlace && measuredDistanceCM > BALL_DISTANCE_THRESHOLD_CM) {
    remainingTimerLength = setTimerLength; //start the descending timer
    ballInPlace = false;
  }
}

/*
  Reads the timer increase buttons, increasing the timer and starting the button cooldown if they were pressed.
  post: if either button was pressed, buttonCooldown = DEFAULT_BUTTON_COOLDOWN. Else, = max(0, buttonCooldown - deltaTime)
        setTimerLength bounded between [0,MAX_TIMER)
*/
void listenForButtons(unsigned short deltaTime) {
  //reduce button cooldown, bound to be no lower than 0 to prevent underflow bugs
  if (buttonCooldown <= deltaTime) {
    buttonCooldown = 0;
  } else {
    buttonCooldown -= deltaTime;
  }

  //get pressed state
  bool secondButtonPressed = isButtonPressed(SECOND_BUTTON_PIN);
  bool hundredthButtonPressed = isButtonPressed(HUNDREDTH_BUTTON_PIN);

  //update for hundredths button
  if (buttonCooldown == 0 && hundredthButtonPressed) {
    setTimerLength += HUNDREDS_TO_MILLIS;
  }

  //update for seconds button
  if (buttonCooldown == 0 && secondButtonPressed) {
    setTimerLength += SECONDS_TO_MILLIS;
  }

  //bound timer to be within max
  if (setTimerLength > MAX_TIMER) {
    setTimerLength = 0;
  }

  //start cooldown if either button was pressed
  if (secondButtonPressed || hundredthButtonPressed) {
    buttonCooldown = DEFAULT_BUTTON_COOLDOWN;
  }
}

/*
  Render the current or set timer to the four digit display. 
  Pre: setTimerLength, remainingTimerLength representable in five digits
*/
void renderTimer() {
  //if timer not ticking down, show set time
  if (remainingTimerLength == 0) {
    //divide by ten to convert milliseconds to hundredths of a second (five digits to four)
    display.showNumberDec(setTimerLength / 10, true, 4); 
  //time is ticking down, show current time
  } else {
    //divide by ten to convert milliseconds to hundredths of a second (five digits to four)
    display.showNumberDec(remainingTimerLength / 10, true, 4);
  }
}

/*
  Decrease the current timer if ticking down. If timer ends, plays the buzzer.
  Post: remainingTimerLength >= 0, strictly non-increasing in this method
*/
void updateMainTimer(unsigned short deltaTime) {
  //if timer is ticking down
  if (remainingTimerLength > 0) {
    //if we've reached the end of the timer
    if (remainingTimerLength <= deltaTime) {
      //play buzzer
      buzzerRemainingTime = BUZZER_DURATION; 
      digitalWrite(BUZZER_PIN, HIGH); //high means buzzer is on
      //end timer
      remainingTimerLength = 0;
    //else, the timer continues counting down
    } else {
      remainingTimerLength -= deltaTime;
    }
  }
}

/*
  Decrease the current buzzer timer if ticking down. Turns off the buzzer when the timer is out.
  post: buzzerTimer >= 0, strictly non-increasing in this method
*/
void updateBuzzerTimer(unsigned short deltaTime) {
  //if timer is ticking down
  if (buzzerRemainingTime > 0) {
    //if we've reached the end of the timer
    if (buzzerRemainingTime <= deltaTime) {
      buzzerRemainingTime = 0;
      digitalWrite(BUZZER_PIN, LOW); //low means buzzer is off
    } else {
      buzzerRemainingTime -= deltaTime;
    }
  }
}

/*
  Return true if the button at pin is currently pressed. False otherwise. 
  pre: pin corresponds to the pin of a button. Undefined behavior otherwise.
*/
bool isButtonPressed(int pin) {
  return digitalRead(pin) == 0; //digitalRead returns 0 if pressed, 1 otherwise. 
}