#pragma once 

#include <stdio.h>

typedef unsigned char KomaInf;


enum
{
	EMPTY = 0,
	EMP = 0,
	PROMOTED = 1 << 3,

	// 駒種
	FU = 1,
	KY = 2,
	KE = 3,
	GI = 4,
	KI = 5,
	KA = 6,
	HI = 7,
	OU = 8,
	TO = FU + PROMOTED,
	NY = KY + PROMOTED,
	NK = KE + PROMOTED,
	NG = GI + PROMOTED,
	UM = KA + PROMOTED,
	RY = HI + PROMOTED,

	SELF = 1 << 4,
	ENEMY = 1 << 5,
	WALL = SELF + ENEMY,

	// 自駒
	SFU = SELF + FU,
	STO = SELF + TO,
	SKY = SELF + KY,
	SNY = SELF + NY,
	SKE = SELF + KE,
	SNK = SELF + NK,
	SGI = SELF + GI,
	SNG = SELF + NG,
	SKI = SELF + KI,
	SKA = SELF + KA,
	SUM = SELF + UM,
	SHI = SELF + HI,
	SRY = SELF + RY,
	SOU = SELF + OU,

	// 敵駒
	EFU = ENEMY + FU,
	ETO = ENEMY + TO,
	EKY = ENEMY + KY,
	ENY = ENEMY + NY,
	EKE = ENEMY + KE,
	ENK = ENEMY + NK,
	EGI = ENEMY + GI,
	ENG = ENEMY + NG,
	EKI = ENEMY + KI,
	EKA = ENEMY + KA,
	EUM = ENEMY + UM,
	EHI = ENEMY + HI,
	ERY = ENEMY + RY,
	EOU = ENEMY + OU
};

extern const int Direct[12];
extern const int CanPromote[];
extern const int CanMove[12][64];
extern const int CanJump[12][64];
extern const char *komaStr[];
extern const char *komaStr2[];

extern const int KomaValue[];
extern const int HandValue[];

typedef unsigned int Kiki;

#ifdef _GCC_
typedef long long int64;
typedef unsigned long long uint64;
#else
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif

class Kyokumen;

class Te
{
public:
	unsigned char from, to;		// 動く駒（どこから、どこまで)
	KomaInf koma;				// 動かした駒
	KomaInf capture;			// 取った駒
	unsigned char promote;		// 成り、不成
	unsigned char Kind;

	short value;
public:

	inline bool IsNull(void)
	{
		return from == 0 && to == 0;
	}

	inline Te(void){};

	inline Te(int i) 
	{
		from = to = koma = capture = promote = Kind = 0;
		value = 0;
	}
	inline Te(int f, int t, KomaInf k, KomaInf c, int p = 0, int K = 0, int v = 0)
	{
		from	= f;
		to		= t;
		koma	= k;
		capture = c;
		promote = p;
		Kind	= K;
		value	= v;
	}

	Te(int SorE, unsigned char f, unsigned char t, const Kyokumen &k);

	void Print(void) 
	{
		FPrint(stdout);
	}

	void FPrint(FILE *fp);

	int operator==(const Te &a)
	{
		return a.from == from && a.to == to && a.koma == koma && a.promote == promote;
	}	
};

class Kyokumen
{
public:
	KomaInf banpadding[16];

	KomaInf ban[16 * 11];

	Kiki controlS[16 * 11];		// 自分の利き駒
	Kiki controlE[16 * 11];		// 敵の利き駒

	int Hand[EHI + 1];			// 持ち駒

	int Tesu;

	// 互いの王の位置
	int KingS;
	int KingE;

	int value;

protected:
	void InitControl(void);

	int Utifudume(int SorE, int to, int *pin);

	void MoveKing(int SorE, int &teNum, Te *teTop, Kiki kiki);			// 玉の動く手の生成
	void MoveTo(int SorE, int &teNum, Te *teTop, int to, int *pin);		// toに動く手の生成
	void PutTo(int SorE, int &teNum, Te *teTop, int to, int *pin);		// toに駒を打つ手の生成

	Kiki CountControlS(int pos);
	Kiki CountControlE(int pos);
	Kiki CountMove(int SorE, int pos, int *pin);

	void AddMoves(int SorE, int &teNum, Te *teTop, int from, int pin, int Rpin = 0);
	void AddStraight(int SorE, int &teNum, Te *teTop, int from, int dir, int pin, int Rpin = 0);
	void AddMove(int SorE, int &teNum, Te *teTop, int from, int diff, int pin, int Rpin = 0);

	int IsCorrectMove(Te &te);
	int EvalMin(Te *MoveS, int NumMoveS, Te *MoveE, int NumMoveE);
	int EvalMax(Te *MoveS, int NumMoveS, Te *MoveE, int NumMoveE);
	int Eval(int pos);

	static uint64 HashSeed[ERY + 1][0x99 + 1];
	static uint64 HandHashSeed[EHI + 1][18 + 1];
public:
	static void HashInit(void);

