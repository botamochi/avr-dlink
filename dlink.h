//--------------------------------------------------
//
// D-LINKインターフェース
// 
// dlink.h
// created: botamochi
// 2014/12/22 作成
// 2017/06/12 関数の削除と名称変更
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
void   dlink_init(void);

// フレーム送信
void   dlink_send_frame(dframe frame);

// フレーム受信
dframe dlink_recv_frame(unsigned long timeout);

#endif
