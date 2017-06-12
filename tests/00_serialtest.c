//--------------------------------------------------
// シリアル通信ライブラリのテスト
// D-Linkは関係ない
//
// created: botamochi
// 2017/06/12 作成
//
// [動作確認済みAVR]
// - atmega328p  @ F_CPU=8000000L, BAUDRATE=9600
// - attiny2313a @ F_CPU=8000000L, BAUDRATE=9600
// - attiny13a   @ F_CPU=9600000L, BAUDRATE=9600
//--------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

int main()
{
  int c;

  serial_init();

  while (1) {
    c = serial_getc();
    serial_puts("---\r\n");
    serial_puts("CHAR:");
    serial_putc(c);
    serial_puts("\r\nHEX :");
    serial_print_value(c, 16);
    serial_puts("\r\n");
  }
  return 0;
}
