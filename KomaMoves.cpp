#include "kyokumen.h"

// 方向を示す定数。
const int Direct[12] = 
{
	17,1,-15,16,-16,15,-1,-17,14,-18,18,-14
};

// 成ることが出来る駒か？
const int CanPromote[] = 
{
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,
		//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
			0,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

// その方向に動けるか？その方向に飛んで動くものは入れてはいけない。
const int CanMove[12][64] = 
{
	{
		// Direct[0]=17,
		// |／
		//  ￣
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,
		//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
		0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=1,
	// ↓
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,1,0,0,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-15,
	// ＼|
	// ￣
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=16,
	// ←
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-16,
	// →
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=15,
	//  __
	// |＼
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,1,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-1,
	// ↑
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1,1,1,1,1,1,1,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-17
	// __
	// ／|
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,1,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=14,先手の桂馬
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-18,先手の桂馬
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-14,後手の桂馬
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=18,後手の桂馬
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	}
};

// その方向に飛んで動くことが出来るか？
// 飛車角香車と龍と馬しかそういう駒はない
const int CanJump[12][64] = 
{
	// Direct[0]=17,
	// ＼|
	// ￣
	{
		//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
		//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
		0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=1,
	// ↓
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-15,
	// ＼|
	// ￣
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=16,
	// ←
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-16,
	// →
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=15,
	//  __
	// |＼
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-1,
	// ↑
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-17
	// __
	// ／|
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=14,先手の桂馬飛び
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-18,先手の桂馬飛び
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=-14,後手の桂馬飛び
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	},
	{
	// Direct[0]=18,後手の桂馬飛び
	//  空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//	空歩香桂銀金角飛王と杏圭全金馬龍壁空空空空空空空空空空空空空空空
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	}
};

// 駒を盤面に表示するための文字列
const char *komaStr[] = 
{
"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
"   "," 歩"," 香"," 桂"," 銀"," 金"," 角"," 飛"," 王"," と"," 杏"," 圭"," 全"," 金"," 馬"," 龍",
"   ","v歩","v香","v桂","v銀","v金","v角","v飛","v王","vと","v杏","v圭","v全","v金","v馬","v龍",
" 壁","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};

// 駒の種類を表示するための文字列
const char *komaStr2[] = 
{
"  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ",
"  ","歩","香","桂","銀","金","角","飛","王","と","杏","圭","全","金","馬","龍",
"  ","歩","香","桂","銀","金","角","飛","王","と","杏","圭","全","金","馬","龍",
" 壁","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ","  ",
};

const int KomaValue[] = 
{
	//  空空空空空空空空空空空空空空空空空 歩  香  桂   銀   金   角   飛    王    と   杏  圭   全   金   馬   龍
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,100,600,700,1000,1200,1800,2000,10000,1200,1200,1200,1200,1200,2000,2200,
		//	空  歩   香   桂    銀    金    角    飛     王    と    杏    圭    全    金    馬    龍 壁
			0,-100,-600,-700,-1000,-1200,-1800,-2000,-10000,-1200,-1200,-1200,-1200,-1200,-2000,-2200,0
};

const int HandValue[] =
{
	//  空空空空空空空空空空空空空空空空空 歩  香  桂   銀   金   角   飛王と杏圭全金馬龍
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,105,630,735,1050,1260,1890,2100,0,0,0,0,0,0,0,0,
		//	空  歩   香   桂    銀    金    角    飛
			0,-105,-630,-735,-1050,-1260,-1890,-2100
};
