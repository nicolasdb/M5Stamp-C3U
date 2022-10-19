#include <Arduino.h>
#include <ButtonEvents.h> // we have to include the library in order to use it


#define buzzer      6
#define buttonPin   9

ButtonEvents myButton; // create an instance of the ButtonEvents class to attach to our button


void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin,INPUT_PULLUP);
  myButton.attach(buttonPin);
  pinMode(buzzer,OUTPUT);
}


void loop() {
  myButton.update();

  if(myButton.doubleTapped() == true){
    delay(400);
    int count = 3;
    while(count >0){
      for( int i = 180; i>0; i--){
        digitalWrite(buzzer,HIGH);
        delayMicroseconds(i);
        digitalWrite(buzzer,LOW);
        delayMicroseconds(i);
      } 
    delay(125);
    count = count -1;
    }       
  }
}