#--------------------------------------------------
# テストのhexファイル生成用
#
# created: botamochi
# 2017/06/12 作成
#
# 実行環境 : cygwin
# Toolchain : Atmel Studio 6.2に入ってるヤツ
#
# 以下のようにしてcygwinからavr-gccが使えるようにしておく
# $ PATH=${PATH}:"/cygdrive/c/Program Files (x86)/Atmel/Atmel Toolchain/AVR8 GCC/Native/3.4.1061/avr8-gnu-toolchain/bin"
#--------------------------------------------------
CC      = avr-gcc
OBJCOPY = avr-objcopy

MCU = atmega328p
CLOCK = 8000000L

INC_DIR  = ../

HEXS = 00_serialtest.hex 01_receive.hex 02_sandbag.hex
OBJS = serial.o ../dlink.o

CFLAGS  = -W -Wall -Os -I$(INC_DIR)
CFLAGS  += -mmcu=$(MCU) -DF_CPU=$(CLOCK)
CPFLAGS  = -j .text -j .data

all: $(HEXS) Makefile

.SUFFIXES:
.SUFFIXES: .hex .elf .o .c

%.elf:%.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.elf.hex:
	$(OBJCOPY) $(OFLAGS) -O ihex $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.PRECIOUS: %.elf %.o

.PHONY: clean
clean:
	rm -rf *.o *.elf *.hex ../*.o
