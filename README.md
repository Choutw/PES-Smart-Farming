# Smart vertical farming

# Hardware

* Pi Pico maker

# Sensor

* BME 680
  * Temp
  * Hudmidity

* VEML 7700
  * Light

* SEN 0114
  * Moisture level
  
# State machine

* No need state machine

# Command
 
```
openocd -f interface/cmsis-dap.cfg -c 'transport select swd' -f target/rp2040.cfg -c "adapter speed 5000" -c 'targets rp2040.core0' -c 'program /home/chou/zephyrproject/part7/build/zephyr/zephyr.elf verify reset exit'
```

### Minicom

Then use minicom;

```
sudo minicom -D /dev/ttyACM0 -b 115200
```
