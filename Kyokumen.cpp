#include "kyokumen.h"
#include <memory.h>
#include <stdlib.h>

uint64 Kyokumen::HashSeed[ERY + 1][0x99 + 1];
uint64 Kyokumen::HandHashSeed[EHI + 1][18 + 1];

uint64 Kyokumen::HashHistory[1000];
int Kyokumen::OuteHistory[1000];

void Kyokumen::HashInit()
{
	int j;
	for (j = 0; j <= ERY; j++)
	{
		for (int pos = 0x11; pos <= 0x99; pos++) 
		{
			HashSeed[j][pos] = ((uint64)rand()) << 49 |
				((uint64)rand()) << 34 |
				((uint64)rand()) << 19 |
				((uint64)rand()) << 4 |
				rand() & 0x07;
		}
	}

	for (j = SFU; j <= EHI; j++)
	{
		for (int maisuu = 0; maisuu <= 18; maisuu++) 
		{
			HandHashSeed[j][maisuu] = ((uint64)rand()) << 49 |
				((uint64)rand()) << 34 |
				((uint64)rand()) << 19 |
				((uint64)rand()) << 4 |
				rand() & 0x07;
		}
	}
}


Kyokumen::Kyokumen(int tesu, KomaInf board[9][9], int Motigoma[])
{
	memset(banpadding, WALL, sizeof(banpadding));
	memset(ban, WALL, sizeof(ban));
	value = 0;
	printf("value:%d\n");
	KingS = 0;
	KingE = 0;
	Tesu = tesu;

	HashVal = 0;
	HandHashVal = 0;
	KyokumenHashVal = 0;
	for (int dan = 1; dan <= 9; dan++) 
	{
		for (int suji = 0x10; suji <= 0x90; suji += 0x10) 
		{
			ban[suji + dan] = board[dan - 1][9 - suji / 0x10];
			KyokumenHashVal ^= HashSeed[ban[suji + dan]][suji + dan];
			if (ban[suji + dan] == SOU) 
			{
				KingS = suji + dan;
			}
			if (ban[suji + dan] == EOU)
			{
				KingE = suji + dan;
			}
			value += KomaValue[ban[suji + dan]];
		}
	}
	int i;
	for (i = 0; i <= EHI; i++) 
	{
		Hand[i] = Motigoma[i];
		value += HandValue[i] * Hand[i];
		for (int j = 1; j <= Hand[i]; j++)
		{
			HandHashVal ^= HandHashSeed[i][j];
		}
	}
	HashVal = KyokumenHashVal ^ HandHashVal;
	for (i = 0; i < Tesu; i++) 
	{
		HashHistory[i] = 0;
		OuteHistory[i] = 0;
	}
	HashHistory[Tesu] = HashVal;
	OuteHistory[Tesu] = ((Tesu % 2) == 0) ? controlS[KingE] : controlE[KingS];
	InitControl();
}

void Kyokumen::InitControl()
{
	int dan, suji;
	int j, k, b, bj;

	memset(controlS, 0, sizeof(controlS));
	memset(controlE, 0, sizeof(controlE));

	for (suji = 0x10; suji <= 0x90; suji += 0x10) 
	{
		for (dan = 1; dan <= 9; dan++)
		{
			if (ban[suji + dan] & ENEMY)
			{ 
				for (j = 0, b = 1, bj = (1 << 16); j < 12; j++, b <<= 1, bj <<= 1) 
				{
					if (CanJump[j][ban[dan + suji]])
					{
						k = dan + suji;
						do {
							k += Direct[j];
							controlE[k] |= bj;
						} while (ban[k] == EMPTY);
					}
					else if (CanMove[j][ban[dan + suji]]) 
					{
						controlE[dan + suji + Direct[j]] |= b;
					}
				}
			}
			else if (ban[suji + dan] & SELF) 
			{ 
				for (j = 0, b = 1, bj = (1 << 16); j < 12; j++, b <<= 1, bj <<= 1) 
				{
					if (CanJump[j][ban[dan + suji]])
					{
						k = dan + suji;
						do 
						{
							k += Direct[j];
							controlS[k] |= bj;
						} while (ban[k] == EMPTY);
					}
					else if (CanMove[j][ban[dan + suji]]) 
					{
						controlS[dan + suji + Direct[j]] |= b;
					}
				}
			}
		}
	}
}


void Kyokumen::Move(int SorE, const Te &te)
{
	int j, k, b, bj;
	if (te.from > 0x10) 
	{
		int dir;
		for (dir = 0, b = 1, bj = 1 << 16; dir < 12; dir++, b <<= 1, bj <<= 1)
		{
			if (SorE == SELF)
			{
				controlS[te.from + Direct[dir]] &= ~b;
			}
			else
			{
				controlE[te.from + Direct[dir]] &= ~b;
			}
			if (CanJump[dir][te.koma])
			{
				int j = te.from;
				do
				{
					j += Direct[dir];
					if (SorE == SELF)
					{
						controlS[j] &= ~bj;
					}
					else
					{
						controlE[j] &= ~bj;
					}
				} while (ban[j] == EMPTY);
			}
		}
		ban[te.from] = EMPTY;
		KyokumenHashVal ^= HashSeed[te.koma][te.from];
		KyokumenHashVal ^= HashSeed[EMPTY][te.from];
		for (j = 0, bj = (1 << 16); j < 8; j++, bj <<= 1)
		{
			int Dir = Direct[j];
			if (controlS[te.from] & bj)
			{
				k = te.from;
				do 
				{
					k += Dir;
					controlS[k] |= bj;
				} while (ban[k] == EMPTY);
			}
			if (controlE[te.from] & bj)
			{
				k = te.from;
				do 
				{
					k += Dir;
					controlE[k] |= bj;
				} while (ban[k] == EMPTY);
			}
		}
	}
	else 
	{
		HandHashVal ^= HandHashSeed[te.koma][Hand[te.koma]];
		Hand[te.koma]--;
		value -= HandValue[te.koma];
		value += KomaValue[te.koma];
	}
	if (ban[te.to] != EMPTY) 
	{
		value -= KomaValue[ban[te.to]];
		value += HandValue[SorE | (ban[te.to] & ~PROMOTED&~SELF&~ENEMY)];
		int koma = SorE | (ban[te.to] & ~PROMOTED&~SELF&~ENEMY);
		Hand[koma]++;
		HandHashVal ^= HandHashSeed[koma][Hand[koma]];
		for (j = 0, b = 1, bj = (1 << 16); j < 12; j++, b <<= 1, bj <<= 1)
		{
			int Dir = Direct[j];
			if (CanJump[j][ban[te.to]])
			{
				k = te.to;
				do 
				{
					k += Dir;
					if (SorE == SELF) 
					{
						controlE[k] &= ~bj;
					}
					else
					{
						controlS[k] &= ~bj;
					}
				} while (ban[k] == EMPTY);
			}
			else
			{
				k = te.to + Dir;
				if (SorE == SELF) 
				{
					controlE[k] &= ~b;
				}
				else {
					controlS[k] &= ~b;
				}
			}
		}
	}
	else 
	{
		for (j = 0, bj = (1 << 16); j < 8; j++, bj <<= 1) 
		{
			int Dir = Direct[j];
			if (controlS[te.to] & bj) 
			{
				k = te.to;
				do 
				{
					k += Dir;
					controlS[k] &= ~bj;
				} while (ban[k] == EMPTY);
			}
			if (controlE[te.to] & bj) {
				k = te.to;
				do 
				{
					k += Dir;
					controlE[k] &= ~bj;
				} while (ban[k] == EMPTY);
			}
		}
	}
	KyokumenHashVal ^= HashSeed[ban[te.to]][te.to];
	if (te.promote) 
	{
		value -= KomaValue[te.koma];
		value += KomaValue[te.koma | PROMOTED];
		ban[te.to] = te.koma | PROMOTED;
	}
	else 
	{
		ban[te.to] = te.koma;
	}
	KyokumenHashVal ^= HashSeed[ban[te.to]][te.to];
	for (j = 0, b = 1, bj = (1 << 16); j < 12; j++, b <<= 1, bj <<= 1)
	{
		if (CanJump[j][ban[te.to]])
		{
			k = te.to;
			do 
			{
				k += Direct[j];
				if (SorE == SELF) 
				{
					controlS[k] |= bj;
				}
				else 
				{
					controlE[k] |= bj;
				}
			} while (ban[k] == EMPTY);
		}
		else if (CanMove[j][ban[te.to]]) 
		{
			if (SorE == SELF) 
			{
				controlS[te.to + Direct[j]] |= b;
			}
			else 
			{
				controlE[te.to + Direct[j]] |= b;
			}
		}
	}
	if (te.koma == SOU) 
	{
		KingS = te.to;
	}
	if (te.koma == EOU) 
	{
		KingE = te.to;
	}

	HashVal = KyokumenHashVal ^ HandHashVal;
	Tesu++;
	HashHistory[Tesu] = HashVal;
	OuteHistory[Tesu] = (SorE == SELF) ? controlS[KingE] : controlE[KingS];
}

void Kyokumen::MakePinInf(int *pin) const
{
	int j;
	for (j = 0x11; j <= 0x99; j++) 
	{
		pin[j] = 0;
	}
	if (KingS) 
	{
		for (j = 0; j < 8; j++)
		{
			int p;
			p = search(KingS, -Direct[j]);
			if ((ban[p] != WALL) && !(ban[p] & ENEMY))
			{
				if (controlE[p] & (1 << (16 + j))) 
				{
					pin[p] = Direct[j];
				}
			}
		}
	}
	if (KingE) 
	{	
		for (j = 0; j < 8; j++)
		{
			int p;
			p = search(KingE, -Direct[j]);
			if ((ban[p] != WALL) && (ban[p] & ENEMY)) 
			{ 
				if (controlS[p] & (1 << (16 + j))) 
				{
					pin[p] = Direct[j];
				}
			}
		}
	}
}

