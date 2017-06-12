//--------------------------------------------------
//
// シリアル通信用ライブラリ
// atmega88系用
// 2014/12/23 作成
//
//--------------------------------------------------
#include <avr/io.h>
#include "serial.h"

/*----- 数値を文字列に変換する関数 -----*/
static char get_digit_dec(unsigned int *pVal);
static char get_digit_hex(unsigned int *pVal);
static char get_digit_bin(unsigned int *pVal);

//--------------------------------------------------
// 初期化
// 引数でボーレートを設定する
//--------------------------------------------------
void serial_init(unsigned long baud)
{
  UBRR0 = (F_CPU / baud / 16) - 1;
  UCSR0B = (1<<RXEN0) | (1<<TXEN0); /* 送信と受信を許可 */
  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* 8bit, no parity, 1 stop bit */
}

//--------------------------------------------------
// 1byte送信
//--------------------------------------------------
void serial_putc(unsigned char dat)
{
  while (!(UCSR0A & (1<<UDRE0)));
  UDR0 = dat;
}

//--------------------------------------------------
// 文字列送信
//--------------------------------------------------
void serial_puts(const char *str)
{
  while (*str != '\0') {
    serial_putc(*str++);
  }
}

//--------------------------------------------------
// 数値を引数で与えられたフォーマットに
// 変換して送信する
//--------------------------------------------------
void serial_print(unsigned int val, int fmt)
{
  int cnt = 0;
  char digits[16];
  char (*get_digit)(unsigned int *pVal) = 0;

  if (val == 0) {
    serial_putc('0');
    return;
  }
  switch (fmt) {
  case DEC: get_digit = get_digit_dec; break;
  case HEX: get_digit = get_digit_hex; break;
  case BIN: get_digit = get_digit_bin; break;
  }
  while (val > 0) {
    digits[cnt++] = get_digit(&val);
  }
  while (cnt > 0) {
    serial_putc(digits[--cnt]);
  }
}

//--------------------------------------------------
// serial_print()に改行をつけ加えるだけ
//--------------------------------------------------
void serial_println(unsigned int val, int fmt)
{
  serial_print(val, fmt);
  serial_puts("\r\n");
}

//--------------------------------------------------
// 1byte受信
// 受信するまでループする
//--------------------------------------------------
int serial_getc()
{
  while (!(UCSR0A & (1<<RXC0)));
  return (int)UDR0;
}

//==================================================
// 数値を文字列に変換する
//==================================================

/*----- 10進数で変換 -----*/
static char get_digit_dec(unsigned int *pVal)
{
  char digit = *pVal % 10 + '0';
  *pVal = *pVal / 10;
  return digit;
}

/*----- 16進数で変換 -----*/
static char get_digit_hex(unsigned int *pVal)
{
  int v = *pVal % 16;
  char digit = (v < 10) ? v + '0' : v - 10 + 'A';
  *pVal = *pVal / 16;
  return digit;
}

/*----- 2進数で変換 -----*/
static char get_digit_bin(unsigned int *pVal)
{
  char digit = *pVal % 2 + '0';
  *pVal = *pVal / 2;
  return digit;
}