	uint64 KyokumenHashVal;
	uint64 HandHashVal;
	uint64 HashVal;

	static uint64 HashHistory[1000];
	static int OuteHistory[1000];

	inline int search(int pos, int dir) const
	{
		do
		{
			pos += dir;
		} while (ban[pos] == EMPTY);
		return pos;
	}

	Kyokumen() {};

	Kyokumen(int tesu, KomaInf ban[9][9], int Motigoma[]);

	void Print(void)
	{
		FPrint(stdout);
	}

	void FPrint(FILE *fp);

	void MakePinInf(int *pin) const;

	int MakeLegalMoves(int SorE, Te *tebus, int *pin = nullptr);
	int AntiCheck(int SorE, Te *tebuf, int *pin, Kiki control);

	void Move(int SorE, const Te &te);

	int BestEval(int SorE);

	int IsLegalMove(int SorE, Te &te);

	int operator==(const Kyokumen &k)
	{
		int ret = true;
		for (int suji = 0x10; suji <= 0x90; suji += 0x10)
		{
			for (int dan = 1; dan <= 9; dan++)
			{
				if (ban[suji + dan] != k.ban[suji + dan]) {

					ret = false;
					break;
				}
			}
		}
		return ret;
	}

	void Dump(void);

	int MakeChecks(int SorE, Te *tebuf, int *pin = nullptr);
	int Mate(int SorE, int maxDepth, Te &te);
	int CheckMate(int SorE, int depth, int depthMax, Te *checks,Te &te);
	int AntiCheckMate(int SorE, int depth, int depthMax, Te *checks);
};

class KyokumenKomagumi : public Kyokumen
{
public :
	static int KomagumiValue[ERY + 1][16 * 11];
	static int SemegomaValueS[16 * 11][16 * 11];
	static int SemegomaValueE[16 * 11][16 * 11];
	static int MamorigomaValueS[16 * 11][16 * 11];
	static int MamorigomaValueE[16 * 11][16 * 11];
public:
	int Shuubando[2];
	int KomagumiBonus[2];
	int SemegomaBonus[2];
	int MamorigomaBonus[2];

	KyokumenKomagumi(int tesu, KomaInf ban[9][9], int Motigoma[]) : Kyokumen(tesu, ban, Motigoma) 
	{
		InitShuubando();
		InitBonus();
	}

	void Initialize(void);

	static void InitKanagomaValue(void);

	void InitShuubando(void);
	void InitBonus(void);
	void SenkeiInit(void);
	void Move(int SorE, const Te &te);
	//void narabe(int tesu, const char *t[]);
	//void Print(void);

	int Evaluate(void);

	void EvaluateTe(int SorE, int teNum, Te *te);
};

class Joseki
{
	unsigned char **JosekiData;
	int JosekiSize;
	Joseki *child;
public:
	Joseki(char *filenames);

	void fromJoseki(Kyokumen &shoki, int shokiTeban, Kyokumen &k, int tesu, int &teNum, Te te[], int hindo[]);
};

#define RETRY_MAX	(9)

struct TsumeVal
{
	uint64 HashVal;
	uint64 Motigoma;
	int NextEntry;
	int mate;
	Te te;
};

#define TSUME_HASH_SIZE	(1024 * 1024)
#define TSUME_HASH_AND	(TSUME_HASH_SIZE - 1)

class TsumeHash {
	static TsumeVal HashTbl[TSUME_HASH_SIZE];
	static uint64 FU_BIT_TBL[19];
	static uint64 KY_BIT_TBL[5];
	static uint64 KE_BIT_TBL[5];
	static uint64 GI_BIT_TBL[5];
	static uint64 KI_BIT_TBL[5];
	static uint64 KA_BIT_TBL[3];
	static uint64 HI_BIT_TBL[3];
	static TsumeVal *FindFirst(uint64 KyokumenHashVal);
	static TsumeVal *FindNext(TsumeVal* Now);
	static uint64 CalcHand(int Motigoma[]);
public:
	static void Clear();
	static void Add(uint64 KyokumenHashVal, uint64 HandHashVal, int Motigoma[], int mate, Te te);
	static TsumeVal *Find(uint64 KyokumenHashVal, uint64 HandHashVal, int Motigoma[]);
	static TsumeVal *DomSearchCheckMate(uint64 KyokumenHashVal, int Motigoma[]);
	//static TsumeVal *DomSearchAntiMate(uint64 KyokumenHashVal, int Motigoma[]);
};


template<class T> void swap(T &x, T &y)
{
	T temp = x;
	x = y;
	y = temp;
}

#if defined(_MSC_VER) || defined(_GCC_)
template<class T> T min(const T &x, const T &y)
{
	return x < y ? x : y;
}

template<class T> T max(const T &x, const T &y)
{
	return x > y ? x : y;
}
#endif
