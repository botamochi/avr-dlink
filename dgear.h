//--------------------------------------------------
//
// デジモン育成ギア解析ライブラリ(2端子用)
// created: botamochi
// 2014/12/22 作成
//
//--------------------------------------------------
#ifndef _DIGIMON_GEAR_H_
#define _DIGIMON_GEAR_H_

/*----- データフレームの型(16bit符号無し整数) -----*/
typedef unsigned int dframe;

//==================================================
// ライブラリ関数
//==================================================
/* ----- 初期化処理 -----*/
void   dgear_initialize(void);

/*----- 受信データがあるか -----*/
int    dgear_available(void);

/*----- データ取得 -----*/
dframe dgear_get_frame(void);

/*----- 送信開始信号を送る -----*/
void   dgear_connect(void);

/*----- データ送信 -----*/
void   dgear_put_frame(dframe dat);

#endif
