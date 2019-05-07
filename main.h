#pragma once

#include <algorithm>
#include <stdio.h>
#include "Kyokumen.h"

#define MAX_DEPTH	(32)
#define INFINITE	(999999)

enum 
{
	EXACTLY_VALUE,
	LOWER_BOUND,
	UPPER_BOUND
};

class HashEntry
{
public:
	uint64 HashVal;		// ハッシュ値
	Te Best;			// 前回の反復深化での最善手
	Te Second;			// 前々回以前の反復深化での最善手
	int value;			// αβ探索で得た局面の評価値
	int flag;			// αβ探索で得た値が、局面の評価値そのものか、上限値か下限値か
	int Tesu;			// αβ探索を行った際の手数
	short depth;		// αβ探索を行った際の深さ
	short remainDepth;	// αβ探索を行った際の残り深さ

};

class Sikou
{
protected:
	static HashEntry HashTbl[1024 * 1024];
	int MakeMoveFirst(int SorE, int depth, Te teBuf[], KyokumenKomagumi k);
public:
	Te Think(int SorE, KyokumenKomagumi k);

	Te Best[MAX_DEPTH][MAX_DEPTH];

	int NegaAlphaBeta(int SorE, KyokumenKomagumi &k, int alpha, int beta, int depth, int depthMax, bool bITDeep = true);
	int ITDeep(int SorE, KyokumenKomagumi &k, int alpha, int beta, int depth, int depthMax);
};