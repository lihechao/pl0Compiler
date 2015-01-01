#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "optimize.h"
#include "pl0Compiler.h"
int cistack[lmax],citop=-1,cicnt;
extern order optimizedcode[pmax][cmax];
/********************************���´��������******************************************/
void errormsg()	/* ��ӡ������Ϣ��ժҪ */
{
	int k;
	char msg[ermax][45]={"number too large","wrong character","expect single quote mark","expect double quote mark","illegal character",/*0-4*/
		"expect ';'","identifier redefinition","parameter too few","illegal symbol","undeclared identifier","expect identifier",	/*5-10*/
		"expect constant","expect integer","expect right bracket","expect 'of'","wrong type begin symbol","expect left bracket",/*11-16*/
		"expect 'integer' or 'char'","wrong parameter begin symbol","expect colon","expect right parentheses","expect '='","parameters do not match",/*17-22*/
		"no such array","wrong factor begin symbol","procedure in expression","array subscript too large","only integer or char can be compare",/*23-27*/
		"expect ':='","function assignment do not match","wrong assignment type","expect 'end'","condition type should be integer","expect 'then'",/*28-33*/
		"expect 'until'","expect variable","wrong for-loop variable type","for-loop expression wrong","expect 'to' or 'downto'","expect 'do'",/*34-39*/
		"expect function/procedure/variable identifier","expect 'begin'","wrong read parameter","expect left parentheses","wrong write parameter",/*40-44*/
		"expect '.'","divide 0","wrong real parameter type"/*45-47*/
	};
	fprintf(psout,"\n");
	fprintf(psout,"Meaning of error codes��\n");
	for (k=0;k<ermax;k++)
	{
		if (errs[k])
			fprintf(psout,"%2d: %s\n",k,msg[k]);
	}
}

void endskip()		/* Դ�����б������Ĳ������滮�߱�� */
{
	while(errpos<cc)	/* ����λ���뵱ǰ�ַ�֮�����������ַ� */
	{
		fprintf(psout,"-");	/* �����ַ����滮�� */
		errpos++;			/* ����һ���ַ� */
	}
	skipflag=false;			/* ���������Ϊfalse��ʾ�������� */
}

void error(errcode n)		/* �ڵ�ǰλ�ã�cc������ӡ����λ�úͳ����� */
{
	errcnt++;
	if (0==errpos)			/* ��ʾ�¿�ʼ��һ��Ҫ���ĸ��� */
		fprintf(psout,"****");
	if (cc>errpos)			/* ��ǰ���ַ�λ�����ϴδ�ӡ����λ��֮�� */
	{
		int i;
		for (i=0;i<cc-errpos;i++)
			fprintf(psout," ");
		fprintf(psout,"^%2d",n);
		errpos=cc+3;		/* ��ӡ�����λ�õ��˵�ǰ�ַ�������������������^��ռ����λ�õ�n�� */
		errs[n]=true;		/* n�Ŵ��������󼯺� */		
	}
	printf("ERROR: line:%d\tcode:%d\n",linecount,n);
}

void fatal(fatalcode n)	/* ��ӡ�������󣨱�������Ϣ�� */
{
	char msg[fatmax][15];
	strcpy(msg[IDENTIFIER],"symbol table");
	strcpy(msg[BLOCK],"block table");
	strcpy(msg[ARRAYS],"array table");
	strcpy(msg[LEVELS],"display table");
	strcpy(msg[CODE],"code table");
	strcpy(msg[STRINGS],"string table");
	fprintf(psout,"%s overflow!\n",msg[n]);		
}

void skip(symset fsys,errcode n)	/* ����Դ����ֱ��ȡ���ķ������ڸ����ķ��ż�Ϊֹ������ӡ������ */
{
	error(n);	/* ��ӡ������ */
	skipflag=true;
	while(!belongset(sy,fsys))	/* ��ǰ���ŵ�����ڸ����ķ��ż��� */
		insymbol();
	if (skipflag)
		endskip();	/* ֹͣ���� */
}

void test(symset s1,symset s2,errcode n)	/* ���Ե�ǰ�����Ƿ�Ϸ��������Ϸ�����ӡ�����־���������� */
{
	if (!belongset(sy,s1))
		skip(setsadd(s1,s2),n);
}

void testsemicolon(symset fsys)	/* ���Ե�ǰ�����Ƿ�Ϊ�ֺ� */
{
	if (SEMICN==sy)	/* ��ǰ����Ϊ�ֺ� */
	{
		insymbol();/* �Ϸ�������һ�� */
	}
	else
	{
		error(EXPECTSEMICOLON);
		if (COMMA==sy||COLON==sy)
		{
			insymbol();	/* ��Ϊ���Ż�ð��ʱ����Ϊ�Ƿֺŵ���д����������һ���ַ� */
		}
	}
	test(elemaddset(IDEN,blockbegsys),fsys,ILEGALSYM);
}
/********************************���ϴ��������******************************************/



/********************************���´ʷ��������******************************************/

void switchs(bool *b)	/* ��������ѡ���е�'+'��'-'��־ */
{
	*b=ch=='+';		/* ����Ϊ'+'����b=true */
	if (ch!='+')
	{
		if (ch!='-')	/* chҲ��Ϊ'-' */
		{
			while(ch!='*'&&ch!=',')
				nextch();
		}
		else
			nextch();
	}
	else
		nextch();
}

void options()	/* �������ʱ�Ŀ�ѡ�� */
{
	do 
	{
		nextch();
		if (ch!='*')
		{
			if (ch=='t')	/* ��ӡ���ѡ�� */
			{
				nextch();
				switchs(&prtables);	/* �ж���'+'����'-'�������prtables */
			}
			else if (ch=='e')
			{
				nextch();
				switchs(&execute);
			}
		}
	} while (ch==',');
}

void nextch()	/* ��ȡ��һ�ַ��������н�������ӡ���������Դ���� */
{
	if (cc==ll)	/* ����һ�н��� */
	{
		if (feof(psin))	/* �ļ��ѽ��� */
		{
			fputc('\n',psout);
			fprintf(psout,"Դ�����ļ�����������ļ���β��\n");
			errormsg();		/* ��ӡ������Ϣ */
			exit(1);		/* �˳����� */
		}
		else if (errpos!=0)
		{
			if (skipflag)	/* �������Ϊ�棬˵��Ҫ���������ֻ��� */
				endskip();
			fputc('\n',psout);
			errpos=0;		/* ��ӡ����λ����Ϊ0 */
		}
		fprintf(psout,"%-4d ",++linecount);
		ll=0;	/* ��ǰ�г�����Ϊ0 */
		cc=0;	/* ��ǰ�ж������ַ�����Ϊ0 */
		ch='\0';
		while (ch!='\n'&&ch!=EOF)
		{	
			ch=fgetc(psin);		/* ��һ���ַ� */
			if (EOF!=ch)
			fputc(ch,psout);	/* д������ļ� */
			line[ll++]=ch;		/* ���ַ������л����� */
		}
	}
	ch=line[cc++];
}

void insymbol()	/* ��ȡ��һ���ʷ��ţ�����ע���� */
{
	int low,high,k;
	while(isspace(ch))	/* �����հ��ַ� */
		nextch();
	if (isalpha(ch))	/* ��ǰ�������ַ�����ĸ  <��ĸ>::= a|b|c|d��x|y|z |A|B��|Z */
	{
		k=0;
		memset(id,0,sizeof(id));	/* ��ʶ����� */
		do 
		{
			if (k<alng)/* ������ʳ���û�дﵽ����ʶ�����ȣ��ͼ�����ȡ�ַ���
							����������������ʶ�����ȵĲ��� */
				id[k++]=ch;/* �ѵ�ǰ�������ַ�ƴ�ӵ����ڷ����ı�ʶ�����棬
												������ʶ�����ȼ�1 */
			nextch();
		} while (isalnum(ch));	/* ����<��ʶ��>::= <��ĸ>{<��ĸ>|<����>} */

		low=0;high=nkw-1;
		k=-1;
		while (low<=high)	/* �ڱ����ֱ��ж��ֲ���id */
		{
			int mid=(low+high)/2;
			int cmp=strcmp(id,key[mid]);
			if (0==cmp)
			{
				k=mid;
				break;
			}
			else if (cmp>0)
				low=mid+1;
			else
				high=mid-1;
		}
		sy=k>=0?ksy[k]:IDEN;	/* ����ڱ����ֱ����ҵ���id�򵥴����Ϊ�ñ�������𣬷���Ϊ��ʶ�� */
	}
	else if (isdigit(ch))	/* ��ǰ�������ַ�������  <����>	::=	0|1|2|3��8|9 */
	{
		k=0;				/* ��ǰ����λ����Ϊ0 */
		inum=0;				/* ���һ�ζ������޷���������Ϊ0 */
		sy=INTCON;	/* �ǰ�����ĵ��ʵ�����Ϊ�������� */
		while('0'==ch)	/* ����ǰ��0 */
		{
			nextch();
		}
		if (isdigit(ch))	/* �������ǰ��0�������� */
		{
			do 
			{
				inum=10*inum+ch-'0';
				k++;				/* ����λ����1 */
				nextch();	/* ����һ���ַ� */
			} while (isdigit(ch));	/* ����<�޷�������>	::=	<����>{<����>} */
		}
		else	/* ����ֻ��һλ����0 */
			k=1;
		if (k>kmax||inum>nmax)	/* ����������λ��������������������λ����
				�����ִ�����������������*/
		{
				error(NUMOVERFLOW);	/* ����������� */
				inum=0;
				k=0;
		}
	}
	else if (':'==ch)	/* ��ǰ�������ַ���ð��':' */
	{
		nextch();	/* ����һ���ַ������ǲ���'=' */
		if ('='==ch)		/* �����һ���ַ���'=' */
		{
			sy=ASSIGN;	/* ��������Ϊ��ֵ����� */
			nextch();		/* ����һ���ַ� */
		}
		else
		{
			sy=COLON;		/* ���򵥴�����Ϊð�� */
		}
	}
	else if ('<'==ch)		/* ��ǰ�������ַ���С�ں� */
	{
		nextch();			/* ����һ���ַ� */
		if ('='==ch)		/* �����һ���ַ��ǵȺ�'=' */
		{
			sy=LEQ;		/* ��������ΪС�ڵ�������� */
			nextch();		/* Ԥ����һ���ַ� */
		}
		else if ('>'==ch)		/* �����һ���ַ��Ǵ��ں�'>' */
		{
			sy=NEQ;		/* ��������Ϊ����������� */
			nextch();		/* Ԥ����һ���ַ� */
		}
		else
		{
			sy=LSS;		/* ��������ΪС������� */
		}
	}
	else if ('>'==ch)		/* ��ǰ�������ַ��Ǵ��ں� */
	{
		nextch();			/* ����һ���ַ� */
		if ('='==ch)		/* �����һ���ַ��ǵ��ں�'=' */
		{
			sy=GEQ;		/* ��������Ϊ���ڵ�������� */
			nextch();		/* Ԥ����һ���ַ� */
		}
		else
		{
			sy=GRE;		/* ��������Ϊ��������� */
		}
	}
	else if ('\''==ch)		/* ��ǰ�������ַ��ǵ����� */
	{
		nextch();			/* ����һ���ַ������ǲ�����ĸ������
						<�ַ�>	::=	'<��ĸ>' | '<����>'  */
		if (!isalnum(ch))	/* ��һ���ַ�������ĸ������ */
		{
			error(WRONGCHARCON);		/* ���󣺴�����ַ����� */
			sy=NUL;
		}
		else						/* ��һ���ַ�����ĸ������ */
		{
			inum=ch;	/* �ַ���ascii��ֵ */
			sy=CHARCON;	/* ��������Ϊ�ַ����� */
		}
		nextch();			/* ����һ���ַ����ǲ��ǵ�����' */
		if (ch!='\'')		/* �����ַ�������ŵĲ��ǵ�����' */
		{
			error(EXPECTQMARK);	/* ����ӦΪ������' */
		}
		else
			nextch();			/* Ԥ����һ���ַ� */
	}
	else if ('"'==ch)		/* ��ǰ�������ַ���˫����" */
	{
		sx++;
		k=0;						/* �ַ������ȳ�ʼ��Ϊ0 */
		sy=STRCON;					/* ��������Ϊ�ַ������� */
		nextch();					/* ����һ���ַ� */
		while(ch!='"'&&ch!='\n'&&ch!=EOF)		
		{
			if (sx+k>=smax)
				fatal(STRINGS);
			stab[sx][k++]=ch;
			nextch();
		}
		if (ch!='"')
			error(EXPECTDQMARK);
		nextch();
	}
	else if ('('==ch)	/* ��ǰ�ַ�Ϊ������ */
	{
		nextch();
		if ('*'!=ch)	/* ����ע�� */
		{
			sy=LPARENT;
		}
		else			/* ��ע�ͣ����� */
		{
			nextch();
			if ('$'==ch)	/* �Ǳ����ѡ�� */
				options();
			do 
			{
				while('*'!=ch)	/* ֱ���ҵ�*����ע�ͽ������ */
					nextch();
			} while (ch!=')');	/* ���*���治�������ţ��ͼ����� */
			nextch();
			insymbol();/* ������ע�ͣ���Ҫ�ٶ���һ������ */
		}
	}
	else if ('{'==ch)
	{
		nextch();
		if ('$'==ch)	/* �����ѡ�� */
		{
			options();
		}
		while('}'!=ch)	/* ����ע�� */
			nextch();
		nextch();		/* ��ע�ͺ�ĵ�һ���ַ� */
		insymbol();		/* ������ע�ͣ���Ҫ����һ������ */
	}
	else if ('+'==ch||'-'==ch||'*'==ch||'/'==ch||','==ch||';'==ch||
		'.'==ch||'='==ch||')'==ch||'['==ch||']'==ch)	
		/* �����ַ� */
	{
		sy=sps[ch];
		nextch();
	}
	else
	{
		error(ILEGALCHAR);
		nextch();
		insymbol();		/* ���ԷǷ��ַ����ٶ���һ������ */
	}
}
/********************************���ϴʷ��������******************************************/

/********************************���µ�¼������******************************************/

