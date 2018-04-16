#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pcf8591.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <wiringPiI2C.h>
#include "bme280.h"

#define PCF       120
int fd;
bme280_calib_data cal;
bme280_raw_data raw;
float temperature;
int moisture;
int RCpin = 5;
int dry = 0;
int wet = 1;
int Rpin = 0;

int bme280_init() {
  fd = wiringPiI2CSetup(BME280_ADDRESS);
  if(fd < 0) {
    printf("Device not found");
    exit(-1);
  }

   return 0;
}

int stream_sensor_data() {

  readCalibrationData(fd, &cal);

  wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   // humidity oversampling x 1
  wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   // pressure and temperature oversampling x 1, mode normal
      
  getRawData(fd, &raw);

  int32_t t_fine = getTemperatureCalibration(&cal, raw.temperature);
  temperature = compensateTemperature(t_fine); // C
 // float p = compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
 // float h = compensateHumidity(raw.humidity, &cal, t_fine);       // %
 // float a = getAltitude(p);                         // meters
/*
  printf("{\"sensor\":\"bme280\", \"humidity\":%.2f, \"pressure\":%.2f,"
    " \"temperature\":%.2f, \"altitude\":%.2f, \"timestamp\":%d}\n",
    h, p, t, a, (int)time(NULL));
*/
  return 0;
}

int32_t getTemperatureCalibration(bme280_calib_data *cal, int32_t adc_T) {
  int32_t var1  = ((((adc_T>>3) - ((int32_t)cal->dig_T1 <<1))) *
     ((int32_t)cal->dig_T2)) >> 11;

  int32_t var2  = (((((adc_T>>4) - ((int32_t)cal->dig_T1)) *
       ((adc_T>>4) - ((int32_t)cal->dig_T1))) >> 12) *
     ((int32_t)cal->dig_T3)) >> 14;

  return var1 + var2;
}

void readCalibrationData(int fd, bme280_calib_data *data) {
  data->dig_T1 = (uint16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_T1);
  data->dig_T2 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_T2);
  data->dig_T3 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_T3);

  data->dig_P1 = (uint16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P1);
  data->dig_P2 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P2);
  data->dig_P3 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P3);
  data->dig_P4 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P4);
  data->dig_P5 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P5);
  data->dig_P6 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P6);
  data->dig_P7 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P7);
  data->dig_P8 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P8);
  data->dig_P9 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P9);

  data->dig_H1 = (uint8_t)wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H1);
  data->dig_H2 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_H2);
  data->dig_H3 = (uint8_t)wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H3);
  data->dig_H4 = (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H4) << 4) | (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H4+1) & 0xF);
  data->dig_H5 = (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H5+1) << 4) | (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H5) >> 4);
  data->dig_H6 = (int8_t)wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H6);
}

float compensateTemperature(int32_t t_fine) {
  float T  = (t_fine * 5 + 128) >> 8;
  return T/100;
}

float compensatePressure(int32_t adc_P, bme280_calib_data *cal, int32_t t_fine) {
  int64_t var1, var2, p;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)cal->dig_P6;
  var2 = var2 + ((var1*(int64_t)cal->dig_P5)<<17);
  var2 = var2 + (((int64_t)cal->dig_P4)<<35);
  var1 = ((var1 * var1 * (int64_t)cal->dig_P3)>>8) +
    ((var1 * (int64_t)cal->dig_P2)<<12);
  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)cal->dig_P1)>>33;

  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p<<31) - var2)*3125) / var1;
  var1 = (((int64_t)cal->dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((int64_t)cal->dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)cal->dig_P7)<<4);
  return (float)p/256;
}


