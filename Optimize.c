#include "global.h"
#include "optimize.h"
#include <stdlib.h>
#include <string.h>
order optimizedcode[pmax][cmax];	/* �Ż�����м���� */

index ndx,dagx;	/* �����dagͼ������ֵ */

/************************************���������ֲ������ӱ��ʽ�Ż����****************************************/

int locnodenum(int nodename)	/* �ڽ����в��ҽ����Ϊnodename�Ľ�㣬�ҵ��򷵻ؽ��ţ����򷵻�0 */
{
	int i;
	for (i=ndx;i>0;i--)
		if (nodetab[i].name==nodename)
			return nodetab[i].nodenum;	/* ���ؽ��� */
	return 0;
}

int locnodeidx(int nodename)	/* �ڽ����в��ҽ����Ϊnodename�ķ�Ҷ��㣬�ҵ��򷵻����ڽ����е��±꣬���򷵻�0 */
{
	int i;
	for (i=ndx;i>0;i--)
		if (nodetab[i].name==nodename&&(dag[nodetab[i].nodenum].op!=NOP||nodename>0&&arrays==tab[nodename].typ))
			return i;
	return 0;
}

int locopnode(fct op,int i,int j)	/* ��dagͼ��Ѱ�ұ��Ϊop���м��㣬�������������Ϊi���Ҳ���������Ϊj������ҵ������ظý��ţ����򷵻�0 */
{
	int k;
	for (k=dagx;k>0;k--)
		if (dag[k].op==op)
		{
			if ((dag[k].left==i&&dag[k].right==j)
				||(dag[k].left==j&&dag[k].right==i&&(ADD==op||IMUL==op)))	/* �ӷ��ͳ˷��ǿɽ����� */
				return k;
		}
	return 0;
}

