#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "air_pressure_log.h"

int fd;

char I2C_readByte(int reg)
{
	return wiringPiI2CReadReg8(fd, reg);
}

unsigned short I2C_readU16(int reg)
{
	return wiringPiI2CReadReg16(fd, reg);
}
void I2C_writeByte(int reg, int val)
{
	wiringPiI2CWriteReg8(fd, reg, val);
}
void LPS22HB_RESET()
{   unsigned char Buf;
    Buf=I2C_readU16(LPS_CTRL_REG2);
    Buf|=0x04;                                         
    I2C_writeByte(LPS_CTRL_REG2,Buf);                  //SWRESET Set 1
    while(Buf)
    {
        Buf=I2C_readU16(LPS_CTRL_REG2);
        Buf&=0x04;
    }
}
void LPS22HB_START_ONESHOT()
{
    unsigned char Buf;
    Buf=I2C_readU16(LPS_CTRL_REG2);
    Buf|=0x01;                                         //ONE_SHOT Set 1
    I2C_writeByte(LPS_CTRL_REG2,Buf);
}
unsigned char LPS22HB_INIT()
{
    fd=wiringPiI2CSetup(LPS22HB_I2C_ADDRESS);
    if(I2C_readByte(LPS_WHO_AM_I)!=LPS_ID) return 0;    //Check device ID 
    LPS22HB_RESET();                                    //Wait for reset to complete
    I2C_writeByte(LPS_CTRL_REG1 ,   0x02);              //Low-pass filter disabled , output registers not updated until MSB and LSB have been read , Enable Block Data Update , Set Output Data Rate to 0 
    return 1;
}

float obtain_single_pressure_value() {   
    unsigned char u8Buf[3];
    LPS22HB_START_ONESHOT();
    if((I2C_readByte(LPS_STATUS) & 0x01) == 0x01) {
        u8Buf[0] = I2C_readByte(LPS_PRESS_OUT_XL);
        u8Buf[1] = I2C_readByte(LPS_PRESS_OUT_L);
        u8Buf[2] = I2C_readByte(LPS_PRESS_OUT_H);
        return (float) ((u8Buf[2]<<16)+(u8Buf[1]<<8)+u8Buf[0]) / 4096.0f / 1.3332f;
    } else {
        sleep(1);
        return obtain_single_pressure_value();
    }
}

int main() {
    float pressure = 0;
    FILE * fptr;
    time_t rawtime;
    struct tm * ltime;
    int str_len = 40;
    char * str = malloc(str_len * sizeof(char));

    if (wiringPiSetup() < 0) {
        printf("\nWiringPi setup error\n");
        return 1;
    }
    if(!LPS22HB_INIT()) {
        printf("\nLPS22HB initialization error\n");
        return 2;
    }
    pressure = obtain_single_pressure_value();

    fptr = fopen("/var/www/html/pressure.csv", "a");
    time(&rawtime);
    ltime = localtime(&rawtime);
    snprintf(str, str_len, "%04d-%02d-%02dT%02d:%02d:%02d, %6.2f\r\n",
        ltime->tm_year + 1900,
        ltime->tm_mon + 1,
        ltime->tm_mday,
        ltime->tm_hour,
        ltime->tm_min,
        ltime->tm_sec,
        pressure);
    fprintf(fptr, str);
    fclose(fptr);

    if (str != NULL) {
        free(str);
        str = NULL;
    }
    return 0;
}