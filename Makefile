PORT=/dev/ttyUSB0

SPEED=115200
ESPTOOL=esptool.py
FIRMWARE=esp8266-20170612-v1.9.1.bin

erase:
	$(ESPTOOL) --port $(PORT) erase_flash

flash:
	$(ESPTOOL) --port /dev/ttyUSB0  write_flash -fm dio -fs 32m 0 $(FIRMWARE)

screen:
	screen $(PORT) 115200
