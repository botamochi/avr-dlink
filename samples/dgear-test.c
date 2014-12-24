//--------------------------------------------------
//
// 育成ギア解析ライブラリのテストプログラム
// 1フレームを育成ギアに送信し
// その後返ってきたフレームを
// 16進数でターミナルに送る
// 2014/12/22
//
//--------------------------------------------------
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "dgear.h"

/*----- シリアル通信用 -----*/
void serial_init(unsigned long baud);
void serial_putc(char dat);
void serial_puts(const char *str);
int  serial_getc(void);

//==================================================
// main
//==================================================
int main(void)
{
  char str[32];
  dframe frm;

  serial_init(9600);
  dgear_initialize();
  sei();
  while (1) {
    /* ターミナルから何かしら受信 */
    serial_getc();
    /* 育成ギアに1フレーム送信 */
    dgear_connect();
    dgear_put_frame(0xFC02);
    /* 受信 */
    while (!dgear_available());
    frm = dgear_get_frame();
    /* 16進数の文字列に変換して送信 */
    sprintf(str, "0x%04X\r\n", frm);
    serial_puts(str);
  }
  return 0;
}


//==================================================
// シリアル通信
//==================================================

/*----- 初期化 -----*/
void serial_init(unsigned long baud)
{
  UBRR0 = (F_CPU / baud / 16) - 1;
  UCSR0B = (1<<RXEN0) | (1<<TXEN0);
  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
}

/*----- 1byte送信 -----*/
void serial_putc(char dat)
{
  while (!(UCSR0A & (1<<UDRE0)));
  UDR0 = (unsigned char)dat;
}

/*----- 文字列送信 -----*/
void serial_puts(const char *str)
{
  while (*str != '\0') {
    serial_putc(*str++);
  }
}

/*----- 1byte受信 -----*/
int serial_getc()
{
  while (!(UCSR0A & (1<<RXC0)));
  return (int)UDR0;
}