int Kyokumen::MakeLegalMoves(int SorE, Te *teBuf, int *pin)
{
	int pbuf[16 * 11];
	int teNum = 0;
	if (pin == nullptr)
	{
		MakePinInf(pbuf);
		pin = pbuf;
	}
	if (SorE == SELF && controlE[KingS] != 0)
	{
		return AntiCheck(SorE, teBuf, pin, controlE[KingS]);
	}
	if (SorE == ENEMY && controlS[KingE] != 0)
	{
		return AntiCheck(SorE, teBuf, pin, controlS[KingE]);
	}

	int suji, dan;
	int StartDan, EndDan;
	for (suji = 0x10; suji <= 0x90; suji += 0x10)
	{
		for (dan = 1; dan <= 9; dan++)
		{
			if (ban[suji + dan] & SorE)
			{
				AddMoves(SorE, teNum, teBuf, suji + dan, pin[suji + dan]);
			}
		}
	}
	if (Hand[SorE | FU] > 0)
	{
		for (suji = 0x10; suji <= 0x90; suji += 0x10)
		{
			int nifu = 0;

			for (dan = 1; dan <= 9; dan++)
			{
				if (ban[suji + dan] == (SorE | FU))
				{
					nifu = true;
					break;
				}
			}
			if (nifu)
			{
				continue;
			}
			if (SorE == SELF)
			{
				StartDan = 2;
				EndDan = 9;
			}
			else
			{
				StartDan = 1;
				EndDan = 8;
			}
			for (dan = StartDan; dan <= EndDan; dan++)
			{
				if (ban[dan + suji] == EMPTY && !Utifudume(SorE, dan + suji, pin))
				{
					teBuf[teNum++] = Te(0, suji + dan, SorE | FU, EMPTY);
				}
			}
		}
	}
	if (Hand[SorE | KY] > 0)
	{
		for (suji = 0x10; suji <= 0x90; suji += 0x10)
		{
			if (SorE == SELF)
			{
				StartDan = 2;
				EndDan = 9;
			}
			else
			{
				StartDan = 1;
				EndDan = 8;
			}
			for (dan = StartDan; dan <= EndDan; dan++)
			{
				if (ban[dan + suji] == EMPTY)
				{
					teBuf[teNum++] = Te(0, suji + dan, SorE | KY, EMPTY);
				}
			}
		}
	}
	if (Hand[SorE | KE] > 0)
	{
		for (suji = 0x10; suji <= 0x90; suji += 0x10)
		{
			if (SorE == SELF)
			{
				StartDan = 3;
				EndDan = 9;
			}
			else
			{
				StartDan = 1;
				EndDan = 7;
			}
			for (dan = StartDan; dan <= EndDan; dan++)
			{
				if (ban[dan + suji] == EMPTY)
				{
					teBuf[teNum++] = Te(0, suji + dan, SorE | KE, EMPTY);
				}
			}
		}
	}
	for (int koma = GI; koma <= HI; koma++)
	{
		if (Hand[SorE | koma] > 0)
		{
			for (suji = 0x10; suji <= 0x90; suji += 0x10)
			{
				for (dan = 1; dan <= 9; dan++)
				{
					if (ban[dan + suji] == EMPTY)
					{
						teBuf[teNum++] = Te(0, suji + dan, SorE | koma, EMPTY);
					}
				}
			}
		}
	}

	return teNum;
}

void Kyokumen::AddMoves(int SorE, int &teNum, Te *teTop, int from, int pin, int Rpin)
{
	switch (ban[from])
	{
	case SFU:
		AddMove(SorE, teNum, teTop, from, -1, pin, Rpin);
		break;
	case EFU:
		AddMove(SorE, teNum, teTop, from, +1, pin, Rpin);
		break;
	case SKY:
		AddStraight(SorE, teNum, teTop, from, -1, pin, Rpin);
		break;
	case EKY:
		AddStraight(SorE, teNum, teTop, from, +1, pin, Rpin);
		break;
	case SKE:
		AddMove(SorE, teNum, teTop, from, +14, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -18, pin, Rpin);
		break;
	case EKE:
		AddMove(SorE, teNum, teTop, from, -14, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, +18, pin, Rpin);
		break;
	case SGI:
		AddMove(SorE, teNum, teTop, from, -1, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 15, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -15, pin, Rpin);
		break;
	case EGI:
		AddMove(SorE, teNum, teTop, from, +1, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -15, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 15, pin, Rpin);
		break;
	case SKI:case STO:case SNY:case SNK:case SNG:
		AddMove(SorE, teNum, teTop, from, -1, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 15, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, +1, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -16, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 16, pin, Rpin);
		break;
	case EKI:case ETO:case ENY:case ENK:case ENG:
		AddMove(SorE, teNum, teTop, from, +1, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -15, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -1, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -16, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, 16, pin, Rpin);
		break;
	case SRY:case ERY:
		AddMove(SorE, teNum, teTop, from, 17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -15, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -17, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, +15, pin, Rpin);
	case SHI:case EHI:
		AddStraight(SorE, teNum, teTop, from, +1, pin, Rpin);
		AddStraight(SorE, teNum, teTop, from, -1, pin, Rpin);
		AddStraight(SorE, teNum, teTop, from, -16, pin, Rpin);
		AddStraight(SorE, teNum, teTop, from, +16, pin, Rpin);
		break;
	case SUM:case EUM:
		AddMove(SorE, teNum, teTop, from, +1, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, +16, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -16, pin, Rpin);
		AddMove(SorE, teNum, teTop, from, -1, pin, Rpin);
	case SKA:case EKA:
		AddStraight(SorE, teNum, teTop, from, +17, pin, Rpin);
		AddStraight(SorE, teNum, teTop, from, -17, pin, Rpin);
		AddStraight(SorE, teNum, teTop, from, +15, pin, Rpin);
		AddStraight(SorE, teNum, teTop, from, -15, pin, Rpin);
		break;
	case SOU:case EOU:
		MoveKing(SorE, teNum, teTop, 0);
	}
}

Kiki Kyokumen::CountControlS(int pos)
{
	Kiki ret = 0;
	int j, b, bj;
	b = 1;
	bj = 1 << 16;
	for (j = 0; j < 12; j++, b <<= 1, bj <<= 1)
	{
		if (CanMove[j][ban[pos - Direct[j]]] && (ban[pos - Direct[j]] & SELF))
		{
			ret |= b;
		}
		else if (CanJump[j][ban[search(pos, -Direct[j])]] && (ban[search(pos, -Direct[j])] & SELF))
		{
			ret |= bj;
		}
	}
	return ret;
}

Kiki Kyokumen::CountControlE(int pos)
{
	Kiki ret = 0;
	int j, b, bj;
	b = 1;
	bj = 1 << 16;
	for (j = 0; j < 12; j++, b <<= 1, bj <<= 1)
	{
		if (CanMove[j][ban[pos - Direct[j]]] && (ban[pos - Direct[j]] & ENEMY))
		{
			ret |= b;
		}
		else if (CanJump[j][ban[search(pos, -Direct[j])]] && (ban[search(pos, -Direct[j])] & ENEMY))
		{
			ret |= bj;
		}
	}
	return ret;
}

Kiki Kyokumen::CountMove(int SorE, int pos, int *pin)
{
	Kiki ret = 0;
	int j, b, bj;
	b = 1;
	bj = 1 << 16;
	for (j = 0; j < 12; j++, b <<= 1, bj <<= 1)
	{
		if (CanMove[j][ban[pos - Direct[j]]] && (ban[pos - Direct[j]] & SorE)
			&& (pin[pos - Direct[j]] == 0 || pin[pos - Direct[j]] == Direct[j] || pin[pos - Direct[j]] == -Direct[j]))
		{
			ret |= b;
		}
		else if (CanJump[j][ban[search(pos, -Direct[j])]] && (ban[search(pos, -Direct[j])] & SorE)
			&& (pin[search(pos, -Direct[j])] == 0 || pin[search(pos, -Direct[j])] == Direct[j] || pin[search(pos, -Direct[j])] == -Direct[j]))
		{
			ret |= bj;
		}
	}
	return ret;
}

int Kyokumen::Utifudume(int SorE, int to, int *pin)
{
	if (SorE == SELF)
	{
		if (KingE + 1 != to)
		{
			return 0;
		}
	}
	else
	{
		if (KingS - 1 != to)
		{
			return 0;
		}
	}
	ban[to] = FU | SorE;
	if (SorE == SELF)
	{
		if (controlS[to] && (CountMove(ENEMY, to, pin) == 1 << 1))
		{
			for (int i = 0; i < 8; i++)
			{
				KomaInf koma = ban[KingE + Direct[i]];
				if (!(koma & ENEMY) && !CountControlS(KingE + Direct[i]))
				{
					ban[to] = EMPTY;
					return 0;
				}
			}
			ban[to] = EMPTY;
			return 1;
		}
		ban[to] = EMPTY;
		return 0;
	}
	else
	{
		if (controlE[to] && (CountMove(SELF, to, pin) == 1 << 6))
		{
			for (int i = 0; i < 8; i++)
			{
				KomaInf koma = ban[KingS + Direct[i]];
				if (!(koma & SELF) && !CountControlE(KingS + Direct[i]))
				{
					ban[to] = EMPTY;
					return 0;
				}
			}
			ban[to] = EMPTY;
			return 1;
		}
		ban[to] = EMPTY;
		return 0;
	}
}

void Kyokumen::PutTo(int SorE, int &teNum, Te *teTop, int to, int *pin)
{
	int dan = to & 0x0f;
	if (SorE == ENEMY)
	{
		dan = 10 - dan;
	}
	if (Hand[SorE | FU] > 0 && dan > 1)
	{
		int suji = to & 0xf0;
		int nifu = 0;
		for (int d = 1; d <= 9; d++)
		{
			if (ban[suji + d] == (SorE | FU))
			{
				nifu = 1;
				break;
			}
		}
		if (!nifu && !Utifudume(SorE, to, pin))
		{
			teTop[teNum++] = Te(0, to, SorE | FU, EMPTY);
		}
	}
	if (Hand[SorE | KY] > 0 && dan > 1)
	{
		teTop[teNum++] = Te(0, to, SorE | KY, EMPTY);
	}
	if (Hand[SorE | KE] > 0 && dan > 2)
	{
		teTop[teNum++] = Te(0, to, SorE | KE, EMPTY);
	}
	for (int koma = GI; koma <= HI; koma++)
	{
		if (Hand[SorE | koma] > 0) {
			teTop[teNum++] = Te(0, to, SorE | koma, EMPTY);
		}
	}
}

