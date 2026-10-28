all : flash

TARGET:=main
CH32FUN:=../rv003usb/ch32fun/ch32fun
TARGET_MCU:=CH32V006

ADDITIONAL_C_FILES+=../rv003usb/rv003usb/rv003usb.S ../rv003usb/rv003usb/rv003usb.c
EXTRA_CFLAGS:=-I../rv003usb/lib -I../rv003usb/rv003usb

include $(CH32FUN)/ch32fun.mk

flash : cv_flash
clean : cv_clean


