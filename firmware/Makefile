
# The MIT License (MIT)
# Copyright (c) 2018 Ivor Wanders
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# This makefile allows use of the Arduino command line interface and sets the sketch folder to the current folder.
# This allows a per project libraries folder.


# Path to your arduino installation, can be overwritten with environment variables.
ARDUINOPATH ?= ./arduino-1.8.5/

# Always use bash.
SHELL := /bin/bash

TEENSY_TYPE ?= 31

# see hardware/teensy/avr/boards.txt for teensy options
# see Arduino_STM32/STM32F1/boards.txt for stm32 options

ifeq ($(TEENSY_TYPE),31)
    CPU_SPEED = 72
    OPT = o2std
    VARIANT = mk20dx256
    USB = serial
    KEYS = en-us
    BOARD = teensy:avr:teensy31
endif

# TEENSY_TYPE=LC make
ifeq ($(TEENSY_TYPE),LC)
    CPU_SPEED = 48
    OPT = o2std
    VARIANT = mkl26z64
    USB = serial
    KEYS = en-us
    BOARD = teensy:avr:teensyLC
endif

# Additional flags to pass to the build & upload.
EXTRAFLAGS = --verbose-build --preserve-temp-files

# Create path to the arduino executable.
ARDUINOEXEC = $(ARDUINOPATH)/arduino

BUILDDIR ?= /tmp/arduino_builddir_$(TEENSY_TYPE)

PREFERENCES = --pref device_variant=$(VARIANT) --pref opt=$(OPT) --pref speed=$(CPU_SPEED) --pref build.path=$(BUILDDIR) --pref usb=$(USB) --pref keys=$(KEYS)
FQBN=$(BOARD):opt=$(OPT),speed=$(CPU_SPEED),usb=$(USB),keys=$(KEYS)

FIXLIBRARIES=--pref sketchbook.path=$(shell pwd)

all:
	# This sketch .ino file is actually unused during the build, it doesn't have to exist, because Arduino builds all files?
	$(ARDUINOEXEC) $(EXTRAFLAGS) --board $(FQBN) $(PREFERENCES) $(FIXLIBRARIES) --upload main.ino

help:
	$(ARDUINOEXEC) $(EXTRAFLAGS) --board $(FQBN) $(PREFERENCES) $(FIXLIBRARIES) --list

clean:
	rm -rf $(BUILDDIR)

libs:
	@echo placeholder


env:
	@echo "alias arduino=\"$(ARDUINOEXEC)\""