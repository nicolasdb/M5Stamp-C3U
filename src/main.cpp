/*Example sketch to control a stepper motor with A4988 stepper motor driver, AccelStepper library and Arduino: acceleration and deceleration. More info: https://www.makerguides.com */

// Include the AccelStepper library:
#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin        4
#define stepPin       5
#define downPin       0
#define upPin         1
#define home1Pin      3
#define callHomePin   9

#define motorType   1
#define speed       2000        // Set the speed in steps per second

// Stepper Travel Variables
long TravelX;  // Used to store the X value entered in the Serial Monitor
int move_finished=1;  // Used to check if move is completed
long initial_homing=-1;  // Used to Home Stepper at startup

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorType, stepPin, dirPin);

int upState;
int downState;
// Create a handle for buttonRead, will help to suspend the control when not playing.
TaskHandle_t TaskHandle_buttonRead;

void homing(void* pvParameters){

   //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
 

// Start Homing procedure of Stepper Motor at startup

  Serial.print("Stepper is Homing . . . . . . . . . . . ");

  while (digitalRead(home1Pin)) {  // Make the Stepper move CCW until the switch is activated   
    stepper.moveTo(initial_homing);  // Set the position to move to
    initial_homing--;  // Decrease by 1 for next move if needed
    stepper.run();  // Start moving the stepper
    vTaskDelay(5/portTICK_RATE_MS);
}

  stepper.setCurrentPosition(0);  // Set the current position as zero for now
  stepper.setMaxSpeed(10.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
  initial_homing=1;

  while (!digitalRead(home1Pin)) { // Make the Stepper move CW until the switch is deactivated
    stepper.moveTo(initial_homing);  
    stepper.run();
    initial_homing++;
    vTaskDelay(5/portTICK_RATE_MS);
  }
  
  stepper.setCurrentPosition(0);
  Serial.println("Homing Completed");
  Serial.println("");
  stepper.setMaxSpeed(1000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepper.setAcceleration(1000.0);  // Set Acceleration of Stepper



vTaskDelete( nullptr );
}

void buttonRead(void* pvParameters) {
  while(true) {
    upState = digitalRead(upPin);
    if(upState == LOW) {
      //Serial.println("Going Up!");
      stepper.setSpeed(speed);
        // Step the motor with a constant speed as set by setSpeed():
      stepper.runSpeed();
    } 

    downState = digitalRead(downPin);
    if(downState == LOW) {
      //Serial.println("Going Down!");
      stepper.setSpeed(-speed);
      stepper.runSpeed();
      }

  vTaskDelay(5 / portTICK_RATE_MS);
  }
}



void setup() {
  Serial.begin(115200);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(home1Pin, INPUT_PULLUP);
  pinMode(callHomePin, INPUT_PULLUP);

  // xTaskCreate(
  //                   taskOne,          /* Task function. */
  //                   "TaskOne",        /* String with name of task. */
  //                   10000,            /* Stack size in bytes. */
  //                   nullptr,          /* Parameter passed as input of the task */
  //                   1,                /* Priority of the task. */
  //                   nullptr);         /* Task handle. */
  
  xTaskCreate(buttonRead, "buttonRead", 2048 * 1, nullptr, 2, &TaskHandle_buttonRead);
  xTaskCreate(homing, "homing", 2048 * 1, nullptr, 1, nullptr);

  
  // Set the maximum speed and acceleration:
  stepper.setMaxSpeed(1.5*speed);


}

void loop() {
    if(digitalRead(callHomePin) == LOW) {
vTaskResume(nullptr);
    } 
}