//--------------------------------------------------
//
// 育成ギア解析ライブラリのテストプログラム
// 1フレームを育成ギアに送信し
// その後返ってきたフレームを
// 16進数でターミナルに送る
//
// 2014/12/22 作成
//--------------------------------------------------
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "serial.h"
#include "dgear.h"

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