float compensateHumidity(int32_t adc_H, bme280_calib_data *cal, int32_t t_fine) {
  int32_t v_x1_u32r;

  v_x1_u32r = (t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t)cal->dig_H4) << 20) -
      (((int32_t)cal->dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
         (((((((v_x1_u32r * ((int32_t)cal->dig_H6)) >> 10) *
        (((v_x1_u32r * ((int32_t)cal->dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
      ((int32_t)2097152)) * ((int32_t)cal->dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
           ((int32_t)cal->dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r>>12);
  return  h / 1024.0;
}

void getRawData(int fd, bme280_raw_data *raw) {
  wiringPiI2CWrite(fd, 0xf7);

  raw->pmsb = wiringPiI2CRead(fd);
  raw->plsb = wiringPiI2CRead(fd);
  raw->pxsb = wiringPiI2CRead(fd);

  raw->tmsb = wiringPiI2CRead(fd);
  raw->tlsb = wiringPiI2CRead(fd);
  raw->txsb = wiringPiI2CRead(fd);

  raw->hmsb = wiringPiI2CRead(fd);
  raw->hlsb = wiringPiI2CRead(fd);

  raw->temperature = 0;
  raw->temperature = (raw->temperature | raw->tmsb) << 8;
  raw->temperature = (raw->temperature | raw->tlsb) << 8;
  raw->temperature = (raw->temperature | raw->txsb) >> 4;

  raw->pressure = 0;
  raw->pressure = (raw->pressure | raw->pmsb) << 8;
  raw->pressure = (raw->pressure | raw->plsb) << 8;
  raw->pressure = (raw->pressure | raw->pxsb) >> 4;

  raw->humidity = 0;
  raw->humidity = (raw->humidity | raw->hmsb) << 8;
  raw->humidity = (raw->humidity | raw->hlsb);
}

float getAltitude(float pressure) {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  return 44330.0 * (1.0 - pow(pressure / MEAN_SEA_LEVEL_PRESSURE, 0.190294957));
}

int RCtime(RCpin){
      //wiringPiSetup();
      pinMode(RCpin, INPUT);
      int reading = 0;	
      delay(1);
      int sensorState = digitalRead(RCpin);
      while(1){
	if (sensorState == 0){
		reading += 1;
	}
	if  (reading >= 1000) {
		return 0;
	}	
		if (sensorState != 0){
			return 1;
		}
	}
	return 0;
}

void buzz_on(Rpin){
	pinMode(Rpin, INPUT);
	pinMode(Rpin, HIGH);
}


void buzz_off(Rpin){
	pinMode(Rpin, INPUT);
	pinMode(Rpin, LOW);
}

int moistureSensor(){
      if (RCtime(RCpin) == dry)
            {
                  //printf("Sensor is still dry... \n");
                  buzz_on(Rpin);
                  return 0;
            }
      if (RCtime(RCpin) == wet)
            {
                  buzz_off(Rpin);
                  //printf("Sensor is wet... \n");
                  return 1;
            }
}
int main (void)
{
      char hvacStatus[128] = "";
	int padPin = 4;
	int alreadyPressed = 0;
	int value1, value2;
	int data1, data2;
	int counter = 0;
	int step = 1; //1
	int i = 0;
	int offset = 85;  //85
	int deviation = 10; //10
	int noiseCounter = 0;
	int noiseDuration = 3; //20
	int noiseOffCounter = 0;
	int noiseOffDuration = 1000; //1000
	int noise = 0;
      int padPressed;
      
	if (wiringPiSetup () == -1) {
		printf("Error at wiringPiSetup()");
    		return 1 ;
	}
	pinMode(padPin, INPUT);
	pinMode (0, OUTPUT) ;         // aka BCM_GPIO pin 17
	pinMode (1, OUTPUT) ;	// aka BCM_GPIO pin 18
 

	// Setup pcf8591 on base pin 120, and address 0x48
//	printf("%d\n", ++step);

	if (pcf8591Setup (PCF, 0x48) == -1) {
		printf("Error at pcf8591Setup()");
    		return 1 ;
	}
	
       printf("Lowest noise level \t\t\t%d\n", offset);
       printf("Data has been generalized with weight value of %d\n", step);
      

      bme280_init();

      while(1)
      {
                  stream_sensor_data();
                  moisture = moistureSensor();

			padPressed = digitalRead(padPin);
			if(padPressed)
			{
				if(alreadyPressed){}
				else
				{
                              strcpy(hvacStatus, "Pressed");
					alreadyPressed = 1;
			      }
			}
			else
			{
				alreadyPressed = 0;
                        strcpy(hvacStatus, "Unpressed");
			}
		data1 += analogRead  (PCF + 0);	//white - local
		data2 += analogRead  (PCF + 1);	//yellow - remote
		counter++;
		if(noise) {
			noiseOffCounter++;
		}

		if(counter == step ) {
			counter = 0;
			value1 = round((double)data1 / step);
			value2 = round((double)data2 / step);
			data1 = 0;
			data2 = 0;

			if ((value1 < offset) || (value2 < offset)) {

				if( abs (value1 - value2) < deviation)  {
					if (++noiseCounter > noiseDuration) {
						noiseCounter = 0;
						noiseOffCounter = 0;
						noise = 1;
					}
				}
				else  {
					if(noise) {
						if(noiseOffCounter > noiseOffDuration) {
							noiseOffCounter = 0;
							noise = 0;						
						}
					}
					else	if( --noiseCounter < 0)  {
							noiseCounter = 0;
					}
				}
                        

				if(noise) {
					printf("\n\t%s\tNOISE !!!!  #1:%3d #2:%3d Temperature: %4.2f Moisture: %d", hvacStatus, value1, value2, temperature, moisture);
					//both microphones are sensing noise - red 
					digitalWrite (0, LOW);
					digitalWrite (1, HIGH);
				}
				else {
					printf("\n\t%s\tVoice In!!  #1:%3d #2:%3d Temperature: %4.2f Moisture: %d", hvacStatus, value1, value2, temperature, moisture);
					// Voice in at any of microphones - green
					digitalWrite (0, HIGH);
					digitalWrite (1, LOW);
				}

				for(i = offset; (i > offset - value1) && (i > 0); i--) {
					printf(" ");
				}
				for(i = offset; i > value1; i--) {
					printf("-");
				}
				printf("|");
				for(i = offset; i > value2; i--) {
					printf("+");
				}
			}
			else  {
				if(!noise) {
					digitalWrite (0, LOW);
					digitalWrite (1, LOW);
				}
			}
		}
	}
	return 0;
}

