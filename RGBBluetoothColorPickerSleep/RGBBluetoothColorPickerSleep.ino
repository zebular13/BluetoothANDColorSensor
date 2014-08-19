//include the libraries for the color sensor
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <avr/interrupt.h> 
#include <avr/power.h>
#include <avr/sleep.h>
#include <Button.h>        //https://github.com/JChristensen/Button

// set to false if using a common cathode LED
#define commonAnode true

//Define pins to put to sleep
#define sdapin 2
#define sclpin 3


//PWM pins for the LEDs
#define redpin 12
#define greenpin 9
#define bluepin 6

//and the pushbutton pin
#define buttonpin 10

//--this is stuff from Button.h:
#define PULLUP false       //To keep things simple, we use the Arduino's internal pullup resistor.
#define INVERT false        //Since theRED_PIN pullup resistor will keep the pin high unless the
//switch is closed, this is negative logic, i.e. a high state
//means the button is NOT pressed. (Assuming a normally open switch.)
#define DEBOUNCE_MS 20     //A debounce time of 20 milliseconds usually works well for tactile button switches.
#define LONG_PRESS 1000    //We define a "long press" to be 1000 milliseconds.
#define SLEEP_INTERVAL 100 //In the SLEEP state, switch the LED every 100 milliseconds.
Button myBtn(buttonpin, PULLUP, INVERT, DEBOUNCE_MS);    //Declare the button

//The list of possible states for the state machine. This state machine has a fixed
//sequence of states, i.e. ONOFF --> TO_SLEEP --> SLEEP --> TO_ONOFF --> ONOFF
//Note that while the user perceives two "modes", i.e. ON/OFF mode and rapid blink mode,
//two extra states are needed in the state machine to transition between these modes.
enum {ONOFF, TO_SLEEP, SLEEP, TO_ONOFF};   

  uint8_t STATE;                   //The current state machine state
  boolean sleepState;                //The current LED status
  unsigned long ms;                //The current time from millis()
  unsigned long msLast;            //The last time the LED was switched

int sleeping = 0;
// our RGB -> eye-recognized gamma color
byte gammatable[256];

//declare the color sensorw
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);


void setup(){ 
  //configure pins
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  pinMode(buttonpin, INPUT);

  //check for proper baud rate
  Serial1.begin(9600);
  
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for Leonardo only  
  }
  //code to check if sensor is available
  if (tcs.begin()) 
  {
    Serial.println("Found sensor");
  } 
  else 
  {
    Serial.println("No TCS34725 found ... check your connections");
    //while (1); // halt!
  }

  // this gamma table helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) 
  {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    if (commonAnode) 
    {
      gammatable[i] = 255 - x;
    } 
    else 
    {
      gammatable[i] = x;      
    }
  }
  attachInterrupt(2, wakeUp, RISING);
} 
void sleepNow()         // here we put the arduino to sleep
{
  Serial1.end (); 
  sleeping == 1;
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_IDLE);   // sleep mode is set here
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();
  sleep_mode();
}
void wakeUp()
{
  sleep_disable();
  power_all_enable();
  Serial1.begin(9600);
  while (!Serial1) { }  // wait for Serial to initialize
  detachInterrupt(2);

  Serial.println("I'm awake");
  Serial.flush();
}
void loop() 
{
  ms = millis();               //record the current time
  myBtn.read();                //Read the button

  switch (STATE) {

    //This state watches for short and long presses, switches the LED for
    //short presses, and moves to the TO_SLEEP state for long presses.
  case ONOFF:                
    if (myBtn.wasReleased())
      senseColor();
    else if (myBtn.pressedFor(LONG_PRESS))
      STATE = TO_SLEEP;
    break;

    //This is a transition state where we start the fast blink as feedback to the user,
    //but we also need to wait for the user to release the button, i.e. end the
    //long press, before moving to the SLEEP state.
  case TO_SLEEP:
    if (myBtn.wasReleased())
    {
      senseColor();
      switchSleep();
    }
    else
      goToSleep();
    break;

    //The fast-blink state. Watch for another long press which will cause us to
    //turn the LED off (as feedback to the user) and move to the TO_ONOFF state.
  case SLEEP:
    if (myBtn.pressedFor(LONG_PRESS)) {
      STATE = TO_ONOFF;
      sleepState = false;
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
    
    //check here to see if the string is being sent over bluetooth (Serial1)
    //App Inventor sends a string that contains three bytes (RGB, 0 to 255). 
    if (Serial1.available())
    {

      //read each of the 3 bytes for brightness into the variables
      String split = Serial1.readString();  
      //split the string and get the first value, then the second and third
      String redvalue = getValue(split, ',', 0); 
      String greenvalue = getValue(split, ',', 1);
      String bluevalue = getValue(split, ',', 2);
      //print each value to the serial monitor to make sure it's coming in correctly
      Serial.print(redvalue);
      Serial.print(", ");
      Serial.print(bluevalue);
      Serial.print(", ");
      Serial.println(greenvalue);

      //write the current values to the pwm pins.
      analogWrite(redpin, redvalue.toInt());
      analogWrite(greenpin, greenvalue.toInt());
      analogWrite(bluepin, bluevalue.toInt());
    } 
    

    
  } 


//this is a function to split the incoming string at the comma
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void senseColor()
{ 
  Serial.println("button is pressed");
  uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // turn on LED

  delay(3000);  // takes 3 seconds to read 
  
  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true);  // turn off LED

  // Figure out some basic hex code for visualization
  uint32_t sum = red;
  sum += green;
  sum += blue;
  sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  
  
  //print the color values to the Serial monitor to make sure that they're coming in correctly
  Serial.print("C:\t"); Serial.print(clear);
  Serial.print("\tR:\t"); Serial.print((int)r);
  Serial.print("\tG:\t"); Serial.print((int)g);
  Serial.print("\tB:\t"); Serial.print((int)b);
  Serial.println();
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();
  
  //write the values that the color sensor is sensing to the LED
  analogWrite(redpin, (int)r);
  analogWrite(greenpin, (int)g);
  analogWrite(bluepin, (int)b);
}
void switchSleep()
{
  msLast = ms;                 //record the last switch time
  sleepState = !sleepState;
}

//Switch the LED on and off every SLEEP_INETERVAL milliseconds.
void goToSleep()
{
  if (ms - msLast >= SLEEP_INTERVAL && sleeping == 0)
    digitalWrite(redpin, LOW);
    digitalWrite(greenpin, LOW);
    digitalWrite(bluepin, LOW);
    digitalWrite(sclpin, LOW);  
    digitalWrite(sdapin, LOW);    
    Serial.println("Entering Sleep mode");
    delay(100);     // this delay is needed, the sleep
    //function will provoke a Serial error otherwise!!
    sleepNow();

}



