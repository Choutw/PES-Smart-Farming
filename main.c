#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <bme680_reg.h>

#define BME680_ADDR     0x77
#define MY_STACK_SIZE   5000

const struct device *i2c_dev ;
int32_t temp_convert(uint32_t temp_adc, int32_t  p1,int32_t  p2,int32_t  p3 ){
    int32_t var1 = ((int32_t)temp_adc >> 3) - ((int32_t)p1<< 1);
    int32_t var2 = (var1* (int32_t)p2) >> 11;
    int32_t var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)p3 << 4)) >> 14;
    int32_t t_fine = var2 + var3;
    int32_t temp_comp = (int32_t)((t_fine * 5) + 128) >> 8;
    return temp_comp;
}

int32_t humidity_convert(int32_t temp_comp,uint32_t humidity_adc,int32_t h1,int32_t h2,int32_t h3,int32_t h4,int32_t h5,int32_t h6,int32_t h7){
    int32_t temp_scaled=temp_comp;
    int32_t var1 = (int32_t)humidity_adc - (int32_t)(h1<<4) - (((temp_scaled*h3)/((int32_t)100))>>1);
    int32_t var2 = (h2*(((temp_scaled*h4)/((int32_t)100))+(((temp_scaled*((temp_scaled*h5)/((int32_t)100)))>>6)/((int32_t)100))+((int32_t)(1<<14))))>>10;
    int32_t var3 = var1*var2;
    int32_t var4 = ((h6<<7)+((temp_scaled*h7)/((int32_t)100)))>>4;
    int32_t var5 = ((var3 >> 14)* (var3 >>14))>>10;
    int32_t var6 =(var4 *var5)>>1;
    int32_t hum_comp = (((var3 +var6)>>10)*((int32_t)1000))>>12;
    return hum_comp;
}

void main(void)
{
    uint8_t  data[3];
    uint8_t h_data[2];//humidity register
    uint32_t temp_adc, humidity_adc;
    uint8_t  par_t[5];
    uint8_t par_h[9];//humidity register
    int32_t p1,p2,p3;
    int32_t h1,h2,h3,h4,h5,h6,h7;
    int32_t temp_comp;
    // Initialize I2C bus
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0)); 

    //tempurature register
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE9,&par_t[0]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0XEA,&par_t[1]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0x8A,&par_t[2]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0x8B,&par_t[3]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0x8c,&par_t[4]);

    p1=(int32_t) ((uint16_t)par_t[0] | ((uint16_t)par_t[1])<<8);
    p2=(int32_t)((uint16_t)par_t[2] | ((uint16_t)par_t[3])<<8);
    p3=(int32_t)par_t[4];

    //humidity register
    //i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE2,&par_h[0]);//vary
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE3,&par_h[1]);
    //i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE2,&par_h[2]);//vary
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE1,&par_h[3]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE4,&par_h[4]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE5,&par_h[5]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE6,&par_h[6]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE7,&par_h[7]);
    i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE8,&par_h[8]);

    // h1=(int32_t)((uint16_t)par_h[0] | (uint16_t)par_h[1]<<8);
    // h2=(int32_t)((uint16_t)par_h[2] | (uint16_t)par_h[3]<<8);
    h3=(int32_t)par_h[4];
    h4=(int32_t)par_h[5];
    h5=(int32_t)par_h[6];
    h6=(int32_t)par_h[7];
    h7=(int32_t)par_h[8];



    while(1){

        i2c_reg_write_byte(i2c_dev, BME680_ADDR,BME680_CTRL_MEAS,0b010 << 5 | 0b01);//force mode: 01

        //tempuratue below
        i2c_reg_read_byte(i2c_dev, BME680_ADDR,BME680_TEMP_MSB,&data[0]);
        i2c_reg_read_byte(i2c_dev, BME680_ADDR,BME680_TEMP_LSB,&data[1]);
        i2c_reg_read_byte(i2c_dev, BME680_ADDR,BME680_TEMP_XLSB,&data[2]); 

        temp_adc=((uint32_t)data[0])<<12 | ((uint32_t)data[1])<<4 | ((uint32_t)data[2])>>4;
        temp_comp=temp_convert(temp_adc,p1,p2,p3)/100;
        printk("Room Temperature is %d degrees Celsius\n", temp_comp);

        //humidity below
        i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE2,&par_h[0]);//vary
        i2c_reg_read_byte(i2c_dev, BME680_ADDR,0xE2,&par_h[2]);//vary
        h1=(int32_t)((uint16_t)par_h[0] | (uint16_t)par_h[1]<<8);
        h2=(int32_t)((uint16_t)par_h[2] | (uint16_t)par_h[3]<<8);
        
        // BME680_HUM_LSB      0x26
        // BME680_HUM_MSB      0x25 
        i2c_reg_read_byte(i2c_dev, BME680_ADDR,BME680_HUM_LSB,&h_data[0]);
        i2c_reg_read_byte(i2c_dev, BME680_ADDR,BME680_HUM_MSB,&h_data[1]);

        humidity_adc = ((uint32_t)h_data[0]) | ((uint32_t)h_data[1])<<8 ;

        printk("Room humidity is %d percent\n", humidity_convert(temp_comp,humidity_adc,h1,h2,h3,h4,h5,h6,h7)/10000);



        k_msleep(3000);

    } 
}
