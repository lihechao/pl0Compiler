#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "optimize.h"
#include "pl0Compiler.h"
int cistack[lmax],citop=-1,cicnt;
extern order optimizedcode[pmax][cmax];
/********************************以下错误处理相关******************************************/
void errormsg()	/* 打印错误信息的摘要 */
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
	fprintf(psout,"Meaning of error codes：\n");
	for (k=0;k<ermax;k++)
	{
		if (errs[k])
			fprintf(psout,"%2d: %s\n",k,msg[k]);
	}
}

void endskip()		/* 源程序中被跳过的部分下面划线标记 */
{
	while(errpos<cc)	/* 错误位置与当前字符之间是跳过的字符 */
	{
		fprintf(psout,"-");	/* 跳过字符下面划线 */
		errpos++;			/* 划过一个字符 */
	}
	skipflag=false;			/* 跳过标记置为false表示不再跳过 */
}

void error(errcode n)		/* 在当前位置（cc处）打印出错位置和出错编号 */
{
	errcnt++;
	if (0==errpos)			/* 表示新开始的一行要空四个格 */
		fprintf(psout,"****");
	if (cc>errpos)			/* 当前读字符位置在上次打印错误位置之后 */
	{
		int i;
		for (i=0;i<cc-errpos;i++)
			fprintf(psout," ");
		fprintf(psout,"^%2d",n);
		errpos=cc+3;		/* 打印错误的位置到了当前字符后面三个（即跳过了^和占两个位置的n） */
		errs[n]=true;		/* n号错误加入错误集合 */		
	}
	printf("ERROR: line:%d\tcode:%d\n",linecount,n);
}

void fatal(fatalcode n)	/* 打印致命错误（表格溢出信息） */
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

void skip(symset fsys,errcode n)	/* 跳读源程序，直至取来的符号属于给出的符号集为止，并打印出错标记 */
{
	error(n);	/* 打印出错标记 */
	skipflag=true;
	while(!belongset(sy,fsys))	/* 当前符号的类别不在给出的符号集里 */
		insymbol();
	if (skipflag)
		endskip();	/* 停止跳读 */
}

void test(symset s1,symset s2,errcode n)	/* 测试当前符号是否合法，若不合法，打印出错标志并进行跳读 */
{
	if (!belongset(sy,s1))
		skip(setsadd(s1,s2),n);
}

void testsemicolon(symset fsys)	/* 测试当前符号是否为分号 */
{
	if (SEMICN==sy)	/* 当前符号为分号 */
	{
		insymbol();/* 合法，读下一个 */
	}
	else
	{
		error(EXPECTSEMICOLON);
		if (COMMA==sy||COLON==sy)
		{
			insymbol();	/* 在为逗号或冒号时，认为是分号的误写，照样读下一个字符 */
		}
	}
	test(elemaddset(IDEN,blockbegsys),fsys,ILEGALSYM);
}
/********************************以上错误处理相关******************************************/



/********************************以下词法分析相关******************************************/

