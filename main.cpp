#include "main.h"
#include <stdlib.h>
#include <time.h>

HashEntry Sikou::HashTbl[1024 * 1024];

int Sikou::MakeMoveFirst(int SorE, int depth, Te teBuf[], KyokumenKomagumi k)
{
	int teNum = 0;
	if (HashTbl[k.HashVal & 0xfffff].HashVal != k.HashVal)
	{
		return 0;
	}
	if (HashTbl[k.HashVal & 0xfffff].Tesu % 2 != k.Tesu % 2) 
	{
		return 0;
	}

	Te te = HashTbl[k.HashVal & 0xfffff].Best;
	if (!te.IsNull()) 
	{
		if (k.IsLegalMove(SorE, te)) 
		{
			teBuf[teNum++] = te;
		}
	}
	te = HashTbl[k.HashVal & 0xfffff].Second;
	if (!te.IsNull()) 
	{
		if (k.IsLegalMove(SorE, te))
		{
			teBuf[teNum++] = te;
		}
	}
	if (depth > 1)
	{
		te = Best[depth - 1][depth];
		if (!te.IsNull() && k.IsLegalMove(SorE, te)) 
		{
			teBuf[teNum++] = te;
		}
	}
	return teNum;
}

Te Stack[32];

int Sikou::NegaAlphaBeta(int SorE, KyokumenKomagumi &k, int alpha, int beta, int depth, int depthMax, bool bITDeep)
{
	if (depth == 1)
	{
		int sennitite = 0;
		for (int i = k.Tesu; i > 0; i -= 2)
		{
			if (k.HashHistory[i] == k.HashVal) 
			{
				sennitite++;
			}
		}
		if (sennitite >= 4)
		{
			sennitite = 0;
			int i;
			for (i = k.Tesu; sennitite <= 3 && i > 0; i -= 2) 
			{
				if (!Kyokumen::OuteHistory[i]) 
				{
					break;
				}
				if (k.HashHistory[i] == k.HashVal)
				{
					sennitite++;
				}
			}
			if (sennitite == 4) 
			{
				return INFINITE;
			}
			sennitite = 0;
			for (i = k.Tesu; sennitite <= 3 && i > 1; i -= 2) 
			{
				if (!Kyokumen::OuteHistory[i - 1])
				{
					break;
				}
				if (k.HashHistory[i] == k.HashVal) 
				{
					sennitite++;
				}
			}
			if (sennitite == 4) 
			{
				return -INFINITE;
			}
			return 0;
		}
	}
	if (depth == depthMax)
	{
		int value = k.Evaluate() + k.BestEval(SorE);
		if (SorE == SELF) 
		{
			return value;
		}
		else {
			return -value;
		}
	}
	if (HashTbl[k.HashVal & 0x0fffff].HashVal == k.HashVal)
	{
		HashEntry e = HashTbl[k.HashVal & 0x0fffff];
		if (e.value >= beta && e.Tesu >= k.Tesu && e.Tesu % 2 == k.Tesu % 2 && e.depth <= depth && e.remainDepth >= depthMax - depth && e.flag != UPPER_BOUND) 
		{
			return e.value;
		}
		if (e.value <= alpha && e.Tesu >= k.Tesu && e.Tesu % 2 == k.Tesu % 2 && e.depth <= depth && e.remainDepth >= depthMax - depth && e.flag != LOWER_BOUND) 
		{
			return e.value;
		}
	}
	else if (depthMax - depth > 2 && bITDeep)
	{
		return ITDeep(SorE, k, alpha, beta, depth, depthMax);
	}
	Te teBuf[600];
	int retval = -INFINITE - 1;
	if (depth < 2 && k.Mate(SorE, 7, teBuf[0]) == 1)
	{
		Best[depth][depth] = teBuf[0];
		Best[depth][depth + 1] = Te(0);
		retval = INFINITE + 1;
		goto HashAdd;
	}
	int teNum;
	if (depth == 0) 
	{
		teNum = MakeMoveFirst(SorE, depth, teBuf, k);
	}
	else 
	{
		teNum = MakeMoveFirst(SorE, depth, teBuf, k);
	}
	int i;
	k.EvaluateTe(SorE, teNum, teBuf);
	for (i = 0; i < teNum; i++)
	{
		KyokumenKomagumi kk(k);
		if (teBuf[i].IsNull()) 
		{
			continue;
		}
		Stack[depth] = teBuf[i];
		kk.Move(SorE, teBuf[i]);
		int v = -NegaAlphaBeta(SorE == SELF ? ENEMY : SELF, kk, -beta, -max(alpha, retval), depth + 1, depthMax);
		
		if (depth > 1 && Stack[depth - 1].value < 0 && Stack[depth - 1].to == Stack[depth].to && v <= retval) 
		{
			v = -NegaAlphaBeta(SorE == SELF ? ENEMY : SELF, kk, -beta, -max(alpha, retval), depth + 1, depthMax + 2);
		}
		if (v > retval) 
		{
			retval = v;
			Best[depth][depth] = teBuf[i];
			for (int i = depth + 1; i < depthMax; i++)
			{
				Best[depth][i] = Best[depth + 1][i];
			}
			if (depth == 0) {
				printf("retval:%d,te:", retval);
				for (int j = 0; j < depthMax; j++) 
				{
					Best[0][j].Print();
				}
				printf("\n");
			}
			if (retval >= beta)
			{
				goto HashAdd;
			}
		}
	}
	teNum = k.MakeLegalMoves(SorE, teBuf);
	if (teNum == 0)
	{
		return -INFINITE;
	}
	k.EvaluateTe(SorE, teNum, teBuf);
	for (i = 0; i < teNum; i++) {
		if ((teBuf[i].value < -100 || i>30) && i > 0 && retval > -INFINITE) 
		{
			break;
		}
		KyokumenKomagumi kk(k);
		Stack[depth] = teBuf[i];
		kk.Move(SorE, teBuf[i]);
		int v = -NegaAlphaBeta(SorE == SELF ? ENEMY : SELF, kk, -beta, -max(alpha, retval), depth + 1, depthMax);
		
		if (depth > 1 && Stack[depth - 1].value < 0 && Stack[depth - 1].to == Stack[depth].to && v <= beta) 
		{
			v = -NegaAlphaBeta(SorE == SELF ? ENEMY : SELF, kk, -beta, -max(alpha, retval), depth + 1, depthMax + 2);
		}
		if (v > retval)
		{
			retval = v;
			Best[depth][depth] = teBuf[i];
			for (int i = depth + 1; i < depthMax; i++)
			{
				Best[depth][i] = Best[depth + 1][i];
			}
			if (depth == 0) {
				printf("retval:%d,te:", retval);
				for (int j = 0; j < depthMax; j++)
				{
					Best[0][j].Print();
				}
				printf("\n");
			}
			if (retval >= beta)
			{
				goto HashAdd;
			}
		}
	}
HashAdd:
	HashEntry e;
	e = HashTbl[k.HashVal & 0x0fffff];
	if (e.HashVal == k.HashVal) {
		e.Second = e.Best;
	}
	else
	{
		if (e.Tesu - e.depth == k.Tesu - depth && e.remainDepth > depthMax - depth) 
		{
			if (depth == 0)
			{
				k.Print();
				Best[depth][depth].Print();
			}
			goto NotAdd;
		}
		e.HashVal = k.HashVal;
		e.Second = Te(0);
	}
	if (retval > alpha) 
	{
		e.Best = Best[depth][depth];
	}
	else
	{
		e.Best = Te(0);
	}
	e.value = retval;
	if (retval <= alpha) 
	{
		e.flag = UPPER_BOUND;
	}
	else if (retval >= beta) 
	{
		e.flag = LOWER_BOUND;
	}
	else 
	{
		e.flag = EXACTLY_VALUE;
	}
	e.depth = depth;
	e.remainDepth = depthMax - depth;
	e.Tesu = k.Tesu;
	HashTbl[k.HashVal & 0x0fffff] = e;
NotAdd:
	return retval;
}

