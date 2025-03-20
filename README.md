# Introduction
You have a micro-controller and two identical sensors, meaning that they have the same I2C address. Without a multiplexer or the possibility to configure the I2C address of the sensor, you can not attach them to the same I2C bus. On a board providing the possibility to configure any GPIO pin pairs as a I2C bus, you can connect the two sensors to their own I2C Bus. In this article we will explain how to set up different I2C Bus.

# The Ingredients
In this example we use a ESP32, which supports multiple I2C Busses. You can use any two sensors, being the same product or different sensors with the same I2C address. For the example we will use two Sensirion SCD41 CO2 sensors. To be able to connect the sensors by cable without soldering, we use the Sensirion Development Kit Boards for SCD41.

# General working principle
The two SCD41 sensors we want to connect have an I2C address 0x62, which cannot be changed. Thus, in order to communicate with both from the micro controller, we use a separate I2C Bus for each. Each I2C Bus needs one Pin for the SDA line and one for the SCL line. The first one we will attach to the standard I2C Pins. For the second I2C Bus, we need to define two GPIO Pins we want to use and connect the second sensor to those pins. 
On the software side, you can use the standard "Wire" for the first sensor connected to the default pins. For the second one we will need to configure to use the chosen Pins. As a last step, we will create two instances of our sensor driver class and initialize one with the standard I2C Bus and one with the additionally set up one.

# Pull Up Resistors on I2C Lines

When checking out the I2C protocol, you will notice that the I2C SCL and SDA lines have a high state when idle. When the master or slave communicates, they pull the line to low state. An external component is needed to pull the lines back to high, which are the Pull-up resistors. Without those Pull-up's the lines would be floating, which means in an undefined state. This can lead to wrong detection of Low-State and thus introduce errors in your communication.

Having Pull-up resistors in place is essential for good signal quality and reliable data exchange. If you have unreliable communication, for example sporadic data drop outs or only part of the data arrives, you should check the Pull-up resistor configuration. 

Where can Pull-up Resistors be placed:

1. on the development board you are connecting 
2. on the micro-controller
3. manually wired on the SDA and SCL line between micro-controller and development board, e.g. by using a bread board

Pull-up resistors are connected from SDA resp SCL line to positive supply voltage. Standard resistor values are 4.7 kΩ or 10 kΩ. You might also find a optimal Pull-up resistor value for your sensor in the sensor or development board datasheet.

If there are Pull-up resistors on the board and the micro-controller, the Pull-up resistances are in parallel meaning that there overall value drops. This could well lead to a too small Pull-up resistance, meaning that your devices cannot drive the lines to the low level reliably anymore.

The Pull-up resistance value depends also on the I2C bus frequency you want to use. In this tutorial we use the default frequency of 100kHz.

The good news is that for the example we do here with an ESP32 DevKit, you do not need to do anything to configure or wire Pull-up resistoras as it is handled by the board. 

# Wiring

First, we need to define the Pins we use for the two I2C Buses.
For I2C Bus A we can use the default I2C Pins of the board. For the ESP32 DevKitC those are Pin 21 (SCL) and 22 (SDA). 
For I2C Bus B, we can choose any GPIO (General Purpose Input Output) pins. On the ESP32 DevKitC, all pins can be used. If you have another board, check the specification to see if some pins have a special configuration and cannot be used as a general GPIO pin. We chose Pin 17 and 16.
As our sensor works with 3.3V as well as 5V, we can plug in one sensor at the 3V pin and the other at the 5V pin. Otherwise you would have to connect both sensors to the respective pin, e.g. over a bread board.
With that information, the wiring should be done as follwoing:

    SEK-SCD41 A - Pin 1 to ESP32 Pin 22 (SCL, yellow cable) 
    SEK-SCD41 A - Pin 2 to ESP32 GND (Ground, black cable) 
    SEK-SCD41 A - Pin 3 to ESP32 3V3 (Sensor supply voltage, red cable) 
    SEK-SCD41 A - Pin 4 to ESP32 Pin 21 (SDA, green cable) 


    SEK-SCD41 B - Pin 1 to ESP32 Pin 17 (SCL, yellow cable) 
    SEK-SCD41 B - Pin 2 to ESP32 GND (Ground, black cable) 
    SEK-SCD41 B - Pin 3 to ESP32 5V (Sensor supply voltage, red cable) 
    SEK-SCD41 B - Pin 4 to ESP32 Pin 16 (SDA, green cable)

What we have to remember for the configuration in the software later is the pins we used for the second I2C Bus, namely pin 17 for I2C clock (SCL) and pin 18 for I2C data (SDA).

The ESP32 DevKitC Board does make sure that GPIO lines are pulled to high state. So you do not manually wire or configure Pull Up resistors for the pins you are going to use. For more on Pull ups !!!! TODO !!!

For other boards, you find the default I2C wiring on: https://docs.arduino.cc/learn/communication/wire/

# Software I2C Bus Setup


Depending on your board and the implementation, there is a second Wire1 object predefined. If not, you have to dig one layer deeper and instantiate a TwoWire object and assign the correct pins. There is a good explanation with different variants for the ESP32 boards using the Arduino IDE: https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/

For the ESP32 on Arduino IDE Platform there is a predefined instance of "TwoWire" named "Wire" which is configured for the default I2C Bus on pins 21/22. 
We can use this instance without any modification for the sensor attached to the "I2C Bus A" (default I2C Bus).

For the "I2C Bus B" we need to configure a custom "TwoWire" instance. There is a predefined instance named "Wire1" we can configure to use the pins we defined with the following lines of code within the `setup()` function:

```
const int sda_B = 16;
const int scl_B = 17;
Wire1.begin(sda_B, scl_B);
```

# Software Sensor Communication

The code sending the commands to the sensor over the I2C Bus needs to know wich Bus to use for which sensor. 
Thus, you need to configure the sensors instances accordingly. First, create a driver instance per sensor. 
This should be done outside of any function, such that those can be refered to from within `setup()` and `loop()`.

```
SensirionI2cScd4x sensorA;
SensirionI2cScd4x sensorB;
```

Then, in the `setup()` function, assign the I2C Buses to the sensors:

```
sensorA.begin(Wire, SCD41_I2C_ADDR_62);
sensorB.begin(Wire1, SCD41_I2C_ADDR_62);
```

Look out that you really have `Wire1` assigned for sensorB, so that it uses the custom set up I2C Bus.

Know you can issue any I2C command on the sensor, e.g. starting the measurement and reading out values. 
The full example code is linked.