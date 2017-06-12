//--------------------------------------------------
// テスト用のUARTライブラリ
// attiny13a用にSoft Serialに自動で切り替わる仕様
//
// created: botamochi
// 2017/06/12 作成
//--------------------------------------------------

// 通信ボーレート
// PCのターミナルソフトとそろえる
#define SERIAL_BAUDRATE  9600

// Soft Serial用のマクロ
// UARTペリフェラルがある場合は使用しない
#define SERIAL_IO_DDR    DDRB
#define SERIAL_IO_PORT   PORTB
#define SERIAL_IO_PIN    PINB
#define SERIAL_IO_BIT_TX PB3
#define SERIAL_IO_BIT_RX PB4

// serial_printfを使うかどうか
#define SERIAL_USE_PRINTF 0

void serial_init(void);
int  serial_putc(char c);
int  serial_getc(void);
void serial_puts(const char *str);
void serial_print_value(unsigned int val, int rdx);
void serial_printf(const char *fmt, ...);
