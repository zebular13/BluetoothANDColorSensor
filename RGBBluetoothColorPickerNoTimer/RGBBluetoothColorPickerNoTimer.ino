//include the libraries for the color sensor
#include <Wire.h>
#include "Adafruit_TCS34725.h"


// constants won't change. They're used here to set certain pins to be used in certain ways

// set to false if using a common cathode LED
#define commonAnode true

//PWM pins for the LEDs
#define redpin 12
#define greenpin 9
#define bluepin 6
//and the pushbutton pin
#define buttonpin 10

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status


// our RGB -> eye-recognized gamma color
byte gammatable[256];

//declare the color sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);


void setup(){ 
  //configure pins
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  pinMode(buttonpin, INPUT);

  //check for proper baud rate
  Serial1.begin(9600);
  
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
} 


void loop() 
{
     buttonState = digitalRead(buttonpin);

    
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
//check to see if the pushbutton is pressed.
    if (buttonState ==  HIGH) 
    
// it changed, do something
      { 
      Serial.println("button is pressed");
      uint16_t clear, red, green, blue;
    
      tcs.setInterrupt(false);      // turn on LED
    
      delay(60);  // takes 50ms to read 
      
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