int Kyokumen::AntiCheck(int SorE, Te *teBuf, int *pin, Kiki kiki)
{
	int King;
	int teNum = 0;
	if ((kiki & (kiki - 1)) != 0)
	{
		MoveKing(SorE, teNum, teBuf, kiki);
	}
	else
	{
		if (SorE == SELF)
		{
			King = KingS;
		}
		else
		{
			King = KingE;
		}
		unsigned int id;
		int check;
		for (id = 0; id <= 31; id++)
		{
			if (kiki == (1u << id))
			{
				break;
			}
		}
		if (id < 16)
		{
			check = King - Direct[id];
		}
		else
		{
			check = search(King, -Direct[id - 16]);
		}
		MoveTo(SorE, teNum, teBuf, check, pin);

		MoveKing(SorE, teNum, teBuf, kiki);

		if (id >= 16)
		{
			for (int j = King - Direct[id - 16]; ban[j] == EMPTY; j -= Direct[id - 16])
			{
				MoveTo(SorE, teNum, teBuf, j, pin);
			}
			for (int j = King - Direct[id - 16]; ban[j] == EMPTY; j -= Direct[id - 16])
			{
				PutTo(SorE, teNum, teBuf, j, pin);
			}
		}
	}
	return teNum;
}

void Kyokumen::MoveKing(int SorE, int &teNum, Te *teTop, Kiki kiki)
{
	int id = -1;
	for (int j = 0; j < 8; j++)
	{
		if (kiki & (1 << j))
		{
			id = j;
			break;
		}
	}
	if (id >= 0)
	{
		if (SorE == SELF)
		{
			KomaInf koma = ban[KingS - Direct[id]];
			if ((koma == EMPTY || (koma & ENEMY))
				&& !CountControlE(KingS - Direct[id])
				&& !(kiki & (1 << (23 - id))))
			{
				AddMove(SorE, teNum, teTop, KingS, -Direct[id], 0);
			}
		}
		else
		{
			KomaInf koma = ban[KingE - Direct[id]];
			if ((koma == EMPTY || (koma & SELF))
				&& !CountControlS(KingE - Direct[id])
				&& !(kiki & (1 << (23 - id))))
			{
				AddMove(SorE, teNum, teTop, KingE, -Direct[id], 0);
			}
		}
	}
	for (int j = 0; j < 8; j++)
	{
		if (j == id) continue;
		if (SorE == SELF)
		{
			KomaInf koma = ban[KingS - Direct[j]];
			if ((koma == EMPTY || (koma & ENEMY))
				&& !CountControlE(KingS - Direct[j])
				&& !(kiki & (1 << (23 - j))))
			{
				AddMove(SorE, teNum, teTop, KingS, -Direct[j], 0);
			}
		}
		else
		{
			KomaInf koma = ban[KingE - Direct[j]];
			if ((koma == EMPTY || (koma & SELF))
				&& !CountControlS(KingE - Direct[j])
				&& !(kiki & (1 << (23 - j))))
			{
				AddMove(SorE, teNum, teTop, KingE, -Direct[j], 0);
			}
		}
	}
}

void Kyokumen::AddMove(int SorE, int &teNum, Te *teTop, int from, int diff, int pin, int Rpin)
{
	if (Rpin == diff || Rpin == -diff)
	{
		return;
	}
	int to = from + diff;
	int dan = to & 0x0f;
	int fromDan = from & 0x0f;

	if ((pin == 0 || pin == diff || pin == -diff) && !(ban[to] & SorE))
	{
		if (ban[from] == SKE && dan <= 2)
		{
			teTop[teNum++] = Te(from, to, ban[from], ban[to], 1);
		}
		else if ((ban[from] == SFU || ban[from] == SKY) && dan <= 1)
		{
			teTop[teNum++] = Te(from, to, ban[from], ban[to], 1);
		}
		else if (ban[from] == EKE && dan >= 8)
		{
			teTop[teNum++] = Te(from, to, ban[from], ban[to], 1);
		}
		else if ((ban[from] == EFU || ban[from] == EKY) && dan >= 9)
		{
			teTop[teNum++] = Te(from, to, ban[from], ban[to], 1);
		}
		else
		{
			if (SorE == SELF && (fromDan <= 3 || dan <= 3) && CanPromote[ban[from]])
			{
				teTop[teNum++] = Te(from, to, ban[from], ban[to], 1);
			}
			else if (SorE == ENEMY && (fromDan >= 7 || dan >= 7) && CanPromote[ban[from]])
			{
				teTop[teNum++] = Te(from, to, ban[from], ban[to], 1);
			}
			teTop[teNum++] = Te(from, to, ban[from], ban[to], 0);
		}
	}
}

void Kyokumen::AddStraight(int SorE, int &teNum, Te *teTop, int from, int dir, int pin, int Rpin)
{
	if (dir == Rpin || dir == -Rpin)
	{
		return;
	}
	int j;
	if (pin == 0 || pin == dir || pin == -dir)
	{
		for (j = dir; ban[from + j] == EMPTY; j += dir)
		{
			AddMove(SorE, teNum, teTop, from, j, 0);
		}
		if (!(ban[from + j] & SorE))
		{
			AddMove(SorE, teNum, teTop, from, j, 0);
		}
	}
}

void Kyokumen::MoveTo(int SorE, int &teNum, Te *teTop, int to, int* pin)
{
	int p;
	KomaInf koma;

	for (int i = 0; i < 12; i++)
	{
		if ((koma = ban[to - Direct[i]]) == EMPTY)
		{
			p = search(to, -Direct[i]);
			if ((ban[p] & SorE) && CanJump[i][ban[p]])
			{
				AddMove(SorE, teNum, teTop, p, to - p, pin[p]);
			}
		}
		else
		{
			if ((koma&~SorE) != OU && (koma&SorE) && (CanMove[i][koma] || CanJump[i][koma]))
			{
				AddMove(SorE, teNum, teTop, to - Direct[i], Direct[i], pin[to - Direct[i]]);
			}
		}
	}
}

void Kyokumen::FPrint(FILE *fp)
{
	int x, y;
	y = 0;
	fprintf(fp, "éùÇøãÓÅF");
	for (x = EHI; x >= EFU; x--)
	{
		if (Hand[x] > 1)
		{
			y = 1;
			fprintf(fp, "%s%2.2s", komaStr2[x], "àÍìÒéOélå‹òZéµî™ã„101112131415161718" + 2 * Hand[x] - 2);
		}
		else if (Hand[x] == 1)
		{
			y = 1;
			fprintf(fp, "%s", komaStr2[x]);
		}
	}
	if (y)
	{
		fprintf(fp, "\n");
	}
	else
	{
		fprintf(fp, "Ç»Çµ\n");
	}
	fprintf(fp, "  ÇX ÇW ÇV ÇU ÇT ÇS ÇR ÇQ ÇP \n");
	fprintf(fp, "+---------------------------+\n");
	for (y = 1; y <= 9; y++)
	{
		fprintf(fp, "|");
		for (x = 9; x >= 1; x--)
		{
			fprintf(fp, komaStr[ban[x * 16 + y]]);
		}
		fprintf(fp, "|%2.2s", "àÍìÒéOélå‹òZéµî™ã„" + y * 2 - 2);
		fprintf(fp, "\n");
	}
	fprintf(fp, "+---------------------------+\n");
	fprintf(fp, "éùÇøãÓÅF");
	y = 0;
	for (x = SHI; x >= SFU; x--)
	{
		if (Hand[x] > 1)
		{
			y = 1;
			fprintf(fp, "%s%2.2s", komaStr2[x], "àÍìÒéOélå‹òZéµî™ã„101112131415161718" + 2 * Hand[x] - 2);
		}
		else if (Hand[x] == 1)
		{
			y = 1;
			fprintf(fp, "%s", komaStr2[x]);
		}
	}
	if (y)
	{
		fprintf(fp, "\n");
	}
	else
	{
		fprintf(fp, "Ç»Çµ\n");
	}
}

