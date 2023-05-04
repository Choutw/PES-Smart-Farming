# Smart vertical farming 

SVF ...



# Hardware

## Board  

* Pi Pico maker

## Sensor

* BME 680
  * Temperature
  * Hudmidity

* VEML 7700 
  * Light

* SEN 0114 (Analog Soil Moisture Sensor)
  * Moisture level

## RTOS

* Zephyr


  
  
# State machine

* No need state machine

# How to run

### Command (for ubuntu 2X.04 LTS)

```
west build -b rpi_pico
```

```
openocd -f interface/cmsis-dap.cfg -c 'transport select swd' -f target/rp2040.cfg -c "adapter speed 5000" -c 'targets rp2040.core0' -c 'program /home/chou/zephyrproject/part7/build/zephyr/zephyr.elf verify reset exit'
```

### Minicom

Then use minicom;

```
sudo minicom -D /dev/ttyACM0 -b 115200
```