void switchs(bool *b)	/* 处理编译可选项中的'+'、'-'标志 */
{
	*b=ch=='+';		/* 符号为'+'，则b=true */
	if (ch!='+')
	{
		if (ch!='-')	/* ch也不为'-' */
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

void options()	/* 处理编译时的可选项 */
{
	do 
	{
		nextch();
		if (ch!='*')
		{
			if (ch=='t')	/* 打印表格选项 */
			{
				nextch();
				switchs(&prtables);	/* 判断是'+'还是'-'，结果给prtables */
			}
			else if (ch=='e')
			{
				nextch();
				switchs(&execute);
			}
		}
	} while (ch==',');
}

void nextch()	/* 读取下一字符，处理行结束符，印出被编译的源程序 */
{
	if (cc==ll)	/* 到达一行结束 */
	{
		if (feof(psin))	/* 文件已结束 */
		{
			fputc('\n',psout);
			fprintf(psout,"源程序文件遭遇意外的文件结尾！\n");
			errormsg();		/* 打印错误信息 */
			exit(1);		/* 退出编译 */
		}
		else if (errpos!=0)
		{
			if (skipflag)	/* 跳读标记为真，说明要对跳读部分划线 */
				endskip();
			fputc('\n',psout);
			errpos=0;		/* 打印错误位置置为0 */
		}
		fprintf(psout,"%-4d ",++linecount);
		ll=0;	/* 当前行长度置为0 */
		cc=0;	/* 当前行读到的字符数置为0 */
		ch='\0';
		while (ch!='\n'&&ch!=EOF)
		{	
			ch=fgetc(psin);		/* 读一个字符 */
			if (EOF!=ch)
			fputc(ch,psout);	/* 写到输出文件 */
			line[ll++]=ch;		/* 将字符放入行缓冲区 */
		}
	}
	ch=line[cc++];
}

void insymbol()	/* 读取下一单词符号，处理注释行 */
{
	int low,high,k;
	while(isspace(ch))	/* 跳过空白字符 */
		nextch();
	if (isalpha(ch))	/* 当前读到的字符是字母  <字母>::= a|b|c|d…x|y|z |A|B…|Z */
	{
		k=0;
		memset(id,0,sizeof(id));	/* 标识符清空 */
		do 
		{
			if (k<alng)/* 如果单词长度没有达到最大标识符长度，就继续读取字符；
							否则舍弃长于最大标识符长度的部分 */
				id[k++]=ch;/* 把当前读到的字符拼接到正在分析的标识符后面，
												并将标识符长度加1 */
			nextch();
		} while (isalnum(ch));	/* 根据<标识符>::= <字母>{<字母>|<数字>} */

		low=0;high=nkw-1;
		k=-1;
		while (low<=high)	/* 在保留字表中二分查找id */
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
		sy=k>=0?ksy[k]:IDEN;	/* 如果在保留字表中找到了id则单词类别为该保留字类别，否则为标识符 */
	}
	else if (isdigit(ch))	/* 当前读到的字符是数字  <数字>	::=	0|1|2|3…8|9 */
	{
		k=0;				/* 当前数字位数置为0 */
		inum=0;				/* 最近一次读到的无符号整数置为0 */
		sy=INTCON;	/* 令当前读到的单词的类型为整数常量 */
		while('0'==ch)	/* 跳过前导0 */
		{
			nextch();
		}
		if (isdigit(ch))	/* 如果跳过前导0后还有数字 */
		{
			do 
			{
				inum=10*inum+ch-'0';
				k++;				/* 数字位数加1 */
				nextch();	/* 读下一个字符 */
			} while (isdigit(ch));	/* 根据<无符号整数>	::=	<数字>{<数字>} */
		}
		else	/* 否则只有一位数字0 */
			k=1;
		if (k>kmax||inum>nmax)	/* 读到的数字位数超过了最大允许的数字位数，
				或数字大于了允许的最大数字*/
		{
				error(NUMOVERFLOW);	/* 数字溢出错误 */
				inum=0;
				k=0;
		}
	}
	else if (':'==ch)	/* 当前读到的字符是冒号':' */
	{
		nextch();	/* 读下一个字符，看是不是'=' */
		if ('='==ch)		/* 如果下一个字符是'=' */
		{
			sy=ASSIGN;	/* 单词类型为赋值运算符 */
			nextch();		/* 读下一个字符 */
		}
		else
		{
			sy=COLON;		/* 否则单词类型为冒号 */
		}
	}
	else if ('<'==ch)		/* 当前读到的字符是小于号 */
	{
		nextch();			/* 读下一个字符 */
		if ('='==ch)		/* 如果下一个字符是等号'=' */
		{
			sy=LEQ;		/* 单词类型为小于等于运算符 */
			nextch();		/* 预读下一个字符 */
		}
		else if ('>'==ch)		/* 如果下一个字符是大于号'>' */
		{
			sy=NEQ;		/* 单词类型为不等于运算符 */
			nextch();		/* 预读下一个字符 */
		}
		else
		{
			sy=LSS;		/* 单词类型为小于运算符 */
		}
	}
	else if ('>'==ch)		/* 当前读到的字符是大于号 */
	{
		nextch();			/* 读下一个字符 */
		if ('='==ch)		/* 如果下一个字符是等于号'=' */
		{
			sy=GEQ;		/* 单词类型为大于等于运算符 */
			nextch();		/* 预读下一个字符 */
		}
		else
		{
			sy=GRE;		/* 单词类型为大于运算符 */
		}
	}
	else if ('\''==ch)		/* 当前读到的字符是单引号 */
	{
		nextch();			/* 读下一个字符，看是不是字母或数字
						<字符>	::=	'<字母>' | '<数字>'  */
		if (!isalnum(ch))	/* 下一个字符不是字母或数字 */
		{
			error(WRONGCHARCON);		/* 错误：错误的字符常量 */
			sy=NUL;
		}
		else						/* 下一个字符是字母或数字 */
		{
			inum=ch;	/* 字符的ascii码值 */
			sy=CHARCON;	/* 单词类型为字符常量 */
		}
		nextch();			/* 读下一个字符看是不是单引号' */
		if (ch!='\'')		/* 单个字符后面跟着的不是单引号' */
		{
			error(EXPECTQMARK);	/* 错误：应为单引号' */
		}
		else
			nextch();			/* 预读下一个字符 */
	}
	else if ('"'==ch)		/* 当前读到的字符是双引号" */
	{
		sx++;
		k=0;						/* 字符串长度初始置为0 */
		sy=STRCON;					/* 单词类型为字符串常量 */
		nextch();					/* 读下一个字符 */
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
	else if ('('==ch)	/* 当前字符为左括号 */
	{
		nextch();
		if ('*'!=ch)	/* 不是注释 */
		{
			sy=LPARENT;
		}
		else			/* 是注释，跳过 */
		{
			nextch();
			if ('$'==ch)	/* 是编译可选项 */
				options();
			do 
			{
				while('*'!=ch)	/* 直到找到*，即注释结束标记 */
					nextch();
			} while (ch!=')');	/* 如果*后面不是右括号，就继续找 */
			nextch();
			insymbol();/* 跳过了注释，还要再读下一个单词 */
		}
	}
	else if ('{'==ch)
	{
		nextch();
		if ('$'==ch)	/* 编译可选项 */
		{
			options();
		}
		while('}'!=ch)	/* 跳过注释 */
			nextch();
		nextch();		/* 读注释后的第一个字符 */
		insymbol();		/* 跳过了注释，还要读下一个单词 */
	}
	else if ('+'==ch||'-'==ch||'*'==ch||'/'==ch||','==ch||';'==ch||
		'.'==ch||'='==ch||')'==ch||'['==ch||']'==ch)	
		/* 其他字符 */
	{
		sy=sps[ch];
		nextch();
	}
	else
	{
		error(ILEGALCHAR);
		nextch();
		insymbol();		/* 忽略非法字符，再读下一个单词 */
	}
}
/********************************以上词法分析相关******************************************/

/********************************以下登录表格相关******************************************/

void enter(char id[],objecttype k)	/* 分程序内，在符号表中登录分程序说明部分出现的标识符 */
{
	int i,j;
	if (tmax==t)	/* 符号表溢出 */
		fatal(IDENTIFIER);
	else
	{
		strcpy(tab[0].name,id);/* 将标识符名称暂时保存在符号表的0号位置 */
		i=btab[display[level]].last;/* 令i等于当前分程序中最后一个标识符在符号表中的位置 */
		j=i;
		while(0!=strcmp(tab[i].name,id))
		{
			i=tab[i].link;	/* 不断向上查找id */
		}
		if(i!=0)	/* 说明找到了与id同名的标识符 */
			error(IDENTREDEFINED);
		else	/* 同一层分程序内没有同名标识符 */
		{
			t++;	/* 符号表索引值加1 */
			/* 将标识符对应的信息填入符号表 */
			strcpy(tab[t].name,id);
			tab[t].link=j;		/* 同一层分程序中的上一个标识符 */
			tab[t].obj=k;		/* 种类（常量、变量、过程、函数） */
			tab[t].typ=notyp;	/* 类型先设置为空，后面反填 */
			tab[t].ref=0;
			tab[t].lev=level;	/* 符号的当前层次 */
			tab[t].adr=0;
			tab[t].normal=false;	/* 默认为false */
			/* 将分程序表中最后一个标识符在符号表中的位置更新为当前标识符索引 */
			btab[display[level]].last=t;
		}
	}
}

void enterarray(types type,int size)/* 登录数组信息向量表atab */
{
	if (amax==a)
		fatal(ARRAYS);
	if (size>=arrmax)
		error(ARRAYFLOWOVER);	/* 数组下标太大 */
	else
	{
		a++;
		atab[a].size=size;
		atab[a].eltyp=type;
	}
}

void enterblock()/* 登录分程序表btab */
{
	if (bmax==b)	/* 分程序表溢出 */
	{
		fatal(BLOCK);
	}
	else
	{
		b++;	/* 分程序表索引值加1 */
		btab[b].last=0;		/* 暂时填为0 */
		btab[b].lastpar=0;	/* 暂时填为0 */
	}
}

void entercontab()	/* 登录常量表 */
{
	cnx++;
	contab[cnx].tx=t;
	contab[cnx].con.val=tab[t].adr;	/* 常量值 */
	contab[cnx].con.tp=tab[t].typ;
}


void entervariable()	/* 将变量名登录到符号表中 */
{
	if (IDEN==sy)	/* 当前符号是标识符 */
	{
		enter(id,variable);
		insymbol();
	}
	else
		error(EXPECTIDEN);
}

int loc(char id[])	/* 查找标识符在符号表中的位置 */
{
	int i,j;
	i=level;	
	
	/* 先将标识符放在符号表的0号位置，这样找不到时直接返回0 */
	strcpy(tab[0].name,id);
	do 
	{
		j=btab[display[i]].last;	/* 在当前层查找 */
		while(0!=strcmp(tab[j].name,id))
			j=tab[j].link;	/* 不断向上查找 */
		i--;	/* 向上一层 */
	} while (i>=0&&0==j);	/* 不断向上层查找 */
	if (0==j)	/* 在符号表中没有找到标识符 */
		error(IDENTUNDEFINED);	/* 错误：标识符未定义 */
	return j;	/* 返回id在符号表中的位置 */
}

int loccontab(int val,types type)	/* 查找常量在常量表中的位置 */
{
	int i;
	for (i=1;i<=cnx;i++)
		if (contab[i].con.val==val&&contab[i].con.tp==type)
			return contab[i].tx;	/* 找到了返回常量在符号表中的索引 */
	return 0;	/* 没找到返回0 */
}

/************************************以上登录表格相关******************************************/

/************************************以下代码生成相关******************************************/
void emit(fct op)	/* 生成只有指令助记符的四元式 */
{
	if (cmax==ci)
		fatal(CODE);
	code[ci][cj].op=op;
//	printf("%d,%d:%s\n",ci,cj,fctword[op]);
	cj++;
}

void emit1(fct op,int dst)	/* 生成只有助记符与dst的四元式 */
{
	if (cmax==ci)
		fatal(CODE);
	code[ci][cj].op=op;
	code[ci][cj].dst=dst;
//	printf("%d,%d:%s\t%d\n",ci,cj,fctword[op],dst);
	cj++;
}

void emit2(fct op,int src1,int dst)	/* 生成只有助记符、src1与dst的四元式 */
{
	if (cmax==ci)
		fatal(CODE);
	code[ci][cj].op=op;
	code[ci][cj].src1=src1;
	code[ci][cj].dst=dst;
//	printf("%d,%d:%s\t%d\t%d\n",ci,cj,fctword[op],src1,dst);
	cj++;
}

void emit3(fct op,int src1,int src2,int dst)	/* 生成有助记符、src1、src2与dst的四元式 */
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
/************************************以上代码生成相关******************************************/


/************************************以下声明处理相关******************************************/

/* 处理程序中出现的常量，并由c返回常量类型和值
<常量>	::=	[+| -] <无符号整数>|<字符>*/
void constant(symset fsys,conrec *c)	
{
	int sign;		/* 符号（正负号） */
	c->tp=notyp;	/* 先将类型设为空，值置为0 */
	c->val=0;
	test(constbegsys,fsys,EXPECTCONST);	/* 检查当前符号是否为常量开始符号 */
	if (belongset(sy,constbegsys))
	{
		if (CHARCON==sy)		/* 字符常量 */
		{
			c->tp=chars;
			c->val=inum;		/* 字符的ASCII码值 */
			insymbol();
		}
		else
		{
			sign=1;
			if (MINU==sy)	/* 是负号 */
			{
				sign=-1;
				insymbol();
			}
			if (INTCON==sy)	/* 接着是无符号整数 */
			{
				c->tp=ints;
				c->val=sign*inum;	
				insymbol();
			}
			else
				skip(fsys,EXPECTCONST);
		}
		test(fsys,fsys,ILEGALSYM);		/* 检查常量后面的符号是否合法 */
	}
}

/* 处理常量定义，并将常量名及相关信息登录到符号表中 
	<常量定义>	::=	<标识符>= <常量>*/
void constantdef(symset fsys)
{
	conrec c;
	symset settmp=fsys;
	insertelem(SEMICN,&settmp);
	insertelem(COMMA,&settmp);
	insertelem(IDEN,&settmp);
	if (IDEN==sy)
	{
		enter(id,konstant);	/* 将常量登录符号表 */
		insymbol();	/* 读取等号 */
		if (EQL==sy)	/*  是等号 */
			insymbol();	/* 读常量 */
		else
		{
			error(EXPECTEQL);
			if (ASSIGN==sy)	/* 认为赋值运算符是等号的误写 */
				insymbol();
		}
		constant(settmp,&c);	/* 处理常量值 */
		tab[t].typ=c.tp;
		tab[t].ref=0;
		tab[t].adr=c.val;
		tab[t].normal=true;
		entercontab();	/* 登录常量表 */
	}
	else
		error(EXPECTIDEN);
}

/* 处理常量说明部分，将常量名及相应信息填入符号表
<常量说明部分>	::=  const<常量定义>{,<常量定义>}; */
void constdec(symset fsys)
{
	insymbol();	/* 读常量的第一个符号 */
	test(elem2set(IDEN),blockbegsys,EXPECTIDEN);/* 检查是不是标识符 */
	if (IDEN==sy)
	{
		constantdef(fsys);	/* 处理常量定义 */
		while (COMMA==sy)	/* 逗号说明后面还有常量定义 */
		{
			insymbol();
			constantdef(fsys);
		}
		testsemicolon(fsys);
	}
}

void arraytyp(symset fsys,int *rf,int *tp)	/* 处理数组类型：array'['<无符号整数>']' of <基本类型> */
{
	conrec size;
	types eltp,elrf,tp1;
	symset tmpset=fsys;
	insertelem(COLON,&tmpset);
	insertelem(RBRACK,&tmpset);
	insertelem(RPARENT,&tmpset);
	insertelem(OFTK,&tmpset);
	constant(tmpset,&size);	/* 读数组的规模大小 */
	if (size.tp!=ints)	/* 读出来的不是整数 */
	{
		error(EXPECTUNUM);	/* 报错 */
		size.val=0;			/* 大小设为0 */
	}
	if (RBRACK==sy)	/* 无符号整数后面应该遇到右方括号 */
		insymbol();
	else
	{
		error(EXPECTRBRACK);	/* 应该是有方括号 */
		if (RPARENT==sy)	/* 认为右圆括号是右方括号的误写 */
			insymbol();
	}
	if (OFTK==sy)	/* 右方括号后面如果是of */
		insymbol();
	else
		error(EXPECTOF);
	typ(fsys,&eltp,&elrf,&tp1);	/* 处理类型 */
	if (eltp!=ints&&eltp!=chars)	/* 不是基本类型 */
	{
		error(EXPECTBASICTYPE);
		eltp=notyp;
	}
	*rf=size.val;
	*tp=eltp;
}

/* 处理类型描述
<基本类型>	::=   integer | char
<类型>	::=	<基本类型>|array'['<无符号整数>']' of <基本类型> */
void typ(symset fsys,types *tp,int *rf,types *tp1)
{
	*tp=notyp;
	*rf=0;
	test(typebegsys,fsys,WRONGTYPEHEAD);/* 检查当前符号是否为合法的类型开始符号 */
	if (belongset(sy,typebegsys))
	{
		if (INTTK==sy)		/* 当前符号为integer */
		{
			*tp=ints;
			insymbol();
		}
		else if (CHARTK==sy)	/* 当前符号为char */
		{
			*tp=chars;
			insymbol();
		}
		else if (ARRAYTK==sy)	/* 当前符号为array */
		{
			insymbol();	/* 再读一个符号 */
			if (LBRACK==sy)	/* 如果是左方括号 */
				insymbol();	/* 读数组元素个数 */
			else
			{
				error(EXPECTLBRACK);
				if (LPARENT==sy)	/* 认为左圆括号是左方括号的误写 */
					insymbol();
			}
			*tp=arrays;	/* 类型为数组 */
			arraytyp(fsys,rf,tp1);	/* 处理数组类型 */
		}
		test(fsys,fsys,ILEGALSYM);	/* 检查类型后继符号是否合法 */
	}
}

/* 处理变量声明
<变量说明部分>	::=  var <变量说明> ; {<变量说明>;}
<变量说明>		::=  <标识符>{, <标识符>} :  <类型>*/
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
		t0=t;				/* 保存当前符号表索引值，为后续反填准备 */
		entervariable();	/* 将变量填入符号表 */
		while(COMMA==sy)	/* 变量说明后面是逗号就说明有变量说明 */
		{
			insymbol();
			entervariable();
		}
		if (COLON==sy)	/* 最后一个变量后应该是冒号 */
			insymbol();
		else
			error(EXPECTCOLON);
		t1=t;	/* t1为登录了一些变量说明后符号表的索引 */
		typ(settmp,&tp,&rf,&tp1);/* 冒号后面是类型 */
		while(t0<t1)	/* 反填符号表 */
		{
			t0++;
			tab[t0].obj=variable;
			tab[t0].typ=tp; 
			tab[t0].normal=true;
			tab[t0].lev=level;
			tab[t0].adr=dx;
			if (tp==arrays)
			{
				enterarray(tp1,rf);		/* 登录数组信息向量表 */
				tab[t0].ref=a;
				dx=dx+atab[a].size;
			}
			else 
			{
				tab[t0].ref=rf;
				dx++;
			}
		}
		testsemicolon(fsys);	/* 变量说明后面应该是分号 */
	}
}

/* 处理过程或函数说明中的形参表，将形参及其有关信息登录到符号表中 
<形式参数表>	::= [var] <标识符>{, <标识符>}: <基本类型>{; <形式参数表>}*/
void parameterlist(symset fsys)	
{
	types tp;
	bool valpar;	/* 是否为值形参 */
	int t0;
	tp=notyp;
	insymbol();	/* 读第一个符号，检查是不是标识符或var */
	while (IDEN==sy||VARTK==sy)
	{
		if (VARTK!=sy)		/* 不是var开头 */
			valpar=true;	/* 是值形参 */	
		else	/* 是var开头 */
		{
			insymbol();	/* 再读一个才是参数的标识符 */
			valpar=false;	/* 不是值形参 */
		}
		t0=t;	/* 保存当前符号表索引值，为后面反填符号表做准备 */
		entervariable();	/* 将参数名作为变量登录到符号表 */
		while(COMMA==sy)	/* 只要还有逗号就说明还有形参 */
		{
			insymbol();		/* 读取形参变量名 */
			entervariable();	/* 将形参变量名登录符号表 */
		}
		if (COLON==sy)	/* 每个形参表最后一个标识符后面应该是冒号 */
		{
			insymbol();	/* 冒号后面应该是类型 */
			if (INTTK==sy)	/* 类型为无符号整数 */
			{
				tp=ints;
				insymbol();
			}
			else if (CHARTK==sy)	/* 类型为符号 */
			{
				tp=chars;
				insymbol();
			}
			else
				error(EXPECTBASICTYPE);
			/* 检查冒号后面是不是分号或者右括号 */
			test(elemaddelem(SEMICN,RPARENT),setsadd(elemaddelem(COMMA,IDEN),fsys),EXPECTSEMICOLON);
		}
		else
			error(EXPECTCOLON);
		while(t0<t)	/* 开始反填符号表 */
		{
			t0++;
			tab[t0].typ=tp;
			tab[t0].ref=0;
			tab[t0].adr=dx;	/* 填入变量（形参）在运行栈中分配的存储单元相对地址 */
			tab[t0].lev=level;
			tab[t0].normal=valpar;	/* 是否为值形参 */
			dx++;		/* 相当于为变量分配空间 */
		}
		if (RPARENT!=sy)	/* 不是右括号，说明还有形参变量要说明 */
		{
			if (SEMICN==sy)	/* 形参表之间应该以分号分隔 */
				insymbol();
			else
			{
				error(EXPECTSEMICOLON);
				if (COMMA==sy)	/* 认为逗号是分号的误写 */
					insymbol();
			}
			/* 检查形参说明的开始符号 */
			test(elemaddelem(IDEN,VARTK),elemaddset(RPARENT,fsys),WRONGPARHEAD);
		}
	}
	if(RPARENT==sy)	/* 全部形参说明完成后应该有右括号 */
	{
		insymbol();
		/* 检查形参表说明的后继符号是否合法 */
		test(elemaddelem(SEMICN,COLON),fsys,ILEGALSYM);
	}
	else
		error(EXPECTRPARENT);
}

/*	处理过程或函数说明 
	<过程说明部分>		::=	<过程首部><分程序>{;<过程首部><分程序>};
	<函数说明部分>		::=	<函数首部><分程序>{;<函数首部><分程序>};
	<过程首部>			::=	procedure<标识符>'('[<形式参数表>]')';
	<函数首部>			::=	function <标识符>'('[<形式参数表>]')': <基本类型>;
*/
void procdeclaration(symset fsys)		
{
	bool isfun=sy==FUNCTK;	/* 是不是函数的标记 */
	int prt;
	dx=5;/* 预留3个保存全局寄存器的位置，1个保存ebp的位置 */
	insymbol();
	if (IDEN!=sy)	/* 如果不是标识符 */
	{
		error(EXPECTIDEN);	/* 报错 */
		memset(id,0,sizeof(id));	/* 清空标识符 */
	}
	enter(id,isfun?function:procedure);	/* 将过程/函数名登录符号表 */
	tab[t].normal=true;
	prt=t;
	insymbol();
	level++;	/* 层次加1 */
	enterblock();	/* 登录分程序表 */
	display[level]=b;	/* 更新分程序索引表 */
	tab[t].ref=b;	/* 更新符号表的ref域，指向过程或函数在分程序索引表中的位置 */
	if (isfun) tab[t].adr=dx++;	/* 为函数还要保留返回结果的位置 */
	ci++;
	codelevel[ci]=level;
	cj=0;
	cicnt++;
	emit2(ENTER,b,t);	/* 生成进入过程/函数的指令 */
	if (LPARENT==sy)	/* 参数声明 */
		parameterlist(fsys);
	else
		error(EXPECTLPARENT);
	btab[b].lastpar=t;	/* 更新分程序中最后一个参数的位置 */
	btab[b].psize=dx;	/* 更新分程序中参数+保留空间大小 */
	if (isfun)
	{
		if (COLON==sy)	/* 函数的参数表后面是冒号 */
		{
			insymbol();
			if (INTTK==sy||CHARTK==sy)	/* 冒号后面是基本类型 */
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
	if (SEMICN==sy)	/* 函数/过程首部后面都是冒号 */
		insymbol();
	else
		error(EXPECTSEMICOLON);
	block(fsys);	/* 调用分程序分析 */
	level--;
	if (SEMICN==sy)	/* 分程序后面应该是分号 */
		insymbol();
	else
		error(EXPECTSEMICOLON);
	emit2(LEAVE,b,prt);	/* 生成离开过程的代码 */
}

/*<分程序>	::=   [<常量说明部分>][<变量说明部分>]{[<过程说明部分>]| [<函数说明部分>]}<复合语句>*/
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
/*	btab[b].vsize=dx;*/	/* 这句放这不包括临时变量 */
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
	statement(setsadd(fsys,elemaddelem(SEMICN,ENDTK)));	/* 语句 */
	while(belongset(sy,elemaddset(SEMICN,statbegsys)))	/* sy是分号或属于语句开始符号集合 */
	{
		if (SEMICN==sy)	/* 分号是语句分隔，说明还有语句 */
			insymbol();
		else
			error(EXPECTSEMICOLON);
		statement(setsadd(fsys,elemaddelem(SEMICN,ENDTK)));	/* 处理语句 */
	}
	btab[prb].vsize=dx;	/* 这句放这就包括了临时变量 */
	if (ENDTK==sy)		/* 语句后面应该是end */
		insymbol();
	else
		error(EXPECTEND);
	test(setsadd(elemaddelem(SEMICN,PERIOD),fsys),setsadd(elemaddelem(SEMICN,PERIOD),fsys),ILEGALSYM);
}

/*<语句>	::=  <赋值语句>|<条件语句>|<repeat循环语句>|<过程调用语句>|<复合语句>|<读语句>|<写语句>|<for循环语句>|<空>*/
void statement(symset fsys)
{
	int i;
	if (belongset(sy,statbegsys))
	{
		switch(sy)
		{
		case IDEN:	/* 匹配赋值语句或过程调用语句 */
			i=loc(id);
			insymbol();
			if (0!=i)
			{
				switch (tab[i].obj)
				{
				case konstant:
					error(WRONGIDEN);	/* 常量不能被赋值 */
					break;
				case variable:
					assignment(fsys,i);	/* 为变量赋值 */
					break;
				case procedure:	/* 匹配过程调用语句 */
					call(fsys,i);
					break;
				case function:	/* 为函数赋值 */
					if (tab[i].ref==display[level])	/* 仅可以对本层函数赋值 */
						assignment(fsys,i);
					else
						error(WRONGIDEN);
					break;
				}
			}
			break;
		case BEGINTK:	/* 匹配复合语句 */
			compoundstatement(fsys);
			break;
		case IFTK:	/* 匹配条件语句 */
			ifstatement(fsys);
			break;
		case REPTTK:	/* 匹配repeat循环语句 */
			repeatstatement(fsys);
			break;
		case FORTK:		/* 匹配for循环语句 */
			forstatement(fsys);
			break;
		case WRITETK:	/* 匹配写语句 */
			insymbol();
			writestatement(fsys);
			break;
		case READTK:	/* 匹配读语句 */
			insymbol();
			readstatement(fsys);
			break;
		}
	}
	test(fsys,elemaddset(SEMICN,fsys),EXPECTSEMICOLON);	
}

/************************************以上声明处理相关******************************************/


/************************************以下语句处理相关******************************************/

/* 处理因子
	<因子>	::=	<标识符>|<标识符>'['<表达式>']'|
	<无符号整数>|'('<表达式>')' | <函数调用语句> */
void factor(symset fsys,item *x)	

{
	int i;
	x->typ=notyp;	/* 先设置为空类型，后面修改 */
	x->ref=0;
	test(facbegsys,fsys,WRONGFACHEAD);	/* 检查是不是因子开始符号 */
//	while(belongset(sy,facbegsys))
	if(belongset(sy,facbegsys))
	{
		if (IDEN==sy)	/* 是标识符 */
		{
			i=loc(id);
			if (0==i)
				error(IDENTUNDEFINED);	/* 标识符未定义 */
			insymbol();
			if (konstant==tab[i].obj)	/* 标识符种类为常量 */
			{
				x->typ=tab[i].typ;	/* 类型为该常量对应类型 */
				x->ref=i;
			}
			else if (variable==tab[i].obj)	/* 标识符类型为变量，匹配<标识符>|
				<标识符>'['<表达式>']' */
			{
				x->typ=tab[i].typ;	/* 变量类型 */
				x->ref=i;
				if (LBRACK==sy||LPARENT==sy)	/* 变量是数组元素 */
				{
					item idx;	/* 数组下标表达式 */
					insymbol();
					expression(elemaddset(RBRACK,fsys),&idx);
					if (idx.typ!=ints&&idx.typ!=chars)	/* 下标不是基本类型 */
						error(EXPECTBASICTYPE);
					x->typ=atab[tab[i].ref].eltyp;
					x->ref=-(dx);dx++;
					emit3(LOADARRAY,i,idx.ref,x->ref);	/* 新开辟一个空间存储数组元素 */
					if (RBRACK==sy)	/* 右方括号 */
						insymbol();
					else
					{
						error(EXPECTRBRACK);
						if (RPARENT==sy)		/* 认为右圆括号是右方括号的误写 */
							insymbol();
					}
				}
			}
			else if (function==tab[i].obj)	/* 函数标识符，匹配<函数调用语句> 
				<函数调用语句>	::=   <标识符>'('[<实在参数表>]')'*/
			{
				x->typ=tab[i].typ;
				x->ref=-(dx);dx++;	/* 为函数返回值分配临时变量空间 */
				call(fsys,i);
				emit2(RET,i,x->ref);	/* 函数返回值返回到临时空间 */
			}
		}
		else if (INTCON==sy)	/* 匹配<无符号整数> */
		{
			x->typ=ints;
			if (0==(x->ref=loccontab(inum,ints)))	/* 常量表里没有这个数 */
			{
				char name[alng]="#";
				int link=btab[display[level]].last;
				itoa(inum,name+1,10);
				enter(name,konstant);	/* 登录到符号表 */
				tab[t].typ=ints;
				tab[t].adr=inum;
				tab[t].normal=true;
				tab[t].link=link;
				x->ref=t;				/* 在符号表中的位置 */
				entercontab();
			}
			insymbol();
		}
		else if (LPARENT==sy)	/* 匹配'('<表达式>')' */
		{
			insymbol();	/* 读表达式的第一个符号 */
			expression(elemaddset(RPARENT,fsys),x);	/* 处理表达式 */
			if (RPARENT==sy)	/* 表达式完了是右括号 */
				insymbol();
			else
				error(EXPECTRPARENT);
		}
		else
			error(WRONGPROCIDENT);		/* 因子不能为过程标识符 */
		test(setsadd(elemaddelem(SEMICN,RBRACK),fsys),facbegsys,ILEGALSYM);
	}
}

void term(symset fsys,item *x)	/* 处理项 <项>	::=	<因子>{<乘法运算符><因子>} */
{
	item y,z;
	symbol op;
	symset settmp=fsys;
	insertelem(MULT,&settmp);
	insertelem(DIV,&settmp);
	factor(settmp,x);/* 匹配因子 */
	
	while(MULT==sy||DIV==sy)
	{
		op=sy;	/* 保存当前运算符 */
		insymbol();
		factor(settmp,&y);
		z.ref=-(dx);dx++;		/* 为结果z分配栈空间 */
		z.typ=x->typ;
		if (MULT==op)	/* 运算符是乘号 */
		{
			emit3(IMUL,x->ref,y.ref,z.ref);
		}
		else	/* 运算符是除号 */
		{
			
			if (y.ref>0&&tab[y.ref].obj==konstant&&tab[y.ref].adr==0)
				error(DIVZERO);
			else
				emit3(IDIV,x->ref,y.ref,z.ref);
		}
		*x=z;
	}
}

void expression(symset fsys,item *x)	/* 处理表达式<表达式>	::=	[+|-]<项>{<加法运算符><项>} */
{
	item y,z;
	symbol op; 
	symset settmp=fsys;
	insertelem(PLUS,&settmp);
	insertelem(MINU,&settmp);
	if (PLUS==sy||MINU==sy)	/* 匹配[+ | -] */
	{
		op=sy;	/* 保存符号 */
		insymbol();
		term(settmp,x);	/* 匹配项 */
		if (x->typ!=ints&&x->typ!=chars)	/* 正负号后面的不是整数 */
		{
			error(EXPECTUNUM);
		}
		else if (MINU==op)	/* 前面是负号 */
		{
			z.typ=x->typ;
			z.ref=-(dx);dx++;
			emit2(NEG,x->ref,z.ref);	/* 求负 */
			*x=z;
		}
	}
	else	/* 前面没有正负号，直接是项 */
		term(settmp,x);
	while(PLUS==sy||MINU==sy)
	{
		op=sy;	/* 保存运算符 */
		insymbol();	/* 读取右面的项 */
		term(settmp,&y);
		z.typ=x->typ;
		z.ref=-(dx);dx++;
		if (PLUS==op)	/* 运算符是加号 */
			emit3(ADD,x->ref,y.ref,z.ref);	/* 生成加法运算 */
		else
			emit3(SUB,x->ref,y.ref,z.ref);	/* 生成减法运算 */
		*x=z;
	}
}

void condition(symset fsys,item *x,item *y,symbol *op)	/* 处理条件<条件>	::=	<表达式><关系运算符><表达式> */
{
	symset settmp=fsys;
	insertelem(EQL,&settmp);	/* 等号 */
	insertelem(NEQ,&settmp);	/* 不等号 */
	insertelem(LSS,&settmp);	/* 小于号 */
	insertelem(LEQ,&settmp);	/* 小于等于号 */
	insertelem(GRE,&settmp);	/* 大于号 */
	insertelem(GEQ,&settmp);	/* 大于等于号 */
	expression(settmp,x);	/* 匹配表达式 */
	if (EQL==sy||NEQ==sy||LSS==sy||LEQ==sy||GRE==sy||GEQ==sy)	/* 匹配关系运算符 */
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

void assignment(symset fsys,int i)	/* 处理赋值语句 */
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
		if (LBRACK==sy||LPARENT==sy)	/* 变量是数组元素 */
		{
			insymbol();
			expression(elemaddset(RBRACK,fsys),&x);/* x是数组下标表达式 */
			if (x.typ!=ints&&x.typ!=chars)	/* 下标不是基本类型 */
				error(EXPECTBASICTYPE);
			if (RBRACK==sy)	/* 右方括号 */
				insymbol();
			else
			{
				error(EXPECTRBRACK);
				if (RPARENT==sy)		/* 认为右圆括号是右方括号的误写 */
					insymbol();
			}
			isarray=true;
		}
		else
			error(EXPECTLBRACK);
	}

	if (ASSIGN==sy)		/* 是赋值号 */
		insymbol();
	else
	{
		error(EXPECTASSIGN);
		if (EQL==sy)	/* 认为等号是赋值号的误写 */
			insymbol();
	}
	expression(fsys,&y);	/* 求解赋值号右侧的表达式 */
	if (function==tab[i].obj&&x.typ!=y.typ)
		error(WRONGASSIGNTYPE);
	else if (x.typ!=notyp&&y.typ!=notyp)
	{
		if (!isarray)
			emit2(MOV,y.ref,x.ref);
		else
		{
			emit3(STOARRAY,x.ref,y.ref,i);/* 数组赋值 */	/* 不开辟临时变量的版本 */
		}
	}
	else
		error(WRONGASSIGNTYPE);
}

void compoundstatement(symset fsys)	/* 处理复合语句 */
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

void ifstatement(symset fsys)	/* 处理if语句 <条件语句>	::=	if<条件>then<语句> | if<条件>then<语句>else<语句>*/
{
	item x,y;
	types op;
	int lab1;
	insymbol();
	condition(setsadd(fsys,elemaddelem(THENTK,DOTK)),&x,&y,&op);
	//	emit2(CMP,x.ref,y.ref);		/* 生成比较指令 */
	lab1=labcount++;
	switch (op)
	{
	case EQL:		/* 条件是等于，不满足条件是不等于 */
		emit3(JNE,x.ref,y.ref,lab1);
		break;
	case NEQ:		/* 条件是不等于，不满足条件是等于 */
		emit3(JE,x.ref,y.ref,lab1);
		break;
	case LSS:		/* 条件是小于，不满足条件是大于等于 */
		emit3(JGE,x.ref,y.ref,lab1);
		break;
	case LEQ:		/* 条件是小于等于，不满足条件是大于 */
		emit3(JG,x.ref,y.ref,lab1);
		break;
	case GRE:		/* 条件是大于，不满足条件是小于等于 */
		emit3(JLE,x.ref,y.ref,lab1);
		break;
	case GEQ:		/* 条件是大于等于，不满足条件是小于 */
		emit3(JL,x.ref,y.ref,lab1);
		break;
	}
	if (THENTK==sy)	/* if后面必须有then */
		insymbol();
	else
	{
		error(EXPECTTHEN);
		if (DOTK==sy)	/* 认为do是then的误写 */
			insymbol();
	}
	statement(elemaddset(ELSETK,fsys));
	if (ELSETK==sy)		/* if后面有eles，匹配了if<条件>then<语句>else<语句> */
	{
		int lab2=labcount++;
		insymbol();
		emit1(JMP,lab2);
		emit1(SETLAB,lab1);
		statement(fsys);
		emit1(SETLAB,lab2);
	}
	else	/* 没有else，匹配了if<条件>then<语句> */
		emit1(SETLAB,lab1);
}

void repeatstatement(symset fsys)	/* <repeat循环语句>  ::=  repeat <语句>until<条件> */
{
	item x,y;
	types op;
	int lab1=labcount++;
	insymbol();
	emit1(SETLAB,lab1);
	statement(elemaddset(UNLTK,fsys));
// 	while (SEMICN==sy)
// 		insymbol();	/* 跳过空语句 */
	if (UNLTK==sy)
	{
		insymbol();
		condition(fsys,&x,&y,&op);
		//		emit2(CMP,x.ref,y.ref);
		switch (op)
		{
		case EQL:		/* 条件是等于，不满足条件是不等于 */
			emit3(JNE,x.ref,y.ref,lab1);
			break;
		case NEQ:		/* 条件是不等于，不满足条件是等于 */
			emit3(JE,x.ref,y.ref,lab1);
			break;
		case LSS:		/* 条件是小于，不满足条件是大于等于 */
			emit3(JGE,x.ref,y.ref,lab1);
			break;
		case LEQ:		/* 条件是小于等于，不满足条件是大于 */
			emit3(JG,x.ref,y.ref,lab1);
			break;
		case GRE:		/* 条件是大于，不满足条件是小于等于 */
			emit3(JLE,x.ref,y.ref,lab1);
			break;
		case GEQ:		/* 条件是大于等于，不满足条件是小于 */
			emit3(JL,x.ref,y.ref,lab1);
			break;
		}
	}
	else
		error(EXPECTUNTIL);
}

void forstatement(symset fsys)/*<for循环语句>::=  for <标识符> := <表达式> (to|downto) <表达式> do <语句>  */
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
	if (IDEN==sy)	/* 标识符 */
	{
		i=loc(id);	/* 查找在符号表中的位置 */
		insymbol();
		if (0==i)	/* 在符号表中没找到 */
		{
			error(IDENTUNDEFINED);
			cvt=ints;	/* 默认为整数型 */
		}
		else if (variable==tab[i].obj)		/* 标识符的种类为变量 */
		{
			cvt=tab[i].typ;		/* 为符号表中该标识符的类型 */
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
		expression(settmp,&x);	/* 表达式 */
		if (cvt!=x.typ)			/* 表达式值与循环变量类型不相符 */
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
		emit1(SETLAB,labtest);	/* 测试标号，到这里测试循环条件是否满足 */

		expression(elemaddset(DOTK,fsys),&x);	/* 处理to / downto 后面的表达式 */
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
	emit1(TOTK==op?INC:DEC,i);	/* 增加或减少循环变量 */
	emit1(JMP,labtest);		/* 跳转到条件测试处 */
	emit1(SETLAB,labend);	/* 结束标号，转到这结束循环 */
}

/* 处理过程或函数调用 i：函数/过程在tab中的索引值 
<函数调用语句>	::=   <标识符>'('[<实在参数表>]')'
<实在参数表>	::=  <实在参数> {, <实在参数>}
<实在参数>		::=  <表达式>
*/
void call(symset fsys,int i)
{
	item x;
	int lastp,cp;
	symset settmp=fsys;
	insertelem(COMMA,&settmp);
	insertelem(COLON,&settmp);
	insertelem(RPARENT,&settmp);
	lastp=btab[tab[i].ref].lastpar;	/* lastp为过程或函数的最后一个参数在标识符表中的位置 */
	cp=i;	/* 令cp初始为函数/过程在tab中的索引值 */
	if (sy!=LPARENT) error(EXPECTLPARENT);
	if (cp==lastp)	/* 没有形参 */
		insymbol();
	else 
	{
		do 
		{
			insymbol();
			cp++;	/* cp为下一个形参的索引 */
			if (cp>lastp)
				error(WRONGACTPARNUM);/* 形参个数与实参个数不匹配 */
			else
			{
				if (tab[cp].normal)	/* 值形参 */
				{
					expression(settmp,&x);	/* 调用表达式处理，根据<实在参数>	::=  <表达式> */
					if ((tab[cp].typ==ints||tab[cp].typ==chars||x.typ==ints||x.typ==chars)/*&&tab[cp].typ==x.typ*/)	/* 表达式的类型与形参类型一致 */
						emit2(PUSH,0,x.ref);	/* 后面的参数入栈，0表示参数内容压入栈 */
					else
						error(WRONGPARTYP);
				}
				else	/* 变量形参 */
				{
 					if (sy!=IDEN)	/* 变量实参肯定是标识符 */
 						error(EXPECTIDEN);
					else
 					{
 						int k=loc(id);	/* 在符号表中查找标识符 */
 						insymbol();
 						if (k!=0)	/* 找到 */
 						{
 							if(tab[k].obj!=variable)
 								error(EXPECTVARIBLE);
 							if (tab[k].typ!=arrays)	/* 实参不是数组元素 */
 							{
 								emit2(PUSH,1,k);	/* 1代表令地址入栈，并且不是数组元素 */
 							}
 							else	/* 实参是数组元素 */
 							{
								if (LBRACK!=sy)	/* 左括号 */
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
								emit3(PUSH,2,x.ref,k);		/* 2代表令地址入栈，并且是数组元素 */
 							}
 						}
 					}
				}
			}
			test(elemaddelem(COMMA,RPARENT),fsys,ILEGALSYM);
		} while (COMMA==sy);	/* 逗号说明还有实参 */
	}
	if (RPARENT==sy)
		insymbol();
	else
		error(EXPECTRPARENT);
	if (cp<lastp)
		error(TOOFEWACTPAR);
	emit1(CALL,i);
}

/* 处理读语句
<读语句>	::=  read'('<标识符>{,<标识符>}')' */
void readstatement(symset fsys)	

{
	int i;
	if (LPARENT==sy)	/* 是左括号 */
	{
		do 
		{
			insymbol();
			if (IDEN!=sy)
				error(EXPECTIDEN);
			else
			{
				i=loc(id);	/* 找到标识符在符号表中的位置 */
				if (0==i)
					error(IDENTUNDEFINED);
				else
				{
					insymbol();
					if (variable!=tab[i].obj)	/* 该符号不是变量 */
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

/* 处理写语句
<写语句>	::=   write'(' <字符串>,<表达式> ')'|write'(' <字符串>')'|write'('<表达式>')' 
这里做了扩充，可以输出无限多表达式，即可以有write'('[<字符串>,]<表达式>{,<表达式>}')'	*/
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
				flag=1;		/* 要输出字符串 */
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
				if (ints==x.typ)	/* 输出表达式为整型 */
					flag=flag==2?5:3;	/* flag=3 仅输出数字，flag=5，输出字符串和数字 */
				else if (chars==x.typ)
					flag=flag==2?6:4;	/* flag=4 仅输出字符，flag=6，输出字符串和字符 */
			}
			emit3(WRITE,flag,sx,x.ref);	/* 输出表达式 */
		} //while (sy==COMMA);
		if (RPARENT==sy)
			insymbol();
		else
			error(EXPECTRPARENT);
	}
	else
		error(EXPECTLPARENT);
}
/************************************以上语句处理相关******************************************/

/************************************以下寄存器分配相关****************************************/

int locreg(int data)	/* 查找data所在的寄存器，如data不在寄存器中则返回-1 */
{
	int i;
	for (i=0;i<tmpregnum+glbregnum;i++)
		if (regpool[i]==data)	/* i号寄存器的内容是data */
			return i;
	return -1;
}

void mem2reg(int adr,int r)	/* 把内存adr处的数据写到寄存器r */
{
	if (adr>0)	/* 符号表中的数据 */
	{
		if (konstant==tab[adr].obj)
		{
			fprintf(pasm,"\tmov\t%s,%d\n",regword[r],tab[adr].adr);
		}
		else if (variable==tab[adr].obj)
		{
			if (tab[adr].normal)	/* 值形参 */
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
			else	/* 变量形参 */
			{
				if (tab[adr].lev<level)	/* 是上层的变量 */
				{
					int j;
					fprintf(pasm,"\tpush\tebp\n");
					for (j=tab[adr].lev;j<level;j++)
						fprintf(pasm,"\tmov\tebp,[ebp-16]\n");
					fprintf(pasm,"\tmov\tebp,[ebp-%d]\n",tab[adr].adr*4);	/* [ebp-%d]里的内容是变量形参的地址 */
					fprintf(pasm,"\tmov\t%s,[ebp]\n",regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
				else	/* 同一层的变量 */
				{
					fprintf(pasm,"\tpush\tebp\n");
					fprintf(pasm,"\tmov\tebp,[ebp-%d]\n",tab[adr].adr*4);
					fprintf(pasm,"\tmov\t%s,[ebp]\n",regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
			}
		}
	}
	else	/* 临时变量 */
		fprintf(pasm,"\tmov\t%s,[ebp-%d]\n",regword[r],-4*adr);
}

void reg2mem(int r)	/* 把寄存器r的内容写回内存 */
{
	int adr=regpool[r];
	if (adr<0)	/* 寄存器中存的是临时变量 */
		fprintf(pasm,"\tmov\t[ebp-%d],%s\n",-4*adr,regword[r]);
	else	/* 寄存器中的是参数或临时变量 */
	{
		if (tab[adr].obj==function)
			fprintf(pasm,"\tmov\t[ebp-20],%s\n",regword[r]);	/* 函数返回结果保存 */
		else if (tab[adr].obj==variable)
		{
			if (tab[adr].normal==true)	/* 值形参 */
			{
				if (tab[adr].lev<level)	/* 是上层的变量 */
				{
					int j;
					fprintf(pasm,"\tpush\tebp\n");
					for (j=tab[adr].lev;j<level;j++)
						fprintf(pasm,"\tmov\tebp,[ebp-16]\n");
					fprintf(pasm,"\tmov\t[ebp-%d],%s\n",tab[adr].adr*4,regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
				else	/* 同一层的变量 */
					fprintf(pasm,"\tmov\t[ebp-%d],%s\n",tab[adr].adr*4,regword[r]);
			}
			else	/* 变量形参 */
			{
				if (tab[adr].lev<level)	/* 是上层的变量 */
				{
					int j;
					fprintf(pasm,"\tpush\tebp\n");
					for (j=tab[adr].lev;j<level;j++)
						fprintf(pasm,"\tmov\tebp,[ebp-16]\n");
					fprintf(pasm,"\tmov\tebp,[ebp-%d]\n",tab[adr].adr*4);	/* [ebx-%d]里的内容是变量形参的地址 */
					fprintf(pasm,"\tmov\t[ebp],%s\n",regword[r]);
					fprintf(pasm,"\tpop\tebp\n");
				}
				else	/* 同一层的变量 */
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


int allocatereg(int x,int y,int data)	/* 为data分配寄存器，并且寄存器中原来的内容不能是x也不能是y */
{
	int i;
	if (data>0&&regalloc[data]>0)	/* data是分配全局寄存器的变量 */
	{
		if (regpool[regalloc[data]]!=0&&true==regwrtback[regalloc[data]])	/* 为data分配的全局寄存器里已有变量，并且需要写回 */
		{

			reg2mem(regalloc[data]);
		}
		regpool[regalloc[data]]=data;
		regwrtback[regalloc[data]]=true;
		return regalloc[data];
	}

	for (i=0;i<tmpregnum;i++)
		if (0==regpool[i])		/* i号临时寄存器是空的 */
		{
			regpool[i]=data;
			regwrtback[i]=false;	/* 刚被写入寄存器，无需写回 */
			return i;
		}
	for (i=0;i<tmpregnum;i++)		/* 没找到空寄存器，优先找一个存了临时变量的寄存器写回内存 */
	{
		int tmp=regpool[i];	/* tmp是寄存器i存的内容 */
		if (tmp<0&&tmp!=x&&tmp!=y)	/* 是临时变量且不是x或y */
		{
			if (regwrtback[i])	/* 该被写回内存 */
				reg2mem(i);	/* 写回内存 */
			regpool[i]=data;		/* 更新寄存器内容 */
			regwrtback[i]=false;	/* 刚被写入寄存器，无需写回 */
			return i;
		}
	}

	for (i=0;i<tmpregnum;i++)	/* 也没有存了临时变量的寄存器，再找存了参数或局部变量的寄存器 */
	{
		int tmp=regpool[i];
		if (tmp>0&&tmp!=x&&tmp!=y)	/* 是局部变量且不是x或y */
		{
			if (regwrtback[i])	/* 该被写回内存 */
				reg2mem(i);	/* 写回内存 */
			regpool[i]=data;
			regwrtback[i]=false;
			return i;
		}
	}

	if (regwrtback[0])	/* 还没分到寄存器就强制分配0号寄存器 */
	{
		reg2mem(0);
	}
	regpool[0]=data;
	regwrtback[0]=false;
	return 0;
}

void clearreg()	/* 清空临时寄存器 */
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


/************************************以上寄存器分配相关****************************************/


/************************************以下打印信息相关******************************************/

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
			case CALL:	/* 函数调用 */
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
				if (1==src1||5==src1||6==src1)	/* 要输出字符串 */
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
			case LOADARRAY:	/* 载入数组元素 dst=src1[src2] */
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
			case STOARRAY:	/* 数组赋值 dst[src1]=src2 */
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
				if (0==src1)	/* 值形参入栈 */
				{
					if(dst>0)
						fprintf(pcode,"\t\t\t\t%s\n",tab[dst].name);
					else
						fprintf(pcode,"\t\t\t\tmem[%d]\n",-dst);
				}
				else if (1==src1)	/* 变量形参地址入栈 */
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


void gencmp(int src1,int src2)	/* 生成cmp指令用到较多，单独写一个函数 */
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

void interpret(const order code[][cmax],int cicnt)		/* 生成汇编代码 */
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
	for(i=0;i<=sx;i++)	/* 把字符串表写入data域 */
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
				fprintf(pasm,";ENTER\t\t\t%s\n",dst!=0?tab[dst].name:"main");	/* 注释 */
				if (0==dst)	
					fprintf(pasm,"@@main:\n");
				else
					fprintf(pasm,"@@%s:\n",tab[dst].name);
				fprintf(pasm,"\tpush\tebp\n");		/* 保存原栈底 */
				fprintf(pasm,"\tmov\tebp,esp\n");	/* 令ebp 等于当前栈顶 */
				//fprintf(pasm,"\tenter\n");	/* 一句enter可以替换以上两句 */

				
 				fprintf(pasm,"\tpush\tebx\n");	/* 全局寄存器EBX入栈 */
 				fprintf(pasm,"\tpush\tesi\n");	/* 全局寄存器ESI入栈 */
 				fprintf(pasm,"\tpush\tedi\n");	/* 全局寄存器EDI入栈 */

				if (0!=tab[dst].lev)		/* 除了第一层的程序，其他层要把调用它的分程序的ebp用eax传入栈 */
					fprintf(pasm,"\tpush\teax\n");	
				if (function==tab[dst].obj)
					fprintf(pasm,"\tsub\tesp,4\t;function result\n");
				for (k=btab[src1].lastpar-dst;k>=1;k--)	/* 参数入栈，因为压栈顺序原因，所以从后往前 */
				{
					fprintf(pasm,"\tmov\teax,[ebp+%d]\n",4+k*4);	/* 用eax传递。+4是跳过调用时压入的返回地址 */
					fprintf(pasm,"\tpush\teax\n");
				}
				fprintf(pasm,"\tsub\tesp,%d\n",0==dst?(btab[src1].vsize-4)*4:(btab[src1].vsize-btab[src1].psize)*4);	/* 为局部变量和临时变量分配空间 */
				break;
			case LEAVE:
				fprintf(pasm,";LEAVE\t\t\t%s\n",dst!=0?tab[dst].name:"main");/* 注释 */
				for (k=0;k<tmpregnum;k++)
				{
					if (regpool[k]>0&&regwrtback[k])/* 离开的时候临时变量不用存了 */
						reg2mem(k);
					regpool[k]=0;
					regwrtback[k]=false;
				}
				if (function==tab[dst].obj)	/* 从函数返回时要把结果用eax传递回去 */
					fprintf(pasm,"\tmov\teax,[ebp-20]\t;return result\n");
			
				fprintf(pasm,"\tmov\tebx,[ebp-4]\n");	/* 全局寄存器EBX出栈 */
				fprintf(pasm,"\tmov\tesi,[ebp-8]\n");	/* 全局寄存器ESI出栈 */
				fprintf(pasm,"\tmov\tedi,[ebp-12]\n");	/* 全局寄存器EDI出栈 */
				fprintf(pasm,"\tmov\tesp,ebp\n");	/* 恢复原栈顶 */
				fprintf(pasm,"\tpop\tebp\n");		/* 恢复原栈底 */
				//fprintf(pasm,"\tleave\n");	/* 一句leave可以替换以上两句 */	
				if (0==dst)	/* 主程序返回 */
					fprintf(pasm,"\tWriteString\tszPause\n\tinvoke StdIn, addr szPause,0\n");
				fprintf(pasm,"\tret\n");
				break;
			case SETLAB:
				fprintf(pasm,";SETLAB\t\t\t\tlab%d\n",dst);	/* 注释 */
				clearreg();	/* 这步其实是告诉编译器的，编译过程不等于执行过程，不加这句后果很严重！新标号等于进入新过程 */
				fprintf(pasm,"@@lab%d:\n",dst);
				break;
			case NEG:	/* NEG src1  dst 求src1的负，结果给dst */
				fprintf(pasm,";NEG\t");/* 注释 */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else
					fprintf(pasm,"mem[%d]\t\t",-src1);
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else
					fprintf(pasm,"mem[%d]\n",-dst);

				regdst=locreg(dst);	/* 看目的操作数在不在寄存器里 */
				if (regdst<0)	/* 不在，导入 */
				{
					regdst=allocatereg(0,src1,dst);
					if (src1==dst)	/* 目的操作数自身求反时把目的操作数原来的值载入寄存器 */
						mem2reg(dst,regdst);
				}
				regwrtback[regdst]=true;	/* 目的操作数将被改变，故写回标志为true */
				regsrc1=locreg(src1);	/* 看源操作数在不在寄存器里 */
				if (regsrc1<0)
				{
					regsrc1=allocatereg(0,dst,src1);
					mem2reg(src1,regsrc1);	/* 源操作数内容写入寄存器 */
				}
				
				if(dst!=src1)	/* 不是自身求反时，要把源操作数先赋值给目的操作数 */
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);
				fprintf(pasm,"\tneg\t%s\n",regword[regdst]);
				break;
			case MOV:	/* 赋值语句dst=src1 */
				fprintf(pasm,";MOV\t");		/* 注释 */
				if (src1>0)
					fprintf(pasm,"%s\t\t",tab[src1].name);
				else
					fprintf(pasm,"mem[%d]\t\t",-src1);
				if (dst>0)
					fprintf(pasm,"%s\n",tab[dst].name);
				else
					fprintf(pasm,"mem[%d]\n",-dst);		/* 注释完 */
				
				regdst=locreg(dst);	/* 看目的操作数在不在寄存器中 */
				if (regdst<0)	/* 不在，给目的操作数分配寄存器 */
					regdst=allocatereg(0,src1,dst);
				regwrtback[regdst]=true;	/* 目的值要被改变了，故写回标志设为true */
				
				if (src1>0&&konstant==tab[src1].obj)	/* 源操作数是常量 */
				{
					fprintf(pasm,"\tmov\t%s,%d\n",regword[regdst],tab[src1].adr);
					break;
				}
				regsrc1=locreg(src1);	/* 看源操作数在不在寄存器中 */
				if (regsrc1<0)	/* 不在，给源操作数分配寄存器 */
				{
					regsrc1=allocatereg(0,dst,src1);	/* 分配新的寄存器 */
					mem2reg(src1,regsrc1);	/* 把源操作数从内存载入寄存器 */
				}
				if (regsrc1!=regdst)
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);
				break;
			case ADD:	/* 加法运算dst=src1+src2 */
				fprintf(pasm,";ADD\t");		/* 注释 */
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
					fprintf(pasm,"\t\t\n");		/* 注释完 */

				regdst=locreg(dst);	/* 看目的操作数在不在寄存器中 */
				if (regdst<0)	/* 不在，给它分配 */
				{
					regdst=allocatereg(src1,src2,dst);
					if (dst==src1||dst==src2)	/* 自加操作 */
						mem2reg(dst,regdst);	/* 把dst从内存中导入寄存器，后面可以直接add dst,src1或add dst,src2 */
				}
				regwrtback[regdst]=true;	/* 将被修改，写回标志置1 */
				if (dst==src2)	/* 是形如dst=x+dst的算式，把它变成dst=dst+x形状 */
				{
					int tmp=src1;
					src1=src2;
					src2=tmp;
				}
				regsrc1=locreg(src1);	/* 看源操作数1在不在寄存器中 */
				if (regsrc1<0)	/* 不在，分配寄存器并写入到寄存器 */
				{
					regsrc1=allocatereg(src2,dst,src1);
					mem2reg(src1,regsrc1);
				}
				regsrc2=locreg(src2);	/* 看源操作数2在不在寄存器中 */
				if (regsrc2<0)	/* 不在，分配寄存器并写入到寄存器 */
				{
					regsrc2=allocatereg(src1,dst,src2);
					mem2reg(src2,regsrc2);
				}
				if (dst!=src2)		/* 不是自加，是形如dst=x+y的算式 */
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);	/* 令dst=src1，后面直接add dst,src2 */
				fprintf(pasm,"\tadd\t%s,%s\n",regword[regdst],regword[regsrc2]);
				break;
			case SUB:	/* 减法运算dst=src1-src2，注意考虑dst=src1或dst=src2的情况 */
				fprintf(pasm,";SUB\t");	/* 注释 */
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

				regdst=locreg(dst);	/* 看目的操作数在不在寄存器中 */
				if (regdst<0)	/* 不在，给它分配 */
				{
					regdst=allocatereg(src1,src2,dst);
					if (dst==src1||dst==src2)	/* 自减操作或自身被减 */
						mem2reg(dst,regdst);	/* 把dst从内存中导入寄存器，后面可以直接sub dst,src1或sub dst,src2 */
				}
				regwrtback[regdst]=true;	/* 将被修改，写回标志置1 */

				regsrc1=locreg(src1);	/* 看源操作数1在不在寄存器中 */
				if (regsrc1<0)	/* 不在，分配寄存器并写入到寄存器 */
				{
					regsrc1=allocatereg(src2,dst,src1);
					mem2reg(src1,regsrc1);
				}
				regsrc2=locreg(src2);	/* 看源操作数2在不在寄存器中 */
				if (regsrc2<0)	/* 不在，分配寄存器并写入到寄存器 */
				{
					regsrc2=allocatereg(src1,dst,src2);
					mem2reg(src2,regsrc2);
				}
				if (dst==src1)		/* 自减操作，是形如dst=dst-x的算式 */
					fprintf(pasm,"\tsub\t%s,%s\n",regword[regdst],regword[regsrc2]);	/* 这种情况前面已经载入了dst */
				else if (dst==src2)	/* 形如dst=x-dst的算式 */
				{
					fprintf(pasm,"\tsub\t%s,%s\n",regword[regdst],regword[regsrc2]);	/* 计算dst=dst-x */
					fprintf(pasm,"\tneg\t%s\n",regword[regdst]);	/* 求反 */
				}
				else	/* 形如dst=x-y的算式 */
				{
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);	/* dst=x */
					fprintf(pasm,"\tsub\t%s,%s\n",regword[regdst],regword[regsrc2]);	/* dst=dst-y，即dst=x-y */
				}
				break;
			case IMUL:	/* 乘法和加法类似，就是改个符号dst=src1*src2 */
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

				regdst=locreg(dst);	/* 看目的操作数在不在寄存器中 */
				if (regdst<0)	/* 不在，给它分配 */
				{
					regdst=allocatereg(src1,src2,dst);
					if (dst==src1||dst==src2)	/* 自乘操作 */
						mem2reg(dst,regdst);	/* 把dst从内存中导入寄存器，后面可以直接imul dst,src1或imul dst,src2 */
				}
				regwrtback[regdst]=true;	/* 将被修改，写回标志置1 */
				if (dst==src2)	/* 是形如dst=x*dst的算式，把它变成dst=dst*x形状 */
				{
					int tmp=src1;
					src1=src2;
					src2=tmp;
				}
				regsrc1=locreg(src1);	/* 看源操作数1在不在寄存器中 */
				if (regsrc1<0)	/* 不在，分配寄存器并写入到寄存器 */
				{
					regsrc1=allocatereg(src2,dst,src1);
					mem2reg(src1,regsrc1);
				}
				regsrc2=locreg(src2);	/* 看源操作数2在不在寄存器中 */
				if (regsrc2<0)	/* 不在，分配寄存器并写入到寄存器 */
				{
					regsrc2=allocatereg(src1,dst,src2);
					mem2reg(src2,regsrc2);
				}
				if (dst!=src2)		/* 不是自乘，是形如dst=x*y的算式 */
					fprintf(pasm,"\tmov\t%s,%s\n",regword[regdst],regword[regsrc1]);	/* 令dst=src1，后面直接add dst,src2 */
				fprintf(pasm,"\timul\t%s,%s\n",regword[regdst],regword[regsrc2]);
				break;
			case IDIV:	/* 除法运算dst=src1/src2。注意考虑dst=src1或dst=src2的情况 */
				fprintf(pasm,";IDIV\t");/* 注释 */
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
					fprintf(pasm,"\t\t\n");	/* 注释完 */

				regdst=locreg(dst);	/* 看目的操作数在不在寄存器中 */
				if (regdst>=0)	/* dst在寄存器中，释放 */
				{
					regwrtback[regdst]=false;
					regpool[regdst]=0;
				}
				if (regpool[EAX]!=0&&regwrtback[EAX])	/* 把eax中的内容存回内存 */
					reg2mem(EAX);

				regdst=EAX;	/* 把eax分配给dst */
				regpool[EAX]=dst;
				regwrtback[EAX]=true;
				if (regpool[EDX]!=0&&regwrtback[EDX])	/* 除法会改变edx */
				{
					reg2mem(EDX);
					regpool[EDX]=0;
					regwrtback[EDX]=false;
				}
				if (dst==src1)	/* 形如dst=dst/src2的式子 */
				{
					mem2reg(dst,EAX);	/* 把dst载入eax */
					regsrc2=locreg(src2);
					if (regsrc2>=0)
					{
						if (regwrtback[regsrc2])
							reg2mem(regsrc2);
						regpool[regsrc2]=0;
						regwrtback[regsrc2]=false;
					}
					if (regpool[ECX]!=0&&regwrtback[ECX])	/* 把ecx中的内容存回内存 */
						reg2mem(ECX);
					mem2reg(src2,ECX);	/* 把src1载入ecx */
					regpool[ECX]=0;
					regwrtback[ECX]=false;
					fprintf(pasm,"\tcdq\n");
					fprintf(pasm,"\tidiv\tecx\n");	/* 计算后eax=dst/src2 */
				}
				else if (dst==src2)	/* 形如dst=src1/dst的式子 */
				{
					mem2reg(dst,EAX);	/* 把dst载入eax */
					regsrc1=locreg(src1);
					if (regsrc1>=0)
					{
						if (regwrtback[regsrc1])
							reg2mem(regsrc1);
						regpool[regsrc1]=0;
						regwrtback[regsrc1]=false;
					}
					if (regpool[ECX]!=0&&regwrtback[ECX])	/* 把ecx中的内容存回内存 */
						reg2mem(ECX);
					mem2reg(src1,ECX);	/* 把src1载入ecx */
					regpool[ECX]=0;
					regwrtback[ECX]=false;
					fprintf(pasm,"\tpush\teax\n");	/* 保存eax值 */
					fprintf(pasm,"\tmov\teax,ecx\n");	/* 这一步让eax=src1 */
					fprintf(pasm,"\tpop\tecx\n");	/* 这一步让ecx=dst */
					fprintf(pasm,"\tcdq\n");
					fprintf(pasm,"\tidiv\tecx\n");	/* 此时eax=src1/dst */
					
				}
				else	/* 形如dst=src1/src2的式子 */
				{
					regsrc1=locreg(src1);
					if (regsrc1>=0)
					{
						if (regwrtback[regsrc1])
							reg2mem(regsrc1);
						regpool[regsrc1]=0;
						regwrtback[regsrc1]=false;
					}
					if (regpool[EDX]!=0&&regwrtback[EDX])	/* 把edx中的内容存回内存 */
						reg2mem(EDX);
					mem2reg(src1,EDX);	/* 把src1载入edx */
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
					if (regpool[ECX]!=0&&regwrtback[ECX])	/* 把ecx中的内容存回内存 */
						reg2mem(ECX);
					mem2reg(src2,ECX);	/* 把src2载入ecx */
					regpool[ECX]=0;
					regwrtback[ECX]=false;
					fprintf(pasm,"\tmov\teax,edx\n");
					fprintf(pasm,"\tcdq\n");
					fprintf(pasm,"\tidiv\tecx\n");
				}
				break;
			case JE:
				fprintf(pasm,";JE\t");		/* 注释 */
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
				fprintf(pasm,";JNE\t");		/* 注释 */
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
				fprintf(pasm,";JL\t");		/* 注释 */
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
				fprintf(pasm,";JLE\t");		/* 注释 */
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
				fprintf(pasm,";JG\t");		/* 注释 */
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
				fprintf(pasm,";JGE\t");		/* 注释 */
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
				clearreg();	/* 原因同setlab处的注释，jmp相当于离开过程。记住编译过程不等于执行过程 */
				fprintf(pasm,";JMP\tlab%d\n",dst);
				fprintf(pasm,"\tjmp\t@@lab%d\n",dst);
				break;
			case CALL:
				fprintf(pasm,";CALL\t\t%s\n",tab[dst].name);	/* 注释 */
				clearreg();
				codelevel[i]--;/* 下面的判断针对的是两个过程，但是当前代码的层级比它属于的过程要大1，所以先减掉，后面再加回来 */
				if (codelevel[i]+1==tab[dst].lev)	/* 调用过程是被调用过程的直接上层 */
					fprintf(pasm,"\tmov\teax,ebp\n");	/* 用eax传递ebp */
 				else if (codelevel[i]>=tab[dst].lev)
				{
					fprintf(pasm,"\tmov\teax,[ebp-16]\n");/* 第level-1层的ebp */
					for(k=codelevel[i];k>tab[dst].lev;k--)	
						fprintf(pasm,"\tmov\teax,[eax-16]\n");	/* 不断找上层ebp */
				}
				codelevel[i]++;	/* 把前面减掉的加回来 */
				fprintf(pasm,"\tcall\t@@%s\n",tab[dst].name);
				break;
			case RET:
				if (dst<0)	/* 注释 */
					fprintf(pasm,";RET\teax\t\tmem[%d]\n",-dst);
				else
					fprintf(pasm,";RET\teax\t\t%s\n",tab[dst].adr);
			
				regdst=locreg(dst);
				if (regdst<0)
				{
					regdst=allocatereg(0,0,dst);
					regwrtback[regdst]=true;
				}
				if (regdst!=EAX)	/* 若regdst已经分配了eax则无需传递 */
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
				fprintf(pasm,";WRITE\t\t");	/* 注释 */
				if (1==src1||5==src1||6==src1)	/* 要输出字符串 */
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
						fprintf(pasm,"\t\t\t\tmem[%d]\n",-dst);	/* 注释完 */
				}
			
				if (1==src1||5==src1||6==src1)		/* 输出字符串 */
				{
					fprintf(pasm,"\tWriteString\t@string%d\n",src2);
					if (1==src1) fprintf(pasm,"\tWriteCrlf\n");	/* 单独输出字符串时换行 */
				}
				if (src1>1)	/* 输出表达式 */
				{
					if (dst>0&&konstant==tab[dst].obj)	/* 常量直接输出 */
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
						if (3==src1||5==src1)	/* 输出数字 */
							fprintf(pasm,"\tWriteSDecDword\t%s\n",regword[regdst]);
						else if (4==src1||6==src1)	/* 输出字符 */
							fprintf(pasm,"\tWriteChar\t%cl\n",regword[regdst][1]);
					}

					fprintf(pasm,"\tWriteCrlf\n");
				}

				break;
			case LOADARRAY:		/* 载入数组 dst=src1[src2] */	/* 暂不支持跨函数调用数组 */
				fprintf(pasm,";LOADARRY\t");	/* 注释 */
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
					fprintf(pasm,"mem[%d]\n",-dst);	/* 注释完 */

				regdst=locreg(dst);
				if (regdst<0)
					regdst=allocatereg(0,src2,dst);
				regwrtback[regdst]=true;	/* 值要被修改 */
				

				if (src2>0&&konstant==tab[src2].obj)	/* 数组元素的下标是常数 */
				{
					
					if (tab[src1].lev<level)	/* 数组是上层的变量 */
					{
						fprintf(pasm,"\tpush\tebp\n");	/* 保存ebp */
						for (k=tab[src1].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* 不断令ebp上移 */
					}
					fprintf(pasm,"\tmov\t%s,[ebp-%d]\n",regword[regdst],(tab[src1].adr+tab[src2].adr)*4);
					if (tab[src1].lev<level)	
						fprintf(pasm,"\tpop\tebp\n");	/* 恢复ebp */
				}
				else	/* 数组下标不是常数 */
				{
					regsrc2=locreg(src2);
					if (regsrc2<0)
					{
						regsrc2=allocatereg(0,dst,src2);
						mem2reg(src2,regsrc2);
					}
					if (regwrtback[regsrc2])	reg2mem(regsrc2);	/* 因为下面的操作要改变寄存器内容，所以没存的赶紧存 */
					regwrtback[regsrc2]=false;	/* 只是读取src2的内容，并不修改 */

					fprintf(pasm,"\tpush\tebp\n");	/* 保存ebp */
					if (tab[src1].lev<level)	/* 数组是上层的变量 */
					{

						for (k=tab[src1].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* 不断令ebp上移 */
					}

					fprintf(pasm,"\tadd\t%s,%d\n",regword[regsrc2],tab[src1].adr);	/* 数组元素偏移地址 */
					fprintf(pasm,"\timul\t%s,4\n",regword[regsrc2]);
					fprintf(pasm,"\tsub\tebp,%s\n",regword[regsrc2]);/* 定位数组元素 */
					fprintf(pasm,"\tmov\t%s,[ebp]\n",regword[regdst]);
					fprintf(pasm,"\tpop\tebp\n");	/* 恢复ebp */

					regpool[regsrc2]=0;	/* 释放下标占用的寄存器 */
				}
				break;
			case STOARRAY:	/* 为数组赋值 dst[src1]=src2 */
				fprintf(pasm,";STOARRY\t");	/* 注释 */
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
					fprintf(pasm,"[mem[%d]]\n",-src1);		/* 注释完 */
				

				regsrc2=locreg(src2);
				if (regsrc2<0)
				{
					regsrc2=allocatereg(0,src1,src2);
					mem2reg(src2,regsrc2);
					regwrtback[regsrc2]=false;	/* 只是读取src2的内容，并不修改 */
				}

				if (src1>0&&konstant==tab[src1].obj)	/* 数组元素的下标是常数 */
				{
					
					if (tab[dst].lev<level)	/* 数组是上层的变量 */
					{
						fprintf(pasm,"\tpush\tebp\n");/* 保存ebp */
						for (k=tab[dst].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* 不断令ebp上移 */
					}
					fprintf(pasm,"\tmov\t[ebp-%d],%s\n",(tab[dst].adr+tab[src1].adr)*4,regword[regsrc2]);
					if (tab[dst].lev<level)	/* 数组是上层的变量 */
						fprintf(pasm,"\tpop\tebp\n");/* 恢复ebp */
				}
				else	/* 数组下标不是常数 */
				{
					regsrc1=locreg(src1);
					if (regsrc1<0)
					{
						regsrc1=allocatereg(0,src2,src1);
						mem2reg(src1,regsrc1);

					}
					if (regwrtback[regsrc1])	reg2mem(regsrc1);	/* 因为下面的操作要改变寄存器内容，所以没存的赶紧存 */
					regwrtback[regsrc1]=false;	/* 只是读取src1的内容，并不修改 */
					
					fprintf(pasm,"\tpush\tebp\n");/* 保存ebp */
					if (tab[dst].lev<level)	/* 数组是上层的变量 */
					{
	
						for (k=tab[dst].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* 不断令ebp上移 */
					}
					fprintf(pasm,"\tadd\t%s,%d\n",regword[regsrc1],tab[dst].adr);	/* 数组元素偏移地址 */
					fprintf(pasm,"\timul\t%s,4\n",regword[regsrc1]);
					fprintf(pasm,"\tsub\tebp,%s\n",regword[regsrc1]);/* 定位数组元素 */
					fprintf(pasm,"\tmov\t[ebp],%s\n",regword[regsrc2]);
					
					fprintf(pasm,"\tpop\tebp\n");	/* 恢复ebp */
					regpool[regsrc1]=0;	/* 释放regsrc1，并且不写回内存，也不能写回 */
				}
				
				break;
			case INC:	
				fprintf(pasm,";INC\t%s\n",tab[dst].name);	/* 注释 */
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
				fprintf(pasm,";DEC\t%s\n",tab[dst].name);	/* 注释 */
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
				fprintf(pasm,";PUSH\t");	/* 注释 */
				if (0==src1)	/* 值形参入栈 */
				{
					if(dst>0)
						fprintf(pasm,"\t\t%s\n",tab[dst].name);
					else
						fprintf(pasm,"\t\tmem[%d]\n",-dst);
				}
				else if (1==src1)	/* 变量形参地址入栈 */
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
				/* 注释完 */

	
				if (0==src1)	/* 值形参入栈 */
				{
					regdst=locreg(dst);
					if (regdst<0)
					{
						regdst=allocatereg(0,0,dst);
						mem2reg(dst,regdst);
					}
					fprintf(pasm,"\tpush\t%s\n",regword[regdst]);
				}
				else if (1==src1)	/* 变量形参地址入栈 */
				{
					if (regpool[EAX]!=0&&regwrtback[EAX])
					{
						reg2mem(EAX);
						regpool[EAX]=0;
						regwrtback[EAX]=false;
					}
					fprintf(pasm,"\tpush\tebp\n");
					if (tab[dst].lev<level)	/* 是上层的变量 */
					{
						for (k=tab[dst].lev;k<level;k++)
							fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* 不断令ebp上移 */
					}
					fprintf(pasm,"\tlea\teax,[ebp-%d]\n",4*tab[dst].adr);	/* 把变量地址给eax */
					
					fprintf(pasm,"\tpop\tebp\n");
					fprintf(pasm,"\tpush\teax\n");	/* 地址入栈 */
				}
				else if (2==src1)	/* dst[src2]入栈 */
				{
					fprintf(pasm,"\tpush\tebp\n");

					if (src2>0&&konstant==tab[src2].obj)	/* 数组元素的下标是常数 */
					{
						if (regpool[EAX]!=0&&regwrtback[EAX])
						{
							reg2mem(EAX);
							regpool[EAX]=0;
							regwrtback[EAX]=false;
						}
						if (tab[dst].lev<level)	/* 数组是上层的变量 */
						{
							for (k=tab[dst].lev;k<level;k++)
								fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* 不断令ebp上移 */
						}
						fprintf(pasm,"\tlea\teax,[ebp-%d]\n",(tab[dst].adr+tab[src2].adr)*4);
						fprintf(pasm,"\tpop\tebp\n");
						fprintf(pasm,"\tpush\teax\n");
					}
					else	/* 数组下标不是常数 */
					{
						regsrc2=locreg(src2);
						if (regsrc2<0)
						{
							int tmp=regpool[EAX];
							regpool[EAX]=101;
							regsrc2=allocatereg(0,regpool[EAX],src2);	/* 不能让src2分到eax */
							mem2reg(src2,regsrc2);
							regpool[EAX]=tmp;
						}
						if (regwrtback[regsrc2])	reg2mem(regsrc2);	/* 因为下面的操作要改变寄存器内容，所以没存的赶紧存 */
						regwrtback[regsrc2]=false;	/* 只是读取src2的内容，并不修改 */
						if (regpool[EAX]!=0&&regwrtback[EAX])
						{
							reg2mem(EAX);
							regpool[EAX]=0;
							regwrtback[EAX]=false;
						}
						if (tab[dst].lev<level)	/* 数组是上层的变量 */
						{
							for (k=tab[dst].lev;k<level;k++)
								fprintf(pasm,"\tmov\tebp,[ebp-16]\n");	/* 不断令ebp上移 */
						}
						fprintf(pasm,"\tadd\t%s,%d\n",regword[regsrc2],tab[dst].adr);	/* 数组元素偏移地址 */
						fprintf(pasm,"\timul\t%s,4\n",regword[regsrc2]);
						fprintf(pasm,"\tsub\tebp,%s\n",regword[regsrc2]);/* 定位数组元素 */

						fprintf(pasm,"\tlea\teax,[ebp]\n");
						fprintf(pasm,"\tpop\tebp\n");
						fprintf(pasm,"\tpush\teax\n");
						regpool[regsrc2]=0;	/* 释放下标占用的寄存器 */
					}
					
				}
				break;
			}
		}
		fprintf(pasm,"\n\n");
	}
	fprintf(pasm,"end start\n");
}

/************************************以上打印信息相关******************************************/

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
	inpath[strlen(inpath)-1]='\0';/* 去掉回车 */
	if (inpath[0]=='"')	/* 奇怪有的时候拖过来会有双引号 */
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
	dx=4;/* 主程序分配的四个空间：ebp+3个全局寄存器 */
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
		printf("\n\nCompiling……\n\n");
		system(szmlcmd);
		printf("\n\nLinking……\n\n");
		system(szlinkcmd);
		if (execute)	/* 现场运行 */
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