char *komaStrForDump[] = {
"EMP","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
"   ","SFU","SKY","SKE","SGI","SKI","SKA","SHI","SOU","STO","SNY","SNK","SNG","SKI","SUM","SRY",
"   ","EFU","EKY","EKE","EGI","EKI","EKA","EHI","EOU","ETO","ENY","ENK","ENG","EKI","EUM","ERY",
" ï«","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};


void Kyokumen::Dump()
{
	int x, y;
	printf("TestBan[9][9]={\n");
	for (y = 1; y <= 9; y++)
	{
		printf("\t{");
		for (x = 9; x >= 1; x--)
		{
			printf("%s", komaStrForDump[ban[x * 16 + y]]);
			if (x > 1)
			{
				printf(",");
			}
		}
		printf("}");
		if (y < 9)
		{
			printf(",");
		}
		printf("\n");
	}
	printf("};\n");
	printf("Motigoma[EHI+1]={");
	for (int koma = 0; koma <= EHI; koma++)
	{
		printf("%d,", Hand[koma]);
	}
	printf("};\n");
}

int kyori(int p1, int p2)
{
	return max(abs(p1 / 16 - p2 / 16), abs((p1 & 0x0f) - (p2 & 0x0f)));
}

int Kyokumen::IsCorrectMove(Te &te)
{
	if (te.from == 0)
	{
		if (ban[te.to] != EMPTY)
		{
			return 0;
		}
		if (te.koma == SFU)
		{
			for (int dan = 1; dan <= 9; dan++)
			{
				if (ban[(te.to & 0xf0) + dan] == SFU)
				{
					return 0;
				}
			}
			if (te.to == KingE + 1)
			{
				int pin[16 * 11];
				MakePinInf(pin);
				if (Utifudume(SELF, te.to, pin))
				{
					return 0;
				}
			}
		}
		if (te.koma == EFU)
		{
			for (int dan = 1; dan <= 9; dan++)
			{
				if (ban[(te.to & 0xf0) + dan] == EFU)
				{
					return 0;
				}
			}
			if (te.to == KingS - 1)
			{
				int pin[16 * 11];
				MakePinInf(pin);
				if (Utifudume(SELF, te.to, pin))
				{
					return 0;
				}
			}
		}
		return 1;
	}
	if (ban[te.from] == SOU)
	{
		if (controlE[te.to] != 0)
		{
			return 0;
		}
		else {
			te.capture = ban[te.to];
			return 1;
		}
	}
	else if (ban[te.from] == EOU)
	{
		if (controlS[te.to] != 0)
		{
			return 0;
		}
		else
		{
			te.capture = ban[te.to];
			return 1;
		}
	}
	if (ban[te.from] == SKE || ban[te.from] == EKE)
	{
		te.capture = ban[te.to];
		return 1;
	}
	int d = kyori(te.from, te.to);
	if (d == 0)
	{
		return 0;
	}
	int dir = (te.to - te.from) / d;
	if (d == 1)
	{
		te.capture = ban[te.to];
		return 1;
	}

	for (int i = 1, pos = te.from + dir; i < d; i++, pos = pos + dir)
	{
		if (ban[pos] != EMPTY)
		{
			return 0;
		}
	}
	te.capture = ban[te.to];
	return 1;
}

int Kyokumen::IsLegalMove(int SorE, Te &te)
{
	if (!(te.koma & SorE))
	{
		return 0;
	}
	if (te.from < OU)
	{
		if (Hand[te.koma] == 0)
		{
			return 0;
		}
	}
	else
	{
		if (ban[te.from] != te.koma)
		{
			return 0;
		}
	}
	if (ban[te.to] & SorE)
	{
		return 0;
	}
	if (IsCorrectMove(te))
	{
		Kyokumen kk(*this);
		kk.Move(SorE, te);
		if (SorE == SELF && kk.controlE[kk.KingS])
		{
			return 0;
		}
		if (SorE == ENEMY && kk.controlS[kk.KingE])
		{
			return 0;
		}
		return 1;
	}
	return 0;
}

int Kyokumen::EvalMin(Te *AtackS, int NumAtackS, Te *AtackE, int NumAtackE)
{
	int v = value;
	if (NumAtackE > 0)
	{
		int k = 0;
		while (!IsCorrectMove(AtackE[k]) && k < NumAtackE)
		{
			k++;
		}
		if (k == 0) {}
		else if (k < NumAtackE)
		{
			Te t = AtackE[k];
			for (int j = k; j > 0; j--)
			{
				AtackE[j] = AtackE[j - 1];
			}
			AtackE[0] = t;
		}
		else
		{

			return v;
		}
		AtackE[0].capture = ban[AtackE[0].to];
		Move(ENEMY, AtackE[0]);
		return min(v, EvalMax(AtackS, NumAtackS, AtackE + 1, NumAtackE - 1));
	}
	else {
		return v;
	}
}

int Kyokumen::EvalMax(Te *AtackS, int NumAtackS, Te *AtackE, int NumAtackE)
{
	int v = value;
	if (NumAtackS > 0)
	{
		int k = 0;
		while (!IsCorrectMove(AtackS[k]) && k < NumAtackS)
		{
			k++;
		}
		if (k == 0) {}
		else if (k < NumAtackS)
		{
			Te t = AtackS[k];
			for (int j = k; j > 0; j--)
			{
				AtackS[j] = AtackS[j - 1];
			}
			AtackS[0] = t;
		}
		else
		{
			return v;
		}
		AtackS[0].capture = ban[AtackS[0].to];
		Move(SELF, AtackS[0]);
		return max(v, EvalMin(AtackS + 1, NumAtackS - 1, AtackE, NumAtackE));
	}
	else
	{
		return v;
	}
}

int Kyokumen::Eval(int position)
{
	if (ban[position] == EMPTY)
	{
		return 0;
	}
	if ((ban[position] & SELF) && !controlE[position])
	{
		return 0;
	}
	if ((ban[position] & ENEMY) && !controlS[position])
	{
		return 0;
	}

	int ret;
	Te teTop[40];
	int ToPos = position;

	Te *AtackS = teTop;
	Te *AtackE = teTop + 18;

	int AtackCountE = 0;
	int AtackCountS = 0;

	int PromoteS, PromoteE;
	int b = 1;
	int bj = 1 << 16;
	int j;
	int pos = ToPos;
	int pos2;

	if ((ToPos & 0x0f) <= 3)
	{
		PromoteS = 1;
	}
	else
	{
		PromoteS = 0;
	}
	if ((ToPos & 0x0f) >= 7)
	{
		PromoteE = 1;
	}
	else
	{
		PromoteE = 0;
	}

	for (j = 0; j < 8; j++)
	{
		pos2 = pos;
		if (controlS[pos] & b)
		{
			pos2 -= Direct[j];
			AtackS[AtackCountS].from = pos2;
			AtackS[AtackCountS].koma = ban[pos2];
			AtackS[AtackCountS].to = pos;
			if ((PromoteS || (pos2 & 0x0f) <= 3) && CanPromote[AtackS[AtackCountS].koma])
			{
				AtackS[AtackCountS].promote = 1;
			}
			else
			{
				AtackS[AtackCountS].promote = 0;
			}
			AtackCountS++;
		}
		else if (controlE[pos] & b)
		{
			pos2 -= Direct[j];
			AtackE[AtackCountE].from = pos2;
			AtackE[AtackCountE].koma = ban[pos2];
			AtackE[AtackCountE].to = pos;
			if ((PromoteE || (pos2 & 0x0f) >= 7) && CanPromote[AtackE[AtackCountE].koma])
			{
				AtackE[AtackCountE].promote = 1;
			}
			else
			{
				AtackE[AtackCountE].promote = 0;
			}
			AtackCountE++;
		}
		if (ban[pos - Direct[j]] != OU && ban[pos - Direct[j]] != EOU)
		{
			while ((controlS[pos2] & bj) || (controlE[pos2] & bj))
			{
				pos2 -= Direct[j];
				while (ban[pos2] == EMPTY)
				{
					pos2 -= Direct[j];
				}
				if ((ban[pos2])&ENEMY)
				{
					AtackE[AtackCountE].from = pos2;
					AtackE[AtackCountE].koma = ban[pos2];
					AtackE[AtackCountE].to = pos;
					if ((PromoteE || (pos2 & 0x0f) >= 7) && CanPromote[AtackE[AtackCountE].koma])
					{
						AtackE[AtackCountE].promote = 1;
					}
					else
					{
						AtackE[AtackCountE].promote = 0;
					}
					AtackCountE++;
				}
				else if ((ban[pos2])&SELF)
				{
					AtackS[AtackCountS].from = pos2;
					AtackS[AtackCountS].koma = ban[pos2];
					AtackS[AtackCountS].to = pos;
					if ((PromoteS || (pos2 & 0x0f) <= 3) && CanPromote[AtackS[AtackCountS].koma])
					{
						AtackS[AtackCountS].promote = 1;
					}
					else
					{
						AtackS[AtackCountS].promote = 0;
					}
					AtackCountS++;
				}
			}
		}
		b <<= 1;
		bj <<= 1;
	}

	b = 1 << 8;
	for (j = 8; j < 12; j++)
	{
		if (controlS[pos] & b)
		{
			pos2 = pos - Direct[j];
			AtackS[AtackCountS].from = pos2;
			AtackS[AtackCountS].koma = ban[pos2];
			AtackS[AtackCountS].to = pos;
			if (PromoteS && CanPromote[AtackS[AtackCountS].koma])
			{
				AtackS[AtackCountS].promote = 1;
			}
			else
			{
				AtackS[AtackCountS].promote = 0;
			}
			AtackCountS++;
		}
		if (controlE[pos] & b)
		{
			pos2 = pos - Direct[j];
		}
		AtackE[AtackCountE].from = pos2;
		AtackE[AtackCountE].koma = ban[pos2];
		AtackE[AtackCountE].to = pos;
		if (PromoteE && CanPromote[AtackE[AtackCountE].koma]) {
			AtackE[AtackCountE].promote = 1;
		}
		else {
			AtackE[AtackCountE].promote = 0;
		}
		AtackCountE++;
	}
	b <<= 1;

	for (j = 0; j < AtackCountS - 1; j++)
	{
		int max_id = j;
		int max_val = KomaValue[AtackS[j].koma];
		for (int k = j + 1; k < AtackCountS; k++)
		{
			int v = KomaValue[AtackS[k].koma];
			if (v < max_val)
			{
				max_id = k;
				max_val = v;
			}
			else if (v == max_val) {
				if (KomaValue[AtackS[k].koma] < KomaValue[AtackS[max_id].koma])
				{
					max_id = k;
				}
			}
		}

		if (j != max_id)
		{
			swap(AtackS[j], AtackS[max_id]);
		}
	}

	for (j = 0; j < AtackCountE - 1; j++)
	{
		int max_id = j;
		int max_val = KomaValue[AtackE[j].koma];
		for (int k = j + 1; k < AtackCountE; k++)
		{
			int v = KomaValue[AtackE[k].koma];
			if (v > max_val)
			{
				max_id = k;
				max_val = v;
			}
			else if (v == max_val) {
				if (KomaValue[AtackE[k].koma] > KomaValue[AtackE[max_id].koma])
				{
					max_id = k;
				}
			}
		}
		//ç≈ëÂílÇ∆ÇÃåä∑
		if (j != max_id)
		{
			swap(AtackE[j], AtackE[max_id]);
		}
	}

	int IsEnemy = (ban[position] & ENEMY);
	int IsSelf = !IsEnemy && ban[position] != EMPTY;
	if (IsEnemy && AtackCountS > 0)
	{
		int Eval = value;
		Kyokumen now(*this);
		ret = now.EvalMax(AtackS, AtackCountS, AtackE, AtackCountE) - Eval;
	}
	else if (IsSelf && AtackCountE > 0)
	{
		int Eval = value;
		Kyokumen now(*this);
		ret = Eval - now.EvalMin(AtackS, AtackCountS, AtackE, AtackCountE);
	}
	else
	{
		ret = 0;
	}
	return ret;
}

int Kyokumen::BestEval(int SorE)
{
	int best = 0;
	for (int suji = 0x10; suji <= 0x90; suji += 0x10)
	{
		for (int dan = 1; dan <= 9; dan++)
		{
			if ((ban[suji + dan] & SorE) == 0)
			{

			}int value = Eval(suji + dan);
			if (value > best)
			{
				best = value;
			}
		}
	}
	if (SorE == ENEMY)
	{
		return -best;
		return best;
	}
	return best;
}

int KyokumenKomagumi::KomagumiValue[ERY + 1][16 * 11];
int KyokumenKomagumi::SemegomaValueS[16 * 11][16 * 11];
int KyokumenKomagumi::SemegomaValueE[16 * 11][16 * 11];
int KyokumenKomagumi::MamorigomaValueS[16 * 11][16 * 11];
int KyokumenKomagumi::MamorigomaValueE[16 * 11][16 * 11];

int DanValue[ERY + 1][10] = {
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ãÛ
	{0,0,0,0,0,0,0,0,0,0},
	//ï‡
	{ 0,  0,15,15,15,3,1, 0, 0, 0},
	//çÅ
	{ 0, 1,2,3,4,5,6,7,8,9},
	//åj
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ã‚
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ã‡
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//äp
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//îÚ
	{ 0,10,10,10, 0, 0, 0,  -5, 0, 0},
	//â§
	{ 0,1200,1200,900,600,300,-10,0,0,0},
	//Ç∆
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//à«
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//å\
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ëS
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ã‡
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//în
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ó¥
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ãÛ
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ï‡
	{ 0, 0, 0, 0, -1, -3,-15,-15,-15, 0},
	//çÅ
	{ 0,-9,-8,-7, -6, -5, -4, -3, -2,-1},
	//åj
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ã‚
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ã‡
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//äp
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//îÚ
	{ 0, 0, 0, 5, 0, 0, 0,-10,-10,-10},
	//â§
	{ 0, 0, 0, 0,10,-300,-600,-900,-1200,-1200},
	//Ç∆
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//à«
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//å\
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ëS
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ã‡
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//în
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//ó¥
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

enum {
	IvsFURI,		// ãèîÚé‘ëŒêUÇËîÚé‘
	IvsNAKA,		// ãèîÚé‘ëŒíÜîÚé‘
	FURIvsFURI,		// ëäêUÇËîÚé‘
	FURIvsI,		// êUÇËîÚé‘ëŒãèîÚé‘
	NAKAvsI,		// íÜîÚé‘ëŒãèîÚé‘
	KAKUGAWARI,		// äpä∑ÇË
	AIGAKARI,		// ëää|Ç©ÇËÅiÇ‹ÇΩÇÕãèîÚé‘ÇÃëŒçRånÅj
	HUMEI			// êÌå`ïsñæ
};

int JosekiKomagumiSGI[HUMEI + 1][9][9] =
{
	{	// IvsFURI èMàÕÇ¢ÅAî¸îZÅAã‚ä•
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7,-10,-10,-10,-10,-10,  7,-10},
		{-10,  7, -8, -7, 10,-10, 10,  6,-10},
		{-10, -2, -6, -5,-10,  6,-10,-10,-10},
		{-10, -7,  0,-10,-10,-10,-10,-10,-10}
	},{	// IvsNAKAÅ@èMàÕÇ¢
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7,-10,-10, -7,-10,-10,  7,-10},
		{-10, -5, -8, -7, 10,-10, 10,  6,-10},
		{-10, -2, -3,  0,-10,  6,-10,-10,-10},
		{-10, -7, -5,-10,-10,-10,-10,-10,-10}
	},{ // FURIvsFURIÅ@ñÓëqÅiãtÅjÅAî¸îZÅAã‚ä•
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10, -7, -7,-10},
		{-10,-10,-10,-10,-10,  5, 10, 10,-10},
		{-10,-10,-10,-10,-10,-10,  0,-10,-10},
		{-10,-10,-10,-10,-10,-10, -5,-10,-10}
	},{ // FURIvsI î¸îZàÕÇ¢ÅAã‚ä•
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10, -3, -7,-10,-10,-10,-10,-10},
		{-10, -7,  4,  6,-10,-10,-10,  6,-10},
		{-10,  2,  3,  3,-10,-10,  4,-10,-10},
		{-10,-10,-10,  0,-10,-10,  0,-10,-10}
	},{ // NAKAvsI íÜîÚé‘
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  8,  5,  8,-10,-10,-10},
		{-10,-10,  4,  4,  3,  4,  4,-10,-10},
		{-10,-10,  0,-10,-10,-10,  0,-10,-10}
	},{ // KAKUGAWARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,  7,  5, -3,-10,-10},
		{-10,  8, 10,  7,  4,  0, -4,-10,-10},
		{-10,  0,-8,  -4,-10,-10, -5,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10}
	},{ // AIGAKARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,  0,-10,-10,-10,-10,-10,-10},
		{-10, -5,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10}
	},{ // HUMEI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,  5,-10,-10},
		{-10,-10,-10,-10,-10,-10, -4,  0,-10},
		{-10,-10,  0,-10,-10,-10, -4, -3,-10},
		{-10, -5,-10, -5,-10,-10, -5,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10}
	}
};

