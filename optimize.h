#ifndef __OPTIMIZE_H
#define __OPTIMIZE_H

#define ndmax		100		/* ����������� */
#define dagmax		100		/* dagͼ������� */
#define bblkmax		100		/* ������������� */
#define varmax		100		/* ����������� */
struct _nodetab/* ���� */
{
	int name;		/* ����� */
	int nodenum;	/* ���� */
}nodetab[ndmax];

struct	_dagnode/* dagͼ */
{
	fct op;				/* ������ */
	int left,right;		/* �����ӽ�� */
	int parent[20];		/* ����� */
	int parentnum;		/* ������� */
}dag[dagmax];

struct _basicblk	/* ������ */
{
	int istart,iend;	/* ���Ŀ�ʼ������� */
	int left,right;		/* ���ĺ�̻����飬���������left����˳��ִ��ʱ�ĺ�̻����飬right������תʱ�ĺ�̻����� */
	int parents[bblkmax];	/* ����ǰ�������� */
	int parentsnum;			/* ǰ���������� */
	int def[varmax],use[varmax];	/* def��use���� */
	int defnum,usenum;		/* def����use����Ԫ����Ŀ */
	int in[varmax],out[varmax];	/* in������out���� */
	int innum,outnum;	/* in����out��Ԫ����Ŀ */
}bscblock[bblkmax];

struct _blkinfo		/* ���������Ϣ�����ֻ�����ʱ�õ� */
{
	int haslab;		/* ������ӵ�еı�� */
	int gotolab;	/* ������ת�Ƶ��ı�� */
}blockinfo[bblkmax];

int bblkidx;	/* ���������� */

int rewritetab[bblkmax];	/* ��¼��Ҫ��д�Ļ�����ı� */
int rewrtidx;			/* rewrite������ */

bool cflctgraph[varmax][varmax];	/* ��ͻͼ */

void dagoptimaize(order code[][cmax],int cicnt);	/* ����DAGͼ�����ֲ������ӱ��ʽ */

void splitbasicblk(const order code[]);

void allocateglobalreg(const order code[]);	/* ȫ�ּĴ������� */
#endif