void enter(char id[],objecttype k)	/* �ֳ����ڣ��ڷ��ű��е�¼�ֳ���˵�����ֳ��ֵı�ʶ�� */
{
	int i,j;
	if (tmax==t)	/* ���ű���� */
		fatal(IDENTIFIER);
	else
	{
		strcpy(tab[0].name,id);/* ����ʶ��������ʱ�����ڷ��ű��0��λ�� */
		i=btab[display[level]].last;/* ��i���ڵ�ǰ�ֳ��������һ����ʶ���ڷ��ű��е�λ�� */
		j=i;
		while(0!=strcmp(tab[i].name,id))
		{
			i=tab[i].link;	/* �������ϲ���id */
		}
		if(i!=0)	/* ˵���ҵ�����idͬ���ı�ʶ�� */
			error(IDENTREDEFINED);
		else	/* ͬһ��ֳ�����û��ͬ����ʶ�� */
		{
			t++;	/* ���ű�����ֵ��1 */
			/* ����ʶ����Ӧ����Ϣ������ű� */
			strcpy(tab[t].name,id);
			tab[t].link=j;		/* ͬһ��ֳ����е���һ����ʶ�� */
			tab[t].obj=k;		/* ���ࣨ���������������̡������� */
			tab[t].typ=notyp;	/* ����������Ϊ�գ����淴�� */
			tab[t].ref=0;
			tab[t].lev=level;	/* ���ŵĵ�ǰ��� */
			tab[t].adr=0;
			tab[t].normal=false;	/* Ĭ��Ϊfalse */
			/* ���ֳ���������һ����ʶ���ڷ��ű��е�λ�ø���Ϊ��ǰ��ʶ������ */
			btab[display[level]].last=t;
		}
	}
}

void enterarray(types type,int size)/* ��¼������Ϣ������atab */
{
	if (amax==a)
		fatal(ARRAYS);
	if (size>=arrmax)
		error(ARRAYFLOWOVER);	/* �����±�̫�� */
	else
	{
		a++;
		atab[a].size=size;
		atab[a].eltyp=type;
	}
}

void enterblock()/* ��¼�ֳ����btab */
{
	if (bmax==b)	/* �ֳ������� */
	{
		fatal(BLOCK);
	}
	else
	{
		b++;	/* �ֳ��������ֵ��1 */
		btab[b].last=0;		/* ��ʱ��Ϊ0 */
		btab[b].lastpar=0;	/* ��ʱ��Ϊ0 */
	}
}

void entercontab()	/* ��¼������ */
{
	cnx++;
	contab[cnx].tx=t;
	contab[cnx].con.val=tab[t].adr;	/* ����ֵ */
	contab[cnx].con.tp=tab[t].typ;
}


void entervariable()	/* ����������¼�����ű��� */
{
	if (IDEN==sy)	/* ��ǰ�����Ǳ�ʶ�� */
	{
		enter(id,variable);
		insymbol();
	}
	else
		error(EXPECTIDEN);
}

int loc(char id[])	/* ���ұ�ʶ���ڷ��ű��е�λ�� */
{
	int i,j;
	i=level;	
	
	/* �Ƚ���ʶ�����ڷ��ű��0��λ�ã������Ҳ���ʱֱ�ӷ���0 */
	strcpy(tab[0].name,id);
	do 
	{
		j=btab[display[i]].last;	/* �ڵ�ǰ����� */
		while(0!=strcmp(tab[j].name,id))
			j=tab[j].link;	/* �������ϲ��� */
		i--;	/* ����һ�� */
	} while (i>=0&&0==j);	/* �������ϲ���� */
	if (0==j)	/* �ڷ��ű���û���ҵ���ʶ�� */
		error(IDENTUNDEFINED);	/* ���󣺱�ʶ��δ���� */
	return j;	/* ����id�ڷ��ű��е�λ�� */
}

int loccontab(int val,types type)	/* ���ҳ����ڳ������е�λ�� */
{
	int i;
	for (i=1;i<=cnx;i++)
		if (contab[i].con.val==val&&contab[i].con.tp==type)
			return contab[i].tx;	/* �ҵ��˷��س����ڷ��ű��е����� */
	return 0;	/* û�ҵ�����0 */
}

/************************************���ϵ�¼������******************************************/

/************************************���´����������******************************************/
void emit(fct op)	/* ����ֻ��ָ�����Ƿ�����Ԫʽ */
{
	if (cmax==ci)
		fatal(CODE);
	code[ci][cj].op=op;
//	printf("%d,%d:%s\n",ci,cj,fctword[op]);
	cj++;
}

void emit1(fct op,int dst)	/* ����ֻ�����Ƿ���dst����Ԫʽ */
{
	if (cmax==ci)
		fatal(CODE);
	code[ci][cj].op=op;
	code[ci][cj].dst=dst;
//	printf("%d,%d:%s\t%d\n",ci,cj,fctword[op],dst);
	cj++;
}

void emit2(fct op,int src1,int dst)	/* ����ֻ�����Ƿ���src1��dst����Ԫʽ */
{
	if (cmax==ci)
		fatal(CODE);
	code[ci][cj].op=op;
	code[ci][cj].src1=src1;
	code[ci][cj].dst=dst;
//	printf("%d,%d:%s\t%d\t%d\n",ci,cj,fctword[op],src1,dst);
	cj++;
}

void emit3(fct op,int src1,int src2,int dst)	/* ���������Ƿ���src1��src2��dst����Ԫʽ */
{
	if (cmax==ci)
		fatal(CODE);
	code[ci][cj].op=op;
	code[ci][cj].src1=src1;
	code[ci][cj].src2=src2;
	code[ci][cj].dst=dst;
//	printf("%d,%d:%s\t%d\t%d\n",ci,cj,fctword[op],src1,src2,dst);
	cj++;
}
/************************************���ϴ����������******************************************/


/************************************���������������******************************************/

/* ��������г��ֵĳ���������c���س������ͺ�ֵ
<����>	::=	[+| -] <�޷�������>|<�ַ�>*/
void constant(symset fsys,conrec *c)	
{
	int sign;		/* ���ţ������ţ� */
	c->tp=notyp;	/* �Ƚ�������Ϊ�գ�ֵ��Ϊ0 */
	c->val=0;
	test(constbegsys,fsys,EXPECTCONST);	/* ��鵱ǰ�����Ƿ�Ϊ������ʼ���� */
	if (belongset(sy,constbegsys))
	{
		if (CHARCON==sy)		/* �ַ����� */
		{
			c->tp=chars;
			c->val=inum;		/* �ַ���ASCII��ֵ */
			insymbol();
		}
		else
		{
			sign=1;
			if (MINU==sy)	/* �Ǹ��� */
			{
				sign=-1;
				insymbol();
			}
			if (INTCON==sy)	/* �������޷������� */
			{
				c->tp=ints;
				c->val=sign*inum;	
				insymbol();
			}
			else
				skip(fsys,EXPECTCONST);
		}
		test(fsys,fsys,ILEGALSYM);		/* ��鳣������ķ����Ƿ�Ϸ� */
	}
}

/* ���������壬�����������������Ϣ��¼�����ű��� 
	<��������>	::=	<��ʶ��>= <����>*/
void constantdef(symset fsys)
{
	conrec c;
	symset settmp=fsys;
	insertelem(SEMICN,&settmp);
	insertelem(COMMA,&settmp);
	insertelem(IDEN,&settmp);
	if (IDEN==sy)
	{
		enter(id,konstant);	/* ��������¼���ű� */
		insymbol();	/* ��ȡ�Ⱥ� */
		if (EQL==sy)	/*  �ǵȺ� */
			insymbol();	/* ������ */
		else
		{
			error(EXPECTEQL);
			if (ASSIGN==sy)	/* ��Ϊ��ֵ������ǵȺŵ���д */
				insymbol();
		}
		constant(settmp,&c);	/* ������ֵ */
		tab[t].typ=c.tp;
		tab[t].ref=0;
		tab[t].adr=c.val;
		tab[t].normal=true;
		entercontab();	/* ��¼������ */
	}
	else
		error(EXPECTIDEN);
}

/* ������˵�����֣�������������Ӧ��Ϣ������ű�
<����˵������>	::=  const<��������>{,<��������>}; */
void constdec(symset fsys)
{
	insymbol();	/* �������ĵ�һ������ */
	test(elem2set(IDEN),blockbegsys,EXPECTIDEN);/* ����ǲ��Ǳ�ʶ�� */
	if (IDEN==sy)
	{
		constantdef(fsys);	/* ���������� */
		while (COMMA==sy)	/* ����˵�����滹�г������� */
		{
			insymbol();
			constantdef(fsys);
		}
		testsemicolon(fsys);
	}
}

void arraytyp(symset fsys,int *rf,int *tp)	/* �����������ͣ�array'['<�޷�������>']' of <��������> */
{
	conrec size;
	types eltp,elrf,tp1;
	symset tmpset=fsys;
	insertelem(COLON,&tmpset);
	insertelem(RBRACK,&tmpset);
	insertelem(RPARENT,&tmpset);
	insertelem(OFTK,&tmpset);
	constant(tmpset,&size);	/* ������Ĺ�ģ��С */
	if (size.tp!=ints)	/* �������Ĳ������� */
	{
		error(EXPECTUNUM);	/* ���� */
		size.val=0;			/* ��С��Ϊ0 */
	}
	if (RBRACK==sy)	/* �޷�����������Ӧ�������ҷ����� */
		insymbol();
	else
	{
		error(EXPECTRBRACK);	/* Ӧ�����з����� */
		if (RPARENT==sy)	/* ��Ϊ��Բ�������ҷ����ŵ���д */
			insymbol();
	}
	if (OFTK==sy)	/* �ҷ����ź��������of */
		insymbol();
	else
		error(EXPECTOF);
	typ(fsys,&eltp,&elrf,&tp1);	/* �������� */
	if (eltp!=ints&&eltp!=chars)	/* ���ǻ������� */
	{
		error(EXPECTBASICTYPE);
		eltp=notyp;
	}
	*rf=size.val;
	*tp=eltp;
}

/* ������������
<��������>	::=   integer | char
<����>	::=	<��������>|array'['<�޷�������>']' of <��������> */
void typ(symset fsys,types *tp,int *rf,types *tp1)
{
	*tp=notyp;
	*rf=0;
	test(typebegsys,fsys,WRONGTYPEHEAD);/* ��鵱ǰ�����Ƿ�Ϊ�Ϸ������Ϳ�ʼ���� */
	if (belongset(sy,typebegsys))
	{
		if (INTTK==sy)		/* ��ǰ����Ϊinteger */
		{
			*tp=ints;
			insymbol();
		}
		else if (CHARTK==sy)	/* ��ǰ����Ϊchar */
		{
			*tp=chars;
			insymbol();
		}
		else if (ARRAYTK==sy)	/* ��ǰ����Ϊarray */
		{
			insymbol();	/* �ٶ�һ������ */
			if (LBRACK==sy)	/* ����������� */
				insymbol();	/* ������Ԫ�ظ��� */
			else
			{
				error(EXPECTLBRACK);
				if (LPARENT==sy)	/* ��Ϊ��Բ�����������ŵ���д */
					insymbol();
			}
			*tp=arrays;	/* ����Ϊ���� */
			arraytyp(fsys,rf,tp1);	/* ������������ */
		}
		test(fsys,fsys,ILEGALSYM);	/* ������ͺ�̷����Ƿ�Ϸ� */
	}
}

/* �����������
<����˵������>	::=  var <����˵��> ; {<����˵��>;}
<����˵��>		::=  <��ʶ��>{, <��ʶ��>} :  <����>*/
void variabledeclaration(symset fsys)
{
	types tp,tp1;
	int t0,t1,rf;
	symset settmp=fsys;
	insertelem(SEMICN,&settmp);
	insertelem(COMMA,&settmp);
	insertelem(IDEN,&settmp);
	insymbol();
	while(IDEN==sy)
	{
		t0=t;				/* ���浱ǰ���ű�����ֵ��Ϊ��������׼�� */
		entervariable();	/* ������������ű� */
		while(COMMA==sy)	/* ����˵�������Ƕ��ž�˵���б���˵�� */
		{
			insymbol();
			entervariable();
		}
		if (COLON==sy)	/* ���һ��������Ӧ����ð�� */
			insymbol();
		else
			error(EXPECTCOLON);
		t1=t;	/* t1Ϊ��¼��һЩ����˵������ű������ */
		typ(settmp,&tp,&rf,&tp1);/* ð�ź��������� */
		while(t0<t1)	/* ������ű� */
		{
			t0++;
			tab[t0].obj=variable;
			tab[t0].typ=tp; 
			tab[t0].normal=true;
			tab[t0].lev=level;
			tab[t0].adr=dx;
			if (tp==arrays)
			{
				enterarray(tp1,rf);		/* ��¼������Ϣ������ */
				tab[t0].ref=a;
				dx=dx+atab[a].size;
			}
			else 
			{
				tab[t0].ref=rf;
				dx++;
			}
		}
		testsemicolon(fsys);	/* ����˵������Ӧ���Ƿֺ� */
	}
}