int Sikou::ITDeep(int SorE, KyokumenKomagumi &k, int alpha, int beta, int depth, int depthMax)
{
	int retval;
	int i;
	for (i = depth + 1; i <= depthMax; i++)
	{
		retval = NegaAlphaBeta(SorE, k, alpha, beta, depth, i, false);
	}
	return retval;
}

Joseki joseki("c:\\public.bin");
Kyokumen *shoki;


Te Sikou::Think(int SorE, KyokumenKomagumi k)
{
	int teNum;
	Te teBuf[600];
	int hindo[600];
	int i, j;
	joseki.fromJoseki(*shoki, SELF, k, k.Tesu, teNum, teBuf, hindo);
	if (teNum > 0) {
		int max, maxhindo;
		max = 0;
		maxhindo = hindo[max];
		for (i = 1; i < teNum; i++) 
		{
			if (hindo[i] > maxhindo) 
			{
				maxhindo = hindo[i];
				max = i;
			}
		}
		return teBuf[max];
	}

	int depthMax = 4;
	for (i = 0; i < MAX_DEPTH; i++) 
	{
		for (j = 0; j < MAX_DEPTH; j++) 
		{
			Best[i][j] = Te(0);
		}
	}
	int bestVal = ITDeep(SorE, k, -INFINITE + 1, INFINITE - 1, 0, depthMax);
	printf("bestVal:%d,te:", bestVal);
	for (i = 0; i < depthMax; i++)
	{
		Best[0][i].Print();
	}
	printf("\n");

	return Best[0][0];
}

