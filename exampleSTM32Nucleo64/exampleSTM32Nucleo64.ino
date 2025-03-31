/*
 * Copyright (c) 2025, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0


// Prepare I2C buses
// I2C Bus A on Pins 14 (SDA) / 15 (SCL)
const int sda_A = 14;
const int scl_A = 15;
TwoWire i2cBusA(sda_A, scl_A);
// I2C Bus B on Pins 3 (SDA) / 6 (SCL)
const int sda_B = 3;
const int scl_B = 6;
TwoWire i2cBusB(sda_B, scl_B);


static char errorMessage[64];
static int16_t error;

SensirionI2cScd4x sensorA;
SensirionI2cScd4x sensorB;

bool sensorAOk;
bool sensorBOk;

void PrintUint64(uint64_t& value) {
    Serial.print("0x");
    Serial.print((uint32_t)(value >> 32), HEX);
    Serial.print((uint32_t)(value & 0xFFFFFFFF), HEX);
}

bool initSensor(SensirionI2cScd4x sensor) {
    uint64_t serialNumber = 0;
    delay(30);
    // Ensure sensor is in clean state
    error = sensor.wakeUp();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute wakeUp(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
    error = sensor.stopPeriodicMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }

    error = sensor.reinit();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute reinit(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
    // Read out information about the sensor
    error = sensor.getSerialNumber(serialNumber);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return false;
    }
    Serial.print("serial number: ");
    PrintUint64(serialNumber);
    Serial.println();
    return true;
}

bool startMeasurement(SensirionI2cScd4x sensor) {
    error = sensor.startPeriodicMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return false;
    }
    return true;
}

// Read measurements from the sensor and print to the console
// The method blocks until measurements are ready
void readAndPrintMeasurement(SensirionI2cScd4x sensor) {
    bool dataReady = false;
    uint16_t co2Concentration = 0;
    float temperature = 0.0;
    float relativeHumidity = 0.0;

    error = sensor.getDataReadyStatus(dataReady);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute getDataReadyStatus(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    while (!dataReady) {
        delay(100);
        error = sensor.getDataReadyStatus(dataReady);
        if (error != NO_ERROR) {
            Serial.print("Error trying to execute getDataReadyStatus(): ");
            errorToString(error, errorMessage, sizeof errorMessage);
            Serial.println(errorMessage);
            return;
        }
    }
    error =
        sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    //
    // Print results in physical units.
    Serial.print("CO2 concentration [ppm]: ");
    Serial.print(co2Concentration);
    Serial.println();
    Serial.print("Temperature [°C]: ");
    Serial.print(temperature);
    Serial.println();
    Serial.print("Relative Humidity [RH]: ");
    Serial.print(relativeHumidity);
    Serial.println();
}

void initI2c() {
    i2cBusA.begin();
    sensorA.begin(i2cBusA, SCD41_I2C_ADDR_62);

    i2cBusB.begin();
    sensorB.begin(i2cBusB, SCD41_I2C_ADDR_62);

    Serial.println("I2C Buses configured for STM32 Nucleo 64 Board.");
    Serial.printf("I2C Bus A SDA Pin %i, SCL Pin %i; I2C Bus B SDA Pin %i, SCL PIN %i\n", sda_A, scl_A, sda_B, scl_B);
}


void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    initI2c();

    Serial.println("----SENSOR A-----");
    sensorAOk = initSensor(sensorA);
    if (sensorAOk) {
        sensorAOk = startMeasurement(sensorA);
    }

    Serial.println("----SENSOR B-----");
    sensorBOk = initSensor(sensorB);
    if (sensorBOk) {
        sensorBOk = startMeasurement(sensorB);
    }
}

void loop() {
    //
    // Slow down the sampling to 0.2Hz.
    //
    delay(5000);

    if (sensorAOk) {
        Serial.println("----SENSOR A-----");
        readAndPrintMeasurement(sensorA);
    }
    if (sensorBOk) {
        Serial.println("----SENSOR B-----");
        readAndPrintMeasurement(sensorB);
    }
}