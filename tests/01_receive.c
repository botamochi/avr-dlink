//--------------------------------------------------
// D-Linkライブラリのテスト
//
// フレーム受信してPCに送るだけ
// created: botamochi
// 2017/06/12 作成
//
// [テスト手順]
// 1. 回路を組む
// 2. PCでターミナルソフトを起動してポートを開く
// 3. デジモンギア側から通信開始(Bボタンを押す)
// 4. 受信したフレームがターミナルに表示される
//
// [動作確認済みAVR]
// - atmega328p  @ F_CPU=8000000L, BAUDRATE=9600
// - attiny2313a @ F_CPU=8000000L, BAUDRATE=9600
// - attiny13a   @ F_CPU=9600000L, BAUDRATE=9600
//--------------------------------------------------
#include <avr/io.h>
#include "serial.h"
#include "dlink.h"

int main(void)
{
  dframe frame;

  dlink_init();
  serial_init();

  while (1) {
    frame = dlink_recv_frame(0);
    serial_puts("---\r\n");
    serial_puts("VAL:");
    serial_print_value(frame, 16);
    serial_puts("\r\n");
  }

  return 0;
}