bool IsIllegal(Te te, int teNum, Te *teBuf)
{
	for (int i = 0; i < teNum; i++) 
	{
		if (te == teBuf[i]) 
		{
			return false;
		}
	}
	return true;
}

int main()
{
	long start = clock();
	Kyokumen::HashInit();

	KomaInf HirateBan[9][9] = {
		{EKY,EKE,EGI,EKI,EOU,EKI,EGI,EKE,EKY},
		{EMP,EHI,EMP,EMP,EMP,EMP,EMP,EKA,EMP},
		{EFU,EFU,EFU,EFU,EFU,EFU,EFU,EFU,EFU},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP,EMP},
		{SFU,SFU,SFU,SFU,SFU,SFU,SFU,SFU,SFU},
		{EMP,SKA,EMP,EMP,EMP,EMP,EMP,SHI,EMP},
		{SKY,SKE,SGI,SKI,SOU,SKI,SGI,SKE,SKY}
	};

	int HirateMotigoma[EHI + 1] = {
		// 空空空空空空空空空空空空空空空空空歩香桂銀金角飛王と杏圭全金馬龍空歩香桂銀金角飛
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};


	KyokumenKomagumi Hirate(0, HirateBan, HirateMotigoma);
	KyokumenKomagumi k(Hirate);
	KyokumenKomagumi::InitKanagomaValue();
	shoki = new Kyokumen(0, HirateBan, HirateMotigoma);
	k.Initialize();

	Sikou sikou;

	Te teBuf[600];
	int teNum;

	int SorE = SELF;

	long long temp;
	srand(time(&temp));

	int bSennitite = false;

	while ((teNum = k.MakeLegalMoves(SorE, teBuf)) > 0)
	{
		k.SenkeiInit();
		k.Print();
		Te te;
		if (SorE == SELF) 
		{
			char buf[80];
			gets_s(buf);
			int from, to;
			int koma, capture;
			char promote[2];
			promote[0] = '\0';
			int ss = sscanf(buf, "%02x%02x%1s", &from, &to, &promote);
			if (ss < 2)
			{
				continue;
			}
			if (from < OU)
			{
				koma = SELF | from;
				from = 0;
			}
			else 
			{
				koma = k.ban[from];
			}
			capture = k.ban[to];
			if (ss = 3 && promote[0] == '*') 
			{
				te = Te(from, to, koma, capture, 1);
			}
			else 
			{
				te = Te(from, to, koma, capture, 0);
			}

			if (IsIllegal(te, teNum, teBuf)) 
			{
				printf("入力された手が異常です。入力しなおしてください。\n");
				continue;
			}
		}
		if (SorE == ENEMY) 
		{
			te = sikou.Think(SorE, k);
		}

		te.Print();
		k.Move(SorE, te);
		int sennitite = 0;
		int i;
		for (i = k.Tesu; i > 0; i -= 2)
		{
			if (k.HashHistory[i] == k.HashVal) {

				sennitite++;
			}
		}
		if (sennitite >= 4)
		{
			bSennitite = true;
			break;
		}
		if (SorE == SELF) 
		{
			SorE = ENEMY;
		}
		else 
		{
			SorE = SELF;
		}
	}
	k.Print();
	if (SorE == SELF && teNum == 0)
	{
		printf("後手の勝ち。\n");
	}
	else if (teNum == 0) 
	{
		printf("先手の勝ち。\n");
	}
	else if (bSennitite)
	{
		printf("千日手です。\n");
		int sennitite = 0;
		if (Kyokumen::OuteHistory[k.Tesu])
		{
			for (int i = k.Tesu; sennitite <= 3 && i > 0; i -= 2)
			{
				if (!Kyokumen::OuteHistory[i])
				{
					break;
				}
				if (k.HashHistory[i] == k.HashVal)
				{
					sennitite++;
				}
			}
			if (sennitite == 4)
			{
				printf("連続王手の千日手です。\n");
				if (SorE == SELF) {
					printf("後手の勝ち。\n");
				}
				else {
					printf("先手の勝ち。\n");
				}
			}
		}
		else if (Kyokumen::OuteHistory[k.Tesu - 1]) 
		{
			for (int i = k.Tesu; sennitite <= 3 && i > 0; i -= 2)
			{
				if (!Kyokumen::OuteHistory[i - 1])
				{
					break;
				}
				if (k.HashHistory[i] == k.HashVal) 
				{
					sennitite++;
				}
			}
			if (sennitite == 4) 
			{
				printf("連続王手の千日手です。\n");
				if (SorE == SELF) 
				{
					printf("先手の勝ち。\n");
				}
				else 
				{
					printf("後手の勝ち。\n");
				}
			}
		}
	}
	printf("%.3lfs", (double(clock() - start)) / CLOCKS_PER_SEC);
	return 0;
}
