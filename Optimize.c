#include "global.h"
#include "optimize.h"
#include <stdlib.h>
#include <string.h>
order optimizedcode[pmax][cmax];	/* 优化后的中间代码 */

index ndx,dagx;	/* 结点表和dag图的索引值 */

/************************************以下消除局部公共子表达式优化相关****************************************/

int locnodenum(int nodename)	/* 在结点表中查找结点名为nodename的结点，找到则返回结点号，否则返回0 */
{
	int i;
	for (i=ndx;i>0;i--)
		if (nodetab[i].name==nodename)
			return nodetab[i].nodenum;	/* 返回结点号 */
	return 0;
}

int locnodeidx(int nodename)	/* 在结点表中查找结点名为nodename的非叶结点，找到则返回其在结点表中的下标，否则返回0 */
{
	int i;
	for (i=ndx;i>0;i--)
		if (nodetab[i].name==nodename&&(dag[nodetab[i].nodenum].op!=NOP||nodename>0&&arrays==tab[nodename].typ))
			return i;
	return 0;
}

int locopnode(fct op,int i,int j)	/* 在dag图中寻找标记为op的中间结点，其左操作数结点号为i，右操作数结点号为j，如果找到，返回该结点号；否则返回0 */
{
	int k;
	for (k=dagx;k>0;k--)
		if (dag[k].op==op)
		{
			if ((dag[k].left==i&&dag[k].right==j)
				||(dag[k].left==j&&dag[k].right==i&&(ADD==op||IMUL==op)))	/* 加法和乘法是可交换的 */
				return k;
		}
	return 0;
}

int locnodename(int nodenum,int name[])	/* 在结点表中查找结点号为nodenum的结点，将其结点名用name[]返回，同时返回这样的结点个数 */
{
	int cnt,i,j;
	bool hasvar=false;	/* 标记结点号为nodenum的结点中是否有局部变量，即hasvar=false时说明全是临时变量 */
	name[0]=0;
	for (i=ndx,j=0;i>0;i--)
		if (nodetab[i].nodenum==nodenum )
		{
			if (nodetab[i].name>0)
				hasvar=true;
			name[j++]=i;
		}
	
	cnt=j;
	j=0;
	if (hasvar)	/* 有局部变量时，只保存局部变量 */
	{
		for (i=0;i<cnt;i++)
		{
			int idx=name[i];
			if (nodetab[idx].name>0)
				name[j++]=nodetab[idx].name;	
			else
				nodetab[idx].nodenum=0;		/* 清空临时变量结点号 */
		}
	}
	else	/* 无局部变量时，只保存地址值最大的临时变量 */
	{
		int max=name[0];
		for (i=1;i<cnt;i++)
		{
			if (nodetab[name[i]].name>nodetab[max].name)
			{
				nodetab[max].nodenum=0;
				max=name[i];
			}
			else
				nodetab[name[i]].nodenum=0;
		}
		j=1;
		name[0]=nodetab[max].name;
	}
	return j;
}

void newdagnode()	/* 新建dag图结点 */
{
	dagx++;
	if (dagx==dagmax)
	{
		printf("dag map overflow!\n");
		exit(1);
	}
	dag[dagx].op=NOP;
	dag[dagx].left=dag[dagx].right=-1;
	dag[dagx].parentnum=0;
}

void enternodetab(int nodename)		/* 登录结点表 */
{
	ndx++;
	if (ndx==ndmax)
	{
		printf("dag map node table overflow!\n");
		exit(1);
	}
	nodetab[ndx].name=nodename;
	nodetab[ndx].nodenum=0;		/* 结点号暂时设为0 */
}

void insertparent(struct _dagnode *node,int p)	/* 把p加入到dag图中的结点node的parent中去 */
{
	int i;
	for (i=0;i<node->parentnum;i++)
		if (node->parent[i]==p)	/* 已经在父结点中，无需插入 */
			return;

	node->parent[node->parentnum++]=p;
}

void delparent(struct _dagnode *node,int p)	/* 把p从dag图中的结点node的parent中删掉 */
{
	int i;
	for (i=0;i<node->parentnum;i++)
		if (node->parent[i]==p)
		{
			node->parent[i]=node->parent[node->parentnum-1]; /* 用最后一个parent的元素覆盖 */
			node->parentnum--;	/* 相当于删除最后一个 */
		}
}

