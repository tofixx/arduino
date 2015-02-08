/*
  Retrieves the raw data from temperature sensor
  Omron D6T44L06 PIR SENSOR 4x4 PIXEL THERMAL PCB
  Result size is 35 Bytes, the I2C buffer size needs 
  to be changed from 32 to 64 bytes in wire.h and twi.h
  some parts are based on sensor whitepaper from
  [1] http://www.mouser.com/pdfdocs/D6T01_ThermalIRSensorWhitepaper.pdf
  use getTemperature(int [] fields), returns -1 on fail if you only need one average value
*/

#ifndef DEBUG
#define DEBUG false
#endif

#include<Wire.h>

#define I2C_BUF_LENGTH 35 // 35 for 4x4 sensor, 19 for 1x8 version

/*
  Writes raw data from sensor to byte buffer of size I2C_BUF_LENGTH.
  Sensor is updating values only 4 times each second.
  Applies a checksum test on retrieved data, 
  returns false on retrieval error, true on success.
  buf contains pTat, p1, p2, ..., p16 in words and a one byte checksum 
  - pTat is temperature on shield
  - p1, ... pX are the temperature pixel values from thermal array sensor
  - checksum is used for crc-8 error detection on i2c bus
*/
boolean getTempRawData(byte * buf)
{
  // wake up sensor for transmission by writing a message
  Wire.beginTransmission(0x0a);                    // use address 0x0a for transmission
  Wire.write(0x4c);                                // send message that initiates an answer 
  Wire.endTransmission(false);                     // stop transmission
  Wire.requestFrom(0x0a, I2C_BUF_LENGTH);          // request 35 bytes from sensor (address 0x0a)
  int i=0; 
  while(Wire.available() && i <= I2C_BUF_LENGTH)   // receive bytes until end
  { 
      buf[i] = Wire.read();                        // write to buf
      if(DEBUG)
      {
        Serial.print(buf[i],DEC);
        Serial.print(" ");
      }
      i++;
  }
  if(DEBUG) Serial.println();
  return checkTemperatureRaw(I2C_BUF_LENGTH - 1);  
}

/*
  Runs a crc-8 check on recieved data [1]
*/
boolean checkTemperatureRaw(int pPEC)
{ 
  byte crc; 
  int i; 
  crc = calc_crc( 0x14 ); 
  crc = calc_crc( 0x4C ^ crc ); 
  crc = calc_crc( 0x15 ^ crc ); 
  for(i=0;i<pPEC;i++)
  { 
    crc = calc_crc(temperatureRaw[i] ^ crc ); 
  } 
  return (crc == temperatureRaw[pPEC]); 
}

/*
  CRC-8 calculation [1]
*/
byte calc_crc( byte data ) 
{ 
  int index; 
  unsigned char temp; 
  for(index=0;index<8;index++)
  { 
    temp = data; 
    data <<= 1; 
    if(temp & 0x80) 
    {
      data ^= 0x07; 
    }
  } 
  return data; 
} 


/*
  Translates the temperature raw data to 10 times celsius degrees
  writes array tempValues which will contain bytes of pTat, p1, ..., pX
*/
int convertTemperaturesFromRaw (byte * raw, int * tempValues)
{
  int i = 0;
  int j = 0;
  // combine all words, ignoring checksum
  while(i<I2C_BUF_LENGTH)
  {
    // create temp values from raw words
    tempValues[j]=raw[i++];       // use as lower part as is is
    tempValues[j++]+=raw[i++]<<8; // use second value as msb only
    if(DEBUG)
    {
      Serial.print(tempValues[j-1],DEC);
      Serial.print(" ");
    }
  }
  if(DEBUG) Serial.println();
}

/*
  Return field Average value in celsius degrees from selected array fields,
  returns -1 on reading error
*/
int getTemperature(int * field){
  int result = 0;
  if(getTempRawData(temperatureRaw))
  {
    // update values if correctly recieved new data
    convertTemperaturesFromRaw(temperatureRaw, temperatureValues);
    int i = 0;
    // summarize all values
    while(i<sizeof(field))
    {
      if(DEBUG)
      {
        Serial.print("Temperature value ");
        Serial.print(i);
        Serial.print(" is ");
        Serial.print(temperatureValues[field[i]],DEC);
        Serial.println();
      }
      result += temperatureValues[field[i++]];
    }
    // dividing through number of values for getting their average
    result = result / i;
    if(DEBUG) 
    {
      Serial.print("Average temperature is ");
      Serial.println(result);
    }
    return result;
  }
  else
  {
    if(DEBUG) Serial.print("error while updating temperature values, continue...");
    return -1;
  }
  
}

