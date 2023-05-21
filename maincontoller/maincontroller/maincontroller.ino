#include <LiquidCrystal.h>     // inkludera programmet för lcd displayn
#include <PID_v1.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>



LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

#define HEATER_RELAY_PIN 11
#define PROG_BTN_PIN 16

#define slaveAddress 9

#define ONE_WIRE_BUS 10

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

// Define Variables we'll be connecting to
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
double Kp = 80, Ki = 10, Kd = 450;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 2000;
unsigned long windowStartTime;

int maxSpeed = 3000;

// Calc variables
// Startbutton
long debounce = 200; // the debounce time, increase if the output flickers
// Time variables
unsigned long timeSinceStartMillis; // Veriable to use with millis to keep count on the sequences
unsigned long timeSinceStartMinutes;

unsigned int minuteInMillis = 60000;

// Stepper motor control
int setSpeed;
int currentSpeed = 0;

// Time values
int nbrOfSequences = 3;
int long times[] = {5,10,15};
int long temperatures[] = {60,70,80};
int long speeds[] = {200 ,500, 800};

void setup()
{

    Serial.begin(9600);
    pinMode(PROG_BTN_PIN, INPUT_PULLUP); // Inpin till startknappen

    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, LOW);

    // initialize the variables we're linked to
    Setpoint = 0;

    // tell the PID to range between 0 and the full window size
    myPID.SetOutputLimits(0, WindowSize);

    // turn the PID on
    myPID.SetMode(AUTOMATIC);

    //lcd.begin(16, 2); 

    sensors.begin();

    Wire.begin(); 

}

void heaterLoop()
{
    if (millis() - windowStartTime > WindowSize)
    {
        // time to shift the Relay Window
        windowStartTime += WindowSize;
    }
    if(Output > millis() - windowStartTime) digitalWrite(HEATER_RELAY_PIN,HIGH);  
    else digitalWrite(HEATER_RELAY_PIN,LOW);  

}

void sendSpeed()
{

    if (currentSpeed != setSpeed)
    {
        Wire.beginTransmission(slaveAddress); // transmit to device #9
        // map the speed to a value between 0 and 255
        int speed = map(setSpeed, 0, maxSpeed, 0, 255);
        Wire.write(speed);                 // sends x
        Wire.endTransmission();               // stop transmitting
        currentSpeed = setSpeed;
    }
}

int sequenceNumber(int time)
{
    for (int i = 0; i < nbrOfSequences; i++)
    {
        if (timeSinceStartMinutes < times[i])
        {
            return i;
        }
    }
    return -1;
}

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
    asm volatile("  jmp 0");
}

void readTemp()
{
    sensors.requestTemperatures(); 
    Input = sensors.getTempCByIndex(0);
}

void maintainPrint(int timeSinceStartMinutes)
{
	static const unsigned long REFRESH_INTERVAL = 1000; // ms
	static unsigned long lastRefreshTime = 0;
	
	if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
	{
		lastRefreshTime += REFRESH_INTERVAL;
                printInformation(timeSinceStartMinutes);
	}
}

void printInformation(int minutesElapsed)
{

    Serial.println("S");
    Serial.println(setSpeed);
    Serial.println("C");
    Serial.println(Input);
    Serial.println("T");
    Serial.println(Setpoint);
    Serial.println("P");
    Serial.println((minutesElapsed / times[nbrOfSequences]) * 100);

}

void loop()
{
    timeSinceStartMillis = millis(); // Counting millis to minuteInMilliss

    Serial.println("timeSinceStartMillis");
    Serial.print(timeSinceStartMillis);


    timeSinceStartMinutes = timeSinceStartMillis / minuteInMillis;

    int programmingMode = digitalRead(PROG_BTN_PIN); // progBtnReading the value of the button and setting progBtnReading

    if (programmingMode == HIGH)
    {
        //software_Reset();
    }

    readTemp();

    int currentSequence = sequenceNumber(timeSinceStartMinutes);

    if (currentSequence == -1)
    {
        while (true)
        {
            Serial.print("done");
            Setpoint = 0;
            setSpeed = 0;
            sendSpeed();
            digitalWrite(HEATER_RELAY_PIN, LOW);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Finished");
            delay(1000);
        }
    }

    Setpoint = temperatures[currentSequence];
    setSpeed = speeds[currentSequence];
    sendSpeed();

    Serial.println(setSpeed);
    Serial.println(timeSinceStartMinutes);

    Serial.println("sequence");
    Serial.println(currentSequence);


    Serial.println("Input");
    Serial.println(Input);

    Serial.println("Output");
    Serial.println(Output);

    //maintainPrint(timeSinceStartMinutes);

    //printInformation(timeSinceStartMinutes);    
    myPID.Compute();
    heaterLoop();
    Serial.println("running");
}