/* ������̻���˵���е��βα����βμ����й���Ϣ��¼�����ű��� 
<��ʽ������>	::= [var] <��ʶ��>{, <��ʶ��>}: <��������>{; <��ʽ������>}*/
void parameterlist(symset fsys)	
{
	types tp;
	bool valpar;	/* �Ƿ�Ϊֵ�β� */
	int t0;
	tp=notyp;
	insymbol();	/* ����һ�����ţ�����ǲ��Ǳ�ʶ����var */
	while (IDEN==sy||VARTK==sy)
	{
		if (VARTK!=sy)		/* ����var��ͷ */
			valpar=true;	/* ��ֵ�β� */	
		else	/* ��var��ͷ */
		{
			insymbol();	/* �ٶ�һ�����ǲ����ı�ʶ�� */
			valpar=false;	/* ����ֵ�β� */
		}
		t0=t;	/* ���浱ǰ���ű�����ֵ��Ϊ���淴����ű���׼�� */
		entervariable();	/* ����������Ϊ������¼�����ű� */
		while(COMMA==sy)	/* ֻҪ���ж��ž�˵�������β� */
		{
			insymbol();		/* ��ȡ�βα����� */
			entervariable();	/* ���βα�������¼���ű� */
		}
		if (COLON==sy)	/* ÿ���βα����һ����ʶ������Ӧ����ð�� */
		{
			insymbol();	/* ð�ź���Ӧ�������� */
			if (INTTK==sy)	/* ����Ϊ�޷������� */
			{
				tp=ints;
				insymbol();
			}
			else if (CHARTK==sy)	/* ����Ϊ���� */
			{
				tp=chars;
				insymbol();
			}
			else
				error(EXPECTBASICTYPE);
			/* ���ð�ź����ǲ��ǷֺŻ��������� */
			test(elemaddelem(SEMICN,RPARENT),setsadd(elemaddelem(COMMA,IDEN),fsys),EXPECTSEMICOLON);
		}
		else
			error(EXPECTCOLON);
		while(t0<t)	/* ��ʼ������ű� */
		{
			t0++;
			tab[t0].typ=tp;
			tab[t0].ref=0;
			tab[t0].adr=dx;	/* ����������βΣ�������ջ�з���Ĵ洢��Ԫ��Ե�ַ */
			tab[t0].lev=level;
			tab[t0].normal=valpar;	/* �Ƿ�Ϊֵ�β� */
			dx++;		/* �൱��Ϊ��������ռ� */
		}
		if (RPARENT!=sy)	/* ���������ţ�˵�������βα���Ҫ˵�� */
		{
			if (SEMICN==sy)	/* �βα�֮��Ӧ���Էֺŷָ� */
				insymbol();
			else
			{
				error(EXPECTSEMICOLON);
				if (COMMA==sy)	/* ��Ϊ�����Ƿֺŵ���д */
					insymbol();
			}
			/* ����β�˵���Ŀ�ʼ���� */
			test(elemaddelem(IDEN,VARTK),elemaddset(RPARENT,fsys),WRONGPARHEAD);
		}
	}
	if(RPARENT==sy)	/* ȫ���β�˵����ɺ�Ӧ���������� */
	{
		insymbol();
		/* ����βα�˵���ĺ�̷����Ƿ�Ϸ� */
		test(elemaddelem(SEMICN,COLON),fsys,ILEGALSYM);
	}
	else
		error(EXPECTRPARENT);
}

/*	������̻���˵�� 
	<����˵������>		::=	<�����ײ�><�ֳ���>{;<�����ײ�><�ֳ���>};
	<����˵������>		::=	<�����ײ�><�ֳ���>{;<�����ײ�><�ֳ���>};
	<�����ײ�>			::=	procedure<��ʶ��>'('[<��ʽ������>]')';
	<�����ײ�>			::=	function <��ʶ��>'('[<��ʽ������>]')': <��������>;
*/
void procdeclaration(symset fsys)		
{
	bool isfun=sy==FUNCTK;	/* �ǲ��Ǻ����ı�� */
	int prt;
	dx=5;/* Ԥ��3������ȫ�ּĴ�����λ�ã�1������ebp��λ�� */
	insymbol();
	if (IDEN!=sy)	/* ������Ǳ�ʶ�� */
	{
		error(EXPECTIDEN);	/* ���� */
		memset(id,0,sizeof(id));	/* ��ձ�ʶ�� */
	}
	enter(id,isfun?function:procedure);	/* ������/��������¼���ű� */
	tab[t].normal=true;
	prt=t;
	insymbol();
	level++;	/* ��μ�1 */
	enterblock();	/* ��¼�ֳ���� */
	display[level]=b;	/* ���·ֳ��������� */
	tab[t].ref=b;	/* ���·��ű��ref��ָ����̻����ڷֳ����������е�λ�� */
	if (isfun) tab[t].adr=dx++;	/* Ϊ������Ҫ�������ؽ����λ�� */
	ci++;
	codelevel[ci]=level;
	cj=0;
	cicnt++;
	emit2(ENTER,b,t);	/* ���ɽ������/������ָ�� */
	if (LPARENT==sy)	/* �������� */
		parameterlist(fsys);
	else
		error(EXPECTLPARENT);
	btab[b].lastpar=t;	/* ���·ֳ��������һ��������λ�� */
	btab[b].psize=dx;	/* ���·ֳ����в���+�����ռ��С */
	if (isfun)
	{
		if (COLON==sy)	/* �����Ĳ����������ð�� */
		{
			insymbol();
			if (INTTK==sy||CHARTK==sy)	/* ð�ź����ǻ������� */
			{
				tab[prt].typ=INTTK==sy?ints:chars;
				insymbol();
			}
			else
				error(EXPECTBASICTYPE);	
		}
		else
			error(EXPECTCOLON);
	}
	if (SEMICN==sy)	/* ����/�����ײ����涼��ð�� */
		insymbol();
	else
		error(EXPECTSEMICOLON);
	block(fsys);	/* ���÷ֳ������ */
	level--;
	if (SEMICN==sy)	/* �ֳ������Ӧ���Ƿֺ� */
		insymbol();
	else
		error(EXPECTSEMICOLON);
	emit2(LEAVE,b,prt);	/* �����뿪���̵Ĵ��� */
}

/*<�ֳ���>	::=   [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>*/
void block(symset fsys)
{
	int dx0,prb;
	int preci,precj;
	if (level>lmax)
		fatal(BLOCK);
//	test(blockbegsys,fsys,);
	if (CONSTTK==sy)
		constdec(fsys);
	if(VARTK==sy)
		variabledeclaration(fsys);
/*	btab[b].vsize=dx;*/	/* �����ⲻ������ʱ���� */
	if (1==level)
	{
		btab[b].psize=dx;
	}
	dx0=dx;
	prb=b;
	preci=ci;precj=cj;
	while(PROCETK==sy||FUNCTK==sy)
	{
		procdeclaration(fsys);
		if (PROCETK==sy)
		{
			ci++;
		}
	}
	test(elem2set(BEGINTK),setsadd(blockbegsys,statbegsys),EXPECTBEGIN);
	dx=dx0;
	ci=preci;cj=precj;
	insymbol();
	statement(setsadd(fsys,elemaddelem(SEMICN,ENDTK)));	/* ��� */
	while(belongset(sy,elemaddset(SEMICN,statbegsys)))	/* sy�ǷֺŻ�������俪ʼ���ż��� */
	{
		if (SEMICN==sy)	/* �ֺ������ָ���˵��������� */
			insymbol();
		else
			error(EXPECTSEMICOLON);
		statement(setsadd(fsys,elemaddelem(SEMICN,ENDTK)));	/* ������� */
	}
	btab[prb].vsize=dx;	/* ������Ͱ�������ʱ���� */
	if (ENDTK==sy)		/* ������Ӧ����end */
		insymbol();
	else
		error(EXPECTEND);
	test(setsadd(elemaddelem(SEMICN,PERIOD),fsys),setsadd(elemaddelem(SEMICN,PERIOD),fsys),ILEGALSYM);
}

/*<���>	::=  <��ֵ���>|<�������>|<repeatѭ�����>|<���̵������>|<�������>|<�����>|<д���>|<forѭ�����>|<��>*/
void statement(symset fsys)
{
	int i;
	if (belongset(sy,statbegsys))
	{
		switch(sy)
		{
		case IDEN:	/* ƥ�丳ֵ������̵������ */
			i=loc(id);
			insymbol();
			if (0!=i)
			{
				switch (tab[i].obj)
				{
				case konstant:
					error(WRONGIDEN);	/* �������ܱ���ֵ */
					break;
				case variable:
					assignment(fsys,i);	/* Ϊ������ֵ */
					break;
				case procedure:	/* ƥ����̵������ */
					call(fsys,i);
					break;
				case function:	/* Ϊ������ֵ */
					if (tab[i].ref==display[level])	/* �����ԶԱ��㺯����ֵ */
						assignment(fsys,i);
					else
						error(WRONGIDEN);
					break;
				}
			}
			break;
		case BEGINTK:	/* ƥ�临����� */
			compoundstatement(fsys);
			break;
		case IFTK:	/* ƥ��������� */
			ifstatement(fsys);
			break;
		case REPTTK:	/* ƥ��repeatѭ����� */
			repeatstatement(fsys);
			break;
		case FORTK:		/* ƥ��forѭ����� */
			forstatement(fsys);
			break;
		case WRITETK:	/* ƥ��д��� */
			insymbol();
			writestatement(fsys);
			break;
		case READTK:	/* ƥ������ */
			insymbol();
			readstatement(fsys);
			break;
		}
	}
	test(fsys,elemaddset(SEMICN,fsys),EXPECTSEMICOLON);	
}

/************************************���������������******************************************/


/************************************������䴦�����******************************************/

/* ��������
	<����>	::=	<��ʶ��>|<��ʶ��>'['<���ʽ>']'|
	<�޷�������>|'('<���ʽ>')' | <�����������> */
void factor(symset fsys,item *x)	

{
	int i;
	x->typ=notyp;	/* ������Ϊ�����ͣ������޸� */
	x->ref=0;
	test(facbegsys,fsys,WRONGFACHEAD);	/* ����ǲ������ӿ�ʼ���� */
//	while(belongset(sy,facbegsys))
	if(belongset(sy,facbegsys))
	{
		if (IDEN==sy)	/* �Ǳ�ʶ�� */
		{
			i=loc(id);
			if (0==i)
				error(IDENTUNDEFINED);	/* ��ʶ��δ���� */
			insymbol();
			if (konstant==tab[i].obj)	/* ��ʶ������Ϊ���� */
			{
				x->typ=tab[i].typ;	/* ����Ϊ�ó�����Ӧ���� */
				x->ref=i;
			}
			else if (variable==tab[i].obj)	/* ��ʶ������Ϊ������ƥ��<��ʶ��>|
				<��ʶ��>'['<���ʽ>']' */
			{
				x->typ=tab[i].typ;	/* �������� */
				x->ref=i;
				if (LBRACK==sy||LPARENT==sy)	/* ����������Ԫ�� */
				{
					item idx;	/* �����±���ʽ */
					insymbol();
					expression(elemaddset(RBRACK,fsys),&idx);
					if (idx.typ!=ints&&idx.typ!=chars)	/* �±겻�ǻ������� */
						error(EXPECTBASICTYPE);
					x->typ=atab[tab[i].ref].eltyp;
					x->ref=-(dx);dx++;
					emit3(LOADARRAY,i,idx.ref,x->ref);	/* �¿���һ���ռ�洢����Ԫ�� */
					if (RBRACK==sy)	/* �ҷ����� */
						insymbol();
					else
					{
						error(EXPECTRBRACK);
						if (RPARENT==sy)		/* ��Ϊ��Բ�������ҷ����ŵ���д */
							insymbol();
					}
				}
			}
			else if (function==tab[i].obj)	/* ������ʶ����ƥ��<�����������> 
				<�����������>	::=   <��ʶ��>'('[<ʵ�ڲ�����>]')'*/
			{
				x->typ=tab[i].typ;
				x->ref=-(dx);dx++;	/* Ϊ��������ֵ������ʱ�����ռ� */
				call(fsys,i);
				emit2(RET,i,x->ref);	/* ��������ֵ���ص���ʱ�ռ� */
			}
		}
		else if (INTCON==sy)	/* ƥ��<�޷�������> */
		{
			x->typ=ints;
			if (0==(x->ref=loccontab(inum,ints)))	/* ��������û������� */
			{
				char name[alng]="#";
				int link=btab[display[level]].last;
				itoa(inum,name+1,10);
				enter(name,konstant);	/* ��¼�����ű� */
				tab[t].typ=ints;
				tab[t].adr=inum;
				tab[t].normal=true;
				tab[t].link=link;
				x->ref=t;				/* �ڷ��ű��е�λ�� */
				entercontab();
			}
			insymbol();
		}
		else if (LPARENT==sy)	/* ƥ��'('<���ʽ>')' */
		{
			insymbol();	/* �����ʽ�ĵ�һ������ */
			expression(elemaddset(RPARENT,fsys),x);	/* ������ʽ */
			if (RPARENT==sy)	/* ���ʽ������������ */
				insymbol();
			else
				error(EXPECTRPARENT);
		}
		else
			error(WRONGPROCIDENT);		/* ���Ӳ���Ϊ���̱�ʶ�� */
		test(setsadd(elemaddelem(SEMICN,RBRACK),fsys),facbegsys,ILEGALSYM);
	}
}

void term(symset fsys,item *x)	/* ������ <��>	::=	<����>{<�˷������><����>} */
{
	item y,z;
	symbol op;
	symset settmp=fsys;
	insertelem(MULT,&settmp);
	insertelem(DIV,&settmp);
	factor(settmp,x);/* ƥ������ */
	
	while(MULT==sy||DIV==sy)
	{
		op=sy;	/* ���浱ǰ����� */
		insymbol();
		factor(settmp,&y);
		z.ref=-(dx);dx++;		/* Ϊ���z����ջ�ռ� */
		z.typ=x->typ;
		if (MULT==op)	/* ������ǳ˺� */
		{
			emit3(IMUL,x->ref,y.ref,z.ref);
		}
		else	/* ������ǳ��� */
		{
			
			if (y.ref>0&&tab[y.ref].obj==konstant&&tab[y.ref].adr==0)
				error(DIVZERO);
			else
				emit3(IDIV,x->ref,y.ref,z.ref);
		}
		*x=z;
	}
}

void expression(symset fsys,item *x)	/* ������ʽ<���ʽ>	::=	[+|-]<��>{<�ӷ������><��>} */
{
	item y,z;
	symbol op; 
	symset settmp=fsys;
	insertelem(PLUS,&settmp);
	insertelem(MINU,&settmp);
	if (PLUS==sy||MINU==sy)	/* ƥ��[+ | -] */
	{
		op=sy;	/* ������� */
		insymbol();
		term(settmp,x);	/* ƥ���� */
		if (x->typ!=ints&&x->typ!=chars)	/* �����ź���Ĳ������� */
		{
			error(EXPECTUNUM);
		}
		else if (MINU==op)	/* ǰ���Ǹ��� */
		{
			z.typ=x->typ;
			z.ref=-(dx);dx++;
			emit2(NEG,x->ref,z.ref);	/* �� */
			*x=z;
		}
	}
	else	/* ǰ��û�������ţ�ֱ������ */
		term(settmp,x);
	while(PLUS==sy||MINU==sy)
	{
		op=sy;	/* ��������� */
		insymbol();	/* ��ȡ������� */
		term(settmp,&y);
		z.typ=x->typ;
		z.ref=-(dx);dx++;
		if (PLUS==op)	/* ������ǼӺ� */
			emit3(ADD,x->ref,y.ref,z.ref);	/* ���ɼӷ����� */
		else
			emit3(SUB,x->ref,y.ref,z.ref);	/* ���ɼ������� */
		*x=z;
	}
}

