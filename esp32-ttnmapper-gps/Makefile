# This is a makefile for using arduino-cli.
#
# Run 'make installdeps' once, then run "make go" to compile and upload your code
#
# Check the BOARD and PORT setting in the first few lines below:

BOARD ?= esp32:esp32:t-beam
PORT ?= /dev/ttyUSB0
BAUD ?= 115200

# You probably do not need to change anything past here.

DEVICE ?= ttnmapper
PROXYHOST ?= raspberrypi
PROXYPORT ?= $(PORT)
CHIP ?= esp32
LIBDIR ?= $(HOME)/Arduino/libraries
BINDIR ?= build/$(shell echo $(BOARD) | cut -d: -f1-3 | tr : .)
SDKVERSION ?= $(shell ls -1 $(HOME)/.arduino15/packages/$(CHIP)/hardware/$(CHIP)/ | tail -1)
OTAPROG ?= $(HOME)/.arduino15/packages/$(CHIP)/hardware/$(CHIP)/$(SDKVERSION)/tools/espota.py
ESPTOOL ?= $(HOME)/.arduino15/packages/$(CHIP)/hardware/$(CHIP)/$(SDKVERSION)/tools/esptool.py
OTAPASS ?= changeme
PROGRAM ?= esp32-ttnmapper-gps
CCFLAGS ?=
#CCFLAGS ?= --verbose --warnings all
MAIN = $(PROGRAM).ino
OBJ = $(BINDIR)/$(PROGRAM).ino.bin
SRCS = $(MAIN) gps.ino lmic_Payload.ino oled.ino pbutton.ino keys.h pins.h Makefile

LIBS = "MCCI LoRaWAN LMIC library" "ESP8266 and ESP32 OLED driver for SSD1306 displays"
EXTRALIBS = git@github.com:mikalhart/TinyGPSPlus.git

build: $(OBJ)

$(OBJ): $(SRCS) Makefile
	arduino-cli compile -b $(BOARD) --build-cache-path . $(CCFLAGS) $(MAIN)

ota: $(OBJ)
	@if [ -z "$$IP" ] ; then \
		IP=`avahi-browse -ptr  "_arduino._tcp" | egrep ^= | cut -d\; -f4,8,9 | grep ^$$DEVICE | cut -d\; -f2` -p `avahi-browse -ptr  "_arduino._tcp" | egrep ^= | cut -d\; -f4,8,9 | grep ^$$DEVICE | cut -d\; -f3` ;\
	fi ;\
	python $(OTAPROG) -i $(IP) "--auth=$(OTAPASS)" -f $(OBJ)

find:
	@if [ `uname -s` = Darwin ] ; then \
		dns-sd -B _arduino._tcp ;\
	else \
		avahi-browse -ptr  "_arduino._tcp" | egrep ^= | cut -d\; -f4,8,9 ;\
	fi

upload: #$(OBJ)
	python $(ESPTOOL) --port $(PORT) write_flash 0x10000 $(OBJ)
# or,
#	arduino-cli upload -b $(BOARD) -p $(PORT) -i $(OBJ) -v -t

proxyupload:
	scp $(OBJ) $(PROXYHOST):tmp/$(PROGRAM).ino.bin
	ssh $(PROXYHOST) esptool -p $(PROXYPORT) -b $(BAUD) write_flash 0x10000 tmp/$(PROGRAM).ino.bin

erase: 
	python $(ESPTOOL) --port $(PORT) erase_flash

monitor:
	miniterm --rts 0 --dtr 0 $(PORT) 115200

proxymonitor:
	ssh -t $(PROXYHOST) miniterm --rts 0 --dtr 0 $(PORT) 115200

go: build upload

gop: build proxyupload

gosho: go monitor

goshop: gop proxymonitor

installcli: 
	@[ -f $(GOPATH)/bin/arduino-cli ] || go get -v -u github.com/arduino/arduino-cli && arduino-cli core update-index

installcore: cliconfig installcli
	@cat arduino-cli.yaml && arduino-cli core update-index && ls -l ~/.arduino15
	@arduino-cli core list
	@arduino-cli core list | grep ^esp8266:esp8266 >/dev/null || arduino-cli core install esp8266:esp8266
	@arduino-cli core list | grep ^esp32:esp32 >/dev/null || arduino-cli core install esp32:esp32

cliconfig:
	@ [ -d $(GOPATH) ] || mkdir -p $(GOPATH)
	@ [ -d $(GOPATH)/bin ] || mkdir -p $(GOPATH)/bin
	@ [ -d $(GOPATH)/src ] || mkdir -p $(GOPATH)/src
	@if [ \! -f $(GOPATH)/arduino-cli.yaml ] ; then \
	echo "board_manager:" >>$(GOPATH)/arduino-cli.yaml ; \
	echo "  additional_urls:" >>$(GOPATH)/arduino-cli.yaml ; \
	echo "    - http://arduino.esp8266.com/stable/package_esp8266com_index.json" >>$(GOPATH)/arduino-cli.yaml ; \
	echo "    - https://dl.espressif.com/dl/package_esp32_index.json" >>$(GOPATH)/arduino-cli.yaml ; \
	fi

libs:
	@for lib in $(LIBS) ; do libdir=`echo "$$lib" | sed -e 's/ /_/g'` ; if [ -d "$(LIBDIR)/$$libdir" ] ; then true ; else echo "Installing $$lib" ; arduino-cli lib install "$$lib" ; fi ; done

extralibs:
	@[ -d $(LIBDIR) ] || mkdir -p $(LIBDIR)
	@for lib in $(EXTRALIBS) ; do repo=`echo $$lib | cut -d% -f1` ; dir=`echo $$lib | cut -d% -f2`; if [ -d "$(LIBDIR)/$$dir" ] ; then echo "Found $$dir" ; else echo "Clone $$repo => $$dir" ; cd $(LIBDIR) && git clone $$repo $$dir ; fi ; done

installdeps: installcore libs extralibs
