#!/bin/bash
idf.py build 
idf.py -p /dev/ttyUSB0 flash 
idf.py -p /dev/ttyUSB1 flash 
idf.py -p /dev/ttyUSB2 flash 
idf.py -p /dev/ttyUSB3 flash 
