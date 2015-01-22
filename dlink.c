//--------------------------------------------------
//
// D-LINKインターフェース
//
// 2端子コネクタのデジモンギアと通信するためのライブラリ．
// 読み取りには外部割り込みとタイマーを使用する．
// タイムアウト処理も行うのでタイマー割り込みも使用する．
//
// dlink.c
// created: botamochi
// 2014/12/22 作成
//
//--------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "dlink.h"

//==================================================
// このへんを各自で設定する
//==================================================

// 受信バッファのサイズ
#define DM_BUFF_SIZE 8

// 制御ポート
#define DM_PIN      PD2
#define DM_PORT_IN  PIND
#define DM_PORT_OUT PORTD
#define DM_PORT_DIR DDRD

#define DM_OUTPUT_MODE() DM_PORT_DIR |= (1<<DM_PIN)
#define DM_INPUT_MODE()  DM_PORT_DIR &= ~(1<<DM_PIN)
#define DM_PIN_SET()     DM_PORT_OUT |= (1<<DM_PIN)
#define DM_PIN_CLR()     DM_PORT_OUT &= ~(1<<DM_PIN)
#define DM_PIN_READ()    DM_PORT_IN & (1<<DM_PIN)

// 外部割り込み
#define DM_EXT_INT()      ISR(INT0_vect)
#define DM_EXT_INT_INIT() EICRA |= (1<<ISC00);
#define DM_EXT_INT_EN()   EIMSK |= (1<<INT0)
#define DM_EXT_INT_DIS()  EIMSK &= ~(1<<INT0)

// タイマー関係
#define DM_TIMER_INIT()    TCCR1B = (1<<CS11) | (1<<CS10);
#define DM_TIMER_CLR()     TCNT1 = 0; TIFR1 = (1<<TOV1) /* 割り込みを発生させないためにフラグをクリア */
#define DM_TIMER_READ()    TCNT1
#define DM_TIMER_TIMEOUT() ISR(TIMER1_OVF_vect)
#define DM_TIMER_INT_EN()  TIMSK1 |= (1<<TOIE1)
#define DM_TIMER_INT_DIS() TIMSK1 &= ~(1<<TOIE1)

// 遅延時間関係
#define DM_DELAY_US(us) _delay_us(us)
#define DM_HEAD_PULSE_HIGH_TIME  2000
#define DM_HEAD_PULSE_LOW_TIME   1000
#define DM_LONG_PULSE_HIGH_TIME  2500
#define DM_LONG_PULSE_LOW_TIME   1500
#define DM_SHORT_PULSE_HIGH_TIME 1000
#define DM_SHORT_PULSE_LOW_TIME  3000
#define DM_BEGIN_PULSE_COUNT     7500 /* 8MHz,1/64プリスケーラで60m秒分のカウント値 */

//==================================================
// ここからライブラリの処理
//==================================================

// 受信状態
#define DM_STAT_IDLE  0   // 通常状態
#define DM_STAT_BEGIN 1   // 読み取り開始
#define DM_STAT_HEAD  2   // 開始ビット読み取り中
#define DM_STAT_DATA  3   // データ読み取り中
#define DM_STAT_END   4   // 読み取り終了

// 受信バッファ(リングバッファ)と受信状態
static volatile dframe dlink_recv_buff[DM_BUFF_SIZE];
static volatile int    dlink_recv_buff_head;
static volatile int    dlink_recv_buff_tail;
static volatile int    dlink_recv_stat;

//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void dlink_initialize()
{
  DM_INPUT_MODE();
  DM_PIN_SET();
  DM_EXT_INT_INIT();
  DM_EXT_INT_EN();
  DM_TIMER_INIT();
  dlink_recv_buff_head = 0;
  dlink_recv_buff_tail = 0;
  dlink_recv_stat = DM_STAT_IDLE;
}

//--------------------------------------------------
// 受信データがあるか
// ある場合はデータ数を返す
//--------------------------------------------------
int dlink_available()
{
  if (dlink_recv_buff_head > dlink_recv_buff_tail) {
    return dlink_recv_buff_head - dlink_recv_buff_tail;
  }
  return dlink_recv_buff_tail - dlink_recv_buff_head;
}

