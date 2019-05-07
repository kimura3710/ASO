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
	uint64 HashVal;		// �n�b�V���l
	Te Best;			// �O��̔����[���ł̍őP��
	Te Second;			// �O�X��ȑO�̔����[���ł̍őP��
	int value;			// �����T���œ����ǖʂ̕]���l
	int flag;			// �����T���œ����l���A�ǖʂ̕]���l���̂��̂��A����l�������l��
	int Tesu;			// �����T�����s�����ۂ̎萔
	short depth;		// �����T�����s�����ۂ̐[��
	short remainDepth;	// �����T�����s�����ۂ̎c��[��

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