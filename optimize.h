#ifndef __OPTIMIZE_H
#define __OPTIMIZE_H

#define ndmax		100		/* 结点表最大容量 */
#define dagmax		100		/* dag图最大结点数 */
#define bblkmax		100		/* 基本块最大数量 */
#define varmax		100		/* 变量最大数量 */
struct _nodetab/* 结点表 */
{
	int name;		/* 结点名 */
	int nodenum;	/* 结点号 */
}nodetab[ndmax];

struct	_dagnode/* dag图 */
{
	fct op;				/* 操作符 */
	int left,right;		/* 左右子结点 */
	int parent[20];		/* 父结点 */
	int parentnum;		/* 父结点数 */
}dag[dagmax];

struct _basicblk	/* 基本块 */
{
	int istart,iend;	/* 语句的开始与结束号 */
	int left,right;		/* 语句的后继基本块，最多两个，left代表顺序执行时的后继基本块，right代表跳转时的后继基本块 */
	int parents[bblkmax];	/* 语句的前驱基本块 */
	int parentsnum;			/* 前驱基本块数 */
	int def[varmax],use[varmax];	/* def和use集合 */
	int defnum,usenum;		/* def集与use集的元素数目 */
	int in[varmax],out[varmax];	/* in集合与out集合 */
	int innum,outnum;	/* in集与out集元素数目 */
}bscblock[bblkmax];

struct _blkinfo		/* 基本块的信息，划分基本块时用到 */
{
	int haslab;		/* 基本块拥有的标号 */
	int gotolab;	/* 基本块转移到的标号 */
}blockinfo[bblkmax];

int bblkidx;	/* 基本块索引 */

int rewritetab[bblkmax];	/* 记录需要重写的基本块的表 */
int rewrtidx;			/* rewrite的索引 */

bool cflctgraph[varmax][varmax];	/* 冲突图 */

void dagoptimaize(order code[][cmax],int cicnt);	/* 利用DAG图消除局部公共子表达式 */

void splitbasicblk(const order code[]);

void allocateglobalreg(const order code[]);	/* 全局寄存器分配 */
#endif