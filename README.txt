****HOLTER MONITOR FIRMWARE ****

1. Add the files 'ver3.ino' and 'ver3_i2c_subsys.ino' as two files to the same sketch (use tab options of the IDE)
2. Compile and upload using Arduino 1.6.4 or higher versions of the IDE. It is not guarenteed to work on previous versions of the IDE
3. If compilation fails with error message "wire.h" not found and "Fat-16Master" not found, install these libraries from the website arduino.cc or github page of these libraries
4. Upload the code to the board. It works on uno, mega, duemilanove
5. Wireup the circuit as shown in comments of the code. Wiring is straight forward. RTC connected to SDA,SCL pins, and microSD connected to MISO, MOSI, SCK of the module, ECG amplifier connected to A0 analog channel(programmable).
6. You are good to go. Powerup the circuit, and it should print log messages onto serial port.