void insert2dag(fct op,int src1,int src2,int dst)	/* 将结点插入dag图 */
{
	int i=0,j=0,k=0;
	int dstidx;

	i=locnodenum(src1);	/* 在结点表中找左操作数的结点号 */
	if (0==i)			/* 未找到 */
	{
		newdagnode();	/* 在dag图中新建结点 */		
		enternodetab(src1);	/* 登录结点表 */
		nodetab[ndx].nodenum=dagx;
		i=dagx;
	}
	if (MOV!=op&&NEG!=op)	/* MOV和NEG指令没有右操作数 */
	{
		j=locnodenum(src2);	/* 在结点表中找右操作数结点号 */
		if (0==j)			/* 未找到 */
		{
			newdagnode();	/* 在dag图中新建结点 */		
			enternodetab(src2);	/* 登录结点表 */
			nodetab[ndx].nodenum=dagx;
			j=dagx;
		}
	}
	else 
		j=-1;

	if (STOARRAY==op)	/* 针对操作符[]=的特殊处理 */
	{
		newdagnode();	/* 在dag图中新建一项 */
		dag[dagx].op=STOARRAY;;
		dag[dagx].left=i;
		dag[dagx].right=j;
		enternodetab(dst);	/* 在结点表中新建一项 */
		nodetab[ndx].nodenum=dagx;
	}
	dstidx=locnodeidx(dst);	/* dst在结点表中的位置 */
	if (0==dstidx)	/* 在结点表中没有dst，要新建 */
	{
		enternodetab(dst);	/* 在结点表中新建一项 */
		dstidx=ndx;
	}

	if (MOV==op)	/* MOV指令，MOV指令总要更新结点表或在结点表中新建项 */
	{
		if (dag[i].op!=NOP)		/* 对于源操作数是非叶结点 */
		{
			nodetab[dstidx].nodenum=i;
		}
		else	/* 对于源操作数是叶结点 */
		{
			newdagnode();	/* 在dag图中新建结点 */
			nodetab[dstidx].nodenum=dagx;
			dag[dagx].op=MOV;
			dag[dagx].left=i;
		}
	}
	else
	{
		k=locopnode(op,i,j);	/* 获取标记为op的中间结点的结点号 */
		if (0==k)	/* 没有这样的结点 */
		{
			newdagnode();	/* 在dag图中新建一个中间结点 */
			k=dagx;
			dag[k].op=op;
			dag[k].left=i;
			dag[k].right=j;
		}
		insertparent(&dag[i],k);
		insertparent(&dag[j],k);
		nodetab[dstidx].nodenum=k;
	}
}

void dag2code(order code[],int *cdx)		/* 从dag图重新导出中间代码 */
{
	int stack[dagmax];	/* 初始化一个放置dag图中间结点的栈 */
	int top=-1,i;
	struct _dagnode tmpdag[dagmax];	/* 用于保存dag图的临时dag图 */
	for (i=1;i<=dagx;i++)
		tmpdag[i]=dag[i];
	while(1)
	{
		for (i=dagx;i>0;i--)
		{
			if (0==tmpdag[i].parentnum&&tmpdag[i].op!=NOP)	/* 选取没有父结点的中间结点（父结点全部进栈的情况也在后面代码中被归到这种情况） */
				break;
		}
		if (0==i) break;	/* 没找到没有父结点或所有父结点全部进栈的中间结点 */

		do
		{
			int left=tmpdag[i].left,right=tmpdag[i].right;	/* 左右子结点 */
			if (0==tmpdag[i].parentnum&&tmpdag[i].op!=NOP)
				stack[++top]=i;	/* 满足条件的结点进栈 */
			else 
				break;
			if (left>0)
				delparent(&tmpdag[left],i);	/* 把i从其左子结点的父结点中除去 */
			if (right>0)
				delparent(&tmpdag[right],i);	/* 把i从其右子结点的父结点中除去 */
			tmpdag[i].op=NOP;	/* 设为nop下次就不会再选中它 */
			i=left;	/* 沿着当前结点的最左边，循环访问其最左子结点 */
		} while(i>0&&tmpdag[i].op!=NOP);
	}


	while (top>=0)
	{
		int src1[ndmax],src2[ndmax],dst[ndmax];
		int cntdst=locnodename(stack[top],dst);
		fct op=dag[stack[top]].op;
		if (dag[stack[top]].left>0)
			locnodename(dag[stack[top]].left,src1);
		else
			src1[0]=0;
		if (dag[stack[top]].right>0)
			locnodename(dag[stack[top]].right,src2);
		else
			src2[0]=0;
		
		code[*cdx].op=op;
		code[*cdx].src1=src1[0];
		code[*cdx].src2=src2[0];
		code[*cdx].dst=dst[0];
		(*cdx)++;
		for (i=1;i<cntdst;i++)	/* 多个目的操作数对应同一结点号，采用赋值方式使它们都为同一值 */
		{
			code[*cdx].op=MOV;
			code[*cdx].src1=dst[0];
			code[*cdx].src2=0;
			code[*cdx].dst=dst[i];
			(*cdx)++;
		}
		top--;
	}
}

