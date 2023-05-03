#include <AccelStepper.h>
#include <Wire.h>

#define RECIVE_PIN 4
#define slaveAddress 9

// AccelStepper Setup
AccelStepper stepper(1, 2, 3); // 1 = Easy Driver interface
                               // NANO Pin 2 connected to STEP pin of Easy Driver
                               // NANO Pin 3 connected to DIR pin of Easy Driver

float maxSpeed = 3000;
float targetSpeed = 0; // # of steps per second to speeed up to
unsigned long time;
unsigned long previousAccel = 0;
int interval = 5; // # of milliseconds between speed increases
float stepperSpeed;

void setup()
{
    stepper.setMaxSpeed(3000);
    pinMode(RECIVE_PIN, INPUT);

    Wire.begin(slaveAddress);
}

void loop()
{
    if (digitalRead(RECIVE_PIN) == HIGH)
    {
        targetSpeed = Wire.read();
    }
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