//--------------------------------------------------
// バッファ内のデータを1つ取得
// データが無ければ-1を返す
//--------------------------------------------------
dframe dlink_get_frame(void)
{
  dframe dat;

  if (dlink_recv_buff_head == dlink_recv_buff_tail) {
    return -1;
  }
  dat = dlink_recv_buff[dlink_recv_buff_head];
  dlink_recv_buff_head = (dlink_recv_buff_head + 1) % DM_BUFF_SIZE;
  return dat;
}

//--------------------------------------------------
// データを送信する前のLowレベル信号
// この処理とdlink_put_frame()との間に
// 約60msの遅延を入れる
//--------------------------------------------------
void dlink_connect()
{
  DM_EXT_INT_DIS();
  DM_OUTPUT_MODE();
  DM_PIN_CLR();
  DM_TIMER_CLR(); /* 60msを計測するため */
}

//--------------------------------------------------
// データを送信
// LSBから送信する
//--------------------------------------------------
void dlink_put_frame(dframe dat)
{
  int i;

  /* dlink_connect()から60ms経過していなければ待機 */
  while (DM_TIMER_READ() < DM_BEGIN_PULSE_COUNT);
  /* 先頭のパルスを送信 */
  DM_PIN_SET();
  DM_DELAY_US(DM_HEAD_PULSE_HIGH_TIME);
  DM_PIN_CLR();
  DM_DELAY_US(DM_HEAD_PULSE_LOW_TIME);
  /* データ送信 */
  for (i = 0; i < 16; i++) {
    if (dat & (1<<i)) {
      DM_PIN_SET();
      DM_DELAY_US(DM_LONG_PULSE_HIGH_TIME);
      DM_PIN_CLR();
      DM_DELAY_US(DM_LONG_PULSE_LOW_TIME);
    } else {
      DM_PIN_SET();
      DM_DELAY_US(DM_SHORT_PULSE_HIGH_TIME);
      DM_PIN_CLR();
      DM_DELAY_US(DM_SHORT_PULSE_LOW_TIME);
    }
  }
  DM_PIN_SET();
  DM_INPUT_MODE();
  DM_DELAY_US(1000); // ピンを安定させるため
  DM_EXT_INT_EN();
}

//==================================================
// 割り込み処理
//==================================================

//--------------------------------------------------
// ピン変化割り込み
// データの受信を行う
// 受信が開始されるとタイマー割り込みがセットされ
// 一定時間ピン変化がないとタイムアウトとなる
//--------------------------------------------------
DM_EXT_INT()
{
  static dframe dat;
  static unsigned int th, cnt;
  unsigned int time = DM_TIMER_READ();
  int pin = DM_PIN_READ();

  switch (dlink_recv_stat) {
  case DM_STAT_IDLE:
    if (pin != 0) return;
    DM_TIMER_CLR();
    DM_TIMER_INT_EN();
    cnt = 0;
    dat = 0;
    dlink_recv_stat = DM_STAT_BEGIN;
    break;
  case DM_STAT_BEGIN:
    if (pin == 0) return;
    DM_TIMER_CLR();
    dlink_recv_stat = DM_STAT_HEAD;
    break;
  case DM_STAT_HEAD:
    if (pin != 0) return;
    th = time;
    dlink_recv_stat = DM_STAT_DATA;
    break;
  case DM_STAT_DATA:
    if (pin != 0) {
      DM_TIMER_CLR();
      return;
    }
    dat = dat >> 1;
    if (time > th) {
      dat |= 0x8000;
    }
    if (++cnt >= 16) {
      DM_TIMER_INT_DIS();
      dlink_recv_buff[dlink_recv_buff_tail] = dat;
      dlink_recv_buff_tail = (dlink_recv_buff_tail + 1) % DM_BUFF_SIZE;
      dlink_recv_stat = DM_STAT_END;
    }
    break;
  case DM_STAT_END:
    dlink_recv_stat = DM_STAT_IDLE;
    break;
  default:
    dlink_recv_stat = DM_STAT_IDLE;
    break;
  }
}

//--------------------------------------------------
// タイマーオーバーフロー割り込み
// データ受信中のタイムアウト
//--------------------------------------------------
DM_TIMER_TIMEOUT()
{
  DM_TIMER_INT_DIS();
  dlink_recv_stat = DM_STAT_IDLE;
}
