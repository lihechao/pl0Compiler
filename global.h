#ifndef __GLOBAL_H
#define __GLOBAL_H
#include <stdio.h>
#define nkw			21		/* 关键字个数 */
#define alng		15		/* 标识符最大长度 */
#define llng		121		/* 一行输入的长度 */
#define kmax		15		/* 最多数字位数 */
#define nmax		35367	/* 最大数字 */
#define tmax		100		/* 最大符号表长度 */
#define bmax		20		/* 最大分程序表长度 */
#define amax		30		/* 最大数组信息向量表长度 */
#define cmax		800		/* 最大中间代码表长度 */
#define lmax		8		/* 最大层次数 */
#define smax		600		/* 最大字符串表长度 */
#define pmax		100		/* 最大过程数 */
#define ermax		58		/* 最大错误代号 */
#define fatmax		6		/* 最大致命错误代号 */
#define omax		63		/* 最大四元式操作码代号 */
#define linleng		132		/* 输出文件长度 */
#define setmax		50		/* 集合中元素的最大数目 */
#define arrmax		500		/* 数组下标最大值 */
#define labmax		100		/* 最大标号 */
#define tmpregnum	3		/* 临时寄存器数量 */
#define glbregnum	3		/* 全局寄存器数量 */
#define tmpmax		200		/* 最多临时变量的个数 */		


#define true		1
#define false		0

typedef int index;	/* 把int定义成索引index */
typedef short bool;	/* 定义布尔型 */

typedef enum{NUL,IDEN,INTCON,CHARCON,STRCON,CONSTTK,INTTK,CHARTK,VARTK,ARRAYTK,OFTK,REPTTK,
			 UNLTK,IFTK,THENTK,ELSETK,DOTK,FORTK,TOTK,DOWNTOTK,PROCETK,FUNCTK,READTK,WRITETK,
			 BEGINTK,ENDTK,PLUS,MINU,MULT,DIV,LSS,LEQ,GRE,GEQ,EQL,NEQ,ASSIGN,SEMICN,COMMA,
			 PERIOD,COLON,QMARK,DQMARK,LPARENT,RPARENT,LBRACK,RBRACK}symbol;	/* 单词的类别码 */

typedef enum{konstant, variable, procedure, function}objecttype;	
/* 标识符种类，即常量、变量、过程还是函数 */

typedef enum{notyp,ints, chars, arrays}types;
/* 标识符类型，即整数、字符还是数组 */

typedef enum{
NOP/*空指令*/,ENTER/*进入过程*/,LEAVE/*离开过程*/,SETLAB/*设置标号*/,NEG/*求负*/,
MOV/*赋值*/, CMP/*比较*/,ADD/*加法*/,SUB/*减法*/,IMUL/*有符号数乘*/,IDIV/*有符号数加*/,
JE/*等于则转移*/,JNE/*不等则转移*/,JG/*大于则转移*/,JL/*小于则转移*/,JGE/*大于等于则转移*/,
JLE/*小于等于则转移*/,JMP/*无条件转移*/,CALL/*调用*/,RET/*返回*/,READ/*读*/,WRITE/*写*/,
INC/*自增1*/,DEC/*自减1*/,LOADARRAY/*读取数组元素*/,STOARRAY/*存储数组元素*/,
PUSH/*入栈*/,POP/*出栈*/}fct;		/* 指令助记符 */

typedef enum {
		NUMOVERFLOW/*数太大溢出*/,
		WRONGCHARCON/*无法识别的字符常量*/,
		EXPECTQMARK/*应为单引号'*/,
		EXPECTDQMARK/*应为双引号"*/,
		ILEGALCHAR/*非法字符*/,
		EXPECTSEMICOLON/*应该是分号;*/,
		IDENTREDEFINED/*标识符重定义*/,
		TOOFEWACTPAR/*实参个数太少*/,
		ILEGALSYM/*非法符号*/,
		IDENTUNDEFINED/*标识符未定义*/,
		EXPECTIDEN/* 应是标识符 */,
		EXPECTCONST/*应是常量*/,
		EXPECTUNUM/*应该是无符号整数*/,
		EXPECTRBRACK/*应该是右方括号*/,
		EXPECTOF/*应该是of*/,
		WRONGTYPEHEAD/*错误的类型开始符号*/,
		EXPECTLBRACK/*应是左方括号*/,
		EXPECTBASICTYPE/*应该是基本类型*/,
		WRONGPARHEAD/*错误的形参开始符号*/,
		EXPECTCOLON/*应该是冒号*/,
		EXPECTRPARENT/*应该是右圆括号*/,
		EXPECTEQL/*应是等号*/,
		WRONGACTPARNUM/*形参个数与实参个数不匹配*/,
		WRONGARRAY/*没有这样的数组*/,
		WRONGFACHEAD/*错误的因子开始符号*/,
		WRONGPROCIDENT/*表达式中不能出现过程标识符*/,
		ARRAYFLOWOVER/*数组下标太大*/,
		WRONGCMPTYPE/*比较类型只能是整型或字符型*/,
		EXPECTASSIGN/*应是赋值运算符*/,
		FUNASSIGNNOTMATCH/*函数赋值语句类型不匹配*/,
		WRONGASSIGNTYPE/*赋值运算类型错误*/,
		EXPECTEND/*应是end*/,
		WRONGCONDITIONTYPE/*条件只能是整数*/,
		EXPECTTHEN/*应是then*/,
		EXPECTUNTIL/*应该是until*/,
		EXPECTVARIBLE/*应是变量*/,
		WRONGFORVAR/*for后面的循环变量只能是int或char*/,
		WRONGFOREXPR/*for语句中初值或中值表达式必须与循环变量类型相同*/,
		EXPECTTODOWNTO/*应该是to或downto*/,
		EXPECTDO/*应是do*/,
		WRONGIDEN/*应该变量/过程/函数标识符*/,
		EXPECTBEGIN/*应是begin*/,
		WRONGREADPAR/*read的参数不正确*/,
		EXPECTLPARENT/*应该是左圆括号*/,
		WRONGWRITEPAR/*write参数不正确*/,
		EXPECTPERIOD/*缺少句号*/,
		DIVZERO/*除数不能为0*/,
		WRONGPARTYP/*错误的实参类型*/
}errcode;

