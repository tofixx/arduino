#include <Wire.h>

/**
  Example code for receiving temperature values over 
  I2C by using the Arduino Wire Library from Omron D6T-44L or D6T-8L thermal array. 
  
  More information about you may find at:
  * http://www.mouser.com/pdfdocs/D6T01_ThermalIRSensorWhitepaper.pdf
  * http://www.omron.com/ecb/products/sensor/11/d6t.html
  
  Connection (Sensor to Arduino):
  1 = SCL to A5                 _____        Lens
  2 = SDA to A4             ____|___|____    Board
  3 = VCC to 5V              |_x_x_x_x_|     Connectors
  4 = GND to GND               1 2 3 4
  
  The Serial Monitor will show the following values:
    XX.X,     XX.X,       XX.X,       ..., XX.X     - X
  = TempSens, TempField1, TempField2, ..., Checksum - ByteCounter
  
  Normally the Sensor transmits it's temperature on the board first,
  a checksum at the end and the single temperature values of the matrix (4x4 or 1x8) between.
  Because the maximum buffer length the arduino will receive is 32 Bytes,
  for the 4x4 matrix the last value and ckecksum will be lost.
  
  You may raise the value if you want, by changing in
   - Wire.h: #define BUFFER_LENGTH 32 -> 64
   - twi.h:  #define TWI_BUFFER_LENGTH 32 -> 64

  @author: https://github.com/tofixx
  
**/

void setup()
{
  Wire.begin();                     // join i2c bus (address optional for master)
  Serial.begin(9600);               // start serial for output
}

void loop()
{
  int i=0; // counter for number of bytes received
  
  // wake up sensor for transmission by writing a message
  Wire.beginTransmission(0x0a);    // use address 0x0a for transmission
  Wire.write(0x4c);                // send message that initiates an answer 
  Wire.endTransmission(false);     // stop transmission
  
  Wire.requestFrom(0x0a, 35);      // request 35 bytes from sensor (address 0x0a)
  
  while(2 <= Wire.available())     // receive bytes in touples, until end
  { 
      int first  = Wire.read();    // read lower bits: 128...1
      int second = Wire.read()<<8; // read most significant bit, shift it for getting 256 or 0
      int result = first+second;   // recieved number / 10.0 will give you a decimal in °C
      
      Serial.print(i>0?", ":"");   // output separator
      Serial.print(result/10.0,1); // print result value as decimal
      
      i+=2; // raise the bytes received counter
   }
  
  if(Wire.available())
  {
    int checksum = Wire.read();
    i++;
    Serial.print(" - ");
    Serial.print(checksum,BIN);
  }
   
  // as described above, the message will be incomplete!
  
  Serial.print(" - ByteCounter: ");    
  Serial.println(i);               

  delay(250);                      // remind, the sensor will update it's values only 4 times a second
}