void condition(symset fsys,item *x,item *y,symbol *op)	/* ��������<����>	::=	<���ʽ><��ϵ�����><���ʽ> */
{
	symset settmp=fsys;
	insertelem(EQL,&settmp);	/* �Ⱥ� */
	insertelem(NEQ,&settmp);	/* ���Ⱥ� */
	insertelem(LSS,&settmp);	/* С�ں� */
	insertelem(LEQ,&settmp);	/* С�ڵ��ں� */
	insertelem(GRE,&settmp);	/* ���ں� */
	insertelem(GEQ,&settmp);	/* ���ڵ��ں� */
	expression(settmp,x);	/* ƥ����ʽ */
	if (EQL==sy||NEQ==sy||LSS==sy||LEQ==sy||GRE==sy||GEQ==sy)	/* ƥ���ϵ����� */
	{
		*op=sy;
		insymbol();
		expression(fsys,y);
		if (x->typ==notyp||x->typ==arrays||y->typ==notyp||y->typ==arrays)
		{
			error(WRONGCONDITIONTYPE);
		}
		x->typ=ints;
		y->typ=ints;
	}
}

void assignment(symset fsys,int i)	/* ����ֵ��� */
{
	item x,y;
	bool isarray=false;
	if (tab[i].obj!=variable&&tab[i].obj!=function)
	{
		error(WRONGASSIGNTYPE);
		x.typ=notyp;
	}
	else
	{
		x.typ=tab[i].typ;
		x.ref=i;
	}
	if (arrays==x.typ)
	{
		if (LBRACK==sy||LPARENT==sy)	/* ����������Ԫ�� */
		{
			insymbol();
			expression(elemaddset(RBRACK,fsys),&x);/* x�������±���ʽ */
			if (x.typ!=ints&&x.typ!=chars)	/* �±겻�ǻ������� */
				error(EXPECTBASICTYPE);
			if (RBRACK==sy)	/* �ҷ����� */
				insymbol();
			else
			{
				error(EXPECTRBRACK);
				if (RPARENT==sy)		/* ��Ϊ��Բ�������ҷ����ŵ���д */
					insymbol();
			}
			isarray=true;
		}
		else
			error(EXPECTLBRACK);
	}

	if (ASSIGN==sy)		/* �Ǹ�ֵ�� */
		insymbol();
	else
	{
		error(EXPECTASSIGN);
		if (EQL==sy)	/* ��Ϊ�Ⱥ��Ǹ�ֵ�ŵ���д */
			insymbol();
	}
	expression(fsys,&y);	/* ��⸳ֵ���Ҳ�ı��ʽ */
	if (function==tab[i].obj&&x.typ!=y.typ)
		error(WRONGASSIGNTYPE);
	else if (x.typ!=notyp&&y.typ!=notyp)
	{
		if (!isarray)
			emit2(MOV,y.ref,x.ref);
		else
		{
			emit3(STOARRAY,x.ref,y.ref,i);/* ���鸳ֵ */	/* ��������ʱ�����İ汾 */
		}
	}
	else
		error(WRONGASSIGNTYPE);
}

void compoundstatement(symset fsys)	/* ��������� */
{
	insymbol();
	statement(setsadd(fsys,elemaddelem(SEMICN,ENDTK)));
	while(belongset(sy,elemaddset(SEMICN,statbegsys)))
	{
		if (SEMICN==sy)
			insymbol();
		else
			error(EXPECTSEMICOLON);
		statement(setsadd(fsys,elemaddelem(SEMICN,ENDTK)));
	}
	if (ENDTK==sy)
		insymbol();
	else
		error(EXPECTEND);
}

void ifstatement(symset fsys)	/* ����if��� <�������>	::=	if<����>then<���> | if<����>then<���>else<���>*/
{
	item x,y;
	types op;
	int lab1;
	insymbol();
	condition(setsadd(fsys,elemaddelem(THENTK,DOTK)),&x,&y,&op);
	//	emit2(CMP,x.ref,y.ref);		/* ���ɱȽ�ָ�� */
	lab1=labcount++;
	switch (op)
	{
	case EQL:		/* �����ǵ��ڣ������������ǲ����� */
		emit3(JNE,x.ref,y.ref,lab1);
		break;
	case NEQ:		/* �����ǲ����ڣ������������ǵ��� */
		emit3(JE,x.ref,y.ref,lab1);
		break;
	case LSS:		/* ������С�ڣ������������Ǵ��ڵ��� */
		emit3(JGE,x.ref,y.ref,lab1);
		break;
	case LEQ:		/* ������С�ڵ��ڣ������������Ǵ��� */
		emit3(JG,x.ref,y.ref,lab1);
		break;
	case GRE:		/* �����Ǵ��ڣ�������������С�ڵ��� */
		emit3(JLE,x.ref,y.ref,lab1);
		break;
	case GEQ:		/* �����Ǵ��ڵ��ڣ�������������С�� */
		emit3(JL,x.ref,y.ref,lab1);
		break;
	}
	if (THENTK==sy)	/* if���������then */
		insymbol();
	else
	{
		error(EXPECTTHEN);
		if (DOTK==sy)	/* ��Ϊdo��then����д */
			insymbol();
	}
	statement(elemaddset(ELSETK,fsys));
	if (ELSETK==sy)		/* if������eles��ƥ����if<����>then<���>else<���> */
	{
		int lab2=labcount++;
		insymbol();
		emit1(JMP,lab2);
		emit1(SETLAB,lab1);
		statement(fsys);
		emit1(SETLAB,lab2);
	}
	else	/* û��else��ƥ����if<����>then<���> */
		emit1(SETLAB,lab1);
}

void repeatstatement(symset fsys)	/* <repeatѭ�����>  ::=  repeat <���>until<����> */
{
	item x,y;
	types op;
	int lab1=labcount++;
	insymbol();
	emit1(SETLAB,lab1);
	statement(elemaddset(UNLTK,fsys));
// 	while (SEMICN==sy)
// 		insymbol();	/* ��������� */
	if (UNLTK==sy)
	{
		insymbol();
		condition(fsys,&x,&y,&op);
		//		emit2(CMP,x.ref,y.ref);
		switch (op)
		{
		case EQL:		/* �����ǵ��ڣ������������ǲ����� */
			emit3(JNE,x.ref,y.ref,lab1);
			break;
		case NEQ:		/* �����ǲ����ڣ������������ǵ��� */
			emit3(JE,x.ref,y.ref,lab1);
			break;
		case LSS:		/* ������С�ڣ������������Ǵ��ڵ��� */
			emit3(JGE,x.ref,y.ref,lab1);
			break;
		case LEQ:		/* ������С�ڵ��ڣ������������Ǵ��� */
			emit3(JG,x.ref,y.ref,lab1);
			break;
		case GRE:		/* �����Ǵ��ڣ�������������С�ڵ��� */
			emit3(JLE,x.ref,y.ref,lab1);
			break;
		case GEQ:		/* �����Ǵ��ڵ��ڣ�������������С�� */
			emit3(JL,x.ref,y.ref,lab1);
			break;
		}
	}
	else
		error(EXPECTUNTIL);
}

void forstatement(symset fsys)/*<forѭ�����>::=  for <��ʶ��> := <���ʽ> (to|downto) <���ʽ> do <���>  */
{
	types cvt,op;
	item x;
	int i;
	int labtest=labcount++,labend=labcount++;
	symset settmp=fsys;
	insertelem(ASSIGN,&settmp);
	insertelem(TOTK,&settmp);
	insertelem(DOWNTOTK,&settmp);
	insertelem(DOTK,&settmp);
	
	insymbol();
	if (IDEN==sy)	/* ��ʶ�� */
	{
		i=loc(id);	/* �����ڷ��ű��е�λ�� */
		insymbol();
		if (0==i)	/* �ڷ��ű���û�ҵ� */
		{
			error(IDENTUNDEFINED);
			cvt=ints;	/* Ĭ��Ϊ������ */
		}
		else if (variable==tab[i].obj)		/* ��ʶ��������Ϊ���� */
		{
			cvt=tab[i].typ;		/* Ϊ���ű��иñ�ʶ�������� */
			if (ints!=cvt&&chars!=cvt)
				error(WRONGFORVAR);
		}
		else
		{
			error(EXPECTVARIBLE);
			cvt=ints;
		}
	}
	else
		skip(settmp,EXPECTIDEN);
	settmp=fsys;
	insertelem(TOTK,&settmp);
	insertelem(DOWNTOTK,&settmp);
	insertelem(DOTK,&settmp);
	if (ASSIGN==sy)
	{
		insymbol();
		expression(settmp,&x);	/* ���ʽ */
		if (cvt!=x.typ)			/* ���ʽֵ��ѭ���������Ͳ���� */
		{
			error(WRONGFOREXPR);
			cvt=ints;
		}
		emit2(MOV,x.ref,i);
	}
	else
		skip(settmp,EXPECTASSIGN);
	
	if (TOTK==sy||DOWNTOTK==sy)
	{
		op=sy;
		insymbol();
		emit1(SETLAB,labtest);	/* ���Ա�ţ����������ѭ�������Ƿ����� */

		expression(elemaddset(DOTK,fsys),&x);	/* ����to / downto ����ı��ʽ */
		if (x.typ!=cvt)
			error(WRONGFOREXPR);
		if (TOTK==op)
			emit3(JG,i,x.ref,labend);
		else
			emit3(JL,i,x.ref,labend);
	}
	else
		skip(elemaddset(DOTK,fsys),EXPECTTODOWNTO);
	if (DOTK==sy)
		insymbol();
	else
		error(EXPECTDO);
	statement(fsys);
	emit1(TOTK==op?INC:DEC,i);	/* ���ӻ����ѭ������ */
	emit1(JMP,labtest);		/* ��ת���������Դ� */
	emit1(SETLAB,labend);	/* ������ţ�ת�������ѭ�� */
}

/* ������̻������� i������/������tab�е�����ֵ 
<�����������>	::=   <��ʶ��>'('[<ʵ�ڲ�����>]')'
<ʵ�ڲ�����>	::=  <ʵ�ڲ���> {, <ʵ�ڲ���>}
<ʵ�ڲ���>		::=  <���ʽ>
*/
void call(symset fsys,int i)
{
	item x;
	int lastp,cp;
	symset settmp=fsys;
	insertelem(COMMA,&settmp);
	insertelem(COLON,&settmp);
	insertelem(RPARENT,&settmp);
	lastp=btab[tab[i].ref].lastpar;	/* lastpΪ���̻��������һ�������ڱ�ʶ�����е�λ�� */
	cp=i;	/* ��cp��ʼΪ����/������tab�е�����ֵ */
	if (sy!=LPARENT) error(EXPECTLPARENT);
	if (cp==lastp)	/* û���β� */
		insymbol();
	else 
	{
		do 
		{
			insymbol();
			cp++;	/* cpΪ��һ���βε����� */
			if (cp>lastp)
				error(WRONGACTPARNUM);/* �βθ�����ʵ�θ�����ƥ�� */
			else
			{
				if (tab[cp].normal)	/* ֵ�β� */
				{
					expression(settmp,&x);	/* ���ñ��ʽ��������<ʵ�ڲ���>	::=  <���ʽ> */
					if ((tab[cp].typ==ints||tab[cp].typ==chars||x.typ==ints||x.typ==chars)/*&&tab[cp].typ==x.typ*/)	/* ���ʽ���������β�����һ�� */
						emit2(PUSH,0,x.ref);	/* ����Ĳ�����ջ��0��ʾ��������ѹ��ջ */
					else
						error(WRONGPARTYP);
				}
				else	/* �����β� */
				{
 					if (sy!=IDEN)	/* ����ʵ�ο϶��Ǳ�ʶ�� */
 						error(EXPECTIDEN);
					else
 					{
 						int k=loc(id);	/* �ڷ��ű��в��ұ�ʶ�� */
 						insymbol();
 						if (k!=0)	/* �ҵ� */
 						{
 							if(tab[k].obj!=variable)
 								error(EXPECTVARIBLE);
 							if (tab[k].typ!=arrays)	/* ʵ�β�������Ԫ�� */
 							{
 								emit2(PUSH,1,k);	/* 1�������ַ��ջ�����Ҳ�������Ԫ�� */
 							}
 							else	/* ʵ��������Ԫ�� */
 							{
								if (LBRACK!=sy)	/* ������ */
									error(EXPECTLBRACK);
								else
								{
									insymbol();
									expression(settmp,&x);
									if (sy!=RBRACK)
									{
										error(EXPECTRBRACK);
									}
									else insymbol();
								}
								emit3(PUSH,2,x.ref,k);		/* 2�������ַ��ջ������������Ԫ�� */
 							}
 						}
 					}
				}
			}
			test(elemaddelem(COMMA,RPARENT),fsys,ILEGALSYM);
		} while (COMMA==sy);	/* ����˵������ʵ�� */
	}
	if (RPARENT==sy)
		insymbol();
	else
		error(EXPECTRPARENT);
	if (cp<lastp)
		error(TOOFEWACTPAR);
	emit1(CALL,i);
}

/* ��������
<�����>	::=  read'('<��ʶ��>{,<��ʶ��>}')' */
void readstatement(symset fsys)	

{
	int i;
	if (LPARENT==sy)	/* �������� */
	{
		do 
		{
			insymbol();
			if (IDEN!=sy)
				error(EXPECTIDEN);
			else
			{
				i=loc(id);	/* �ҵ���ʶ���ڷ��ű��е�λ�� */
				if (0==i)
					error(IDENTUNDEFINED);
				else
				{
					insymbol();
					if (variable!=tab[i].obj)	/* �÷��Ų��Ǳ��� */
						error(EXPECTVARIBLE);
					else
					{
						if (ints==tab[i].typ||chars==tab[i].typ)
							emit1(READ,i);
						else
							error(WRONGREADPAR);
					}
				}
			}
			test(elemaddelem(COMMA,RPARENT),fsys,ILEGALSYM);
		} while (COMMA==sy);
		if (RPARENT==sy)
			insymbol();
		else
			error(EXPECTRPARENT);
	}
	else
		error(EXPECTLPARENT);
}