typedef enum  {
	IDENTIFIER/*符号表*/,BLOCK/*分程序表*/,ARRAYS/*数组信息向量表*/,
	LEVELS/* 分程序索引表 */,CODE/* 中间代码表 */,STRINGS/* 字符串常量表 */
}fatalcode;		/* 致命错误信息 */

typedef enum {
	PARNOTMATCH/*实参与形参类型不匹配*/,ASSIGNTYPE,TYPENOTMATCH
}warncode;	/* 警告信息 */

typedef struct _symset
{
	symbol symbols[setmax];	/* 集合中的元素 */
	bool mark[setmax];	/* mark[i]=true表示i在集合里，mark[i]=0表示i不在集合里 */
	index idx;
}symset;

typedef struct _item	
{
	types typ;
	index ref;
}item;			/* 项 */

typedef struct _quadruple	
{
	fct op;
	int src1;
	int src2;
	int dst;
}order;		/* 四元式 */

int tmptab[tmpmax];

typedef enum {EAX,ECX,EDX}tmpreg;	/* 临时寄存器 */
typedef enum {EBX=3,ESI,EDI}glbreg;
int regpool[tmpregnum+glbregnum];		/* 寄存器池，glbregpool[0]=1，表示寄存器EAX里目前保存的是符号表1号位置的变量 */

bool regwrtback[tmpregnum+glbregnum];	/* 标识寄存器是否该被写回内存 */
int level;	/* 当前所在的层次 */


char ch;	/* 最近一次从源程序文件里读出来的字符 */
char id[alng];	/* 最近一次词法分析得到的标识符 */
char line[llng];	/* 从源程序文件中读到的一行 */


int inum;	/* 最近一次词法分析得到的无符号整数值 */
int sleng;	/* 最近一次词法分析得到的字符串长度 */
int cc;		/* 源程序文件中一行读到的字符数 */
int ll;		/* 当前行长度 */
int errpos;	/* 出错位置 */
int t,a,b,sx;		/* 符号表、数组信息向量表、分程序表、字符串表的索引 */
int display[lmax];	/* 分程序索引表 */
int linecount;/* 行号 */
int errcnt;
int labcount;
bool iflag,oflag,skipflag,prtables,execute;
bool errs[ermax];	/* errs[i]=1表示有i号错误，否则表示没有 */

symbol sy;	/* 最近一次读到的单词类别 */

symbol sps[256];	/* 各种特殊符号对应的类别码 */

symset constbegsys,typebegsys,blockbegsys,facbegsys,statbegsys;	/* 各个语法成分的开始符号集合 */

struct _tab				/* 符号表 */
{
	char name[alng];	/* 符号名 */
	index link;			/* 指向上一符号的索引 */
	objecttype obj;		/* 符号种类（常量、变量、过程、函数） */
	types	typ;		/* 符号类型（整数、字符、数组） */
	index	ref;		/* 如果是数组的话就是指向数组信息向量表中位置的索引 */
	bool normal;		/* 标识符为变量形参时normal=false，其余normal=true */
	int lev;			/* 符号所在层次 */
	int adr;			/* 变量（形参）运行栈中的存储单元相对地址；整数值或字符的ASCII码值*/
}tab[tmax];

struct _atab			/* 数组信息向量表 */
{
	types eltyp;		/* 数组元素类型 */
	int size;			/* 数组元素个数 */
}atab[amax];

struct _btab			/* 分程序表 */
{
	index last;			/* 分程序中当前一个标识符在tab表中的位置 */
	index lastpar;		/* 过程或函数中的最后一个参数在tab表中的位置（顺着往上找到第一个link=0的地方是第一个参数） */
	index psize;		/* 参数及该分程序内务信息区所占的存储单元数 */
	index vsize;		/* 局部变量、参数及内务信息区在运行栈所占的存储单元数 */
}btab[bmax];

typedef struct _conrec	/* 常量 */
{
	types tp;	/* 常量类型 */
	int val;	/* 常量值 */
}conrec;

struct _contab			/* 常量表 */
{
	int tx;			/* 常量在tab表中的位置 */
	conrec con;		/* 常量 */
}contab[bmax];
index cnx;		/* 常量表索引 */
int concount;

char stab[smax][200];		/* 字符串常量表 */
order code[pmax][cmax];		/* 四元式序列 */
int ci,cj;					/* 四元式序列的行列索引 */
int codelevel[cmax];		/* 记录指令的层级 */

glbreg regalloc[tmax];	/* 变量的全局寄存器分配情况 */

int dx;	/* 数据分配索引 */

FILE *psin,*psout,*pcode,*pasm,*ptab;			/*输入输出文件指针*/

void initset(symset *s);	/* 初始化一个空集合 */

void insertelem(symbol elem,symset *s);	/* 向集合中插入元素 */

bool belongset(symbol elem,symset s);	/* 判断元素是否属于集合 */

symset elem2set(symbol elem);	/* 把一个元素转化成集合 */

symset setsadd(symset s1,symset s2);	/* 集合求并 */

symset elemaddset(symbol elem,symset s1);	/* 元素并入集合，返回结果集合 */

symset elemaddelem(symbol elem1,symbol elem2); /* 两个元素合并成一个集合 */

#endif