/*
关于基本块的划分：根据三条规则，可以发现，基本块是被这几种指令分隔开的：ENTER(规则1)、SETLAB(规则2)、J*系列指令(规则3)
所以只要顺序扫描中间代码，遇到其中一个就算一个基本块开始，再遇到其中一个就算基本块结束，无需专门把代码再存成基本块
要注意的地方：1.数组变量作为单独变量考虑，当有STOARRY时，数组变量值改变，体现在节点表中，后面再有LOADARRAY引用的就不再是原来的值
			  2.为解决函数调用前后无法得知改变了哪些数据的问题，采用保守做法，当出现函数调用(CALL+RET)时，也使其作为基本块的划分
			  3.INC、DEC、PUSH、READ、WRITE、LEAVE都不参与到优化中，原样写回即可
综上，实际上参与到DAG优化中的指令只有NEG、MOV、ADD、SUB、IMUL、IDIV、STOARRAY(书上说的[]=)、LOADARRAY(书上说的[])
*/
void dagoptimaize(order code[][cmax],int cicnt)	/* 利用DAG图消除局部公共子表达式 */
{
	int i,j,k;
	for (i=1;i<=cicnt;i++)	/* 针对每个过程的循环 */
	{
		j=0;k=0;
		while (NOP!=code[i][j].op)
		{
			fct op=code[i][j].op;/* 指令 */
			int src1=code[i][j].src1,src2=code[i][j].src2,dst=code[i][j].dst;	/* 操作数 */
			if (NEG==op||MOV==op||ADD==op||SUB==op||IMUL==op||IDIV==op/*||STOARRAY==op||LOADARRAY==op*/)
			{
				ndx=0;
				dagx=0;		/* 新的基本块把结点表和dag图索引初始化为0 */
				do 
				{
					if (MOV==op&&code[i][j].src1>0&&code[i][j].dst>0)
					{
						if (tab[code[i][j].src1].obj!=konstant) break;
					}
					insert2dag(op,src1,src2,dst);	/* 在dag图中加入新的结点 */				
					j++;
					op=code[i][j].op;
					src1=code[i][j].src1;
					src2=code[i][j].src2;
					dst=code[i][j].dst;	/* 操作数 */
				} while (NEG==op||MOV==op||ADD==op||SUB==op||IMUL==op||IDIV==op/*||STOARRAY==op||LOADARRAY==op*/);
				dag2code(optimizedcode[i],&k);
			}
			optimizedcode[i][k++]=code[i][j++];	/* 其余指令原样写入 */
		}
		optimizedcode[i][k].op=NOP;
	}
}

/************************************以上消除局部公共子表达式优化相关****************************************/

/***************************************以下活跃变量数据流分析相关*******************************************/
int searchblkhaslabx(int x)	/* 查找具有标号x的基本块，找到返回基本块号，否则返回-1 */
{
	int i;
	for (i=1;i<=bblkidx;i++)
		if (blockinfo[i].haslab==x)		/* 基本块i有标号x */
			return i;
	return -1;
}

void insertparents(int blk,int p)	/* 把块号p加入到块号是blk的基本块的parents中去 */
{
	int i;
	for (i=0;i<bscblock[blk].parentsnum;i++)
		if (bscblock[blk].parents[i]==p)	/* 已经在前驱基本块中，无需插入 */
			return;
		
	bscblock[blk].parents[bscblock[blk].parentsnum++]=p;	/* 插入parents的最后 */
}

