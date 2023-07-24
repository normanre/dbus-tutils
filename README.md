# dbus-utils

At the moment this project creates a trigger utility for screen orientation using [iio-sensor-proxy](https://gitlab.freedesktop.org/hadess/iio-sensor-proxy). Tested on Ubuntu 22.04.

As this is my first C Project take everything with a grain of salt.

## To Build

### Dependencies
```
sudo apt-get install libglib2.0-dev
```
### Build Command

```
git clone git@github.com:normanre/dbus-utils.git
cd dbus-utils
mkdir build
cd build
cmake ..
cmake --build build
``` 

## To Use
### AccelerometerTrigger

This Binary is used to execute a shell script after the accelerometer registers a change. The shell script gets the new 
orientation passed as the first parameter. This Binary ist strongly inspired by [monitor-sensor](https://gitlab.freedesktop.org/hadess/iio-sensor-proxy/-/blob/master/src/monitor-sensor.c).


The Binary will be generated under `AccelerometerTrigger/output`. 
To test the created binary run the following commands: 
```
cd AccelerometerTrigger/output
./accelerometer-trigger --path "../test.sh"
```
For more information use `./accelerometer-trigger --help`

