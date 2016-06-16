#include<Wire.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define DS1307_CTRL_ID 0x68			// address of the DS1307 real-time clock
#define NumberOfFields 3			// the number of fields (bytes) to request from the RTC
#define ADXL345_DEV_ID 0x53 		// address of the ADXL345 with ALT_ADDRESS pin connected LOW
#define NumberOfFields_adxl 6 		// the number of fields (bytes) to request from ADXL345

#define DATAX0 0x32
#define POWER_CTL 0x2D

/* initialize ADXL and RTC*/
void I2C_init()
{
	Wire.begin();
	Wire.beginTransmission(ADXL345_DEV_ID);
	Wire.write(POWER_CTL); //change mode to measurement
	Wire.write(0x08); //change to measure. Normal mode
	Wire.endTransmission();
	/* Enable 1Hz square wave output from RTC*/
	Wire.beginTransmission(DS1307_CTRL_ID);  // 
	Wire.write(0x07);
	Wire.write(0x10);
	Wire.endTransmission();
}


void get_time(byte*	hours, byte*	mins, byte*	secs)
{
	//byte _Second, _Minute, _Hour;
        /* Point towards seconds field  */
	Wire.beginTransmission(DS1307_CTRL_ID);
	Wire.write(0x00);
	Wire.endTransmission();
	// request the 3 data fields(secs, min, hr)
	Wire.requestFrom(DS1307_CTRL_ID, NumberOfFields);
	*secs = _bcd2dec(Wire.read() & 0x7f);
	*mins = _bcd2dec(Wire.read() );
	*hours = _bcd2dec(Wire.read() & 0x3f); // mask assumes 24hr clock
	//hours = &_Hour;
	//mins = &_Minute;
	//secs = &_Second; 
}

void get_acceleration(int* x, int* y, int* z)
{
        byte _data_x0, _data_x1, _data_y0, _data_y1, _data_z0, _data_z1;
	/* Write register address of 'DATAX0' for initiating multibyte read */
	Wire.beginTransmission(ADXL345_DEV_ID);
	Wire.write(DATAX0); //address of DATAX0
	Wire.endTransmission();
  
	// requests 6 data bytes - X0,X1,Y0,Y1,Z0,Z1
	Wire.requestFrom(ADXL345_DEV_ID, NumberOfFields_adxl);
	_data_x0 = Wire.read();
	_data_x1 = Wire.read();
	_data_y0 = Wire.read();
	_data_y1 = Wire.read();
	_data_z0 = Wire.read();
	_data_z1 = Wire.read();
	*x = (_data_x1<<8) + _data_x0;
	*y = (_data_y1<<8) + _data_y0; 
	*z = (_data_z1<<8) + _data_z0;
}

byte _bcd2dec(byte num)
{
	byte tens = ((num & 0xf0)>>4);
	byte unit = (num & 0x0f);
	return ((tens*10)+unit);
}