void splitbasicblk(const order code[])	/* 划分基本块，code是中间代码序列 */
{
	int i=1;	/* i=0的语句是入口语句enter，略过 */
	rewrtidx=0;
	while (code[i].op!=LEAVE)	/* 遍历中间代码序列 */
	{
		++bblkidx;	/* 基本块索引值加1 */
		bscblock[bblkidx].istart=i;	/* 起始语句位置 */
		if (SETLAB==code[i].op)	/* 本块第一条语句是设置标号 */
		{
			blockinfo[bblkidx].haslab=code[i].dst;	/* 记录本块拥有标号的信息 */
			i++;	/* 指向下一条语句 */
		}
		else 
			blockinfo[bblkidx].haslab=-1;
		blockinfo[bblkidx].gotolab=-1;
		while(code[i].op!=LEAVE)
		{
			if (JMP==code[i].op||JE==code[i].op||JNE==code[i].op||
				JL==code[i].op||JLE==code[i].op||JG==code[i].op||
				JGE==code[i].op)		/* 遇到跳转语句，下一条语句应该是入口语句，故该基本块结束 */
			{
				int gotoright;
				blockinfo[bblkidx].gotolab=code[i].dst;
				gotoright=searchblkhaslabx(code[i].dst);	/* 查找有标号dst的基本块 */
				if (gotoright>0)	/* 找到标号所在基本块 */
				{
					bscblock[bblkidx].right=gotoright;	/* 当前基本块跳转到的后继基本块设为该标号所在基本块 */
					insertparents(gotoright,bblkidx);	/* 把当前基本块号加入到后继基本块的前驱基本块集合中 */
				}
				else	/* 未找到 */
					rewritetab[++rewrtidx]=bblkidx;	/* 记下该基本块需要被重写 */
				if (code[i].op!=JMP)
				{
					bscblock[bblkidx].left=bblkidx+1;	/* 除了无条件跳转语句，其他语句的直接后继都应该是下一个基本块 */
					insertparents(bblkidx+1,bblkidx);	/* 把当前基本块号加入到后继基本块的前驱基本块集合中 */
				}
				i++;
				break;	/* 这个基本块结束，退出循环进入下一个 */
			}
			else if (SETLAB==code[i].op)	/* 标号应该是入口语句，跳出循环，i不加 */
			{
				bscblock[bblkidx].left=bblkidx+1;	/* 除了无条件跳转语句，其他语句的直接后继都应该是下一个基本块 */
				insertparents(bblkidx+1,bblkidx);	/* 把当前基本块号加入到后继基本块的前驱基本块集合中 */
				break;
			}
			else
				i++;
		}
		bscblock[bblkidx].iend=i-1;	/* 循环结束时，i指向下一条语句，故本基本块的结束语句位置应该是i-1 */
	}
	for (i=1;i<=rewrtidx;i++)	/* 重写基本块 */
	{
		int gotoright;

		gotoright=searchblkhaslabx(blockinfo[rewritetab[i]].gotolab);	/* 查找有标号gotolab的基本块，返回值一定大于0 */

		bscblock[rewritetab[i]].right=gotoright;
		insertparents(gotoright,rewritetab[i]);
	}
}

void add2use(int var,int blk)	/* 把变量var加入基本块blk的use集合中 */
{
	int i;
	for (i=0;i<bscblock[blk].usenum;i++)
		if (bscblock[blk].use[i]==var)	/* 已在use集合中，无需加入 */
			return;
	bscblock[blk].use[i]=var;
	bscblock[blk].usenum++;
}

void add2def(int var,int blk)	/* 把变量var加入基本块blk的def集合中 */
{
	int i;
	for (i=0;i<bscblock[blk].defnum;i++)
		if (bscblock[blk].def[i]==var)
			return;
	bscblock[blk].def[i]=var;
	bscblock[blk].defnum++;

}

