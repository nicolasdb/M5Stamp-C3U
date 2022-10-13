/*Example sketch to control a stepper motor with A4988 stepper motor driver, AccelStepper library and Arduino: acceleration and deceleration. More info: https://www.makerguides.com */

// Include the AccelStepper library:
#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin      4
#define stepPin     5
#define downPin       0
#define upPin         1

#define motorType   1
#define stepX       1
#define speed       2000

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorType, stepPin, dirPin);

int upState;
int downState;
// Create a handle for buttonRead, will help to suspend the control when not playing.
TaskHandle_t TaskHandle_buttonRead;



void buttonRead(void* pvParameters) {
  while(true) {
    upState = digitalRead(upPin);
    if(upState == LOW) {
      Serial.println("Going Up!");
        // Set the speed in steps per second:
  stepper.setSpeed(speed);
  // Step the motor with a constant speed as set by setSpeed():
  stepper.runSpeed();

    } 


    downState = digitalRead(downPin);
    if(downState == LOW) {
      Serial.println("Going Down!");
        // Set the speed in steps per second:
  stepper.setSpeed(-speed);
  // Step the motor with a constant speed as set by setSpeed():
  stepper.runSpeed();
      }

  vTaskDelay(1 / portTICK_RATE_MS);
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);
  
  // xTaskCreate(
  //                   taskOne,          /* Task function. */
  //                   "TaskOne",        /* String with name of task. */
  //                   10000,            /* Stack size in bytes. */
  //                   nullptr,          /* Parameter passed as input of the task */
  //                   1,                /* Priority of the task. */
  //                   nullptr);         /* Task handle. */
  
  xTaskCreate(buttonRead, "buttonRead", 2048 * 1, nullptr, 1, &TaskHandle_buttonRead);
  
  
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed(1.5*speed);


}

void loop() {
}