//--------------------------------------------------
//
// D-LINKインターフェース(簡易版)
//
// dlink-min.h
// created: botamochi
// 2015/1/22 作成
//
//--------------------------------------------------
#ifndef _DIGIMON_DLINK_MIN_H_
#define _DIGIMON_DLINK_MIN_H_

// データフレームの型(16bit符号無し整数)
typedef unsigned int dframe;

//==================================================
// ライブラリ関数
//==================================================

// 初期化
void   dlink_initialize(void);

// 受信
dframe dlink_get_frame(void);

// 送信
void   dlink_put_frame(dframe dat);

#endif
