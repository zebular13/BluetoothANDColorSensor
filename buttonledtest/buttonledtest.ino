//and the pushbutton pin
#define buttonpin 10
#define redpin 12
#define greenpin 9
#define bluepin 6

int buttonState = 0;     

void setup(){ 
  pinMode(buttonpin, INPUT);
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  //check for proper baud rate
  Serial.begin(9600);
}
void loop() 
{
   buttonState = digitalRead(buttonpin);
   if (buttonState == HIGH)
   {
     Serial.println("button is pressed");
     digitalWrite(redpin, HIGH);
     digitalWrite(greenpin, HIGH);
     digitalWrite(bluepin, HIGH);
   } 
   if (buttonState == LOW)
   {
      digitalWrite(redpin, LOW);
      digitalWrite(greenpin, LOW);
      digitalWrite(bluepin, LOW);
   }
}
