//--------------------------------------------------
//
// D-LINKインターフェース
// 
// dlink.h
// created: botamochi
// 2014/12/22 作成
//
//--------------------------------------------------
#ifndef _DIGIMON_DLINK_H_
#define _DIGIMON_DLINK_H_

// データフレームの型(16bit符号無し整数)
typedef unsigned int dframe;

//==================================================
// ライブラリ関数
//==================================================

// 初期化処理
void   dlink_initialize(void);

// 受信データがあるか
int    dlink_available(void);

// データ取得
dframe dlink_get_frame(void);

// 送信開始信号を送る
void   dlink_connect(void);

// データ送信
void   dlink_put_frame(dframe dat);

#endif