void calcu_use_def(const order code[])	/* 计算use集和def集 */
{
	int i,j;
	for (i=1;i<=bblkidx;i++)	/* 遍历基本块 */
	{
		bool bdef[tmax]={0},buse[tmax]={0};
		for (j=bscblock[i].istart;j<=bscblock[i].iend;j++)	/* 遍历基本块中的语句 */
		{
			fct op=code[j].op;
			int src1=code[j].src1,src2=code[j].src2,dst=code[j].dst;
			switch (op)
			{
			case READ:	/* 读语句定义了变量 */
				if (!buse[dst]&&!bdef[dst])	/* 变量没有被使用也没有被定义，则把它加入def集 */
				{
					add2def(dst,i);
					bdef[dst]=true;
				}
				break;
			case WRITE:	/* 写语句使用了变量 */
				if (src1!=1&&dst>0&&tab[dst].obj==variable&&!bdef[dst]&&!buse[dst])	/* src1=1时只输出字符串 */
				{
					add2use(dst,i);
					buse[dst]=true;
				}
				break;
			case NEG:	/* dst=-src1 */
			case MOV:	/* dst=src1 */
				if (dst==src1)		/* dst=dst或dst=-dst，认为是被使用 */
				{
					if (dst>0&&tab[dst].obj==variable&&!bdef[dst]&&!buse[dst])
					{
						add2use(dst,i);
						buse[dst]=true;
					}
				}
				else	/* dst!=src1，认为src1被使用，dst被定义 */
				{
					if (src1>0&&tab[src1].obj==variable&&!bdef[src1]&&!buse[src1])	/* src1还没有加入def或use集中，则把它加入use集中 */
					{
						add2use(src1,i);	/* 加入use集 */
						buse[src1]=true;	/* src1标记为已加入use集 */
					}
					if (dst>0&&tab[dst].obj==variable&&!bdef[dst]&&!buse[dst])
					{
						add2def(dst,i);		/* 加入def集 */
						bdef[dst]=true;		/* dst标记为已加入def集 */
					}
				}
				break;
			case INC:	/* dst=dst-1 */
			case DEC:	/* dst=dst+1 */
				if (dst>0&&tab[dst].obj==variable&&!buse[dst]&&!bdef[dst])
				{
					add2use(dst,i);		/* 认为dst被使用 */
					buse[dst]=true;		/* dst标记为已加入use集 */
				}
				break;
			case ADD:	/* dst=src1+src2 */
			case SUB:	/* dst=src1-src2 */
			case IMUL:	/* dst=src1*src2 */
			case IDIV:	/* dst=src1/src2 */
				if (dst>0&&tab[dst].obj==variable&&dst==src1||dst==src2)	/* dst=dst+src? 此时认为dst被使用 */
				{
					if (!bdef[dst]&&!buse[dst])
					{
						add2use(dst,i);
						buse[dst]=true;
					}
				}
				else if (dst>0&&tab[dst].obj==variable)		/* dst=src1+src2，src1和src2都不同于dst，此时认为dst被定义 */
				{
					if (!bdef[dst]&&!buse[dst])
					{
						add2def(dst,i);
						bdef[dst]=true;
					}
				}
				if (src1>0&&tab[src1].obj==variable&&!buse[src1]&&!bdef[src1])
				{
					add2use(src1,i);
					buse[src1]=true;
				}
				if (src2>0&&tab[src2].obj==variable&&!buse[src2]&&!bdef[src2])
				{
					add2use(src2,i);
					buse[src2]=true;
				}
				break;
			case LOADARRAY:	/* 载入数组元素 dst=src1[src2] */
				if (dst>0&&tab[dst].obj==variable&&!buse[dst]&&!bdef[dst])
				{
					add2def(dst,i);		/* 认为dst被定义 */
					bdef[dst]=true;		/* dst标记为已加入def集 */
				}
				if (src2>0&&tab[src2].obj==variable&&!buse[src2]&&!bdef[src2])
				{
					add2use(src2,i);	/* 下标src2被使用 */
					buse[src2]=true;
				}
				break;
			case STOARRAY:	/* 数组赋值 dst[src1]=src2 */
				if (src2>0&&tab[src2].obj==variable&&!buse[src2]&&!bdef[src2])
				{
					add2use(src2,i);	/* src2被使用 */
					buse[src2]=true;
				}
				if (src1>0&&tab[src1].obj==variable&&!buse[src1]&&!bdef[src1])
				{
					add2use(src1,i);	/* 下标src1被使用 */
					buse[src1]=true;
				}
				break;
			case PUSH:	/* 变量入栈，用于参数传递 */
				if (dst>0&&tab[dst].obj==variable&&!buse[dst]&&!bdef[dst])
				{
					add2use(dst,i);		/* 认为dst被使用 */
					buse[dst]=true;		/* dst标记为已加入use集 */
				}
				break;
			case JE:
			case JNE:
			case JL:
			case JLE:
			case JG:
			case JGE:	/* 各种比较指令都是使用了变量 */
				if (src1>0&&tab[src1].obj==variable&&tab[src1].obj==variable&&!buse[src1]&&!bdef[src1])
				{
					add2use(src1,i);
					buse[src1]=true;
				}
				if (src2>0&&tab[src2].obj==variable&&tab[src2].obj==variable&&!buse[src2]&&!bdef[src2])
				{
					add2use(src2,i);
					buse[src2]=true;
				}
				break;
			}
		}
	}
}


