# dbus-utils

With this Project some 

As this is my first C Project take everything with a grain of salt.

## To Build

### Dependencies

### Build Command

```
mkdir build
cd build
cmake ..
cmake --build build
``` 

## To Use
### AccelerometerTrigger

This Binary is used to execute a shell script after the accelerometer registers a change. The shell script get the new orientation then passed as the first parameter. This Binary ist strongly inspired by
The Binary will be generated under `AccelerometerTrigger/output`. 
To test the created binary run the following commands: 
```
cd AccelerometerTrigger/output
./accelerometer-trigger --path "../test.sh"
```
For more information use `./accelerometer-trigger --help`

