# Pull Up Resistors on I2C Lines

When examining the I2C protocol, you will observe that the I2C SCL and SDA lines remain in a high state when idle.
When the I2C leader or follower communicate, they pull the line to a low state. To return the lines to a
high state, external components known as pull-up resistors are required. Without these pull-ups, the lines would be left
floating, resulting in an undefined state. This leads to incorrect detection of the low state and introduces
errors in your communication.

Having pull-up resistors in place is essential for ensuring good signal quality and reliable data exchange. If you are
experiencing unreliable communication, such as sporadic data dropouts or only receiving part of the data, it is
advisable to check the pull-up resistor configuration.

Where can pull-up resistors be placed:

1. On the development board you are connecting.
2. On the microcontroller.
3. Manually wired on the SDA and SCL lines between the microcontroller and development board, for example, by using a
   breadboard.

So if your development board or microcontroller has pull-ups built-in, you should be good to go.
For example, the ESP32 DevKit 4 has pull-ups built-in, but the STM32 Nucleo does not.

Pull-up resistors are connected from the SDA and SCL lines to the positive supply voltage. Standard resistor values are
4.7 kΩ or 10 kΩ. You may also find an optimal pull-up resistor value for your sensor in the sensor or development board
datasheet.

If there are pull-up resistors on both the board and the microcontroller, the pull-up resistances are in parallel,
resulting in a lower overall value. This could potentially lead to a pull-up resistance that is too small, causing your
devices to fail to drive the lines to the low level reliably.

The pull-up resistance value depends on various factors. If you can log the signals with a logic analyzer or
oscilloscope, you can check that the waveform of your I2C signals has sharp edges, indicating that your resistor setup
is appropriate.

For example, you observe the signal of a SEK SCD41 connected to a Nucleo 64 board. Neither the SEK SCD41 nor the STM
Nucleo 64 board includes pull-up resistors. Therefore, we need to connect a resistor between VDD and each SDA/SCL line,
requiring one resistor per line.

**Too low** If you have resistance that is too low or has no pull-ups, the lines will be floating.
In the setup here, you can observe that both lines are low (0V) in the idle state. When the I2C leader attempts to
communicate, you may see some signal, but the SDA line is essentially just following the clock signal.

Trace of setup with no resistors:
![LogicAnalyzer Snapshot with NO resistors](images/Nucleo64_I2c_No_PullUps.png)

**Good** If your resistors are properly dimensioned, the signal shape will appear more rectangular. The lower the
resistor value, the sharper the rectangular shape should be.
Also, note that at the beginning of the trace, you can observe that both the SDA and SCL lines are in a high state (~
3.3V).

Trace of setup with 2.2kOhm resistors:
![LogicAnalyzer Snapshot with 2.2kOhm resistors](images/Nucleo64_I2c_2p2kOhm_PullUps.png)

**Too big** If your resistor values are too large, the signal may take too long to recover to the high state and may not
reach the maximum voltage anymore (the maximum voltage reached in the trace during communication is approximately 3.1V).

Trace of setup with 18kOhm resistors:
![LogicAnalyzer Snapshot with 18kOhm resistors](images/Nucleo64_I2c_18kOhm_PullUps.png)