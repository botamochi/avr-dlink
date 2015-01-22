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
// 使用するマイコンに合わせて値を変える
//==================================================

// 入出力ポート
#define DM_IO_REG_DIR DDRD
#define DM_IO_REG_IN  PIND
#define DM_IO_REG_OUT PORTD
#define DM_IO_PIN     PD2

#define DM_IO_INIT() DM_IO_REG_DIR &= ~(1<<DM_IO_PIN)
#define DM_IO_SET()  DM_IO_REG_DIR &= ~(1<<DM_IO_PIN)
#define DM_IO_CLR()  DM_IO_REG_DIR |= (1<<DM_IO_PIN)
#define DM_IO_READ() (DM_IO_REG_IN & (1<<DM_IO_PIN))

// 外部割り込み
#define DM_EXT_INT()      ISR(INT0_vect)
#define DM_EXT_INT_INIT() EICRA |= (1<<ISC00)
#define DM_EXT_INT_EN()   EIMSK |= (1<<INT0)
#define DM_EXT_INT_DIS()  EIMSK &= ~(1<<INT0)

// タイマー関係
#define DM_TIMER_INIT()    TCCR1B = (1<<CS10)
#define DM_TIMER_CLR()     TCNT1 = 0; TIFR1 = (1<<TOV1) // 割り込みを発生させないためにフラグをクリア
#define DM_TIMER_READ()    TCNT1
#define DM_TIMER_INT()     ISR(TIMER1_OVF_vect)
#define DM_TIMER_INT_EN()  TIMSK1 |= (1<<TOIE1)
#define DM_TIMER_INT_DIS() TIMSK1 &= ~(1<<TOIE1)

// 遅延処理
#define DM_DELAY_US(us) _delay_us(us)

//==================================================
// バッファサイズなどのパラメータ
//==================================================

// 受信バッファのサイズ
#define DM_RECV_BUFF_SIZE 8

// パルス幅(マイクロ秒)
#define DM_RESET_PULSE_LOW_TIME  60000
#define DM_START_PULSE_HIGH_TIME 2000
#define DM_START_PULSE_LOW_TIME  1000
#define DM_LONG_PULSE_HIGH_TIME  2500
#define DM_LONG_PULSE_LOW_TIME   1500
#define DM_SHORT_PULSE_HIGH_TIME 1000
#define DM_SHORT_PULSE_LOW_TIME  3000

// タイムアウト時間(タイマー割り込みの回数)
#define DM_RECV_DATA_TIMEOUT_COUNT  1
#define DM_RECV_RESET_TIMEOUT_COUNT 10
#define DM_SEND_RESET_TIMEOUT_COUNT 8

//==================================================
// ここからライブラリの処理
//==================================================

// 通信状態
#define DM_STAT_IDLE  0    // 通常状態
#define DM_STAT_RESET 1    // リセットパルス受信中
#define DM_STAT_START 2    // スタートパルス受信中
#define DM_STAT_DATA  3    // データパルス受信中
#define DM_STAT_END   4    // 受信完了(次の立ち上がりで通常状態へ)
#define DM_STAT_SEND  5    // 送信中

// グローバル変数
static volatile int    dlink_stat;
static volatile int    dlink_timeout_count;
static volatile dframe dlink_recv_buff[DM_RECV_BUFF_SIZE];
static volatile int    dlink_recv_buff_head;
static volatile int    dlink_recv_buff_tail;

//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void dlink_initialize()
{
  DM_IO_INIT();
  DM_EXT_INT_INIT();
  DM_EXT_INT_EN();
  DM_TIMER_INIT();
  dlink_recv_buff_head = 0;
  dlink_recv_buff_tail = 0;
  dlink_stat = DM_STAT_IDLE;
  dlink_timeout_count = 0;
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
  dlink_recv_buff_head = (dlink_recv_buff_head + 1) % DM_RECV_BUFF_SIZE;
  return dat;
}

//--------------------------------------------------
// リセットパルスを出力する
// 60msはタイマー割り込みで計測するので
// 呼び出し側では遅延を行わずdlink_put_frame()を
// 使用すればよい
//--------------------------------------------------
void dlink_connect()
{
  DM_EXT_INT_DIS();
  DM_IO_CLR();
  DM_TIMER_CLR();
  DM_TIMER_INT_EN();
  dlink_timeout_count = 0;
  dlink_stat = DM_STAT_SEND;
}