/* ����д���
<д���>	::=   write'(' <�ַ���>,<���ʽ> ')'|write'(' <�ַ���>')'|write'('<���ʽ>')' 
�����������䣬����������޶���ʽ����������write'('[<�ַ���>,]<���ʽ>{,<���ʽ>}')'	*/
void writestatement(symset fsys)	
{
	item x;
	if (LPARENT==sy)
	{
		insymbol();
//		do 
		{	
			int flag=0;	
			if (STRCON==sy)
			{
				flag=1;		/* Ҫ����ַ��� */
				insymbol();
			}
			if (COMMA==sy)
			{
				insymbol();
				flag=flag==1?2:0;	
			}
			if (sy==ADD||sy==MINU||belongset(sy,facbegsys))
			{
				expression(setsadd(fsys,elemaddelem(COMMA,RPARENT)),&x);
				if (ints!=x.typ&&chars!=x.typ)
					error(WRONGWRITEPAR);
				if (ints==x.typ)	/* ������ʽΪ���� */
					flag=flag==2?5:3;	/* flag=3 ��������֣�flag=5������ַ��������� */
				else if (chars==x.typ)
					flag=flag==2?6:4;	/* flag=4 ������ַ���flag=6������ַ������ַ� */
			}
			emit3(WRITE,flag,sx,x.ref);	/* ������ʽ */
		} //while (sy==COMMA);
		if (RPARENT==sy)
			insymbol();
		else
			error(EXPECTRPARENT);
	}
	else
		error(EXPECTLPARENT);
}
/************************************������䴦�����******************************************/

/************************************���¼Ĵ����������****************************************/

int locreg(int data)	/* ����data���ڵļĴ�������data���ڼĴ������򷵻�-1 */
{
	int i;
	for (i=0;i<tmpregnum+glbregnum;i++)
		if (regpool[i]==data)	/* i�żĴ�����������data */
			return i;
	return -1;
}

