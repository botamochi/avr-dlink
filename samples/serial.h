//--------------------------------------------------
//
// シリアル通信用ライブラリ
// atmega88系用
// 2014/12/23 作成
//
//--------------------------------------------------
#ifndef _MY_LIBRARY_SERIAL_H_
#define _MY_LIBRARY_SERIAL_H_

/*----- 数値を変換する際のフォーマット -----*/
#define DEC 0
#define HEX 1
#define BIN 2

/*----- ライブラリ関数 -----*/
void serial_init(unsigned long baud);
void serial_putc(unsigned char dat);
void serial_puts(const char *str);
void serial_print(unsigned int val, int fmt);
void serial_println(unsigned int val, int fmt);
int  serial_getc(void);

#endif
