/*----------------------------------------------------------------------*
 * Example sketch for Arduino Button Library by Jack Christensen        *
 *                                                                      *
 * An example that uses both short and long button presses.             *
 *                                                                      *
 * A simple state machine where a short press of the button turns the   *
 * Arduino pin 13 LED on or off, and a long press causes the LED to     *
 * blink rapidly. Once in rapid blink mode, another long press goes     *
 * back to on/off mode.                                                 *
 *                                                                      *
 * This work is licensed under the Creative Commons Attribution-        *
 * ShareAlike 3.0 Unported License. To view a copy of this license,     *
 * visit http://creativecommons.org/licenses/by-sa/3.0/ or send a       *
 * letter to Creative Commons, 171 Second Street, Suite 300,            *
 * San Francisco, California, 94105, USA.                               *
 *----------------------------------------------------------------------*/
#include <avr/sleep.h>
#include <Button.h>        //https://github.com/JChristensen/Button

#define BUTTON_PIN 10       //Connect a tactile button switch (or something similar)
//from Arduino pin 2 to ground.
#define PULLUP false       //To keep things simple, we use the Arduino's internal pullup resistor.
#define INVERT false        //Since theRED_PIN pullup resistor will keep the pin high unless the
//switch is closed, this is negative logic, i.e. a high state
//means the button is NOT pressed. (Assuming a normally open switch.)
#define DEBOUNCE_MS 20     //A debounce time of 20 milliseconds usually works well for tactile button switches.

#define RED_PIN 12         //red LED.

#define LONG_PRESS 1000    //We define a "long press" to be 1000 milliseconds.
#define SLEEPSTATE_INTERVAL 100 //In the SLEEPSTATE state, switch the LED every 100 milliseconds.

Button myBtn(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);    //Declare the button

//The list of possible states for the state machine. This state machine has a fixed
//sequence of states, i.e. ONOFF --> TO_SLEEPSTATE --> SLEEPSTATE --> TO_ONOFF --> ONOFF
//Note that while the user perceives two "modes", i.e. ON/OFF mode and rapid blink mode,
//two extra states are needed in the state machine to transition between these modes.
enum {
  ONOFF, TO_SLEEPSTATE, SLEEPSTATE, TO_ONOFF};       
  uint8_t STATE;                   //The current state machine state
  boolean ledState;                //The current LED status
  boolean sleepState;              //The current sleep status
  unsigned long ms;                //The current time from millis()
  unsigned long msLast;            //The last time the LED was switched

void setup(void)
{
  pinMode(RED_PIN, OUTPUT);    //Set the LED pin as an output
}
void sleepNow()         // here we put the arduino to sleep
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

  sleep_enable();              // enables the sleep bit in the mcucr register

  sleep_mode();                // here the device is actually put to sleep!!
  //

  sleep_disable();             // first thing after waking from sleep:
  delay(1000);                 // wat 2 sec. so humans can notice the
  // interrupt.
  // LED to show the interrupt is handled
  digitalWrite (RED_PIN, HIGH);      // turn off the interrupt LED
}



void wakeUpNow()        // here the interrupt is handled after wakeup
{
  //execute code here after wake-up before returning to the loop() function
  // timers and code using timers (serial.print and more...) will not work here.
  digitalWrite(RED_PIN, HIGH);
}
void loop(void)
{
  ms = millis();               //record the current time
  myBtn.read();                //Read the button

  switch (STATE) {

    //This state watches for short and long presses, switches the LED for
    //short presses, and moves to the TO_SLEEPSTATE state for long presses.
  case ONOFF:                
    if (myBtn.wasReleased())
      switchLED();
    else if (myBtn.pressedFor(LONG_PRESS))
      STATE = TO_SLEEPSTATE;
    break;

    //This is a transition state where we start the fast blink as feedback to the user,
    //but we also need to wait for the user to release the button, i.e. end the
    //long press, before moving to the SLEEPSTATE state.
  case TO_SLEEPSTATE:
    if (myBtn.wasReleased())
      STATE = SLEEPSTATE;
    else
      goToSleep();
    break;

    //The fast-blink state. Watch for another long press which will cause us to
    //turn the LED off (as feedback to the user) and move to the TO_ONOFF state.
  case SLEEPSTATE:
    if (myBtn.pressedFor(LONG_PRESS)) {
      STATE = TO_ONOFF;
      sleepNow(); 
      ledState = false;
    }
    else
      goToSleep();
      break;

    //This is a transition state where we just wait for the user to release the button
    //before moving back to the ONOFF state.
  case TO_ONOFF:
    if (myBtn.wasReleased())
      STATE = ONOFF;
    break;
  }
}

//Reverse the current LED state. If it's on, turn it off. If it's off, turn it on.
void switchLED()
{
  msLast = ms;                 //record the last switch time
  ledState = !ledState;
  digitalWrite(RED_PIN, ledState);
}
//Reverse the current awake status. If it's on, turn it off. If it's off, turn it on.
void switchSleep()
{
  msLast = ms;                 //record the last switch time
  ledState = !ledState;
}

//Switch the LED on and off every SLEEPSTATE_INETERVAL milliseconds.
void goToSleep()
{
  if (ms - msLast >= SLEEPSTATE_INTERVAL)
  Serial.println("Timer: Entering Sleep mode");
  delay(100);     // this delay is needed, the sleep
  //function will provoke a Serial error otherwise!!
  sleepNow(); 
}


