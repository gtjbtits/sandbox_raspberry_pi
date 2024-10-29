Airpressure measurer based on RaspberryPi with LPS22HB sensor

# Hardware
* RaspberryPi 4 Model B
* Sence Hat (B)

# Software
* Debian Bullseye with Raspberry Pi Desktop
* WiringPi 3.10 (gpio 3.10)

# Installation
* Follow the instructions from https://www.waveshare.com/wiki/Sense_HAT_(B)
* Install CUnit: ```bash # apt install libcunit1 libcunit1-doc libcunit1-dev```
* Install Nginx
* Add cronjob (user needs rw permissions to */var/www/html* folder).

File will be accesable at URL: `http://localhost/pressure.csv`

# Build
```bash
gcc -Wall air_pressure_log.c -o pressure -lwiringPi -lm
```