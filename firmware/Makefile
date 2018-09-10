# Always use bash.
SHELL := /bin/bash

# The name of your project (the .ino file / folder name)
# just naming it main.ino and using that works...
#TARGET =  $(shell basename `pwd`)

# Path to your arduino installation
ARDUINOPATH ?= ./arduino-1.8.5/

# which board to target
# arduino:avr:uno for normal UNO.
# arduino:sam:arduino_due_x for Arduino DUE
# Arduino_STM32:STM32F1:genericSTM32F103C

# see Arduino_STM32/STM32F1/boards.txt
# see hardware/teensy/avr/boards.txt
BOARD = teensy:avr:teensy31

#~ UPLOAD_METHOD = STLinkMethod
CPU_SPEED = 72
OPT = o2std
VARIANT = mk20dx256
USB = serial
KEYS = en-us


# Additional flags to pass to the build & upload.
EXTRAFLAGS = --verbose-build --preserve-temp-files

# Create path to the arduino executable.
ARDUINOEXEC = $(ARDUINOPATH)/arduino

BUILDDIR ?= /tmp/arduino_builddir

# Arduino_STM32:STM32F1:genericSTM32F103C:device_variant=STM32F103C8,upload_method=STLinkMethod,cpu_speed=speed_72mhz,opt=osstd


PREFERENCES = --pref device_variant=$(VARIANT) --pref opt=$(OPT) --pref speed=$(CPU_SPEED) --pref build.path=$(BUILDDIR) --pref usb=$(USB) --pref keys=$(KEYS)
FQBN=$(BOARD):opt=$(OPT),speed=$(CPU_SPEED),usb=$(USB),keys=$(KEYS)

FIXLIBRARIES=--pref sketchbook.path=$(shell pwd)


all:
	$(ARDUINOEXEC) $(EXTRAFLAGS) --board $(FQBN) $(PREFERENCES) $(FIXLIBRARIES) --upload main.ino

help:
	$(ARDUINOEXEC) $(EXTRAFLAGS) --board $(FQBN) $(PREFERENCES) $(FIXLIBRARIES) --list

clean:
	rm -rf /tmp/arduino_builddir

libs:
	@echo placeholder
	#$(ARDUINOEXEC) --install-library SerialFlash

env:
	@echo "alias arduino=\"$(ARDUINOEXEC)\""