int JosekiKomagumiSKI[HUMEI + 1][9][9] =
{
	{	// IvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  1,  2,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10, -4,-10,-10,-10}
	},{	// IvsNAKA
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  1,  2,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10, -4,-10,-10,-10}
	},{ // FURIvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  7, -3,-10,-10},
		{-10,-10,-10,-10,  5,  3,  6,-10,-10},
		{-10,-10,-10,-10,-10,  5,  4,-10,-10}
	},{ // FURIvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  5,  1,-10,-10},
		{-10,-10,-10,-10,  4,  3,  7, -3,-10},
		{-10,-10,-10,  0,  1,  5,  2, -7,-10}
	},{ // NAKAvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10, -7, -4, -4,-10, -4, -4, -7,-10},
		{-10, -5, 10,  6,-10,  8, 10, -5,-10},
		{-10, -7, -6, -3, -6, -3, -6, -7,-10}
	},{ // KAKUGAWARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  6, -4, -4, -4, -8,-10},
		{-10,-10, 10,-10,  3,  0,  0, -7,-10},
		{-10,-10,-10,  0,-10,  0, -5, -7,-10}
	},{ // AIGAKARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,  6,-10,-10,-10,-10,-10},
		{-10,-10, 10,-10,  3,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10,  0,-10,-10,-10}
	},{ // HUMEI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,  3,-10,  5,-10,-10,-10,-10},
		{-10,-10,-10,  0,-10,  0,-10,-10,-10}
	}
};

int JosekiKomagumiSOU[HUMEI + 1][9][9] =
{
	{	// IvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-7,  9,-10,-10,-10,-10,-10,-10,-10},
		{  5,  7,  8,  4,-10,-10,-10,-10,-10},
		{ 10,  5,  3,-10,-10,-10,-10,-10,-10}
	},{	// IvsNAKA
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-7,  9,-10,-10,-10,-10,-10,-10,-10},
		{  5,  7,  8,  4,-10,-10,-10,-10,-10},
		{ 10,  5,  3,-10,-10,-10,-10,-10,-10}
	},{ // FURIvsFURI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  4,  6, 10,  6},
		{-10,-10,-10,-10,-10,  4,  6,  5, 10}
	},{ // FURIvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  4,  6, 10,  6},
		{-10,-10,-10,-10,-10,  4,  6,  5, 10}
	},{ // NAKAvsI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,  4,  6, 10,  6},
		{-10,-10,-10,-10,-10,  4,  6,  5, 10}
	},{ // KAKUGAWARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-3, -4, -3,-10,-10,-10,-10,-10,-10},
		{  6,  8, -2,  0, -3,-10,-10,-10,-10},
		{ 10,  6, -4,-6,-7,-10,-10,-10,-10}
	},{ // AIGAKARI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-3, -4, -3,-10,-10,-10,-10,-10,-10},
		{  6,  8,  0,-4,-10,-10,-10,-10,-10},
		{ 10,  6, -4,-6,-7,-10,-10,-10,-10}
	},{ // HUMEI
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-10,-10,-10,-10,-10,-10,-10,-10,-10},
		{-3, -4, -3,-10,-10,-10,-10,-10,-10},
		{  6,  8,  0,-4,-10,-10,-10,-10,-10},
		{ 10,  6, -4,-6,-7,-10,-10,-10,-10}
	}
};

int JosekiKomagumi[9][ERY + 1][16 * 11];

void KyokumenKomagumi::Initialize()
{
	int suji, dan, koma;
	for (suji = 0x10; suji <= 0x90; suji += 0x10)
	{
		for (dan = 1; dan <= 9; dan++)
		{
			for (koma = SFU; koma <= ERY; koma++)
			{
				KomagumiValue[koma][suji + dan] = 0;
				JosekiKomagumi[0][koma][suji + dan] = DanValue[koma][dan];
			}
		}
	}
	InitKanagomaValue();
	InitShuubando();
	InitBonus();
}

