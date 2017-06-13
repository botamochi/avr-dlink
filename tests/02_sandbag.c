//--------------------------------------------------
//
// 育成ギア用サンドバッグ
// バトルは初代ギアの形式で、必ず勝てる
//
// created: botamochi
// 2014/12/24 作成
// 2017/06/12 ライブラリの仕様変更に伴う修正
//
// [テスト手順]
// 1. 回路を組む
// 2. PCでターミナルソフトを起動してポートを開く
// 3. デジモンギアをバトル待機状態にしておく
// 4. ターミナルソフトからデータを送信(何でもOK)
// 5. バトルが完了して受信したデータがターミナルに表示される
//
// [動作確認済みAVR]
// - atmega328p  @ F_CPU=8000000L, BAUDRATE=9600
// - attiny2313a @ F_CPU=8000000L, BAUDRATE=9600
// - attiny13a   @ F_CPU=9600000L, BAUDRATE=9600
//--------------------------------------------------
#include <avr/io.h>
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
  dframe recv1, recv2;

  dlink_init();
  serial_init();

  while (1) {
    // ターミナルからのデータ受信で通信開始
    serial_getc();

    // 最初のフレームを送受信
    dlink_send_frame(FRAME1);
    recv1 = dlink_recv_frame(0);
    // 2つめのフレームを送受信
    dlink_send_frame(FRAME2);
    recv2 = dlink_recv_frame(0);

    // 結果をターミナルに送信
    serial_puts("---\r\n");
    serial_puts("1:");
    serial_print_value(recv1, 16);
    serial_puts("\r\n2:");
    serial_print_value(recv2, 16);
    serial_puts("\r\n");
  }
  return 0;
}
