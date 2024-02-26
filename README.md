# LoveButton

> This library turns the Love Pin (the little heart shaped blob of 
> solder on the back of the Arduino UNO-R4 Minima or Arduino UNO-R4 WiFi) into a capacitive 
> touch sensor.
>
> 
> It requires the addition of a small capacitor be connected between 
> pin 10 and ground on the Arduino UNO-R4 Minima or between
> pin 7 and ground on the Arduino UNO-R4 WiFi
> The size isn't very important but it will affect the threshold
> I used 10uF but even as small as 1nF should still work.
> 
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


# ChangeLog
`v1.0` - Initial Release
<br>
`v1.1` - Bug Fixes
<br>
`v1.2` - Adds support for Arduino UNO-R4 WiFi.  (added by Winnie S. PR#1)
<br>


