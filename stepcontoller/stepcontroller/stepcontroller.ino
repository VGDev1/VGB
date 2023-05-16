#include <AccelStepper.h>
#include <Wire.h>

#define RECIVE_PIN 4
#define slaveAddress 9

// AccelStepper Setup
AccelStepper stepper(1, 2, 3); // 1 = Easy Driver interface
                               // NANO Pin 2 connected to STEP pin of Easy Driver
                               // NANO Pin 3 connected to DIR pin of Easy Driver

int maxSpeed = 3000;
int targetSpeed = 0; // # of steps per second to speeed up to
unsigned long time;
unsigned long previousAccel = 0;
int interval = 5; // # of milliseconds between speed increases

void setup()
{
    stepper.setMaxSpeed(3000);
    Serial.begin(9200);
    Wire.begin(slaveAddress);
    Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes) {
    int input = Wire.read();
    targetSpeed = map(input, 0, 255, 0, maxSpeed);
    Serial.println(targetSpeed);
}

void loop()
{
    while (stepper.speed() < targetSpeed)
    {
        time = millis();
        if (time > previousAccel + interval)
        {
            previousAccel = time;
            stepper.setSpeed(stepper.speed() + 5);
        }
        stepper.runSpeed();
    }

    stepper.setSpeed(targetSpeed);
    stepper.runSpeed();
}