void mem2reg(int adr,int r)	/* ���ڴ�adr��������д���Ĵ���r */
{
	if (adr>0)	/* ���ű��е����� */
	{
		if (konstant==tab[adr].obj)
		{
			fprintf(pasm,"\tmov\t%s,%d\n",regword[r],tab[adr].adr);
		}
		else if (variable==tab[adr].obj)
		{
			if (tab[adr].normal)	/* ֵ�β� */
			{
				if (tab[adr].lev<level)
				{
					int j;
					fprintf(pasm,"\tpush\tebp\n");
					for (j=tab[adr].lev;j<level;j++)
						fprintf(pasm,"\tmov\tebp,[ebp-16]\n");
					fprintf(pasm,"\tmov\t%s,[ebp-%d]\n",regword[r],tab[adr].adr*4);
					fprintf(pasm,"\tpop\tebp\n");
				}
				else
					fprintf(pasm,"\tmov\t%s,[ebp-%d]\n",regword[r],tab[adr].adr*4);
			}
			else	/* �����β� */
			{
				if (tab[adr].lev<level)	/* ���ϲ�ı��� */
				{
					int j;
					fprintf(pasm,"\tpush\tebp\n");
					for (j=tab[adr].lev;j<level;j++)
						fprintf(pasm,"\tmov\tebp,[ebp-16]\n");
					fprintf(pasm,"\tmov\tebp,[ebp-%d]\n",tab[adr].adr*4);	/* [ebp-%d]��������Ǳ����βεĵ�ַ */
					fprintf(pasm,"\tmov\t%s,[ebp]\n",regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
				else	/* ͬһ��ı��� */
				{
					fprintf(pasm,"\tpush\tebp\n");
					fprintf(pasm,"\tmov\tebp,[ebp-%d]\n",tab[adr].adr*4);
					fprintf(pasm,"\tmov\t%s,[ebp]\n",regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
			}
		}
	}
	else	/* ��ʱ���� */
		fprintf(pasm,"\tmov\t%s,[ebp-%d]\n",regword[r],-4*adr);
}

void reg2mem(int r)	/* �ѼĴ���r������д���ڴ� */
{
	int adr=regpool[r];
	if (adr<0)	/* �Ĵ����д������ʱ���� */
		fprintf(pasm,"\tmov\t[ebp-%d],%s\n",-4*adr,regword[r]);
	else	/* �Ĵ����е��ǲ�������ʱ���� */
	{
		if (tab[adr].obj==function)
			fprintf(pasm,"\tmov\t[ebp-20],%s\n",regword[r]);	/* �������ؽ������ */
		else if (tab[adr].obj==variable)
		{
			if (tab[adr].normal==true)	/* ֵ�β� */
			{
				if (tab[adr].lev<level)	/* ���ϲ�ı��� */
				{
					int j;
					fprintf(pasm,"\tpush\tebp\n");
					for (j=tab[adr].lev;j<level;j++)
						fprintf(pasm,"\tmov\tebp,[ebp-16]\n");
					fprintf(pasm,"\tmov\t[ebp-%d],%s\n",tab[adr].adr*4,regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
				else	/* ͬһ��ı��� */
					fprintf(pasm,"\tmov\t[ebp-%d],%s\n",tab[adr].adr*4,regword[r]);
			}
			else	/* �����β� */
			{
				if (tab[adr].lev<level)	/* ���ϲ�ı��� */
				{
					int j;
					fprintf(pasm,"\tpush\tebp\n");
					for (j=tab[adr].lev;j<level;j++)
						fprintf(pasm,"\tmov\tebp,[ebp-16]\n");
					fprintf(pasm,"\tmov\tebp,[ebp-%d]\n",tab[adr].adr*4);	/* [ebx-%d]��������Ǳ����βεĵ�ַ */
					fprintf(pasm,"\tmov\t[ebp],%s\n",regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
				else	/* ͬһ��ı��� */
				{
					fprintf(pasm,"\tpush\tebp\n");
					fprintf(pasm,"\tmov\tebp,[ebp-%d]\n",tab[adr].adr*4);
					fprintf(pasm,"\tmov\t[ebp],%s\n",regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
			}
		}
	}
}


int allocatereg(int x,int y,int data)	/* Ϊdata����Ĵ��������ҼĴ�����ԭ�������ݲ�����xҲ������y */
{
	int i;
	if (data>0&&regalloc[data]>0)	/* data�Ƿ���ȫ�ּĴ����ı��� */
	{
		if (regpool[regalloc[data]]!=0&&true==regwrtback[regalloc[data]])	/* Ϊdata�����ȫ�ּĴ��������б�����������Ҫд�� */
		{

			reg2mem(regalloc[data]);
		}
		regpool[regalloc[data]]=data;
		regwrtback[regalloc[data]]=true;
		return regalloc[data];
	}

	for (i=0;i<tmpregnum;i++)
		if (0==regpool[i])		/* i����ʱ�Ĵ����ǿյ� */
		{
			regpool[i]=data;
			regwrtback[i]=false;	/* �ձ�д��Ĵ���������д�� */
			return i;
		}
	for (i=0;i<tmpregnum;i++)		/* û�ҵ��ռĴ�����������һ��������ʱ�����ļĴ���д���ڴ� */
	{
		int tmp=regpool[i];	/* tmp�ǼĴ���i������� */
		if (tmp<0&&tmp!=x&&tmp!=y)	/* ����ʱ�����Ҳ���x��y */
		{
			if (regwrtback[i])	/* �ñ�д���ڴ� */
				reg2mem(i);	/* д���ڴ� */
			regpool[i]=data;		/* ���¼Ĵ������� */
			regwrtback[i]=false;	/* �ձ�д��Ĵ���������д�� */
			return i;
		}
	}

	for (i=0;i<tmpregnum;i++)	/* Ҳû�д�����ʱ�����ļĴ��������Ҵ��˲�����ֲ������ļĴ��� */
	{
		int tmp=regpool[i];
		if (tmp>0&&tmp!=x&&tmp!=y)	/* �Ǿֲ������Ҳ���x��y */
		{
			if (regwrtback[i])	/* �ñ�д���ڴ� */
				reg2mem(i);	/* д���ڴ� */
			regpool[i]=data;
			regwrtback[i]=false;
			return i;
		}
	}

	if (regwrtback[0])	/* ��û�ֵ��Ĵ�����ǿ�Ʒ���0�żĴ��� */
	{
		reg2mem(0);
	}
	regpool[0]=data;
	regwrtback[0]=false;
	return 0;
}

void clearreg()	/* �����ʱ�Ĵ��� */
{
	int i;
	for(i=0;i<tmpregnum;i++)
	{
		if (regpool[i]!=0&&regwrtback[i])
			reg2mem(i);
		regpool[i]=0;
		regwrtback[i]=false;
	}
}


/************************************���ϼĴ����������****************************************/


/************************************���´�ӡ��Ϣ���******************************************/

void printtables()
{
	int i;
	fprintf(ptab,"*************************************tab***********************************\n");
	fprintf(ptab,"\tident\tlink\t   obj\t      typ\tref\tnrm\tlev\tadr\t\n");
	for (i=1;i<=t;i++)
	{
		fprintf(ptab,"%d\t%s\t%d\t%s    %s\t%d\t%d\t%d\t%d\n",
			i,tab[i].name,tab[i].link,objword[tab[i].obj],typword[tab[i].typ],tab[i].ref,tab[i].normal,tab[i].lev,tab[i].adr);
	}
	fprintf(ptab,"***************************************************************************\n\n");
	fprintf(ptab,"****************btab****************\n");
	fprintf(ptab,"blocks\tlast\tlpar\tpsze\tvsze\n");
	for (i=1;i<=b;i++)
		fprintf(ptab,"%d\t%d\t%d\t%d\t%d\n",i,btab[i].last,btab[i].lastpar,btab[i].psize,btab[i].vsize);

	fprintf(ptab,"************************************\n\n");
	fprintf(ptab,"********atab********\n");
	fprintf(ptab,"atab\teltyp\tsize\n");
	for (i=1;i<=a;i++)
		fprintf(ptab,"%d\t%s\t%d\n",i,typword[atab[i].eltyp],atab[i].size);
	fprintf(ptab,"********************\n");
}

void listcode(order code[][cmax],FILE *pcode)
{
	int i,j;
	fprintf(pcode,"  No. \tOpCode\t\tSrc1\t\tSrc2\t\tDst\n");
	for (i=1;i<=cicnt;i++)
	{
		for (j=0;code[i][j].op!=NOP;j++)
		{
			const int src1=code[i][j].src1,src2=code[i][j].src2,dst=code[i][j].dst;
			fprintf(pcode,"%4d\t%s\t",j,fctword[code[i][j].op]);
			switch (code[i][j].op)
			{
			case ENTER:
			case LEAVE:
					fprintf(pcode,"\t\t\t\t%s\n",dst!=0?tab[dst].name:"main");
				break;
			case SETLAB:
				fprintf(pcode,"\t\t\t\tlab%d\n",dst);
				break;
			case NEG:
			case MOV:
				if (src1>0)
				{
					if (variable==tab[src1].obj)
						fprintf(pcode,"%s\t\t\t\t",tab[src1].name);
					else if (konstant==tab[src1].obj)
						fprintf(pcode,"%d\t\t\t\t",tab[src1].adr);
				}
				else
					fprintf(pcode,"mem[%d]\t\t\t\t",-src1);
				if (dst>0)
				{
					fprintf(pcode,"%s\n",tab[dst].name);
				}
				else
					fprintf(pcode,"mem[%d]\n",-dst);
				break;
			case ADD:
			case SUB:
			case IMUL:
			case IDIV:
				if (src1>0)
					fprintf(pcode,"%s\t\t",tab[src1].name);
				else if (src1<0)
					fprintf(pcode,"mem[%d]\t\t",-src1);
				else
					fprintf(pcode,"\t\t");
				if (src2>0)
					fprintf(pcode,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pcode,"  mem[%d]\t",-src2);
				else
					fprintf(pcode,"\t\t");
				if (dst>0)
					fprintf(pcode,"%s\n",tab[dst].name);
				else if (dst<0)
					fprintf(pcode,"mem[%d]\n",-dst);
				else
					fprintf(pcode,"\t\t\n");
				break;
			case JE:
			case JNE:
			case JL:
			case JLE:
			case JG:
			case JGE:
				if (src1>0)
				{
					if (variable==tab[src1].obj)
						fprintf(pcode,"%s\t\t",tab[src1].name);
					else if (konstant==tab[src1].obj)
						fprintf(pcode,"%d\t\t",tab[src1].adr);
				}
				else if (src1<0)
					fprintf(pcode,"mem[%d]\t\t",-src1);
				else
					fprintf(pcode,"\t\t");
				if (src2>0)
				{
					if (variable==tab[src2].obj)
						fprintf(pcode,"  %s\t\t",tab[src2].name);
					else if (konstant==tab[src2].obj)
						fprintf(pcode,"  %d\t\t",tab[src2].adr);
				}
				else if (src2<0)
					fprintf(pcode,"  mem[%d]\t",-src2);
				else
					fprintf(pcode,"\t\t");
				fprintf(pcode,"lab%d\n",dst);
				break;
			case JMP:
				fprintf(pcode,"\t\t\t\tlab%d\n",dst);
				break;
			case CALL:	/* �������� */
				fprintf(pcode,"\t\t\t\t%s\n",tab[dst].name);
				break;
			case RET:
				if (dst<0)
					fprintf(pcode,"retval\t\t\t\tmem[%d]\n",-dst);
				else
					fprintf(pcode,"retval\t\t\t\t%s\n",tab[dst].name);
				break;
			case READ:
				fprintf(pcode,"\t\t\t\t%s\n",tab[dst].name);
				break;
			case WRITE:
				if (1==src1||5==src1||6==src1)	/* Ҫ����ַ��� */
					fprintf(pcode,"\t\t\t\t\"%s\"%c",stab[src2],1==src1?'\n':',');
				if (src1>1)
				{
					if (dst>0)
					{
						if (src1!=5||src1!=6) fprintf(pcode,"\t\t\t\t");
						if (konstant==tab[dst].obj)			
						{
							if (ints==tab[dst].typ)
								fprintf(pcode,"%d\n",tab[dst].adr);
							else if (chars==tab[dst].typ)
								fprintf(pcode,"%c\n",tab[dst].adr);
						}
						else 
							fprintf(pcode,"%s\n",tab[dst].name);
					}
					else
						fprintf(pcode,"\t\t\t\tmem[%d]\n",-dst);
				}
				
				break;
			case INC:
			case DEC:
				fprintf(pcode,"\t\t\t\t%s\n",tab[dst].name);
				break;
			case LOADARRAY:	/* ��������Ԫ�� dst=src1[src2] */
				fprintf(pcode,"%s",tab[src1].name);
				if (src2>0)
				{
					if (konstant==tab[src2].obj)
						fprintf(pcode,"[%d]\t\t\t\t",tab[src2].adr);
					else
						fprintf(pcode,"[%s]\t\t\t\t",tab[src2].name);
				}
				else
					fprintf(pcode,"[mem[%d]]\t\t\t",-src2);
				if (dst>0)	
					fprintf(pcode,"%s\n",tab[dst].name);
				else
					fprintf(pcode,"mem[%d]\n",-dst);
				break;
			case STOARRAY:	/* ���鸳ֵ dst[src1]=src2 */
				if (src2>0)	
					fprintf(pcode,"%s\t\t\t\t",tab[src2].name);
				else
					fprintf(pcode,"mem[%d]\t\t\t\t",-src2);
				fprintf(pcode,"%s",tab[dst].name);
				if (src1>0)
				{
					if (konstant==tab[src1].obj)
						fprintf(pcode,"[%d]\n",tab[src1].adr);
					else
						fprintf(pcode,"[%s]\n",tab[src1].name);
				}
				else
					fprintf(pcode,"[mem[%d]]\n",-src1);
				break;
			case PUSH:
				if (0==src1)	/* ֵ�β���ջ */
				{
					if(dst>0)
						fprintf(pcode,"\t\t\t\t%s\n",tab[dst].name);
					else
						fprintf(pcode,"\t\t\t\tmem[%d]\n",-dst);
				}
				else if (1==src1)	/* �����βε�ַ��ջ */
				{
					if(dst>0)
						fprintf(pcode,"\t\t\t\taddr %s\n",tab[dst].name);
					else
						fprintf(pcode,"\t\t\t\taddr mem[%d]\n",-dst);
				}
				else if (2==src1)
				{
					fprintf(pcode,"\n");

				}
				break;
			default:

				break;
			}
		}
		fprintf(pcode,"\n\n");
	}
}


void gencmp(int src1,int src2)	/* ����cmpָ���õ��϶࣬����дһ������ */
{
	int regsrc1,regsrc2;
	regsrc1=locreg(src1);
	if (regsrc1<0)
	{
		regsrc1=allocatereg(0,src2,src1);
		mem2reg(src1,regsrc1);
		regwrtback[regsrc1]=false;
	}
	regsrc2=locreg(src2);
	if (regsrc2<0)
	{
		if (src2>0&&konstant==tab[src2].obj)
			fprintf(pasm,"\tcmp\t%s,%d\n",regword[regsrc1],tab[src2].adr);
		else
		{
			regsrc2=allocatereg(0,src1,src2);
			mem2reg(src2,regsrc2);
			fprintf(pasm,"\tcmp\t%s,%s\n",regword[regsrc1],regword[regsrc2]);
		}
	}
	else
		fprintf(pasm,"\tcmp\t%s,%s\n",regword[regsrc1],regword[regsrc2]);	
	clearreg();
}

void interpret(const order code[][cmax],int cicnt)		/* ���ɻ����� */
{
	int i,j,k;
	fprintf(pasm,".386\n");
	fprintf(pasm,".model flat,stdcall\n");
	fprintf(pasm,"option casemap:none\n");
	fprintf(pasm,"include masm32\\include\\io32.inc\n");
	fprintf(pasm,"include masm32\\include\\masm32.inc\n");
	fprintf(pasm,"includelib masm32\\lib\\masm32.lib\n");
	fprintf(pasm,".data\n");
	fprintf(pasm,"szPause\tbyte\t\"Press Enter to continue\",0\n");
	for(i=0;i<=sx;i++)	/* ���ַ�����д��data�� */
	{
        fprintf(pasm,"@string%d\tbyte\t\"%s\",0\n",i,stab[i]);
    }
	fprintf(pasm,".code\n");
	fprintf(pasm,"start:\n");
	for (i=1;i<=cicnt;i++)
	{
		allocateglobalreg(code[i]);
		for (j=0;code[i][j].op!=NOP;j++)
		{
			int src1=code[i][j].src1,src2=code[i][j].src2,dst=code[i][j].dst;
			const fct op=code[i][j].op;
			int regsrc1,regsrc2,regdst;
			level=codelevel[i];
			switch(op)
			{
			case ENTER:
				fprintf(pasm,";ENTER\t\t\t%s\n",dst!=0?tab[dst].name:"main");	/* ע�� */
				if (0==dst)	
					fprintf(pasm,"@@main:\n");
				else
					fprintf(pasm,"@@%s:\n",tab[dst].name);
				fprintf(pasm,"\tpush\tebp\n");		/* ����ԭջ�� */
				fprintf(pasm,"\tmov\tebp,esp\n");	/* ��ebp ���ڵ�ǰջ�� */
				//fprintf(pasm,"\tenter\n");	/* һ��enter�����滻�������� */

				
 				fprintf(pasm,"\tpush\tebx\n");	/* ȫ�ּĴ���EBX��ջ */
 				fprintf(pasm,"\tpush\tesi\n");	/* ȫ�ּĴ���ESI��ջ */
 				fprintf(pasm,"\tpush\tedi\n");	/* ȫ�ּĴ���EDI��ջ */

				if (0!=tab[dst].lev)		/* ���˵�һ��ĳ���������Ҫ�ѵ������ķֳ����ebp��eax����ջ */
					fprintf(pasm,"\tpush\teax\n");	
				if (function==tab[dst].obj)
					fprintf(pasm,"\tsub\tesp,4\t;function result\n");
				for (k=btab[src1].lastpar-dst;k>=1;k--)	/* ������ջ����Ϊѹջ˳��ԭ�����ԴӺ���ǰ */
				{
					fprintf(pasm,"\tmov\teax,[ebp+%d]\n",4+k*4);	/* ��eax���ݡ�+4����������ʱѹ��ķ��ص�ַ */
					fprintf(pasm,"\tpush\teax\n");
				}
				fprintf(pasm,"\tsub\tesp,%d\n",0==dst?(btab[src1].vsize-4)*4:(btab[src1].vsize-btab[src1].psize)*4);	/* Ϊ�ֲ���������ʱ��������ռ� */
				break;
			case LEAVE:
				fprintf(pasm,";LEAVE\t\t\t%s\n",dst!=0?tab[dst].name:"main");/* ע�� */
				for (k=0;k<tmpregnum;k++)
				{
					if (regpool[k]>0&&regwrtback[k])/* �뿪��ʱ����ʱ�������ô��� */
						reg2mem(k);
					regpool[k]=0;
					regwrtback[k]=false;
				}
				if (function==tab[dst].obj)	/* �Ӻ�������ʱҪ�ѽ����eax���ݻ�ȥ */
					fprintf(pasm,"\tmov\teax,[ebp-20]\t;return result\n");
			
				fprintf(pasm,"\tmov\tebx,[ebp-4]\n");	/* ȫ�ּĴ���EBX��ջ */
				fprintf(pasm,"\tmov\tesi,[ebp-8]\n");	/* ȫ�ּĴ���ESI��ջ */
				fprintf(pasm,"\tmov\tedi,[ebp-12]\n");	/* ȫ�ּĴ���EDI��ջ */
				fprintf(pasm,"\tmov\tesp,ebp\n");	/* �ָ�ԭջ�� */
				fprintf(pasm,"\tpop\tebp\n");		/* �ָ�ԭջ�� */
				//fprintf(pasm,"\tleave\n");	/* һ��leave�����滻�������� */	
				if (0==dst)	/* �����򷵻� */
					fprintf(pasm,"\tWriteString\tszPause\n\tinvoke StdIn, addr szPause,0\n");
				fprintf(pasm,"\tret\n");
				break;
			case SETLAB:
				fprintf(pasm,";SETLAB\t\t\t\tlab%d\n",dst);	/* ע�� */
				clearreg();	/* �ⲽ��ʵ�Ǹ��߱������ģ�������̲�����ִ�й��̣���������������أ��±�ŵ��ڽ����¹��� */
				fprintf(pasm,"@@lab%d:\n",dst);
				break;
			case NEG:	/* NEG src1  dst ��src1�ĸ��������dst */
				fprintf(pasm,";NEG\t");/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else
					fprintf(pasm,"mem[%d]\t\t",-src1);
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else
					fprintf(pasm,"mem[%d]\n",-dst);

				regdst=locreg(dst);	/* ��Ŀ�Ĳ������ڲ��ڼĴ����� */
				if (regdst<0)	/* ���ڣ����� */
				{
					regdst=allocatereg(0,src1,dst);
					if (src1==dst)	/* Ŀ�Ĳ�����������ʱ��Ŀ�Ĳ�����ԭ����ֵ����Ĵ��� */
						mem2reg(dst,regdst);
				}
				regwrtback[regdst]=true;	/* Ŀ�Ĳ����������ı䣬��д�ر�־Ϊtrue */
				regsrc1=locreg(src1);	/* ��Դ�������ڲ��ڼĴ����� */
				if (regsrc1<0)
				{
					regsrc1=allocatereg(0,dst,src1);
					mem2reg(src1,regsrc1);	/* Դ����������д��Ĵ��� */
				}
				
				if(dst!=src1)	/* ����������ʱ��Ҫ��Դ�������ȸ�ֵ��Ŀ�Ĳ����� */
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);
				fprintf(pasm,"\tneg\t%s\n",regword[regdst]);
				break;
			case MOV:	/* ��ֵ���dst=src1 */
				fprintf(pasm,";MOV\t");		/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else
					fprintf(pasm,"mem[%d]\t\t",-src1);
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else
					fprintf(pasm,"mem[%d]\n",-dst);		/* ע���� */
				
				regdst=locreg(dst);	/* ��Ŀ�Ĳ������ڲ��ڼĴ����� */
				if (regdst<0)	/* ���ڣ���Ŀ�Ĳ���������Ĵ��� */
					regdst=allocatereg(0,src1,dst);
				regwrtback[regdst]=true;	/* Ŀ��ֵҪ���ı��ˣ���д�ر�־��Ϊtrue */
				
				if (src1>0&&konstant==tab[src1].obj)	/* Դ�������ǳ��� */
				{
					fprintf(pasm,"\tmov\t%s,%d\n",regword[regdst],tab[src1].adr);
					break;
				}
				regsrc1=locreg(src1);	/* ��Դ�������ڲ��ڼĴ����� */
				if (regsrc1<0)	/* ���ڣ���Դ����������Ĵ��� */
				{
					regsrc1=allocatereg(0,dst,src1);	/* �����µļĴ��� */
					mem2reg(src1,regsrc1);	/* ��Դ���������ڴ�����Ĵ��� */
				}
				if (regsrc1!=regdst)
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);
				break;
			case ADD:	/* �ӷ�����dst=src1+src2 */
				fprintf(pasm,";ADD\t");		/* ע�� */
				if (src1>0)	
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)	
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else	
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else if (dst<0)
					fprintf(pasm,"mem[%d]\n",-dst);
				else
					fprintf(pasm,"\t\t\n");		/* ע���� */

				regdst=locreg(dst);	/* ��Ŀ�Ĳ������ڲ��ڼĴ����� */
				if (regdst<0)	/* ���ڣ��������� */
				{
					regdst=allocatereg(src1,src2,dst);
					if (dst==src1||dst==src2)	/* �ԼӲ��� */
						mem2reg(dst,regdst);	/* ��dst���ڴ��е���Ĵ������������ֱ��add dst,src1��add dst,src2 */
				}
				regwrtback[regdst]=true;	/* �����޸ģ�д�ر�־��1 */
				if (dst==src2)	/* ������dst=x+dst����ʽ���������dst=dst+x��״ */
				{
					int tmp=src1;
					src1=src2;
					src2=tmp;
				}
				regsrc1=locreg(src1);	/* ��Դ������1�ڲ��ڼĴ����� */
				if (regsrc1<0)	/* ���ڣ�����Ĵ�����д�뵽�Ĵ��� */
				{
					regsrc1=allocatereg(src2,dst,src1);
					mem2reg(src1,regsrc1);
				}
				regsrc2=locreg(src2);	/* ��Դ������2�ڲ��ڼĴ����� */
				if (regsrc2<0)	/* ���ڣ�����Ĵ�����д�뵽�Ĵ��� */
				{
					regsrc2=allocatereg(src1,dst,src2);
					mem2reg(src2,regsrc2);
				}
				if (dst!=src2)		/* �����Լӣ�������dst=x+y����ʽ */
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);	/* ��dst=src1������ֱ��add dst,src2 */
				fprintf(pasm,"\tadd\t%s,%s\n",regword[regdst],regword[regsrc2]);
				break;
			case SUB:	/* ��������dst=src1-src2��ע�⿼��dst=src1��dst=src2����� */
				fprintf(pasm,";SUB\t");	/* ע�� */
				if (src1>0)	
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)	
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else	
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else if (dst<0)
					fprintf(pasm,"mem[%d]\n",-dst);
				else
					fprintf(pasm,"\t\t\n");

				regdst=locreg(dst);	/* ��Ŀ�Ĳ������ڲ��ڼĴ����� */
				if (regdst<0)	/* ���ڣ��������� */
				{
					regdst=allocatereg(src1,src2,dst);
					if (dst==src1||dst==src2)	/* �Լ������������� */
						mem2reg(dst,regdst);	/* ��dst���ڴ��е���Ĵ������������ֱ��sub dst,src1��sub dst,src2 */
				}
				regwrtback[regdst]=true;	/* �����޸ģ�д�ر�־��1 */

				regsrc1=locreg(src1);	/* ��Դ������1�ڲ��ڼĴ����� */
				if (regsrc1<0)	/* ���ڣ�����Ĵ�����д�뵽�Ĵ��� */
				{
					regsrc1=allocatereg(src2,dst,src1);
					mem2reg(src1,regsrc1);
				}
				regsrc2=locreg(src2);	/* ��Դ������2�ڲ��ڼĴ����� */
				if (regsrc2<0)	/* ���ڣ�����Ĵ�����д�뵽�Ĵ��� */
				{
					regsrc2=allocatereg(src1,dst,src2);
					mem2reg(src2,regsrc2);
				}
				if (dst==src1)		/* �Լ�������������dst=dst-x����ʽ */
					fprintf(pasm,"\tsub\t%s,%s\n",regword[regdst],regword[regsrc2]);	/* �������ǰ���Ѿ�������dst */
				else if (dst==src2)	/* ����dst=x-dst����ʽ */
				{
					fprintf(pasm,"\tsub\t%s,%s\n",regword[regdst],regword[regsrc2]);	/* ����dst=dst-x */
					fprintf(pasm,"\tneg\t%s\n",regword[regdst]);	/* �� */
				}
				else	/* ����dst=x-y����ʽ */
				{
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);	/* dst=x */
					fprintf(pasm,"\tsub\t%s,%s\n",regword[regdst],regword[regsrc2]);	/* dst=dst-y����dst=x-y */
				}
				break;
			case IMUL:	/* �˷��ͼӷ����ƣ����Ǹĸ�����dst=src1*src2 */
				fprintf(pasm,";IMUL\t");
				if (src1>0)	
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)	
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else	
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else if (dst<0)
					fprintf(pasm,"mem[%d]\n",-dst);
				else
					fprintf(pasm,"\t\t\n");

				regdst=locreg(dst);	/* ��Ŀ�Ĳ������ڲ��ڼĴ����� */
				if (regdst<0)	/* ���ڣ��������� */
				{
					regdst=allocatereg(src1,src2,dst);
					if (dst==src1||dst==src2)	/* �Գ˲��� */
						mem2reg(dst,regdst);	/* ��dst���ڴ��е���Ĵ������������ֱ��imul dst,src1��imul dst,src2 */
				}
				regwrtback[regdst]=true;	/* �����޸ģ�д�ر�־��1 */
				if (dst==src2)	/* ������dst=x*dst����ʽ���������dst=dst*x��״ */
				{
					int tmp=src1;
					src1=src2;
					src2=tmp;
				}
				regsrc1=locreg(src1);	/* ��Դ������1�ڲ��ڼĴ����� */
				if (regsrc1<0)	/* ���ڣ�����Ĵ�����д�뵽�Ĵ��� */
				{
					regsrc1=allocatereg(src2,dst,src1);
					mem2reg(src1,regsrc1);
				}
				regsrc2=locreg(src2);	/* ��Դ������2�ڲ��ڼĴ����� */
				if (regsrc2<0)	/* ���ڣ�����Ĵ�����д�뵽�Ĵ��� */
				{
					regsrc2=allocatereg(src1,dst,src2);
					mem2reg(src2,regsrc2);
				}
				if (dst!=src2)		/* �����Գˣ�������dst=x*y����ʽ */
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);	/* ��dst=src1������ֱ��add dst,src2 */
				fprintf(pasm,"\timul\t%s,%s\n",regword[regdst],regword[regsrc2]);
				break;
			case IDIV:	/* ��������dst=src1/src2��ע�⿼��dst=src1��dst=src2����� */
				fprintf(pasm,";IDIV\t");/* ע�� */
				if (src1>0)	
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)	
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else	
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else if (dst<0)
					fprintf(pasm,"mem[%d]\n",-dst);
				else
					fprintf(pasm,"\t\t\n");	/* ע���� */

				regdst=locreg(dst);	/* ��Ŀ�Ĳ������ڲ��ڼĴ����� */
				if (regdst>=0)	/* dst�ڼĴ����У��ͷ� */
				{
					regwrtback[regdst]=false;
					regpool[regdst]=0;
				}
				if (regpool[EAX]!=0&&regwrtback[EAX])	/* ��eax�е����ݴ���ڴ� */
					reg2mem(EAX);

				regdst=EAX;	/* ��eax�����dst */
				regpool[EAX]=dst;
				regwrtback[EAX]=true;
				if (regpool[EDX]!=0&&regwrtback[EDX])	/* ������ı�edx */
				{
					reg2mem(EDX);
					regpool[EDX]=0;
					regwrtback[EDX]=false;
				}
				if (dst==src1)	/* ����dst=dst/src2��ʽ�� */
				{
					mem2reg(dst,EAX);	/* ��dst����eax */
					regsrc2=locreg(src2);
					if (regsrc2>=0)
					{
						if (regwrtback[regsrc2])
							reg2mem(regsrc2);
						regpool[regsrc2]=0;
						regwrtback[regsrc2]=false;
					}
					if (regpool[ECX]!=0&&regwrtback[ECX])	/* ��ecx�е����ݴ���ڴ� */
						reg2mem(ECX);
					mem2reg(src2,ECX);	/* ��src1����ecx */
					regpool[ECX]=0;
					regwrtback[ECX]=false;
					fprintf(pasm,"\tcdq\n");
					fprintf(pasm,"\tidiv\tecx\n");	/* �����eax=dst/src2 */
				}
				else if (dst==src2)	/* ����dst=src1/dst��ʽ�� */
				{
					mem2reg(dst,EAX);	/* ��dst����eax */
					regsrc1=locreg(src1);
					if (regsrc1>=0)
					{
						if (regwrtback[regsrc1])
							reg2mem(regsrc1);
						regpool[regsrc1]=0;
						regwrtback[regsrc1]=false;
					}
					if (regpool[ECX]!=0&&regwrtback[ECX])	/* ��ecx�е����ݴ���ڴ� */
						reg2mem(ECX);
					mem2reg(src1,ECX);	/* ��src1����ecx */
					regpool[ECX]=0;
					regwrtback[ECX]=false;
					fprintf(pasm,"\tpush\teax\n");	/* ����eaxֵ */
					fprintf(pasm,"\tmov\teax,ecx\n");	/* ��һ����eax=src1 */
					fprintf(pasm,"\tpop\tecx\n");	/* ��һ����ecx=dst */
					fprintf(pasm,"\tcdq\n");
					fprintf(pasm,"\tidiv\tecx\n");	/* ��ʱeax=src1/dst */
					
				}
				else	/* ����dst=src1/src2��ʽ�� */
				{
					regsrc1=locreg(src1);
					if (regsrc1>=0)
					{
						if (regwrtback[regsrc1])
							reg2mem(regsrc1);
						regpool[regsrc1]=0;
						regwrtback[regsrc1]=false;
					}
					if (regpool[EDX]!=0&&regwrtback[EDX])	/* ��edx�е����ݴ���ڴ� */
						reg2mem(EDX);
					mem2reg(src1,EDX);	/* ��src1����edx */
					regpool[EDX]=0;
					regwrtback[EDX]=false;

					regsrc2=locreg(src2);
					if (regsrc2>=0)
					{
						if (regwrtback[regsrc2])
							reg2mem(regsrc2);
						regpool[regsrc2]=0;
						regwrtback[regsrc2]=false;
					}
					if (regpool[ECX]!=0&&regwrtback[ECX])	/* ��ecx�е����ݴ���ڴ� */
						reg2mem(ECX);
					mem2reg(src2,ECX);	/* ��src2����ecx */
					regpool[ECX]=0;
					regwrtback[ECX]=false;
					fprintf(pasm,"\tmov\teax,edx\n");
					fprintf(pasm,"\tcdq\n");
					fprintf(pasm,"\tidiv\tecx\n");
				}
				break;
			case JE:
				fprintf(pasm,";JE\t");		/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				fprintf(pasm,"lab%d\n",dst);

				gencmp(src1,src2);
				fprintf(pasm,"\tje\t@@lab%d\n",dst);
				break;
			case JNE:
				fprintf(pasm,";JNE\t");		/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				fprintf(pasm,"lab%d\n",dst);

				gencmp(src1,src2);
				fprintf(pasm,"\tjne\t@@lab%d\n",dst);
				break;
			case JL:
				fprintf(pasm,";JL\t");		/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				fprintf(pasm,"lab%d\n",dst);
				gencmp(src1,src2);
				fprintf(pasm,"\tjl\t@@lab%d\n",dst);
				break;
			case JLE:
				fprintf(pasm,";JLE\t");		/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				fprintf(pasm,"lab%d\n",dst);
				gencmp(src1,src2);
				fprintf(pasm,"\tjle\t@@lab%d\n",dst);
				break;
			case JG:
				fprintf(pasm,";JG\t");		/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				fprintf(pasm,"lab%d\n",dst);
				gencmp(src1,src2);
				fprintf(pasm,"\tjg\t@@lab%d\n",dst);
				break;
			case JGE:
				fprintf(pasm,";JGE\t");		/* ע�� */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else if (src1<0)
					fprintf(pasm,"mem[%d]\t\t",-src1);
				else
					fprintf(pasm,"\t\t");
				if (src2>0)
					fprintf(pasm,"  %s\t\t",tab[src2].name);
				else if (src2<0)
					fprintf(pasm,"  mem[%d]\t",-src2);
				else
					fprintf(pasm,"\t\t");
				fprintf(pasm,"lab%d\n",dst);
				gencmp(src1,src2);
				fprintf(pasm,"\tjge\t@@lab%d\n",dst);
				break;
			case JMP:
				clearreg();	/* ԭ��ͬsetlab����ע�ͣ�jmp�൱���뿪���̡���ס������̲�����ִ�й��� */
				fprintf(pasm,";JMP\tlab%d\n",dst);
				fprintf(pasm,"\tjmp\t@@lab%d\n",dst);
				break;
			case CALL:
				fprintf(pasm,";CALL\t\t%s\n",tab[dst].name);	/* ע�� */
				clearreg();
				codelevel[i]--;/* ������ж���Ե����������̣����ǵ�ǰ����Ĳ㼶�������ڵĹ���Ҫ��1�������ȼ����������ټӻ��� */
				if (codelevel[i]+1==tab[dst].lev)	/* ���ù����Ǳ����ù��̵�ֱ���ϲ� */
					fprintf(pasm,"\tmov\teax,ebp\n");	/* ��eax����ebp */
 				else if (codelevel[i]>=tab[dst].lev)
				{
					fprintf(pasm,"\tmov\teax,[ebp-16]\n");/* ��level-1���ebp */
					for(k=codelevel[i];k>tab[dst].lev;k--)	
						fprintf(pasm,"\tmov\teax,[eax-16]\n");	/* �������ϲ�ebp */
				}
				codelevel[i]++;	/* ��ǰ������ļӻ��� */
				fprintf(pasm,"\tcall\t@@%s\n",tab[dst].name);
				break;
			case RET:
				if (dst<0)	/* ע�� */
					fprintf(pasm,";RET\teax\t\tmem[%d]\n",-dst);
				else
					fprintf(pasm,";RET\teax\t\t%s\n",tab[dst].adr);
			
				regdst=locreg(dst);
				if (regdst<0)
				{
					regdst=allocatereg(0,0,dst);
					regwrtback[regdst]=true;
				}
				if (regdst!=EAX)	/* ��regdst�Ѿ�������eax�����贫�� */
					fprintf(pasm,"\tmov\t%s,eax\n",regword[regdst]);
				break;
			case READ:
				fprintf(pasm,";Read\t%s\n",tab[dst].name);
				
				if (regpool[EAX]!=0&&regwrtback[EAX])
					reg2mem(EAX);
				regpool[EAX]=0;regwrtback[EAX]=false;
				if (tab[dst].typ==ints)
					fprintf(pasm,"\tReadSDecDword\teax\n");
				else if (tab[dst].typ==chars)
					fprintf(pasm,"\tReadChar\tal\n");

				regdst=locreg(dst);
				if (regdst>=0)
				{
					fprintf(pasm,"\tmov\t%s,eax\n",regword[regdst]);
					regwrtback[regdst]=true;
				}
				else
				{
					regpool[EAX]=dst;
					reg2mem(EAX);
					regpool[EAX]=0;
				}
				break;
			case WRITE:
				fprintf(pasm,";WRITE\t\t");	/* ע�� */
				if (1==src1||5==src1||6==src1)	/* Ҫ����ַ��� */
					fprintf(pasm,"\"%s\"%c",stab[src2],1==src1?'\n':',');
				if (src1>1)
				{
					if (dst>0)
					{
						if (1==src1)
							fprintf(pasm,"\t\t\t");
						if (konstant==tab[dst].obj)			
						{
							if (ints==tab[dst].typ)
								fprintf(pasm,"%d\n",tab[dst].adr);
							else if (chars==tab[dst].typ)
								fprintf(pasm,"%c\n",tab[dst].adr);
						}
						else 
							fprintf(pasm,"%s\n",tab[dst].name);
					}	
					else
						fprintf(pasm,"\t\t\t\tmem[%d]\n",-dst);	/* ע���� */
				}
			
				if (1==src1||5==src1||6==src1)		/* ����ַ��� */
				{
					fprintf(pasm,"\tWriteString\t@string%d\n",src2);
					if (1==src1) fprintf(pasm,"\tWriteCrlf\n");	/* ��������ַ���ʱ���� */
				}
				if (src1>1)	/* ������ʽ */
				{
					if (dst>0&&konstant==tab[dst].obj)	/* ����ֱ����� */
					{
						if(ints==tab[dst].typ)
							fprintf(pasm,"\tWriteSDecDword\t\t%d\n",tab[dst].adr);
						else if(chars==tab[dst].typ)
							fprintf(pasm,"\tWriteChar\t\t%d\n",tab[dst].adr);
					}
					else 
					{
						regdst=locreg(dst);
						if (regdst<0)
						{
							regdst=allocatereg(0,0,dst);
							mem2reg(dst,regdst);
						}
						if (3==src1||5==src1)	/* ������� */
							fprintf(pasm,"\tWriteSDecDword\t%s\n",regword[regdst]);
						else if (4==src1||6==src1)	/* ����ַ� */
							fprintf(pasm,"\tWriteChar\t%cl\n",regword[regdst][1]);
					}

					fprintf(pasm,"\tWriteCrlf\n");
				}

				break;
			case LOADARRAY:		/* �������� dst=src1[src2] */	/* �ݲ�֧�ֿ纯���������� */
				fprintf(pasm,";LOADARRY\t");	/* ע�� */
				fprintf(pasm,"%s",tab[src1].name);
				if (src2>0)
				{
					if (konstant==tab[src2].obj)
						fprintf(pasm,"[%d]\t\t\t\t",tab[src2].adr);
					else
						fprintf(pasm,"[%s]\t\t\t\t",tab[src2].name);
				}
				else
					fprintf(pasm,"[mem[%d]]\t\t\t",-src2);
				if (dst>0)	
					fprintf(pasm,"%s\n",tab[dst].name);
				else
					fprintf(pasm,"mem[%d]\n",-dst);	/* ע���� */

				regdst=locreg(dst);
				if (regdst<0)
					regdst=allocatereg(0,src2,dst);
				regwrtback[regdst]=true;	/* ֵҪ���޸� */
				

				if (src2>0&&konstant==tab[src2].obj)	/* ����Ԫ�ص��±��ǳ��� */
				{
					
					if (tab[src1].lev<level)	/* �������ϲ�ı��� */
					{
						fprintf(pasm,"\tpush\tebp\n");	/* ����ebp */
						for (k=tab[src1].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* ������ebp���� */
					}
					fprintf(pasm,"\tmov\t%s,[ebp-%d]\n",regword[regdst],(tab[src1].adr+tab[src2].adr)*4);
					if (tab[src1].lev<level)	
						fprintf(pasm,"\tpop\tebp\n");	/* �ָ�ebp */
				}
				else	/* �����±겻�ǳ��� */
				{
					regsrc2=locreg(src2);
					if (regsrc2<0)
					{
						regsrc2=allocatereg(0,dst,src2);
						mem2reg(src2,regsrc2);
					}
					if (regwrtback[regsrc2])	reg2mem(regsrc2);	/* ��Ϊ����Ĳ���Ҫ�ı�Ĵ������ݣ�����û��ĸϽ��� */
					regwrtback[regsrc2]=false;	/* ֻ�Ƕ�ȡsrc2�����ݣ������޸� */

					fprintf(pasm,"\tpush\tebp\n");	/* ����ebp */
					if (tab[src1].lev<level)	/* �������ϲ�ı��� */
					{

						for (k=tab[src1].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* ������ebp���� */
					}

					fprintf(pasm,"\tadd\t%s,%d\n",regword[regsrc2],tab[src1].adr);	/* ����Ԫ��ƫ�Ƶ�ַ */
					fprintf(pasm,"\timul\t%s,4\n",regword[regsrc2]);
					fprintf(pasm,"\tsub\tebp,%s\n",regword[regsrc2]);/* ��λ����Ԫ�� */
					fprintf(pasm,"\tmov\t%s,[ebp]\n",regword[regdst]);
					fprintf(pasm,"\tpop\tebp\n");	/* �ָ�ebp */

					regpool[regsrc2]=0;	/* �ͷ��±�ռ�õļĴ��� */
				}
				break;
			case STOARRAY:	/* Ϊ���鸳ֵ dst[src1]=src2 */
				fprintf(pasm,";STOARRY\t");	/* ע�� */
				if (src2>0)	
					fprintf(pasm,"%s\t\t\t\t",tab[src2].name);
				else
					fprintf(pasm,"mem[%d]\t\t\t\t",-src2);
				fprintf(pasm,"%s",tab[dst].name);
				if (src1>0)
				{
					if (konstant==tab[src1].obj)
						fprintf(pasm,"[%d]\n",tab[src1].adr);
					else
						fprintf(pasm,"[%s]\n",tab[src1].name);
				}
				else
					fprintf(pasm,"[mem[%d]]\n",-src1);		/* ע���� */
				

				regsrc2=locreg(src2);
				if (regsrc2<0)
				{
					regsrc2=allocatereg(0,src1,src2);
					mem2reg(src2,regsrc2);
					regwrtback[regsrc2]=false;	/* ֻ�Ƕ�ȡsrc2�����ݣ������޸� */
				}

				if (src1>0&&konstant==tab[src1].obj)	/* ����Ԫ�ص��±��ǳ��� */
				{
					
					if (tab[dst].lev<level)	/* �������ϲ�ı��� */
					{
						fprintf(pasm,"\tpush\tebp\n");/* ����ebp */
						for (k=tab[dst].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* ������ebp���� */
					}
					fprintf(pasm,"\tmov\t[ebp-%d],%s\n",(tab[dst].adr+tab[src1].adr)*4,regword[regsrc2]);
					if (tab[dst].lev<level)	/* �������ϲ�ı��� */
						fprintf(pasm,"\tpop\tebp\n");/* �ָ�ebp */
				}
				else	/* �����±겻�ǳ��� */
				{
					regsrc1=locreg(src1);
					if (regsrc1<0)
					{
						regsrc1=allocatereg(0,src2,src1);
						mem2reg(src1,regsrc1);

					}
					if (regwrtback[regsrc1])	reg2mem(regsrc1);	/* ��Ϊ����Ĳ���Ҫ�ı�Ĵ������ݣ�����û��ĸϽ��� */
					regwrtback[regsrc1]=false;	/* ֻ�Ƕ�ȡsrc1�����ݣ������޸� */
					
					fprintf(pasm,"\tpush\tebp\n");/* ����ebp */
					if (tab[dst].lev<level)	/* �������ϲ�ı��� */
					{
	
						for (k=tab[dst].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* ������ebp���� */
					}
					fprintf(pasm,"\tadd\t%s,%d\n",regword[regsrc1],tab[dst].adr);	/* ����Ԫ��ƫ�Ƶ�ַ */
					fprintf(pasm,"\timul\t%s,4\n",regword[regsrc1]);
					fprintf(pasm,"\tsub\tebp,%s\n",regword[regsrc1]);/* ��λ����Ԫ�� */
					fprintf(pasm,"\tmov\t[ebp],%s\n",regword[regsrc2]);
					
					fprintf(pasm,"\tpop\tebp\n");	/* �ָ�ebp */
					regpool[regsrc1]=0;	/* �ͷ�regsrc1�����Ҳ�д���ڴ棬Ҳ����д�� */
				}
				
				break;
			case INC:	
				fprintf(pasm,";INC\t%s\n",tab[dst].name);	/* ע�� */
				regdst=locreg(dst);
				if (regdst<0)
				{
					regdst=allocatereg(0,0,dst);
					mem2reg(dst,regdst);
				}
				regwrtback[regdst]=true;
				fprintf(pasm,"\tinc\t%s\n",regword[regdst]);
				break;
			case DEC:
				fprintf(pasm,";DEC\t%s\n",tab[dst].name);	/* ע�� */
				regdst=locreg(dst);
				if (regdst<0)
				{
					regdst=allocatereg(0,0,dst);
					mem2reg(dst,regdst);
				}
				regwrtback[regdst]=true;
				fprintf(pasm,"\tdec\t%s\n",regword[regdst]);
				break;
			case PUSH:
				fprintf(pasm,";PUSH\t");	/* ע�� */
				if (0==src1)	/* ֵ�β���ջ */
				{
					if(dst>0)
						fprintf(pasm,"\t\t%s\n",tab[dst].name);
					else
						fprintf(pasm,"\t\tmem[%d]\n",-dst);
				}
				else if (1==src1)	/* �����βε�ַ��ջ */
				{
					if(dst>0)
						fprintf(pasm,"\t\taddr %s\n",tab[dst].name);
					else
						fprintf(pasm,"\t\taddr mem[%d]\n",-dst);
				}
				else if (2==src1)
				{
					fprintf(pasm,"\n");
				}
				/* ע���� */

	
				if (0==src1)	/* ֵ�β���ջ */
				{
					regdst=locreg(dst);
					if (regdst<0)
					{
						regdst=allocatereg(0,0,dst);
						mem2reg(dst,regdst);
					}
					fprintf(pasm,"\tpush\t%s\n",regword[regdst]);
				}
				else if (1==src1)	/* �����βε�ַ��ջ */
				{
					if (regpool[EAX]!=0&&regwrtback[EAX])
					{
						reg2mem(EAX);
						regpool[EAX]=0;
						regwrtback[EAX]=false;
					}
					fprintf(pasm,"\tpush\tebp\n");
					if (tab[dst].lev<level)	/* ���ϲ�ı��� */
					{
						for (k=tab[dst].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* ������ebp���� */
					}
					fprintf(pasm,"\tlea\teax,[ebp-%d]\n",4*tab[dst].adr);	/* �ѱ�����ַ��eax */
					
					fprintf(pasm,"\tpop\tebp\n");
					fprintf(pasm,"\tpush\teax\n");	/* ��ַ��ջ */
				}
				else if (2==src1)	/* dst[src2]��ջ */
				{
					fprintf(pasm,"\tpush\tebp\n");

					if (src2>0&&konstant==tab[src2].obj)	/* ����Ԫ�ص��±��ǳ��� */
					{
						if (regpool[EAX]!=0&&regwrtback[EAX])
						{
							reg2mem(EAX);
							regpool[EAX]=0;
							regwrtback[EAX]=false;
						}
						if (tab[dst].lev<level)	/* �������ϲ�ı��� */
						{
							for (k=tab[dst].lev;k<level;k++)
								fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* ������ebp���� */
						}
						fprintf(pasm,"\tlea\teax,[ebp-%d]\n",(tab[dst].adr+tab[src2].adr)*4);
						fprintf(pasm,"\tpop\tebp\n");
						fprintf(pasm,"\tpush\teax\n");
					}
					else	/* �����±겻�ǳ��� */
					{
						regsrc2=locreg(src2);
						if (regsrc2<0)
						{
							int tmp=regpool[EAX];
							regpool[EAX]=101;
							regsrc2=allocatereg(0,regpool[EAX],src2);	/* ������src2�ֵ�eax */
							mem2reg(src2,regsrc2);
							regpool[EAX]=tmp;
						}
						if (regwrtback[regsrc2])	reg2mem(regsrc2);	/* ��Ϊ����Ĳ���Ҫ�ı�Ĵ������ݣ�����û��ĸϽ��� */
						regwrtback[regsrc2]=false;	/* ֻ�Ƕ�ȡsrc2�����ݣ������޸� */
						if (regpool[EAX]!=0&&regwrtback[EAX])
						{
							reg2mem(EAX);
							regpool[EAX]=0;
							regwrtback[EAX]=false;
						}
						if (tab[dst].lev<level)	/* �������ϲ�ı��� */
						{
							for (k=tab[dst].lev;k<level;k++)
								fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* ������ebp���� */
						}
						fprintf(pasm,"\tadd\t%s,%d\n",regword[regsrc2],tab[dst].adr);	/* ����Ԫ��ƫ�Ƶ�ַ */
						fprintf(pasm,"\timul\t%s,4\n",regword[regsrc2]);
						fprintf(pasm,"\tsub\tebp,%s\n",regword[regsrc2]);/* ��λ����Ԫ�� */

						fprintf(pasm,"\tlea\teax,[ebp]\n");
						fprintf(pasm,"\tpop\tebp\n");
						fprintf(pasm,"\tpush\teax\n");
						regpool[regsrc2]=0;	/* �ͷ��±�ռ�õļĴ��� */
					}
					
				}
				break;
			}
		}
		fprintf(pasm,"\n\n");
	}
	fprintf(pasm,"end start\n");
}

/************************************���ϴ�ӡ��Ϣ���******************************************/

int main()
{
	char szmlcmd[260]="ml /c /coff /Cp ";
	char szlinkcmd[260]="linker /subsystem:console ";
	char *inpath=(char*)malloc(260);
	char asmname[260]="asm.asm";
	char objname[260]="asm.obj";
	char exename[260]="asm.exe";
	FILE *pcodeafter=fopen("code_after.txt","w");
	symset fsys;
	execute=true;
	prtables=true;
	printf("---------------------------PL/0 Compiler Version 1.0---------------------------\n\n");
	printf("\tCopyright (C) 11061030 LiHechao	2013-2014. All rights reversed.\n\n");
	printf("Please input source file path or drag source file here directly:\n");
	fgets(inpath,256,stdin);
	inpath[strlen(inpath)-1]='\0';/* ȥ���س� */
	if (inpath[0]=='"')	/* ����е�ʱ���Ϲ�������˫���� */
	{
		inpath++;
	}
	if (inpath[strlen(inpath)-1]=='"')
	{
		inpath[strlen(inpath)-1]=0;
	}
	ptab=fopen("table.txt","w");
	psin=fopen(inpath,"r");
	psout=fopen("out.txt","w");
	pcode=fopen("code_before.txt","w");
	pasm=fopen(asmname,"w");
	sps['+']=PLUS;
	sps['-']=MINU;
	sps['*']=MULT;
	sps['/']=DIV;
	sps['(']=LPARENT;
	sps[')']=RPARENT;
	sps['=']=EQL;
	sps[',']=COMMA;
	sps['[']=LBRACK;
	sps[']']=RBRACK;
	sps[';']=SEMICN;
	sps['.']=PERIOD;
	initset(&constbegsys);
	initset(&typebegsys);
	initset(&blockbegsys);
	initset(&facbegsys);
	initset(&statbegsys);
	
	insertelem(PLUS,&constbegsys);
	insertelem(MINU,&constbegsys);
	insertelem(INTCON,&constbegsys);
	insertelem(CHARCON,&constbegsys);
	
	insertelem(INTTK,&typebegsys);
	insertelem(CHARTK,&typebegsys);
	insertelem(ARRAYTK,&typebegsys);
	
	insertelem(CONSTTK,&blockbegsys);
	insertelem(VARTK,&blockbegsys);
	insertelem(PROCETK,&blockbegsys);
	insertelem(FUNCTK,&blockbegsys);
	insertelem(BEGINTK,&blockbegsys);
	
	insertelem(INTCON,&facbegsys);
	insertelem(IDEN,&facbegsys);
	insertelem(LPARENT,&facbegsys);
	
	insertelem(BEGINTK,&statbegsys);
	insertelem(IFTK,&statbegsys);
	insertelem(REPTTK,&statbegsys);
	insertelem(FORTK,&statbegsys);
	insertelem(READTK,&statbegsys);
	insertelem(WRITETK,&statbegsys);
	insertelem(IDEN,&statbegsys);
	sx=-1;
	ll=0;
	cc=0;
	ch=' ';
	errpos=0;
	level=1;
	dx=4;/* �����������ĸ��ռ䣺ebp+3��ȫ�ּĴ��� */
	ci=1;cj=0;cicnt=1;
	fsys=setsadd(blockbegsys,statbegsys);
	insymbol();
	enterblock();
	display[level]=1;
	codelevel[ci]=level;
	emit2(ENTER,1,0);
	block(fsys);
	emit2(LEAVE,1,0);
	if (prtables)
	{
		printtables();
	}
	listcode(code,pcode);

	dagoptimaize(code,cicnt);
	peepholeoptimize(optimizedcode,cicnt);
	listcode(optimizedcode,pcodeafter); 
	if (0==errcnt)
	{
		interpret(optimizedcode,cicnt);
		fclose(pasm);
		strcat(szmlcmd,asmname);
		strcat(szlinkcmd,objname);
		printf("\n\nCompiling����\n\n");
		system(szmlcmd);
		printf("\n\nLinking����\n\n");
		system(szlinkcmd);
		if (execute)	/* �ֳ����� */
			system(exename);
	}
	else
	{
		printf("Compiled with errors\n");
		errormsg();
		fclose(psout);
		system("out.txt");
	}
	fclose(psin);
	fclose(pcode);
//	system("pause");
	return 0;
}