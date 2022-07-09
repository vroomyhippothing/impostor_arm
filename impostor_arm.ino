
#include <Arduino.h>

uint8_t storedPin = 9;
uint8_t workingPin = 10;
uint8_t clawPin = 20;
uint8_t compressorRunPin = 15;
uint8_t clawVentValvePin = 16;
uint8_t clawPressurizeValvePin = 17; // A3

unsigned long lastMicros = 0;
unsigned long loopTimeMicros = 0;

float storedPressure = 0;
float workingPressure = 0;
float clawPressure = 0;

bool runCompressor = false;
bool clawVent = false;
float clawPressurize = 0;

void setup()
{
    Serial.begin(115200);
    analogWriteResolution(8);
    pinMode(storedPin, OUTPUT);
    analogWriteFrequency(storedPin, 75000);
    pinMode(workingPin, OUTPUT);
    analogWriteFrequency(workingPin, 75000);
    pinMode(clawPin, OUTPUT);
    analogWriteFrequency(clawPin, 75000);
    pinMode(compressorRunPin, INPUT);
    pinMode(clawVentValvePin, INPUT);
    pinMode(clawPressurizeValvePin, INPUT);
}

// the loop routine runs over and over again forever:
void loop()
{
    // time
    unsigned long tempMicros = micros();
    loopTimeMicros = tempMicros - lastMicros;
    float time = loopTimeMicros / 1000000.0;
    lastMicros = tempMicros;
    // read signals
    clawVent = digitalRead(clawVentValvePin);
    runCompressor = digitalRead(compressorRunPin);
    clawPressurize = constrain(map((float)analogRead(clawPressurizeValvePin), 20, 1023, 0.0, 1.0), 0, 1);

    // pneumatics simulation
    if (runCompressor) {
        if (storedPressure < 120) { // simulating pressure switch ensuring the compressor turns off at 120psi
            storedPressure += 120.0 / 40 * time;
        }
    }

    float storedToWorkingFlow = (constrain(storedPressure, 0, 60 /*regulator setting*/) - workingPressure) * 5.0 * time;

    float workingToClawFlow = ((workingPressure - clawPressure) * time * 0.8) * (clawPressurize);

    float clawToAtmosphereFlow = (clawVent ? clawPressure * 1.5 * time : 0);

    storedPressure -= storedToWorkingFlow;
    workingPressure += storedToWorkingFlow;

    workingPressure -= workingToClawFlow;

    clawPressure += 5.0 * (workingToClawFlow - clawToAtmosphereFlow);

    // send sensor data
    analogWrite(storedPin, constrain(map(storedPressure, -10, 200, 0, 255), 0, 255));
    analogWrite(workingPin, constrain(map(workingPressure, -10, 160, 0, 255), 0, 255));
    analogWrite(clawPin, constrain(map(clawPressure, -10, 80, 0, 255), 0, 255));

    Serial.print("stored:");
    Serial.print(storedPressure);
    Serial.print(",working:");
    Serial.print(workingPressure);
    Serial.print(",claw:");
    Serial.print(clawPressure);
    Serial.print(",runCompressor:");
    Serial.print(runCompressor);
    Serial.print(",vent:");
    Serial.print(clawVent);
    Serial.print(",press:");
    Serial.print(clawPressurize);
    Serial.println();
    delay(2);
}