void KyokumenKomagumi::SenkeiInit()
{
	int SHI1, SHI2;
	int EHI1, EHI2;
	int SKA1, SKA2;
	int EKA1, EKA2;
	int suji, dan, koma;
	SHI1 = SHI2 = EHI1 = EHI2 = SKA1 = SKA2 = EKA1 = EKA2 = 0;
	for (suji = 0x10; suji <= 0x90; suji += 0x10)
	{
		for (dan = 1; dan <= 9; dan++)
		{
			if (ban[suji + dan] == SHI)
			{
				if (SHI1 == 0)
				{
					SHI1 = suji + dan;
				}
				else
				{
					SHI2 = suji + dan;
				}
			}
			if (ban[suji + dan] == EHI)
			{
				if (EHI1 == 0)
				{
					EHI1 = suji + dan;
				}
				else
				{
					EHI2 = suji + dan;
				}
			}
			if (ban[suji + dan] == SKA)
			{
				if (SKA1 == 0)
				{
					SKA1 = suji + dan;
				}
				else
				{
					SKA2 = suji + dan;
				}
			}
			if (ban[suji + dan] == EKA)
			{
				if (EKA1 == 0)
				{
					EKA1 = suji + dan;
				}
				else
				{
					EKA2 = suji + dan;
				}
			}
		}
	}
	if (Hand[SHI] == 1)
	{
		if (SHI1 == 0) SHI1 = 1;
	}
	else
	{
		SHI2 = 1;
	}
	if (Hand[SHI] == 2)
	{
		SHI1 = SHI2 = 1;
	}
	if (Hand[EHI] == 1)
	{
		if (EHI1 == 0)
		{
			EHI1 = 1;
		}
		else
		{
			EHI2 = 1;
		}
	}
	if (Hand[EHI] == 2)
	{
		EHI1 = EHI2 = 1;
		if (Hand[SKA] == 1)
		{
			if (SKA1 == 0) SKA1 = 1;
		}
		else
		{
			SKA2 = 1;
		}
	}
	if (Hand[SKA] == 2)
	{
		SKA1 = SKA2 = 1;
	}
	if (Hand[EKA] == 1)
	{
		if (EKA1 == 0)
		{
			EKA1 = 1;
		}
		else
		{
			EKA2 = 1;
		}
	}
	if (Hand[EKA] == 2)
	{
		EKA1 = EKA2 = 1;
	}

	int Senkei, GyakuSenkei;
	if (SHI1 <= 0x50 && EHI1 <= 0x50)
	{
		Senkei = IvsFURI;
		GyakuSenkei = FURIvsI;
	}
	else if (0x50 <= EHI1 && EHI1 <= 0x5f && SHI1 <= 0x50)
	{
		Senkei = IvsNAKA;
		GyakuSenkei = NAKAvsI;
	}
	else if (SHI1 <= 0x5f && EHI1 <= 0x5f) {

		Senkei = FURIvsFURI;
		GyakuSenkei = FURIvsFURI;
	}
	else if (EHI1 >= 0x60 && SHI1 >= 0x60)
	{
		Senkei = FURIvsI;
		GyakuSenkei = IvsFURI;
	}
	else if (0x50 <= SHI1 && SHI1 <= 0x5f && EHI1 <= 0x50)
	{
		Senkei = NAKAvsI;
		GyakuSenkei = IvsNAKA;
	}
	else if (SKA1 == 1 && EKA1 == 1)
	{
		Senkei = KAKUGAWARI;
		GyakuSenkei = KAKUGAWARI;
	}
	else if (0x20 <= SHI1 && SHI1 <= 0x2f && 0x80 <= EHI1 && EHI1 <= 0x8f)
	{
		Senkei = AIGAKARI;
		GyakuSenkei = AIGAKARI;
	}
	else
	{
		Senkei = HUMEI;
		GyakuSenkei = HUMEI;
	}
	KomagumiBonus[0] = KomagumiBonus[1] = 0;
	for (suji = 0x10; suji <= 0x90; suji += 0x10)
	{
		for (dan = 1; dan <= 9; dan++)
		{
			value -= KomagumiValue[ban[suji + dan]][suji + dan];
			for (koma = SFU; koma <= ERY; koma++)
			{
				if (koma == SGI)
				{
					JosekiKomagumi[Senkei][koma][suji + dan] = JosekiKomagumiSGI[Senkei][dan - 1][9 - (suji / 0x10)];
				}
				else if (koma == EGI)
				{
					JosekiKomagumi[Senkei][koma][suji + dan] = -JosekiKomagumiSGI[GyakuSenkei][9 - dan][suji / 0x10 - 1];
				}
				else if (koma == SKI)
				{
					JosekiKomagumi[Senkei][koma][suji + dan] = JosekiKomagumiSKI[Senkei][dan - 1][9 - (suji / 0x10)];
				}
				else if (koma == EKI)
				{
					JosekiKomagumi[Senkei][koma][suji + dan] = -JosekiKomagumiSKI[GyakuSenkei][9 - dan][suji / 0x10 - 1];
				}
				else if (koma == SOU)
				{
					JosekiKomagumi[Senkei][koma][suji + dan] = JosekiKomagumiSOU[Senkei][dan - 1][9 - (suji / 0x10)];
				}
				else if (koma == EOU)
				{
					JosekiKomagumi[Senkei][koma][suji + dan] = -JosekiKomagumiSOU[GyakuSenkei][9 - dan][suji / 0x10 - 1];
				}
				else
				{
					JosekiKomagumi[Senkei][koma][suji + dan] = DanValue[koma][dan];
				}
				KomagumiValue[koma][suji + dan] = JosekiKomagumi[Senkei][koma][suji + dan];
			}
			if (ban[suji + dan] & SELF)
			{
				KomagumiBonus[0] += KomagumiValue[ban[suji + dan]][suji + dan];
			}
			else if (ban[suji + dan] & ENEMY)
			{
				KomagumiBonus[1] += KomagumiValue[ban[suji + dan]][suji + dan];
			}
		}
	}
}

static int Mamorigoma[17][9] = {
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 56, 52, 50, 50, 50, 50, 50, 50, 50},
	{ 64, 61, 55, 50, 50, 50, 50, 50, 50},
	{ 79, 77, 70, 65, 54, 51, 50, 50, 50},
	{100, 99, 95, 87, 74, 58, 50, 50, 50},
	{116,117,101, 95, 88, 67, 54, 50, 50},
	{131,129,124,114, 90, 71, 59, 51, 50},
	{137,138,132,116, 96, 76, 61, 53, 50},
	{142,142,136,118, 98, 79, 64, 52, 50},
	{132,132,129,109, 95, 75, 60, 51, 50},
	{121,120,105, 97, 84, 66, 54, 50, 50},
	{ 95, 93, 89, 75, 68, 58, 51, 50, 50},
	{ 79, 76, 69, 60, 53, 50, 50, 50, 50},
	{ 64, 61, 55, 51, 50, 50, 50, 50, 50},
	{ 56, 52, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
};

static int Semegoma[17][9] = {
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 54, 53, 51, 51, 50, 50, 50, 50, 50},
	{ 70, 66, 62, 55, 53, 50, 50, 50, 50},
	{ 90, 85, 80, 68, 68, 60, 53, 50, 50},
	{100, 97, 95, 85, 84, 71, 51, 50, 50},
	{132,132,129,102, 95, 71, 51, 50, 50},
	{180,145,137,115, 91, 75, 57, 50, 50},
	{170,165,150,121, 94, 78, 58, 52, 50},
	{170,160,142,114, 98, 80, 62, 55, 50},
	{140,130,110,100, 95, 75, 54, 50, 50},
	{100, 99, 95, 87, 78, 69, 50, 50, 50},
	{ 80, 78, 72, 67, 55, 51, 50, 50, 50},
	{ 62, 60, 58, 52, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50},
};

void KyokumenKomagumi::InitKanagomaValue()
{
	for (int kingSdan = 1; kingSdan <= 9; kingSdan++)
	{
		for (int kingSsuji = 0x10; kingSsuji <= 0x90; kingSsuji += 0x10)
		{
			for (int kingEdan = 1; kingEdan <= 9; kingEdan++)
			{
				for (int kingEsuji = 0x10; kingEsuji <= 0x90; kingEsuji += 0x10)
				{
					for (int suji = 0x10; suji <= 0x90; suji += 0x10)
					{
						for (int dan = 1; dan <= 9; dan++)
						{
							int DiffSujiS = abs(kingSsuji - suji) / 0x10;
							int DiffSujiE = abs(kingEsuji - suji) / 0x10;
							int DiffDanSS = 8 + (dan - kingSdan);
							int DiffDanES = 8 + (dan - kingEdan);
							int DiffDanSE = 8 + (-(dan - kingSdan));
							int DiffDanEE = 8 + (-(dan - kingEdan));
							int KingS = kingSsuji + kingSdan;
							int KingE = kingEsuji + kingEdan;

							SemegomaValueS[suji + dan][KingE] = Semegoma[DiffDanES][DiffSujiE] - 100;
							MamorigomaValueS[suji + dan][KingS] = Mamorigoma[DiffDanSS][DiffSujiS] - 100;
							SemegomaValueE[suji + dan][KingS] = -(Semegoma[DiffDanSE][DiffSujiS] - 100);
							MamorigomaValueE[suji + dan][KingE] = -(Mamorigoma[DiffDanEE][DiffSujiE] - 100);
						}
					}
				}
			}
		}
	}
}

//ãÛï‡çÅåjã‚ã‡äpîÚâ§Ç∆à«å\ëSã‡înó¥
int ShuubandoByAtack[] = { 0,1,1,2,3,3,3,4,4,3,3,3,3,3,4,5 };
//ãÛï‡çÅåjã‚ ã‡äpîÚâ§ Ç∆ à« å\ ëS ã‡ în ó¥
int ShuubandoByDefence[] = { 0,0,0,0,-1,-1,0,0,0,-1,-1,-1,-1,-1,-2,0 };
//ãÛï‡çÅåjã‚ã‡äpîÚâ§Ç∆à«å\ëSã‡înó¥
int ShuubandoByHand[] = { 0,0,1,1,2,2,2,3,0,0,0,0,0,0,0,0 };

void KyokumenKomagumi::InitShuubando()
{
	int suji, dan;
	Shuubando[0] = 0;
	Shuubando[1] = 0;
	for (suji = 0x10; suji <= 0x90; suji += 0x10)
	{
		for (dan = 1; dan <= 4; dan++)
		{
			if (ban[suji + dan] & SELF)
			{
				Shuubando[1] += ShuubandoByAtack[ban[suji + dan] & ~SELF];
			}
			if (ban[suji + dan] & ENEMY)
			{
				Shuubando[1] += ShuubandoByDefence[ban[suji + dan] & ~ENEMY];
			}
		}
		for (dan = 6; dan <= 9; dan++)
		{
			if (ban[suji + dan] & ENEMY)
			{
				Shuubando[0] += ShuubandoByAtack[ban[suji + dan] & ~ENEMY];
			}
			if (ban[suji + dan] & SELF)
			{
				Shuubando[0] += ShuubandoByDefence[ban[suji + dan] & ~SELF];
			}
		}
	}
	int koma;
	for (koma = FU; koma <= HI; koma++)
	{
		Shuubando[0] += ShuubandoByHand[koma] * Hand[ENEMY | koma];
		Shuubando[1] += ShuubandoByHand[koma] * Hand[SELF | koma];
	}
}

