# iicky
A stupidly terse I2C terminal interface

This is orginally designed for the Adafruit QT Py RP2040 board

## Build
To build, extract in same directory along side pico-sdk
```  
  cd iicky
  mkdir build
  cd build
  export PICO_SDK_PATH=../../pico-sdk
  cmake -DPICO_BOARD=adafruit_qtpy_rp2040 ..
  make
```

## To Do
* Move QWIIC pins into board definition
* Add help command with command details
* More documentation
* Port to other boards