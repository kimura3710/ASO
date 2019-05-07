#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Kyokumen.h"


Joseki::Joseki(char *filenames)
{
	char *filename = filenames;
	char *nextfile = strchr(filenames, ',');

	if (nextfile != nullptr) 
	{
		*nextfile = '\0';
		nextfile++;
		child = new Joseki(nextfile);
	}
	else 
	{
		child = nullptr;
	}

	FILE *fp = fopen(filename, "rb");

	JosekiSize = 0;
	if (fp != nullptr) 
	{
		for (;;) 
		{
			char buf[512];
			if (fread(buf, 1, 512, fp) <= 0) break;
			JosekiSize++;
		}

		JosekiData = (unsigned char **)malloc(sizeof(unsigned char *)*JosekiSize);
		fseek(fp, 0, SEEK_SET);

		for (int j = 0; j < JosekiSize; j++) 
		{
			JosekiData[j] = (unsigned char*)malloc(512);
			if (fread(JosekiData[j], 1, 512, fp) <= 0) break;
		}

		fclose(fp);
	}
}

void Joseki::fromJoseki(Kyokumen &shoki, int shokiTeban, Kyokumen &k, int tesu, int &teNum, Te te[], int hindo[])
{
	teNum = 0;
	int j;

	for (j = 0; j < JosekiSize; j++) 
	{
		Kyokumen kk(shoki);
		int teban = shokiTeban;
		int j;

		for (j = 0; j < tesu; j++) 
		{
			if (JosekiData[j][j * 2] == 0 || JosekiData[j][j * 2] == 0xff)
			{
				break;
			}
			Te te = Te(teban, JosekiData[j][j * 2 + 1], JosekiData[j][j * 2], kk);
			kk.Move(teban, te);

			if (teban == ENEMY)
			{
				teban = SELF;
			}
			else 
			{
				teban = ENEMY;
			}
		}

		if (j == tesu && k == kk) 
		{
			Te ret = Te(teban, JosekiData[j][j * 2 + 1], JosekiData[j][j * 2], kk);

			if (JosekiData[j][j * 2] == 0 || JosekiData[j][j * 2] == 0xff)
			{
				continue;
			}
			int l;

			for (l = 0; l < teNum; l++) {

				if (ret == te[l]) 
				{
					hindo[l]++;
					break;
				}
			}
			if (l == teNum) 
			{
				te[l] = ret;
				hindo[l] = 1;
				teNum++;
			}
		}
	}
	if (child != nullptr && teNum == 0) {
		child->fromJoseki(shoki, shokiTeban, k, tesu, teNum, te, hindo);
	}
}