int IsKanagoma[] = {
	//  ãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛï‡çÅåjã‚ã‡äpîÚâ§Ç∆à«å\ëSã‡înó¥
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,
		//	ãÛï‡çÅåjã‚ã‡äpîÚâ§Ç∆à«å\ëSã‡înó¥ï«ãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛãÛ
			0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void KyokumenKomagumi::InitBonus()
{
	int suji, dan;
	SemegomaBonus[0] = SemegomaBonus[1] = 0;
	MamorigomaBonus[0] = MamorigomaBonus[1] = 0;

	for (suji = 0x10; suji <= 0x90; suji += 0x10)
	{
		for (dan = 1; dan <= 9; dan++)
		{
			if (IsKanagoma[ban[suji + dan]])
			{
				if (ban[suji + dan] & SELF)
				{
					SemegomaBonus[0] += SemegomaValueS[suji + dan][KingE];
					MamorigomaBonus[0] += MamorigomaValueS[suji + dan][KingS];
				}
				else if (ban[suji + dan] & ENEMY)
				{
					SemegomaBonus[1] += SemegomaValueE[suji + dan][KingS];
					MamorigomaBonus[1] += MamorigomaValueE[suji + dan][KingE];
				}
			}
		}
	}
}

void KyokumenKomagumi::Move(int SorE, const Te &te)
{
	int self, enemy;
	if (SorE == SELF)
	{
		self = 0;
		enemy = 1;
	}
	else
	{
		self = 1;
		enemy = 0;
	}
	if (te.koma == OU || te.koma == EOU)
	{
	}
	else
	{
		if (IsKanagoma[te.koma] && te.from > 0)
		{
			if (SorE == SELF)
			{
				SemegomaBonus[0] -= SemegomaValueS[te.from][KingE];
				MamorigomaBonus[0] -= MamorigomaValueS[te.from][KingS];
			}
			else
			{
				SemegomaBonus[1] -= SemegomaValueE[te.from][KingS];
				MamorigomaBonus[1] -= MamorigomaValueE[te.from][KingE];
			}
		}
		if (te.capture)
		{
			if (IsKanagoma[te.capture])
			{
				if (SorE == SELF)
				{
					SemegomaBonus[1] -= SemegomaValueE[te.to][KingS];
					MamorigomaBonus[1] -= MamorigomaValueE[te.to][KingE];
				}
				else
				{
					SemegomaBonus[0] -= SemegomaValueS[te.to][KingE];
					MamorigomaBonus[0] -= MamorigomaValueS[te.to][KingS];
				}
			}
		}
		if (!te.promote)
		{
			if (IsKanagoma[te.koma])
			{
				if (SorE == SELF)
				{
					SemegomaBonus[0] += SemegomaValueS[te.to][KingE];
					MamorigomaBonus[0] += MamorigomaValueS[te.to][KingS];
				}
				else
				{
					SemegomaBonus[1] += SemegomaValueE[te.to][KingS];
					MamorigomaBonus[1] += MamorigomaValueE[te.to][KingE];
				}
			}
		}
		else
		{
			if (IsKanagoma[te.koma | PROMOTED])
			{
				if (SorE == SELF)
				{
					SemegomaBonus[0] += SemegomaValueS[te.to][KingE];
					MamorigomaBonus[0] += MamorigomaValueS[te.to][KingS];
				}
				else
				{
					SemegomaBonus[1] += SemegomaValueE[te.to][KingS];
					MamorigomaBonus[1] += MamorigomaValueE[te.to][KingE];
				}
			}
		}
	}
	KomagumiBonus[self] -= KomagumiValue[te.koma][te.from];
	if (te.from > 0 && (te.from & 0x0f) <= 4)
	{
		if (SorE == SELF)
		{
			Shuubando[1] -= ShuubandoByAtack[te.koma & ~SELF];
		}
		else
		{
			Shuubando[1] -= ShuubandoByDefence[te.koma & ~ENEMY];
		}
	}
	if (te.from > 0 && (te.from & 0x0f) >= 6)
	{
		if (SorE == SELF)
		{
			Shuubando[0] -= ShuubandoByDefence[te.koma & ~SELF];
		}
		else
		{
			Shuubando[0] -= ShuubandoByAtack[te.koma & ~ENEMY];
		}
	}
	if (te.from == 0)
	{
		if (SorE == SELF)
		{
			Shuubando[1] -= ShuubandoByHand[te.koma&~SELF];
		}
		else
		{
			Shuubando[0] -= ShuubandoByHand[te.koma&~ENEMY];
		}
	}
	if (te.capture)
	{
		if ((te.to & 0x0f) <= 4)
		{
			if (SorE == SELF)
			{
				Shuubando[1] -= ShuubandoByDefence[te.capture & ~ENEMY];
			}
			else
			{
				Shuubando[1] -= ShuubandoByAtack[te.capture & ~SELF];
			}
		}
		if ((te.to & 0x0f) >= 6)
		{
			if (SorE == SELF) {
				Shuubando[0] -= ShuubandoByAtack[te.capture & ~ENEMY];
			}
			else {
				Shuubando[0] -= ShuubandoByDefence[te.capture & ~SELF];
			}
		}
		if (SorE == SELF)
		{
			Shuubando[1] += ShuubandoByHand[te.capture&~ENEMY&~PROMOTED];
		}
		else
		{
			Shuubando[0] += ShuubandoByHand[te.capture&~SELF&~PROMOTED];
		}
		KomagumiBonus[enemy] -= KomagumiValue[te.capture][te.to];
	}
	if (!te.promote)
	{
		if ((te.to & 0x0f) <= 4)
		{
			if (SorE == SELF)
			{
				Shuubando[1] += ShuubandoByAtack[te.koma & ~SELF];
			}
			else {
				Shuubando[1] += ShuubandoByDefence[te.koma & ~ENEMY];
			}
		}
		if ((te.to & 0x0f) >= 6)
		{
			if (SorE == SELF)
			{
				Shuubando[0] += ShuubandoByDefence[te.koma & ~SELF];
			}
			else
			{
				Shuubando[0] += ShuubandoByAtack[te.koma & ~ENEMY];
			}
		}
		KomagumiBonus[self] += KomagumiValue[te.koma][te.to];
	}
	else {
		if ((te.to & 0x0f) <= 4)
		{
			if (SorE == SELF)
			{
				Shuubando[1] += ShuubandoByAtack[(te.koma | PROMOTED) & ~SELF];
			}
			else {
				Shuubando[1] += ShuubandoByDefence[(te.koma | PROMOTED) & ~ENEMY];
			}
		}
		if ((te.to & 0x0f) >= 6)
		{
			if (SorE == SELF)
			{
				Shuubando[0] += ShuubandoByDefence[(te.koma | PROMOTED) & ~SELF];
			}
			else
			{
				Shuubando[0] += ShuubandoByAtack[(te.koma | PROMOTED) & ~ENEMY];
			}
		}
		KomagumiBonus[self] += KomagumiValue[te.koma | PROMOTED][te.to];
	}
	Kyokumen::Move(SorE, te);
	if (te.koma == SOU || te.koma == EOU) {
		InitBonus();
	}
}

int KyokumenKomagumi::Evaluate()
{
	int Shuubando0, Shuubando1;

	if (Shuubando[0] < 0)
	{
		Shuubando0 = 0;
	}
	else if (Shuubando[0] > 16)
	{
		Shuubando0 = 16;
	}
	else
	{
		Shuubando0 = Shuubando[0];
	}
	if (Shuubando[1] < 0)
	{
		Shuubando1 = 0;
	}
	else if (Shuubando[1] > 16)
	{
		Shuubando1 = 16;
	}
	else
	{
		Shuubando1 = Shuubando[1];
	}

	int ret = ((Shuubando1 - Shuubando0) * 200);

	ret += SemegomaBonus[0] * Shuubando1 / 16;
	ret += MamorigomaBonus[0] * Shuubando0 / 16;
	ret += SemegomaBonus[1] * Shuubando0 / 16;
	ret += MamorigomaBonus[1] * Shuubando1 / 16;

	ret += KomagumiBonus[0] + KomagumiBonus[1];

	if (abs(ret) > 10000)
	{
		Print();
		printf("ret = %d\n", ret);
	}

	return ret + value;
}

int IsEnemy(int SorE, KomaInf koma)
{
	return koma != WALL && !(SorE & koma);
}

int IsSelf(int SorE, KomaInf koma)
{
	return koma != WALL && (SorE &koma);
}

int teValueComp(const void *p1, const void *p2)
{
	Te *te1 = (Te *)p1;
	Te *te2 = (Te *)p2;
	return te2->value - te1->value;
}

static const int Tbl[256] = {
	0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
	4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

int bitnum(int x)
{
	return Tbl[((unsigned char*)&x)[0]] + Tbl[((unsigned char*)&x)[1]] + Tbl[((unsigned char*)&x)[2]] + Tbl[((unsigned char*)&x)[3]];
}


void KyokumenKomagumi::EvaluateTe(int SorE, int teNum, Te *te)
{
	int i;
	int nowEval = Evaluate();
	for (i = 0; i < teNum; i++) {
		int LossS, LossE, GainS, GainE;
		LossS = LossE = GainS = GainE = 0;
		KyokumenKomagumi _new(*this);
		KomaInf EnemyKing = SorE == SELF ? EOU : SOU;
		unsigned int *_newControlE = SorE == SELF ? _new.controlE : _new.controlS;
		unsigned int *_newControlS = SorE == SELF ? _new.controlS : _new.controlE;
		unsigned int *_nowControlS = SorE == SELF ? controlS : controlE;

		KomaInf NewKoma = te[i].promote ? te[i].koma | PROMOTED : te[i].koma;
		_new.Move(SorE, te[i]);
		te[i].value = _new.Evaluate() - nowEval;
		if (SorE == ENEMY)
		{
			te[i].value = -te[i].value;
		}
		if (te[i].from != 0)
		{
			LossS -= Eval(te[i].from);
		}
		LossS += _new.Eval(te[i].to);
		int dir;
		for (dir = 0; dir < 12; dir++)
		{
			if (CanMove[dir][NewKoma])
			{
				int p = te[i].to + Direct[dir];
				if (_new.ban[p] != EnemyKing)
				{
					if (IsEnemy(SorE, _new.ban[p]))
					{
						LossE += _new.Eval(p);
					}
					else if (IsSelf(SorE, _new.ban[p]))
					{
						GainS += Eval(p) - _new.Eval(p);
					}
				}
				else
				{
					if (_newControlE[te[i].to])
					{
						if (_newControlS[te[i].to])
						{
							LossE += 1000;
						}
						else
						{
							LossE += 500;
						}
					}
					else
					{
						LossE += 1500;
					}
				}
			}
		}
		if (te[i].from > OU)
		{
			for (dir = 0; dir < 8; dir++)
			{
				if ((_nowControlS[te[i].from] & (1 << (dir + 16))) != 0)
				{
					int p = _new.search(te[i].from, Direct[dir]);
					if (_new.ban[p] != WALL)
					{
						if (IsEnemy(SorE, _new.ban[p]))
						{
							LossE += _new.Eval(p);
						}
						else if (IsSelf(SorE, _new.ban[p]))
						{
							GainS += Eval(p) - _new.Eval(p);
						}
					}
				}
			}
		}
		for (dir = 0; dir < 8; dir++)
		{
			if (CanJump[dir][NewKoma])
			{
				int p = _new.search(te[i].to, Direct[dir]);
				if (_new.ban[p] != EnemyKing)
				{
					if (IsEnemy(SorE, _new.ban[p]))
					{
						LossE += _new.Eval(p);
					}
					else if (IsSelf(SorE, _new.ban[p]))
					{
						GainS += Eval(p) - _new.Eval(p);
					}
				}
				else
				{
					if (_newControlE[te[i].to])
					{
						if (_newControlS[te[i].to])
						{
							LossE += 1000;
						}
						else
						{
							LossE += 500;
						}
					}
					else
					{
						LossE += 1500;
					}
				}
			}
		}
		te[i].value += GainS - LossS;
		if ((te[i].capture != EMPTY && te[i].capture != FU))
		{
			te[i].value += 1500;
		}
		if (te[i].from == FU && bitnum(_newControlE[0xac - te[i].to]) > 1)
		{
			te[i].value += 50;
		}
		te[i].value += LossE * 1 / 10;
	}
	qsort(te, teNum, sizeof(te[0]), teValueComp);
}

int Kyokumen::MakeChecks(int SorE, Te *teBuf, int *pin)
{
	Kyokumen kk(*this);
	unsigned int *selfControl;
	int enemyKing;

	if (SorE == SELF)
	{
		selfControl = kk.controlS;
		enemyKing = KingE;
	}
	else
	{
		selfControl = kk.controlE;
		enemyKing = KingS;
	}
	int teNum = MakeLegalMoves(SorE, teBuf, pin);
	int outeNum = 0;
	for (int i = 0; i < teNum; i++)
	{
		kk = *this;
		kk.Move(SorE, teBuf[i]);
		if (selfControl[enemyKing])
		{
			teBuf[outeNum++] = teBuf[i];
		}
	}
	return outeNum;
}


TsumeVal TsumeHash::HashTbl[TSUME_HASH_SIZE];


uint64 TsumeHash::FU_BIT_TBL[19] = 
{
	0x0000000000000000,
	0x0000000000000001,
	0x0000000000000003,
	0x0000000000000007,
	0x000000000000000f,
	0x000000000000001f,
	0x000000000000003f,
	0x000000000000007f,
	0x00000000000001ff,
	0x00000000000003ff,
	0x00000000000007ff,
	0x0000000000000fff,
	0x0000000000001fff,
	0x0000000000003fff,
	0x0000000000007fff,
	0x000000000000ffff,
	0x000000000001ffff,
	0x000000000003ffff,
	0x000000000007ffff
};

uint64 TsumeHash::KY_BIT_TBL[5] =
{
	0x0000000000000000,
	0x0000000000100000,
	0x0000000000300000,
	0x0000000000700000,
	0x0000000000f00000,
};

uint64 TsumeHash::KE_BIT_TBL[5] = 
{
	0x0000000000000000,
	0x0000000001000000,
	0x0000000003000000,
	0x0000000007000000,
	0x000000000f000000,
};

uint64 TsumeHash::GI_BIT_TBL[5] = 
{
	0x0000000000000000,
	0x0000000010000000,
	0x0000000030000000,
	0x0000000070000000,
	0x00000000f0000000,
};

uint64 TsumeHash::KI_BIT_TBL[5] = 
{
	0x0000000000000000,
	0x0000000100000000,
	0x0000000300000000,
	0x0000000700000000,
	0x0000000f00000000,
};

uint64 TsumeHash::KA_BIT_TBL[3] = 
{
  0x0000000000000000,
  0x0000001000000000,
  0x0000003000000000,
};

uint64 TsumeHash::HI_BIT_TBL[3] = 
{
  0x0000000000000000,
  0x0000004000000000,
  0x000000c000000000,
};

uint64 TsumeHash::CalcHand(int Motigoma[])
{
	return
		FU_BIT_TBL[Motigoma[FU]] |
		KY_BIT_TBL[Motigoma[KY]] |
		KE_BIT_TBL[Motigoma[KE]] |
		GI_BIT_TBL[Motigoma[GI]] |
		KI_BIT_TBL[Motigoma[KI]] |
		KA_BIT_TBL[Motigoma[KA]] |
		HI_BIT_TBL[Motigoma[HI]];
}

void TsumeHash::Add(uint64 KyokumenHashVal, uint64 HandHashVal, int Motigoma[], int mate, Te te)
{
	uint64 NowHashVal = KyokumenHashVal;
	uint64 Hand = CalcHand(Motigoma);
	int i;
	for (i = 0; i < RETRY_MAX; i++)
	{
		if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal == 0)
		{
			TsumeVal &t = HashTbl[NowHashVal&TSUME_HASH_AND];
			t.HashVal = KyokumenHashVal;
			t.Motigoma = Hand;
			t.mate = mate;
			t.NextEntry = 0;
			t.te = te;
			break;
		}
		else if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal == KyokumenHashVal)
		{
			if (HashTbl[NowHashVal&TSUME_HASH_AND].Motigoma == Hand)
			{
			}
			else
			{
				while (HashTbl[NowHashVal&TSUME_HASH_AND].NextEntry != 0)
				{
					NowHashVal = HashTbl[NowHashVal&TSUME_HASH_AND].NextEntry;
					if (HashTbl[NowHashVal&TSUME_HASH_AND].Motigoma == Hand)
					{
						break;
					}
				}
			}
			if (HashTbl[NowHashVal&TSUME_HASH_AND].Motigoma == Hand)
			{
				TsumeVal &t = HashTbl[NowHashVal&TSUME_HASH_AND];
				t.HashVal = KyokumenHashVal;
				t.Motigoma = Hand;
				t.mate = mate;
				t.te = te;
				break;
			}
			TsumeVal &pre = HashTbl[NowHashVal&TSUME_HASH_AND];
			NowHashVal = KyokumenHashVal ^ HandHashVal;
			for (int j = 0; j < RETRY_MAX; j++)
			{
				if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal == 0)
				{
					TsumeVal &t = HashTbl[NowHashVal&TSUME_HASH_AND];
					t.HashVal = KyokumenHashVal;
					t.Motigoma = Hand;
					t.mate = mate;
					t.NextEntry = 0;
					t.te = te;
					pre.NextEntry = NowHashVal & TSUME_HASH_AND;
					break;
				}
				NowHashVal += TSUME_HASH_SIZE / 11;
			}
			break;
		}
		NowHashVal += TSUME_HASH_SIZE / 11;
	}
	if (i == RETRY_MAX)
	{
		printf("OVER MAX\n");
	}
}