void allocateglobalreg(const order code[])	/* 全局寄存器分配 */
{
	int i,j,k;
	bool flag=false;
	int tmpcflctgraph[varmax][varmax]={0};
	int varinblks[varmax]={0};	/* 变量在几个基本块中，用于变量是否跨越基本块仍然活跃 */
	int allocvarnum=0;	/* 分配全局寄存器的变量数目 */
	bool allocvar[varmax]={0};	/* allocvar[i]=x表示要分配寄存器的第i个变量是符号表位置x的变量 */
	int stack[varmax]={0},top=-1;
	bblkidx=0;
	rewrtidx=0;
	memset(bscblock,0,sizeof(bscblock));
	memset(nodetab,0,sizeof(nodetab));
	memset(dag,0,sizeof(dag));
	memset(blockinfo,0,sizeof(blockinfo));
	memset(rewritetab,0,sizeof(rewritetab));
	for (i=0;i<varmax;i++)
	{
		for (j=0;j<varmax;j++)
			cflctgraph[i][j]=false;
	}

	splitbasicblk(code);	/* 划分基本块 */
	calcu_use_def(code);	/* 求基本块的use和def集 */
	do 
	{
		flag=false;		/* in集合是否有改变的标志，只要有一个基本块的in集合改变了，flag就为true */
		for (i=bblkidx;i>=1;i--)
		{
			if (bscblock[i].left)	/* 第一个后继基本块 */
			{
				for (j=0;j<bscblock[bscblock[i].left].innum;j++)	/* 遍历后继基本块的in集合，把没在out集合里的加入 */
				{
					for (k=0;k<bscblock[i].outnum;k++)	/* 遍历out集合 */
					{
						if (bscblock[i].out[k]==bscblock[bscblock[i].left].in[j])	/* 已经在out集合中 */
							break;
					}
					if (k==bscblock[i].outnum)	/* 说明不在out集合中，要加入其中 */
					{
						bscblock[i].out[bscblock[i].outnum++]=bscblock[bscblock[i].left].in[j];
					}
				}
			}
			if (bscblock[i].right)	/* 第二个后继基本块 */
			{
				for (j=0;j<bscblock[bscblock[i].right].innum;j++)	/* 遍历后继基本块的in集合，把没在out集合里的加入 */
				{
					for (k=0;k<bscblock[i].outnum;k++)	/* 遍历out集合 */
					{
						if (bscblock[i].out[k]==bscblock[bscblock[i].right].in[j])	/* 已经在out集合中 */
							break;
					}
					if (k==bscblock[i].outnum)	/* 说明不在out集合中，要加入其中 */
					{
						bscblock[i].out[bscblock[i].outnum++]=bscblock[bscblock[i].right].in[j];
					}
				}
			}
			
			if (bscblock[i].innum==0&&bscblock[i].usenum!=0)	/* in集合为空而use集合不为空 */
			{
				for (j=0;j<bscblock[i].usenum;j++)
				{
					bscblock[i].in[bscblock[i].innum++]=bscblock[i].use[j];
				}
				flag=true;
			}
			for (j=0;j<bscblock[i].outnum;j++)	/* 遍历out集合 */
			{
				for (k=0;k<bscblock[i].defnum;k++)	/* 看在不在def集合中 */
				{
					if (bscblock[i].out[j]==bscblock[i].def[k])	/* 在def集合中就跳出，因此在def集合时，循环结束后k一定小于defnum */
						break;
				}
				if (k==bscblock[i].defnum)	/* 不在def集合中，就加入in集合，但要先判断在不在in集合中 */
				{
					for (k=0;k<bscblock[i].innum;k++)
					{
						if (bscblock[i].out[j]==bscblock[i].in[k])	/* 已经在in集合中 */
							break;
					}
					if (k==bscblock[i].innum)	/* 不在in集合中，加入in集合 */
					{
						bscblock[i].in[bscblock[i].innum++]=bscblock[i].out[j];
						flag=true;	/* in集合有改变的标志 */
					}
				}
			}
		}
	} while (true==flag);	/* 只要还有基本块的in集合改变了就重复这一过程 */


	for (i=1;i<=bblkidx;i++)	/* 遍历基本块，找出跨越基本块仍然活跃的变量 */
	{
		for (j=0;j<bscblock[i].innum;j++)	/* 遍历基本块的in集合，找出在基本块i中活跃的变量 */
		{
			varinblks[bscblock[i].in[j]]++;	/* 将变量所处的基本块数+1 */
		}
	}
	for (i=1;i<=t;i++)
	{
		if (varinblks[i]>1)	/* 变量在大于一个基本块中活跃 */
		{
			allocvar[allocvarnum]=i;
			allocvarnum++;	/* 要分配全局寄存器的变量数目加1 */
		}
		
	}

	for (i=1;i<=bblkidx;i++)	/* 遍历基本块，构建冲突图 */
	{
		for (j=0;j<bscblock[i].defnum;j++)	/* 遍历基本块的def集合，def集合中的各个变量之间存在一条边 */
		{
			for (k=0;k<bscblock[i].defnum;k++)
			{
				int varj=bscblock[i].def[j],vark=bscblock[i].def[k];
				int defj=varinblks[varj],defk=varinblks[vark];
				if (j!=k&&defj>1&&defk>1)	/* 两个变量都是跨基本块活跃的 */
				{
					cflctgraph[varj][vark]=true;	/* 两个变量之间存在一条边 */
					cflctgraph[vark][varj]=true;

					tmpcflctgraph[varj][vark]=true;	/* 两个变量之间存在一条边 */
					tmpcflctgraph[vark][varj]=true;
				}
			}
		}

		for (j=0;j<bscblock[i].defnum;j++)	/* 遍历基本块的def集合，def集合中的各个变量与in集合的各个变量之间存在一条边 */
		{
			for (k=0;k<bscblock[i].innum;k++)
			{
				int varj=bscblock[i].def[j],vark=bscblock[i].in[k];
				int defj=varinblks[varj],ink=varinblks[vark];
				if (varj!=vark&&defj>1&&ink>1)	/* 两个变量都是跨基本块活跃的 */
				{
					cflctgraph[varj][vark]=true;	/* 两个变量之间存在一条边 */
					cflctgraph[vark][varj]=true;

					tmpcflctgraph[varj][vark]=true;	/* 两个变量之间存在一条边 */
					tmpcflctgraph[vark][varj]=true;
				}
			}
		}
	}
	if (allocvarnum<=3)	/* 要分配全局寄存器的变量数目小于等于寄存器数目，直接分配就可以 */
	{
		for (i=0;i<allocvarnum;i++)
		{
			regalloc[allocvar[i]]=glbregnum+i;
		}
	}
	else 
	{
		do /*  */
		{
			int noremov=true;	/* 没有移走结点的标记 */
			int maxcnt=0;
			int cnt=0;
			int imaxcnt;	/* 最大cnt对应的变量 */
			do /* 重复移走边小于glbregnum的结点 */
			{
				for (i=1;i<=t;i++)
				{
					cnt=0;
					for (j=1;j<=t;j++)
					{
						if (true==tmpcflctgraph[i][j])
						{
							cnt++;
						}
					}
					if (cnt>0&&cnt<glbregnum)	/* 结点的连接边数目小于寄存器 */
					{
						stack[++top]=i;	/* 结点进栈 */
						for (k=0;k<allocvarnum;k++)
						{
							if (i==allocvar[k])
							{
								int tmp=allocvar[k];	/* 和最后一个调换 */
								allocvar[k]=allocvar[allocvarnum-1];
								allocvar[allocvarnum-1]=tmp;
							}
						}
						allocvarnum--;	/* 要分配寄存器的变量数目-1 */
						noremov=false;	/* 没有结点被移走的标记设为假 */
						for (j=1;j<=t;j++)
						{
							tmpcflctgraph[i][j]=false;
							tmpcflctgraph[j][i]=false;		/* 移走结点的同时移走边 */
						}
					}
				}
			} while (false==noremov&&allocvarnum>1);
			
			if (allocvarnum>1)	/* 要分配全局寄存器的变量数大于1 */
			{
				cnt=maxcnt=0;
				for (i=1;i<=t;i++)		/* 找到连接边数目最大的一个结点从图中移走 */
				{
					cnt=0;
					for (j=1;j<=t;j++)
						if (true==cflctgraph[i][j])
							cnt++;
						if (cnt>maxcnt)	/* 替换最大的一个 */
						{
							maxcnt=cnt;
							imaxcnt=i;
						}
				}
				i=imaxcnt;
				regalloc[i]=-1;	/* 将该变量标记为“不分配全局寄存器”的变量 */
				for (k=0;k<allocvarnum;k++)
				{
					if (i==allocvar[k])
					{
						int tmp=allocvar[k];	/* 和最后一个调换 */
						allocvar[k]=allocvar[allocvarnum-1];
						allocvar[allocvarnum-1]=tmp;
					}
				}
				allocvarnum--;	/* 要分配寄存器的变量数目-1 */
				for (j=1;j<=t;j++)
				{
					tmpcflctgraph[i][j]=false;
					tmpcflctgraph[j][i]=false;		/* 移走结点的同时移走边 */
				}
			}
			
		} while (allocvarnum>1);		/* 最后应该只剩一个变量allocvar[0] */
		
		regalloc[allocvar[0]]=EBX;	/* 为最后一个变量分配ebx */
		while (top>=0)	/* 栈中还有变量 */
		{
			bool canalloc[4]={true,true,true};	/* 一开始都可以分配 */
			for (j=1;j<=t;j++)
			{
				if (true==cflctgraph[stack[top]][j]&&regalloc[j]>0)
				{
					canalloc[regalloc[j]-EBX]=false;	/* 寄存器regalloc[j]不能分配 */
				}
			}
			for (i=EBX;i<=EDI;i++)
			{
				if (canalloc[i]==true)
				{
					regalloc[stack[top]]=i;
					break;
				}
			}
			top--;
		}
	}

	i=0;
}
/***************************************以上活跃变量数据流分析相关*******************************************/

