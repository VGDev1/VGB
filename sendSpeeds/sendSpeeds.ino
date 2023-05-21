#include <Wire.h>

#define TRANSMIT_PIN 3

//Stepper motor control
int setSpeed;
int currentSpeed = 0;

//communication
int slaveAddress = 9;

void setup()
{
    Serial.begin(9600);
    Wire.begin();
}

void sendSpeed() {

  if(currentSpeed != setSpeed) {
    Wire.beginTransmission(slaveAddress);  // transmit to device #9
    // map speed to one byte
    currentSpeed = setSpeed;
    setSpeed = map(setSpeed, 0, 3000, 0, 255);
    Wire.write(setSpeed);                     // sends x
    Wire.endTransmission();                // stop transmitting
    Serial.write("sent");
  }

}

void loop()
{

    Serial.println("test");

    setSpeed = 3000;
    sendSpeed();
}