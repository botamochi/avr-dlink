//--------------------------------------------------
// UART用ライブラリ
//
// created: botamochi
// 2017/06/12 作成
//--------------------------------------------------
#include <stdarg.h>
#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

#if defined UCSR0A
#define SERIAL_REG_A    UCSR0A
#define SERIAL_REG_B    UCSR0B
#define SERIAL_REG_C    UCSR0C
#define SERIAL_REG_DAT  UDR0
#define SERIAL_REG_BRL  UBRR0L
#define SERIAL_REG_BRH  UBRR0H
#define SERIAL_BIT_RXEN (1<<RXEN0)
#define SERIAL_BIT_TXEN (1<<TXEN0)
#define SERIAL_BIT_CFG0 (1<<UCSZ00)
#define SERIAL_BIT_CFG1 (1<<UCSZ01)
#define SERIAL_BIT_RDY  (1<<UDRE0)
#define SERIAL_BIT_RCV  (1<<RXC0)
#elif defined UCSRA
#define SERIAL_REG_A    UCSRA
#define SERIAL_REG_B    UCSRB
#define SERIAL_REG_C    UCSRC
#define SERIAL_REG_DAT  UDR
#define SERIAL_REG_BRL  UBRRL
#define SERIAL_REG_BRH  UBRRH
#define SERIAL_BIT_RXEN (1<<RXEN)
#define SERIAL_BIT_TXEN (1<<TXEN)
#define SERIAL_BIT_CFG0 (1<<UCSZ0)
#define SERIAL_BIT_CFG1 (1<<UCSZ1)
#define SERIAL_BIT_RDY  (1<<UDRE)
#define SERIAL_BIT_RCV  (1<<RXC)
#else
#define SOFTSERIAL
#define SERIAL_IO_SET()  SERIAL_IO_PORT |=  (1 << SERIAL_IO_BIT_TX)
#define SERIAL_IO_CLR()  SERIAL_IO_PORT &= ~(1 << SERIAL_IO_BIT_TX)
#define SERIAL_IO_READ() (SERIAL_IO_PIN & (1 << SERIAL_IO_BIT_RX))
#define SERIAL_PULSE_WIDTH (1000000 / SERIAL_BAUDRATE)
#endif

//--------------------------------------------------
// 初期化
//--------------------------------------------------
void serial_init()
{
#if !defined(SOFTSERIAL)
  unsigned long tmp = (F_CPU / SERIAL_BAUDRATE / 16) - 1;
  SERIAL_REG_BRH = (uint8_t)(tmp >> 8);
  SERIAL_REG_BRL = (uint8_t)(tmp & 0xff);
  SERIAL_REG_B = SERIAL_BIT_RXEN | SERIAL_BIT_TXEN;
  SERIAL_REG_C = SERIAL_BIT_CFG0 | SERIAL_BIT_CFG1;
#else
  SERIAL_IO_DDR |= (1 << SERIAL_IO_BIT_TX);
  SERIAL_IO_DDR &= ~(1 << SERIAL_IO_BIT_RX);
  SERIAL_IO_SET();
#endif
}

//--------------------------------------------------
// 送信
//--------------------------------------------------
int serial_putc(char c)
{
#if !defined(SOFTSERIAL)
  while (!(SERIAL_REG_A & SERIAL_BIT_RDY));
  SERIAL_REG_DAT = c;
#else
  int i;
  // Start Bit
  SERIAL_IO_CLR();
  _delay_us(SERIAL_PULSE_WIDTH);
  // Data Bit
  for (i = 0; i < 8; i++) {
    if (c & (1 << i)) {
      SERIAL_IO_SET();
    } else {
      SERIAL_IO_CLR();
    }
    _delay_us(SERIAL_PULSE_WIDTH);
  }
  // Stop Bit
  SERIAL_IO_SET();
  _delay_us(SERIAL_PULSE_WIDTH);
#endif
  return 0;
}

//--------------------------------------------------
// 受信
//--------------------------------------------------
int serial_getc()
{
#if !defined(SOFTSERIAL)
  while (!(SERIAL_REG_A & SERIAL_BIT_RCV));
  return (int)SERIAL_REG_DAT;
#else
  int i;
  char dat;
  // Start Bit
  while (SERIAL_IO_READ() != 0);
  _delay_us(SERIAL_PULSE_WIDTH / 2); // 半分ずらして読み取り
  // Data Bit
  dat = 0;
  for (i = 0; i < 8; i++) {
    _delay_us(SERIAL_PULSE_WIDTH);
    if (SERIAL_IO_READ() != 0) {
      dat |= (1 << i);
    }
  }
  return dat;
#endif
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
// 基数を指定して数値を表示
//--------------------------------------------------
void serial_print_value(unsigned int val, int rdx)
{
  char *digits = "0123456789abcdef";
  char buff[16];
  int cnt = 0;

  if (val == 0) {
    serial_putc('0');
    return;
  }
  while (val > 0) {
    // 表示桁がバッファサイズを越えたら'x'を表示して終了
    if (cnt >= 16) {
      serial_putc('x');
      return;
    }
    // 各桁の文字を入れる
    buff[cnt++] = digits[val % rdx];
    val /= rdx;
  }
  // 表示
  while (cnt > 0) {
    serial_putc(buff[--cnt]);
  }
}

#if SERIAL_USE_PRINTF == 1
//--------------------------------------------------
// printf関数
// 対応フォーマット : %c %s %d %x %b
//--------------------------------------------------
void serial_printf(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  while (*fmt != '\0') {
    if (*fmt != '%') {
      serial_putc(*fmt);
    } else {
      fmt++;
      switch (*fmt) {
      case '%':
	serial_putc('%');
	break;
      case 'c':
	serial_putc(va_arg(args, int));
	break;
      case 's':
	serial_puts(va_arg(args, char *));
	break;
      case 'd':
	serial_print_value(va_arg(args, int), 10);
	break;
      case 'x':
	serial_print_value(va_arg(args, int), 16);
	break;
      case 'b':
	serial_print_value(va_arg(args, int),  2);
	break;
      default:
	serial_putc('.');
	break;
      }
    }
    fmt++;
  }
  va_end(args);
}

#endif
