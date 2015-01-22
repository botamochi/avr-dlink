//--------------------------------------------------
//
// 育成ギア用サンドバッグ
// バトルは初代ギアの形式で、必ず勝てる
// 通信の開始は必ずマイコン側から
//
// 2014/12/24 作成
//--------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "serial.h"
#include "dlink.h"

//==================================================
// 送信するフレーム
// バージョン1のアグモン
// 負けで固定
//==================================================
#define FRAME1 0b1110110000010011
#define FRAME2 0b1111110100000010

//==================================================
// main
//==================================================
int main(void)
{
  dframe d1, d2;

  serial_init(9600);
  dlink_initialize();
  sei();
  while (1) {
    // ターミナルからのデータ受信で通信開始
    serial_getc();

    // 最初のフレームを送信
    dlink_connect();
    dlink_put_frame(FRAME1);

    // 受信
    while (!dlink_available());
    d1 = dlink_get_frame();
    _delay_ms(2);

    // 2つめのフレームを送信
    dlink_connect();
    dlink_put_frame(FRAME2);

    // 受信
    while (!dlink_available());
    d2 = dlink_get_frame();

    // 結果をターミナルに送信
    serial_puts("---- receive data ----\r\n");
    serial_puts("1:");
    serial_println(d1, BIN);
    serial_puts("2:");
    serial_println(d2, BIN);
  }
  return 0;
}