void TsumeHash::Clear()
{
	memset(HashTbl, 0, sizeof(HashTbl));
}

TsumeVal* TsumeHash::FindFirst(uint64 KyokumenHashVal)
{
	uint64 NowHashVal = KyokumenHashVal;
	for (int i = 0; i < RETRY_MAX; i++)
	{
		if (HashTbl[NowHashVal&TSUME_HASH_AND].HashVal == KyokumenHashVal)
		{
			return HashTbl + (NowHashVal&TSUME_HASH_AND);
		}
		NowHashVal += TSUME_HASH_SIZE / 11;
	}
	return NULL;
}

TsumeVal* TsumeHash::FindNext(TsumeVal *Now)
{
	if (Now->NextEntry == 0)
	{
		return NULL;
	}
	return HashTbl + Now->NextEntry;
}

TsumeVal *TsumeHash::Find(uint64 KyokumenHashVal, uint64 HandHashVal, int Motigoma[])
{
	uint64 CalcMotigoma = CalcHand(Motigoma);
	TsumeVal *ret = FindFirst(KyokumenHashVal);
	while (ret != NULL)
	{
		if (ret->Motigoma == CalcMotigoma)
		{
			return ret;
		}
		ret = FindNext(ret);
	}
	return NULL;
}

TsumeVal *TsumeHash::DomSearchCheckMate(uint64 KyokumenHashVal, int Motigoma[])
{
	uint64 CalcMotigoma = CalcHand(Motigoma);
	TsumeVal *ret = FindFirst(KyokumenHashVal);
	while (ret != NULL)
	{
		if (ret->Motigoma == CalcMotigoma)
		{
			if (ret->mate != 1)
			{
				return NULL;
			}
		}
		if ((ret->Motigoma & CalcMotigoma) == ret->Motigoma && ret->mate == 1)
		{
			return ret;
		}
		ret = FindNext(ret);
	}
	return NULL;
}

int Kyokumen::Mate(int SorE, int maxDepth, Te &te)
{
	Te teBuf[10000];

	TsumeVal *p;
	if ((p = TsumeHash::Find(KyokumenHashVal, HandHashVal, Hand + SorE)) != NULL)
	{
		if (p->mate == 1)
		{
			te = p->te;
		}
		return p->mate;
	}
	int ret;
	for (int i = 1; i <= maxDepth; i += 2)
	{
		if (ret = CheckMate(SorE, 0, i, teBuf, te))
		{
			break;
		}
	}
	return ret;
}

int Kyokumen::CheckMate(int SorE, int depth, int depthMax, Te *checks, Te &te)
{
	int teNum = MakeChecks(SorE, checks);
	if (teNum == 0)
	{
		TsumeHash::Add(KyokumenHashVal, HandHashVal, Hand + SorE, -1, 0);
		return -1;
	}
	TsumeVal *p;
	if ((p = TsumeHash::DomSearchCheckMate(KyokumenHashVal, Hand + SorE)) != NULL)
	{
		te = p->te;
		return 1;
	}
	int valmax = -1;
	for (int i = 0; i < teNum; i++)
	{
		Kyokumen kk(*this);
		kk.Move(SorE, checks[i]);
		int val = kk.AntiCheckMate(SorE ^ 0x30, depth + 1, depthMax, checks + teNum);
		if (val > valmax)
		{
			valmax = val;
		}
		if (valmax == 1)
		{
			te = checks[i];
			break;
		}
	}
	if (valmax == 1)
	{
		TsumeHash::Add(KyokumenHashVal, HandHashVal, Hand + SorE, 1, te);
	}
	else if (valmax == -1)
	{
		TsumeHash::Add(KyokumenHashVal, HandHashVal, Hand + SorE, -1, 0);
	}
	return valmax;
}

int Kyokumen::AntiCheckMate(int SorE, int depth, int depthMax, Te *antichecks)
{
	Te te;
	int teNum = MakeLegalMoves(SorE, antichecks);

	int i = 0;
	int valmin = 1;
	if (teNum == 0)
	{
		return 1;
	}
	if (depth >= depthMax + 1)
	{
		return 0;
	}

	for (i = 0; i < teNum; i++)
	{
		Kyokumen k(*this);
		k.Move(SorE, antichecks[i]);
		int val = k.CheckMate(SorE ^ 0x30, depth + 1, depthMax, antichecks + teNum, te);
		if (val < valmin)
		{
			valmin = val;
		}
		if (valmin == -1)
		{
			break;
		}
		if (valmin == 0)
		{
			break;
		}
	}
	return valmin;
}
