//and the pushbutton pin
#define buttonpin 10
int buttonState = 0;     

void setup(){ 
  pinMode(buttonpin, INPUT);

  //check for proper baud rate
  Serial.begin(9600);
}
void loop() 
{
   buttonState = digitalRead(buttonpin);
   if (buttonState == HIGH)
   {
     Serial.println("button is pressed");
   
   } 
}