int locnodename(int nodenum,int name[])	/* �ڽ����в��ҽ���Ϊnodenum�Ľ�㣬����������name[]���أ�ͬʱ���������Ľ����� */
{
	int cnt,i,j;
	bool hasvar=false;	/* ��ǽ���Ϊnodenum�Ľ�����Ƿ��оֲ���������hasvar=falseʱ˵��ȫ����ʱ���� */
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
	if (hasvar)	/* �оֲ�����ʱ��ֻ����ֲ����� */
	{
		for (i=0;i<cnt;i++)
		{
			int idx=name[i];
			if (nodetab[idx].name>0)
				name[j++]=nodetab[idx].name;	
			else
				nodetab[idx].nodenum=0;		/* �����ʱ�������� */
		}
	}
	else	/* �޾ֲ�����ʱ��ֻ�����ֵַ������ʱ���� */
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

void newdagnode()	/* �½�dagͼ��� */
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

void enternodetab(int nodename)		/* ��¼���� */
{
	ndx++;
	if (ndx==ndmax)
	{
		printf("dag map node table overflow!\n");
		exit(1);
	}
	nodetab[ndx].name=nodename;
	nodetab[ndx].nodenum=0;		/* ������ʱ��Ϊ0 */
}

void insertparent(struct _dagnode *node,int p)	/* ��p���뵽dagͼ�еĽ��node��parent��ȥ */
{
	int i;
	for (i=0;i<node->parentnum;i++)
		if (node->parent[i]==p)	/* �Ѿ��ڸ�����У�������� */
			return;

	node->parent[node->parentnum++]=p;
}

void delparent(struct _dagnode *node,int p)	/* ��p��dagͼ�еĽ��node��parent��ɾ�� */
{
	int i;
	for (i=0;i<node->parentnum;i++)
		if (node->parent[i]==p)
		{
			node->parent[i]=node->parent[node->parentnum-1]; /* �����һ��parent��Ԫ�ظ��� */
			node->parentnum--;	/* �൱��ɾ�����һ�� */
		}
}

void insert2dag(fct op,int src1,int src2,int dst)	/* ��������dagͼ */
{
	int i=0,j=0,k=0;
	int dstidx;

	i=locnodenum(src1);	/* �ڽ���������������Ľ��� */
	if (0==i)			/* δ�ҵ� */
	{
		newdagnode();	/* ��dagͼ���½���� */		
		enternodetab(src1);	/* ��¼���� */
		nodetab[ndx].nodenum=dagx;
		i=dagx;
	}
	if (MOV!=op&&NEG!=op)	/* MOV��NEGָ��û���Ҳ����� */
	{
		j=locnodenum(src2);	/* �ڽ��������Ҳ��������� */
		if (0==j)			/* δ�ҵ� */
		{
			newdagnode();	/* ��dagͼ���½���� */		
			enternodetab(src2);	/* ��¼���� */
			nodetab[ndx].nodenum=dagx;
			j=dagx;
		}
	}
	else 
		j=-1;

	if (STOARRAY==op)	/* ��Բ�����[]=�����⴦�� */
	{
		newdagnode();	/* ��dagͼ���½�һ�� */
		dag[dagx].op=STOARRAY;;
		dag[dagx].left=i;
		dag[dagx].right=j;
		enternodetab(dst);	/* �ڽ������½�һ�� */
		nodetab[ndx].nodenum=dagx;
	}
	dstidx=locnodeidx(dst);	/* dst�ڽ����е�λ�� */
	if (0==dstidx)	/* �ڽ�����û��dst��Ҫ�½� */
	{
		enternodetab(dst);	/* �ڽ������½�һ�� */
		dstidx=ndx;
	}

	if (MOV==op)	/* MOVָ�MOVָ����Ҫ���½�����ڽ������½��� */
	{
		if (dag[i].op!=NOP)		/* ����Դ�������Ƿ�Ҷ��� */
		{
			nodetab[dstidx].nodenum=i;
		}
		else	/* ����Դ��������Ҷ��� */
		{
			newdagnode();	/* ��dagͼ���½���� */
			nodetab[dstidx].nodenum=dagx;
			dag[dagx].op=MOV;
			dag[dagx].left=i;
		}
	}
	else
	{
		k=locopnode(op,i,j);	/* ��ȡ���Ϊop���м���Ľ��� */
		if (0==k)	/* û�������Ľ�� */
		{
			newdagnode();	/* ��dagͼ���½�һ���м��� */
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

void dag2code(order code[],int *cdx)		/* ��dagͼ���µ����м���� */
{
	int stack[dagmax];	/* ��ʼ��һ������dagͼ�м����ջ */
	int top=-1,i;
	struct _dagnode tmpdag[dagmax];	/* ���ڱ���dagͼ����ʱdagͼ */
	for (i=1;i<=dagx;i++)
		tmpdag[i]=dag[i];
	while(1)
	{
		for (i=dagx;i>0;i--)
		{
			if (0==tmpdag[i].parentnum&&tmpdag[i].op!=NOP)	/* ѡȡû�и������м��㣨�����ȫ����ջ�����Ҳ�ں�������б��鵽��������� */
				break;
		}
		if (0==i) break;	/* û�ҵ�û�и��������и����ȫ����ջ���м��� */

		do
		{
			int left=tmpdag[i].left,right=tmpdag[i].right;	/* �����ӽ�� */
			if (0==tmpdag[i].parentnum&&tmpdag[i].op!=NOP)
				stack[++top]=i;	/* ���������Ľ���ջ */
			else 
				break;
			if (left>0)
				delparent(&tmpdag[left],i);	/* ��i�������ӽ��ĸ�����г�ȥ */
			if (right>0)
				delparent(&tmpdag[right],i);	/* ��i�������ӽ��ĸ�����г�ȥ */
			tmpdag[i].op=NOP;	/* ��Ϊnop�´ξͲ�����ѡ���� */
			i=left;	/* ���ŵ�ǰ��������ߣ�ѭ�������������ӽ�� */
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
		for (i=1;i<cntdst;i++)	/* ���Ŀ�Ĳ�������Ӧͬһ���ţ����ø�ֵ��ʽʹ���Ƕ�Ϊͬһֵ */
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
���ڻ�����Ļ��֣������������򣬿��Է��֣��������Ǳ��⼸��ָ��ָ����ģ�ENTER(����1)��SETLAB(����2)��J*ϵ��ָ��(����3)
����ֻҪ˳��ɨ���м���룬��������һ������һ�������鿪ʼ������������һ��������������������ר�ŰѴ����ٴ�ɻ�����
Ҫע��ĵط���1.���������Ϊ�����������ǣ�����STOARRYʱ���������ֵ�ı䣬�����ڽڵ���У���������LOADARRAY���õľͲ�����ԭ����ֵ
			  2.Ϊ�����������ǰ���޷���֪�ı�����Щ���ݵ����⣬���ñ��������������ֺ�������(CALL+RET)ʱ��Ҳʹ����Ϊ������Ļ���
			  3.INC��DEC��PUSH��READ��WRITE��LEAVE�������뵽�Ż��У�ԭ��д�ؼ���
���ϣ�ʵ���ϲ��뵽DAG�Ż��е�ָ��ֻ��NEG��MOV��ADD��SUB��IMUL��IDIV��STOARRAY(����˵��[]=)��LOADARRAY(����˵��[])
*/
void dagoptimaize(order code[][cmax],int cicnt)	/* ����DAGͼ�����ֲ������ӱ��ʽ */
{
	int i,j,k;
	for (i=1;i<=cicnt;i++)	/* ���ÿ�����̵�ѭ�� */
	{
		j=0;k=0;
		while (NOP!=code[i][j].op)
		{
			fct op=code[i][j].op;/* ָ�� */
			int src1=code[i][j].src1,src2=code[i][j].src2,dst=code[i][j].dst;	/* ������ */
			if (NEG==op||MOV==op||ADD==op||SUB==op||IMUL==op||IDIV==op/*||STOARRAY==op||LOADARRAY==op*/)
			{
				ndx=0;
				dagx=0;		/* �µĻ�����ѽ����dagͼ������ʼ��Ϊ0 */
				do 
				{
					if (MOV==op&&code[i][j].src1>0&&code[i][j].dst>0)
					{
						if (tab[code[i][j].src1].obj!=konstant) break;
					}
					insert2dag(op,src1,src2,dst);	/* ��dagͼ�м����µĽ�� */				
					j++;
					op=code[i][j].op;
					src1=code[i][j].src1;
					src2=code[i][j].src2;
					dst=code[i][j].dst;	/* ������ */
				} while (NEG==op||MOV==op||ADD==op||SUB==op||IMUL==op||IDIV==op/*||STOARRAY==op||LOADARRAY==op*/);
				dag2code(optimizedcode[i],&k);
			}
			optimizedcode[i][k++]=code[i][j++];	/* ����ָ��ԭ��д�� */
		}
		optimizedcode[i][k].op=NOP;
	}
}

/************************************���������ֲ������ӱ��ʽ�Ż����****************************************/

/***************************************���»�Ծ�����������������*******************************************/
int searchblkhaslabx(int x)	/* ���Ҿ��б��x�Ļ����飬�ҵ����ػ�����ţ����򷵻�-1 */
{
	int i;
	for (i=1;i<=bblkidx;i++)
		if (blockinfo[i].haslab==x)		/* ������i�б��x */
			return i;
	return -1;
}

void insertparents(int blk,int p)	/* �ѿ��p���뵽�����blk�Ļ������parents��ȥ */
{
	int i;
	for (i=0;i<bscblock[blk].parentsnum;i++)
		if (bscblock[blk].parents[i]==p)	/* �Ѿ���ǰ���������У�������� */
			return;
		
	bscblock[blk].parents[bscblock[blk].parentsnum++]=p;	/* ����parents����� */
}

void splitbasicblk(const order code[])	/* ���ֻ����飬code���м�������� */
{
	int i=1;	/* i=0�������������enter���Թ� */
	rewrtidx=0;
	while (code[i].op!=LEAVE)	/* �����м�������� */
	{
		++bblkidx;	/* ����������ֵ��1 */
		bscblock[bblkidx].istart=i;	/* ��ʼ���λ�� */
		if (SETLAB==code[i].op)	/* �����һ����������ñ�� */
		{
			blockinfo[bblkidx].haslab=code[i].dst;	/* ��¼����ӵ�б�ŵ���Ϣ */
			i++;	/* ָ����һ����� */
		}
		else 
			blockinfo[bblkidx].haslab=-1;
		blockinfo[bblkidx].gotolab=-1;
		while(code[i].op!=LEAVE)
		{
			if (JMP==code[i].op||JE==code[i].op||JNE==code[i].op||
				JL==code[i].op||JLE==code[i].op||JG==code[i].op||
				JGE==code[i].op)		/* ������ת��䣬��һ�����Ӧ���������䣬�ʸû�������� */
			{
				int gotoright;
				blockinfo[bblkidx].gotolab=code[i].dst;
				gotoright=searchblkhaslabx(code[i].dst);	/* �����б��dst�Ļ����� */
				if (gotoright>0)	/* �ҵ�������ڻ����� */
				{
					bscblock[bblkidx].right=gotoright;	/* ��ǰ��������ת���ĺ�̻�������Ϊ�ñ�����ڻ����� */
					insertparents(gotoright,bblkidx);	/* �ѵ�ǰ������ż��뵽��̻������ǰ�������鼯���� */
				}
				else	/* δ�ҵ� */
					rewritetab[++rewrtidx]=bblkidx;	/* ���¸û�������Ҫ����д */
				if (code[i].op!=JMP)
				{
					bscblock[bblkidx].left=bblkidx+1;	/* ������������ת��䣬��������ֱ�Ӻ�̶�Ӧ������һ�������� */
					insertparents(bblkidx+1,bblkidx);	/* �ѵ�ǰ������ż��뵽��̻������ǰ�������鼯���� */
				}
				i++;
				break;	/* ���������������˳�ѭ��������һ�� */
			}
			else if (SETLAB==code[i].op)	/* ���Ӧ���������䣬����ѭ����i���� */
			{
				bscblock[bblkidx].left=bblkidx+1;	/* ������������ת��䣬��������ֱ�Ӻ�̶�Ӧ������һ�������� */
				insertparents(bblkidx+1,bblkidx);	/* �ѵ�ǰ������ż��뵽��̻������ǰ�������鼯���� */
				break;
			}
			else
				i++;
		}
		bscblock[bblkidx].iend=i-1;	/* ѭ������ʱ��iָ����һ����䣬�ʱ�������Ľ������λ��Ӧ����i-1 */
	}
	for (i=1;i<=rewrtidx;i++)	/* ��д������ */
	{
		int gotoright;

		gotoright=searchblkhaslabx(blockinfo[rewritetab[i]].gotolab);	/* �����б��gotolab�Ļ����飬����ֵһ������0 */

		bscblock[rewritetab[i]].right=gotoright;
		insertparents(gotoright,rewritetab[i]);
	}
}

void add2use(int var,int blk)	/* �ѱ���var���������blk��use������ */
{
	int i;
	for (i=0;i<bscblock[blk].usenum;i++)
		if (bscblock[blk].use[i]==var)	/* ����use�����У�������� */
			return;
	bscblock[blk].use[i]=var;
	bscblock[blk].usenum++;
}

void add2def(int var,int blk)	/* �ѱ���var���������blk��def������ */
{
	int i;
	for (i=0;i<bscblock[blk].defnum;i++)
		if (bscblock[blk].def[i]==var)
			return;
	bscblock[blk].def[i]=var;
	bscblock[blk].defnum++;

}

void calcu_use_def(const order code[])	/* ����use����def�� */
{
	int i,j;
	for (i=1;i<=bblkidx;i++)	/* ���������� */
	{
		bool bdef[tmax]={0},buse[tmax]={0};
		for (j=bscblock[i].istart;j<=bscblock[i].iend;j++)	/* �����������е���� */
		{
			fct op=code[j].op;
			int src1=code[j].src1,src2=code[j].src2,dst=code[j].dst;
			switch (op)
			{
			case READ:	/* ����䶨���˱��� */
				if (!buse[dst]&&!bdef[dst])	/* ����û�б�ʹ��Ҳû�б����壬���������def�� */
				{
					add2def(dst,i);
					bdef[dst]=true;
				}
				break;
			case WRITE:	/* д���ʹ���˱��� */
				if (src1!=1&&dst>0&&tab[dst].obj==variable&&!bdef[dst]&&!buse[dst])	/* src1=1ʱֻ����ַ��� */
				{
					add2use(dst,i);
					buse[dst]=true;
				}
				break;
			case NEG:	/* dst=-src1 */
			case MOV:	/* dst=src1 */
				if (dst==src1)		/* dst=dst��dst=-dst����Ϊ�Ǳ�ʹ�� */
				{
					if (dst>0&&tab[dst].obj==variable&&!bdef[dst]&&!buse[dst])
					{
						add2use(dst,i);
						buse[dst]=true;
					}
				}
				else	/* dst!=src1����Ϊsrc1��ʹ�ã�dst������ */
				{
					if (src1>0&&tab[src1].obj==variable&&!bdef[src1]&&!buse[src1])	/* src1��û�м���def��use���У����������use���� */
					{
						add2use(src1,i);	/* ����use�� */
						buse[src1]=true;	/* src1���Ϊ�Ѽ���use�� */
					}
					if (dst>0&&tab[dst].obj==variable&&!bdef[dst]&&!buse[dst])
					{
						add2def(dst,i);		/* ����def�� */
						bdef[dst]=true;		/* dst���Ϊ�Ѽ���def�� */
					}
				}
				break;
			case INC:	/* dst=dst-1 */
			case DEC:	/* dst=dst+1 */
				if (dst>0&&tab[dst].obj==variable&&!buse[dst]&&!bdef[dst])
				{
					add2use(dst,i);		/* ��Ϊdst��ʹ�� */
					buse[dst]=true;		/* dst���Ϊ�Ѽ���use�� */
				}
				break;
			case ADD:	/* dst=src1+src2 */
			case SUB:	/* dst=src1-src2 */
			case IMUL:	/* dst=src1*src2 */
			case IDIV:	/* dst=src1/src2 */
				if (dst>0&&tab[dst].obj==variable&&dst==src1||dst==src2)	/* dst=dst+src? ��ʱ��Ϊdst��ʹ�� */
				{
					if (!bdef[dst]&&!buse[dst])
					{
						add2use(dst,i);
						buse[dst]=true;
					}
				}
				else if (dst>0&&tab[dst].obj==variable)		/* dst=src1+src2��src1��src2����ͬ��dst����ʱ��Ϊdst������ */
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
			case LOADARRAY:	/* ��������Ԫ�� dst=src1[src2] */
				if (dst>0&&tab[dst].obj==variable&&!buse[dst]&&!bdef[dst])
				{
					add2def(dst,i);		/* ��Ϊdst������ */
					bdef[dst]=true;		/* dst���Ϊ�Ѽ���def�� */
				}
				if (src2>0&&tab[src2].obj==variable&&!buse[src2]&&!bdef[src2])
				{
					add2use(src2,i);	/* �±�src2��ʹ�� */
					buse[src2]=true;
				}
				break;
			case STOARRAY:	/* ���鸳ֵ dst[src1]=src2 */
				if (src2>0&&tab[src2].obj==variable&&!buse[src2]&&!bdef[src2])
				{
					add2use(src2,i);	/* src2��ʹ�� */
					buse[src2]=true;
				}
				if (src1>0&&tab[src1].obj==variable&&!buse[src1]&&!bdef[src1])
				{
					add2use(src1,i);	/* �±�src1��ʹ�� */
					buse[src1]=true;
				}
				break;
			case PUSH:	/* ������ջ�����ڲ������� */
				if (dst>0&&tab[dst].obj==variable&&!buse[dst]&&!bdef[dst])
				{
					add2use(dst,i);		/* ��Ϊdst��ʹ�� */
					buse[dst]=true;		/* dst���Ϊ�Ѽ���use�� */
				}
				break;
			case JE:
			case JNE:
			case JL:
			case JLE:
			case JG:
			case JGE:	/* ���ֱȽ�ָ���ʹ���˱��� */
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


void allocateglobalreg(const order code[])	/* ȫ�ּĴ������� */
{
	int i,j,k;
	bool flag=false;
	int tmpcflctgraph[varmax][varmax]={0};
	int varinblks[varmax]={0};	/* �����ڼ����������У����ڱ����Ƿ��Խ��������Ȼ��Ծ */
	int allocvarnum=0;	/* ����ȫ�ּĴ����ı�����Ŀ */
	bool allocvar[varmax]={0};	/* allocvar[i]=x��ʾҪ����Ĵ����ĵ�i�������Ƿ��ű�λ��x�ı��� */
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

	splitbasicblk(code);	/* ���ֻ����� */
	calcu_use_def(code);	/* ��������use��def�� */
	do 
	{
		flag=false;		/* in�����Ƿ��иı�ı�־��ֻҪ��һ���������in���ϸı��ˣ�flag��Ϊtrue */
		for (i=bblkidx;i>=1;i--)
		{
			if (bscblock[i].left)	/* ��һ����̻����� */
			{
				for (j=0;j<bscblock[bscblock[i].left].innum;j++)	/* ������̻������in���ϣ���û��out������ļ��� */
				{
					for (k=0;k<bscblock[i].outnum;k++)	/* ����out���� */
					{
						if (bscblock[i].out[k]==bscblock[bscblock[i].left].in[j])	/* �Ѿ���out������ */
							break;
					}
					if (k==bscblock[i].outnum)	/* ˵������out�����У�Ҫ�������� */
					{
						bscblock[i].out[bscblock[i].outnum++]=bscblock[bscblock[i].left].in[j];
					}
				}
			}
			if (bscblock[i].right)	/* �ڶ�����̻����� */
			{
				for (j=0;j<bscblock[bscblock[i].right].innum;j++)	/* ������̻������in���ϣ���û��out������ļ��� */
				{
					for (k=0;k<bscblock[i].outnum;k++)	/* ����out���� */
					{
						if (bscblock[i].out[k]==bscblock[bscblock[i].right].in[j])	/* �Ѿ���out������ */
							break;
					}
					if (k==bscblock[i].outnum)	/* ˵������out�����У�Ҫ�������� */
					{
						bscblock[i].out[bscblock[i].outnum++]=bscblock[bscblock[i].right].in[j];
					}
				}
			}
			
			if (bscblock[i].innum==0&&bscblock[i].usenum!=0)	/* in����Ϊ�ն�use���ϲ�Ϊ�� */
			{
				for (j=0;j<bscblock[i].usenum;j++)
				{
					bscblock[i].in[bscblock[i].innum++]=bscblock[i].use[j];
				}
				flag=true;
			}
			for (j=0;j<bscblock[i].outnum;j++)	/* ����out���� */
			{
				for (k=0;k<bscblock[i].defnum;k++)	/* ���ڲ���def������ */
				{
					if (bscblock[i].out[j]==bscblock[i].def[k])	/* ��def�����о������������def����ʱ��ѭ��������kһ��С��defnum */
						break;
				}
				if (k==bscblock[i].defnum)	/* ����def�����У��ͼ���in���ϣ���Ҫ���ж��ڲ���in������ */
				{
					for (k=0;k<bscblock[i].innum;k++)
					{
						if (bscblock[i].out[j]==bscblock[i].in[k])	/* �Ѿ���in������ */
							break;
					}
					if (k==bscblock[i].innum)	/* ����in�����У�����in���� */
					{
						bscblock[i].in[bscblock[i].innum++]=bscblock[i].out[j];
						flag=true;	/* in�����иı�ı�־ */
					}
				}
			}
		}
	} while (true==flag);	/* ֻҪ���л������in���ϸı��˾��ظ���һ���� */


	for (i=1;i<=bblkidx;i++)	/* ���������飬�ҳ���Խ��������Ȼ��Ծ�ı��� */
	{
		for (j=0;j<bscblock[i].innum;j++)	/* �����������in���ϣ��ҳ��ڻ�����i�л�Ծ�ı��� */
		{
			varinblks[bscblock[i].in[j]]++;	/* �����������Ļ�������+1 */
		}
	}
	for (i=1;i<=t;i++)
	{
		if (varinblks[i]>1)	/* �����ڴ���һ���������л�Ծ */
		{
			allocvar[allocvarnum]=i;
			allocvarnum++;	/* Ҫ����ȫ�ּĴ����ı�����Ŀ��1 */
		}
		
	}

	for (i=1;i<=bblkidx;i++)	/* ���������飬������ͻͼ */
	{
		for (j=0;j<bscblock[i].defnum;j++)	/* �����������def���ϣ�def�����еĸ�������֮�����һ���� */
		{
			for (k=0;k<bscblock[i].defnum;k++)
			{
				int varj=bscblock[i].def[j],vark=bscblock[i].def[k];
				int defj=varinblks[varj],defk=varinblks[vark];
				if (j!=k&&defj>1&&defk>1)	/* �����������ǿ�������Ծ�� */
				{
					cflctgraph[varj][vark]=true;	/* ��������֮�����һ���� */
					cflctgraph[vark][varj]=true;

					tmpcflctgraph[varj][vark]=true;	/* ��������֮�����һ���� */
					tmpcflctgraph[vark][varj]=true;
				}
			}
		}

		for (j=0;j<bscblock[i].defnum;j++)	/* �����������def���ϣ�def�����еĸ���������in���ϵĸ�������֮�����һ���� */
		{
			for (k=0;k<bscblock[i].innum;k++)
			{
				int varj=bscblock[i].def[j],vark=bscblock[i].in[k];
				int defj=varinblks[varj],ink=varinblks[vark];
				if (varj!=vark&&defj>1&&ink>1)	/* �����������ǿ�������Ծ�� */
				{
					cflctgraph[varj][vark]=true;	/* ��������֮�����һ���� */
					cflctgraph[vark][varj]=true;

					tmpcflctgraph[varj][vark]=true;	/* ��������֮�����һ���� */
					tmpcflctgraph[vark][varj]=true;
				}
			}
		}
	}
	if (allocvarnum<=3)	/* Ҫ����ȫ�ּĴ����ı�����ĿС�ڵ��ڼĴ�����Ŀ��ֱ�ӷ���Ϳ��� */
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
			int noremov=true;	/* û�����߽��ı�� */
			int maxcnt=0;
			int cnt=0;
			int imaxcnt;	/* ���cnt��Ӧ�ı��� */
			do /* �ظ����߱�С��glbregnum�Ľ�� */
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
					if (cnt>0&&cnt<glbregnum)	/* �������ӱ���ĿС�ڼĴ��� */
					{
						stack[++top]=i;	/* ����ջ */
						for (k=0;k<allocvarnum;k++)
						{
							if (i==allocvar[k])
							{
								int tmp=allocvar[k];	/* �����һ������ */
								allocvar[k]=allocvar[allocvarnum-1];
								allocvar[allocvarnum-1]=tmp;
							}
						}
						allocvarnum--;	/* Ҫ����Ĵ����ı�����Ŀ-1 */
						noremov=false;	/* û�н�㱻���ߵı����Ϊ�� */
						for (j=1;j<=t;j++)
						{
							tmpcflctgraph[i][j]=false;
							tmpcflctgraph[j][i]=false;		/* ���߽���ͬʱ���߱� */
						}
					}
				}
			} while (false==noremov&&allocvarnum>1);
			
			if (allocvarnum>1)	/* Ҫ����ȫ�ּĴ����ı���������1 */
			{
				cnt=maxcnt=0;
				for (i=1;i<=t;i++)		/* �ҵ����ӱ���Ŀ����һ������ͼ������ */
				{
					cnt=0;
					for (j=1;j<=t;j++)
						if (true==cflctgraph[i][j])
							cnt++;
						if (cnt>maxcnt)	/* �滻����һ�� */
						{
							maxcnt=cnt;
							imaxcnt=i;
						}
				}
				i=imaxcnt;
				regalloc[i]=-1;	/* ���ñ������Ϊ��������ȫ�ּĴ������ı��� */
				for (k=0;k<allocvarnum;k++)
				{
					if (i==allocvar[k])
					{
						int tmp=allocvar[k];	/* �����һ������ */
						allocvar[k]=allocvar[allocvarnum-1];
						allocvar[allocvarnum-1]=tmp;
					}
				}
				allocvarnum--;	/* Ҫ����Ĵ����ı�����Ŀ-1 */
				for (j=1;j<=t;j++)
				{
					tmpcflctgraph[i][j]=false;
					tmpcflctgraph[j][i]=false;		/* ���߽���ͬʱ���߱� */
				}
			}
			
		} while (allocvarnum>1);		/* ���Ӧ��ֻʣһ������allocvar[0] */
		
		regalloc[allocvar[0]]=EBX;	/* Ϊ���һ����������ebx */
		while (top>=0)	/* ջ�л��б��� */
		{
			bool canalloc[4]={true,true,true};	/* һ��ʼ�����Է��� */
			for (j=1;j<=t;j++)
			{
				if (true==cflctgraph[stack[top]][j]&&regalloc[j]>0)
				{
					canalloc[regalloc[j]-EBX]=false;	/* �Ĵ���regalloc[j]���ܷ��� */
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
/***************************************���ϻ�Ծ�����������������*******************************************/

/***************************************���¿����Ż����*******************************************/

void peepholeoptimize(order code[][cmax],int cicnt)	/* �������ñ��ʽ */
{
	int i,j,k,l;
	for (l=0;l<2;l++)
	for (i=1;i<=cicnt;i++)	/* ���ÿ�����̵�ѭ�� */
	{
		for (j=0,k=0;NOP!=code[i][j].op;j++)
		{
			fct op=code[i][j].op;
			int src1=code[i][j].src1,src2=code[i][j].src2,dst=code[i][j].dst;
			fct nxtop=code[i][j+1].op;
			int nxtsrc1=code[i][j+1].src1,nxtsrc2=code[i][j+1].src2,nxtdst=code[i][j+1].dst;
			if (MOV==op&&src1==dst)	/* ��������mov */
			{
				
			}
			else if (JMP==op&&SETLAB==nxtop&&dst==nxtdst)	/* ��������jmp */
			{
				j++;	/* ����SETLAB */
			}
			else if (MOV==nxtop&&dst==nxtsrc1&&dst<0&&			/* ��һ����ֵ����Դ����������һ������Ŀ�Ĳ����� */
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
			else if ((JE==op||JGE==op||JLE==op)&&src1>0&&src1==src2)	/* if x=x �� if x>=x �� if x<=x��� */
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
				code[i][k++]=code[i][j];	/* �������ԭ��д�� */
		}
		code[i][k].op=NOP;
	}
}

/***************************************���Ͽ����Ż����*******************************************/

