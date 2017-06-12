//--------------------------------------------------
//
// D-LINKインターフェース
// 2端子コネクタのデジモンギアと通信するためのライブラリ．
//
// dlink.c
// created: botamochi
// 2014/12/22 作成
// 2017/06/12 大改造
//
//--------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>
#include "dlink.h"
#include "dlink-config.h"

// 遅延処理
#define DLINK_DELAY_US(us) _delay_us(us)
#define DLINK_DELAY_MS(ms) _delay_ms(ms)

//==================================================
// IO制御関係のマクロ
//==================================================
#define DLINK_CONCAT(a, b)          a ## b

#define DLINK_IO_DDRPORT(name) DLINK_CONCAT(DDR, name)
#define DLINK_IO_INPORT(name)  DLINK_CONCAT(PIN, name)
#define DLINK_IO_OUTPORT(name) DLINK_CONCAT(PORT, name)

#define DLINK_IO_SET(port)     port |=  (1 << DLINK_CFG_OUTPUT_BIT)
#define DLINK_IO_CLR(port)     port &= ~(1 << DLINK_CFG_OUTPUT_BIT)
#define DLINK_IO_READ(port)    (port & (1 << DLINK_CFG_OUTPUT_BIT))

#define DLINKDDR  DLINK_IO_DDRPORT(DLINK_CFG_IOPORTNAME)
#define DLINKPIN  DLINK_IO_INPORT(DLINK_CFG_IOPORTNAME)
#define DLINKPORT DLINK_IO_OUTPORT(DLINK_CFG_IOPORTNAME)


//==================================================
// 初期化処理
//==================================================
void dlink_init()
{
  DLINK_IO_CLR(DLINKDDR);
  DLINK_IO_SET(DLINKPORT); // pull-up
}

//==================================================
// パルスを送信
// _delay_us()が変数使えないのでマクロで定義
//==================================================
#define dlink_put_pulse(hightime, lowtime) do { \
    DLINK_IO_SET(DLINKPORT);                    \
    DLINK_DELAY_US(hightime);                   \
    DLINK_IO_CLR(DLINKPORT);                    \
    DLINK_DELAY_US(lowtime);                    \
  } while(0)

//==================================================
// フレームを送信
// frame : 送信するデータ
//==================================================
void dlink_send_frame(dframe frame)
{
  int i;

  DLINK_IO_SET(DLINKDDR); // output mode

  // sync
  DLINK_IO_CLR(DLINKPORT);
  DLINK_DELAY_MS(60);
  // head
  dlink_put_pulse(2000, 1000);
  // data
  for (i = 0; i < 16; i++) {
    if (frame & (1 << i)) {
      dlink_put_pulse(2500, 1500);
    } else {
      dlink_put_pulse(1000, 3000);
    }
  }
  DLINK_IO_SET(DLINKPORT);
  DLINK_IO_CLR(DLINKDDR); // input mode
  DLINK_DELAY_US(100); // ポートが安定するまで待機
}


//==================================================
// パルス読み取り
// timeout : タイムアウト時間(マイクロ秒)
//==================================================
static unsigned dlink_read_pulse(unsigned long timeout)
{
  unsigned long tm;

  // Highになるまで待機
  tm = 0;
  while (DLINK_IO_READ(DLINKPIN) == 0) {
    DLINK_DELAY_US(10);
    tm += 10;
    if (timeout != 0 && tm >= timeout) return 0;
  }
  // High時間を計測
  tm = 0;
  while (DLINK_IO_READ(DLINKPIN) != 0) {
    DLINK_DELAY_US(10);
    tm += 10;
    if (timeout != 0 && tm >= timeout) return 0;
  }
  return tm;
}

//==================================================
// フレーム受信
// timeout : syncを受信する際のタイムアウト(ミリ秒)
// 戻り値 : 受信したデータ(失敗すると0を返す)
//==================================================
dframe dlink_recv_frame(unsigned long timeout)
{
  dframe frame;
  unsigned long tmsync, tm, thr;
  int i;

  // sync
  tmsync = 0;
  while (DLINK_IO_READ(DLINKPIN) != 0) {
    DLINK_DELAY_MS(1);
    tmsync += 1;
    if (timeout != 0 && tmsync >= timeout) return 0;
  }
  // head
  thr = dlink_read_pulse(100000);
  if (thr == 0) return 0;
  // data
  frame = 0;
  for (i = 0; i < 16; i++) {
    tm = dlink_read_pulse(8000);
    if (tm == 0) return 0;
    if (tm > thr) frame |= (1 << i);
  }
  while (DLINK_IO_READ(DLINKPIN) == 0); // Highになるまで待機

  return frame;
}
