# LoveButton

> This library turns the Love Pin (the little heart shaped blob of 
> solder on the back of the Arduino UNO-R4 Minima) into a capacitive 
> touch sensor.
<br>
# It includes a pre-created instance called love
<br><br>




# To use LoveButton
 First call `love.begin()` in `setup()`

 Call `love.read()` to read the sensor.  
 This function returns a boolean value touched or not

 Use `Serial.print(love.debug());` to print the raw numbers from the touch sensor

 If the sensor doesn't work, then use the debug method to see what numbers you are 
 getting in the diff reading when touching or not touching.  
 The default threshold is set to 23000, but if you find that your numbers are different
 then you can call `love.setThreshold(NNN)` where NNN is the threshold you want to use.  




