# p100_sensor_manager
A project to connect two MAX31865 sensors to an arduino and have them managed by the serial console.

Just copy the contents into the Arduino directory within your home directory then load onto the Arduino I used Nanos. This project is designed to have two MAX31865 sensors connected but there could be more added with cable select pins. 

The console use and instructions are available if you go to the console monitor and press h

This will allow you to enter the RREF calibration data needed for these sensors and stores it in EEPROM. You can also number the sensor and print out the config status. Pressing enter only return a JSON string of the sensor data.