//--------------------------------------------------
// データを送信
// LSBから送信する
//--------------------------------------------------
void dlink_put_frame(dframe dat)
{
  int i;

  // リセットパルスを送信してなければ送信する
  if (dlink_stat != DM_STAT_SEND) {
    dlink_connect();
  }
  // リセットパルスが60ms以上送信されるまで待機
  while (dlink_timeout_count < DM_SEND_RESET_TIMEOUT_COUNT);
  // スタートパルスを送信
  DM_IO_SET();
  DM_DELAY_US(DM_START_PULSE_HIGH_TIME);
  DM_IO_CLR();
  DM_DELAY_US(DM_START_PULSE_LOW_TIME);
  // データ送信
  for (i = 0; i < 16; i++) {
    if (dat & (1<<i)) {
      DM_IO_SET();
      DM_DELAY_US(DM_LONG_PULSE_HIGH_TIME);
      DM_IO_CLR();
      DM_DELAY_US(DM_LONG_PULSE_LOW_TIME);
    } else {
      DM_IO_SET();
      DM_DELAY_US(DM_SHORT_PULSE_HIGH_TIME);
      DM_IO_CLR();
      DM_DELAY_US(DM_SHORT_PULSE_LOW_TIME);
    }
  }
  DM_IO_SET();
  dlink_stat == DM_STAT_IDLE;
  DM_DELAY_US(1000); // ピンを安定させるため
  DM_EXT_INT_EN();
}

//==================================================
// 割り込み処理
//==================================================

//--------------------------------------------------
// 外部割り込み
// データの受信を行う
// 受信が開始されるとタイマー割り込みがセットされ
// 一定時間ピン変化がないとタイムアウトとなる
//--------------------------------------------------
DM_EXT_INT()
{
  static dframe dat;
  static unsigned int th, cnt;
  unsigned int time = DM_TIMER_READ();
  int pin = DM_IO_READ();

  switch (dlink_stat) {
  case DM_STAT_IDLE:
    if (pin != 0) return;
    DM_TIMER_CLR();
    DM_TIMER_INT_EN();
    dlink_timeout_count = 0;
    cnt = 0;
    dat = 0;
    dlink_stat = DM_STAT_RESET;
    break;
  case DM_STAT_RESET:
    if (pin == 0) return;
    DM_TIMER_CLR();
    dlink_timeout_count = 0;
    dlink_stat = DM_STAT_START;
    break;
  case DM_STAT_START:
    if (pin != 0) return;
    th = time;
    dlink_stat = DM_STAT_DATA;
    break;
  case DM_STAT_DATA:
    if (pin != 0) {
      DM_TIMER_CLR();
      dlink_timeout_count = 0;
      return;
    }
    dat = dat >> 1;
    if (time > th) {
      dat |= 0x8000;
    }
    if (++cnt >= 16) {
      DM_TIMER_INT_DIS();
      dlink_recv_buff[dlink_recv_buff_tail] = dat;
      dlink_recv_buff_tail = (dlink_recv_buff_tail + 1) % DM_RECV_BUFF_SIZE;
      dlink_stat = DM_STAT_END;
    }
    break;
  case DM_STAT_END:
    dlink_stat = DM_STAT_IDLE;
    break;
  default:
    dlink_stat = DM_STAT_IDLE;
    break;
  }
}

//--------------------------------------------------
// タイマーオーバーフロー割り込み
// データ受信中のタイムアウト
// 送信時のリセットパルスの時間をカウントするのにも
// 使用する
//--------------------------------------------------
DM_TIMER_INT()
{
  dlink_timeout_count++;
  if (dlink_stat == DM_STAT_RESET &&
      dlink_timeout_count >= DM_RECV_RESET_TIMEOUT_COUNT) {
    DM_TIMER_INT_DIS();
    dlink_stat = DM_STAT_IDLE;
  } else if (dlink_stat == DM_STAT_START &&
	     dlink_timeout_count >= DM_RECV_DATA_TIMEOUT_COUNT) {
    DM_TIMER_INT_DIS();
    dlink_stat = DM_STAT_IDLE;
  } else if (dlink_stat == DM_STAT_DATA &&
	     dlink_timeout_count >= DM_RECV_DATA_TIMEOUT_COUNT) {
    DM_TIMER_INT_DIS();
    dlink_stat = DM_STAT_IDLE;
  }
}