/***************************************以下窥孔优化相关*******************************************/

void peepholeoptimize(order code[][cmax],int cicnt)	/* 消除无用表达式 */
{
	int i,j,k,l;
	for (l=0;l<2;l++)
	for (i=1;i<=cicnt;i++)	/* 针对每个过程的循环 */
	{
		for (j=0,k=0;NOP!=code[i][j].op;j++)
		{
			fct op=code[i][j].op;
			int src1=code[i][j].src1,src2=code[i][j].src2,dst=code[i][j].dst;
			fct nxtop=code[i][j+1].op;
			int nxtsrc1=code[i][j+1].src1,nxtsrc2=code[i][j+1].src2,nxtdst=code[i][j+1].dst;
			if (MOV==op&&src1==dst)	/* 消除无用mov */
			{
				
			}
			else if (JMP==op&&SETLAB==nxtop&&dst==nxtdst)	/* 消除无用jmp */
			{
				j++;	/* 跳过SETLAB */
			}
			else if (MOV==nxtop&&dst==nxtsrc1&&dst<0&&			/* 下一条赋值语句的源操作数是上一条语句的目的操作数 */
				(INC==op||MOV==op||DEC==op||NEG==op||ADD==op||MINU==op||MULT==op||DIV==op||LOADARRAY==op))
			{
				code[i][k].op=op;
				code[i][k].src1=src1;
				code[i][k].src2=src2;
				code[i][k].dst=nxtdst;
				k++;
				j++;
			}
			else if (ADD==op&&((src1>0&&tab[src1].obj==konstant&&tab[src1].adr==0)
				||(src2>0&&tab[src2].obj==konstant&&tab[src2].adr==0)))	/* x=x+0 */
			{

			}
			else if (IMUL==op&&((src1>0&&tab[src1].obj==konstant&&tab[src1].adr==0)
				||(src2>0&&tab[src2].obj==konstant&&tab[src2].adr==0)))	/* x=x*0 */
			{
				code[i][k].op=MOV;
				code[i][k].dst=dst;
				code[i][k].src1=tab[src1].adr==0?src1:src2;
				code[i][k].src2=0;
				k++;
			}
			else if ((JE==op||JGE==op||JLE==op)&&src1>0&&src1==src2)	/* if x=x 和 if x>=x 和 if x<=x语句 */
			{
				code[i][k].op=JMP;
				code[i][k].dst=dst;
				k++;
				while(!(code[i][j].op==SETLAB&&code[i][j].dst==dst))
					j++;
				code[i][k]=code[i][j];
				k++;
			}
			else
				code[i][k++]=code[i][j];	/* 其他语句原样写入 */
		}
		code[i][k].op=NOP;
	}
}

/***************************************以上窥孔优化相关*******************************************/

