//--------------------------------------------------
//
// D-LINKインターフェース(簡易版)
//
// 割り込みもタイマーも使用しない
// ローエンドマイコン向け
//
// dlink-min.c
// created: botamochi
// 2015/1/22 作成
//
//--------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>
#include "dlink-min.h"

//==================================================
// 使用するマイコンに合わせる
//==================================================

// 入出力ポート
#define DM_IO_REG_DIR DDRB
#define DM_IO_REG_IN  PINB
#define DM_IO_REG_OUT PORTB
#define DM_IO_PIN     PB0

#define DM_IO_INIT() DM_IO_REG_DIR &= ~(1<<DM_IO_PIN)
#define DM_IO_SET()  DM_IO_REG_DIR &= ~(1<<DM_IO_PIN)
#define DM_IO_CLR()  DM_IO_REG_DIR |= (1<<DM_IO_PIN)
#define DM_IO_READ() (DM_IO_REG_IN & (1<<DM_IO_PIN))

// 遅延関数
#define DM_DELAY_US(us) _delay_us(us)
#define DM_DELAY_MS(ms) _delay_ms(ms)

//==================================================
// ライブラリの処理
//==================================================

// パルス幅(マイクロ秒)
#define DM_RESET_PULSE_LOW_TIME  60000
#define DM_START_PULSE_HIGH_TIME 2000
#define DM_START_PULSE_LOW_TIME  1000
#define DM_LONG_PULSE_HIGH_TIME  2500
#define DM_LONG_PULSE_LOW_TIME   1500
#define DM_SHORT_PULSE_HIGH_TIME 1000
#define DM_SHORT_PULSE_LOW_TIME  3000

// 信号が変化するまで待機
#define DM_WAIT_FOR_RISING()  while (DM_IO_READ() == 0)
#define DM_WAIT_FOR_FALLING() while (DM_IO_READ() != 0)

//--------------------------------------------------
// 初期化
//--------------------------------------------------
void dlink_initialize()
{
  DM_IO_INIT();
}

//--------------------------------------------------
// 受信
// 失敗すると0を返す
//--------------------------------------------------
dframe dlink_get_frame()
{
  int i, cnt = 0;
  dframe dat = 0;

  // リセットパルス
  DM_WAIT_FOR_FALLING();
  DM_WAIT_FOR_RISING() {
    DM_DELAY_US(1000);
    if (++cnt > 100) return 0; // 100msでタイムアウト
  }
  // スタートパルス
  cnt = 0;
  DM_WAIT_FOR_FALLING() {
    DM_DELAY_US(100);
    if (++cnt > 30) return 0;
  }
  // データパルス
  for (i = 0; i < 16; i++) {
    cnt = 0;
    DM_WAIT_FOR_RISING();
    DM_WAIT_FOR_FALLING() {
      DM_DELAY_US(100);
      if (++cnt > 40) return 0; // 4msでタイムアウト
    }
    dat = dat >> 1;
    if (cnt > DM_START_PULSE_HIGH_TIME / 100) {
      dat |= 0x8000;
    }
  }
  DM_WAIT_FOR_RISING();
  return dat;
}

//--------------------------------------------------
// 送信
//--------------------------------------------------
void dlink_put_frame(dframe dat)
{
  int i;

  // リセットパルス
  DM_IO_CLR();
  DM_DELAY_MS(DM_RESET_PULSE_LOW_TIME / 1000);
  // スタートパルス
  DM_IO_SET(); DM_DELAY_US(DM_START_PULSE_HIGH_TIME);
  DM_IO_CLR(); DM_DELAY_US(DM_START_PULSE_LOW_TIME);
  // データパルス
  for (i = 0; i < 16; i++) {
    if (dat & (1<<i)) {
      DM_IO_SET(); DM_DELAY_US(DM_LONG_PULSE_HIGH_TIME);
      DM_IO_CLR(); DM_DELAY_US(DM_LONG_PULSE_LOW_TIME);
    } else {
      DM_IO_SET(); DM_DELAY_US(DM_SHORT_PULSE_HIGH_TIME);
      DM_IO_CLR(); DM_DELAY_US(DM_SHORT_PULSE_LOW_TIME);
    }
  }
  DM_IO_SET();
}
