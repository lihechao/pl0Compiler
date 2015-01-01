#ifndef __GLOBAL_H
#define __GLOBAL_H
#include <stdio.h>
#define nkw			21		/* �ؼ��ָ��� */
#define alng		15		/* ��ʶ����󳤶� */
#define llng		121		/* һ������ĳ��� */
#define kmax		15		/* �������λ�� */
#define nmax		35367	/* ������� */
#define tmax		100		/* �����ű��� */
#define bmax		20		/* ���ֳ������ */
#define amax		30		/* ���������Ϣ�������� */
#define cmax		800		/* ����м������� */
#define lmax		8		/* ������� */
#define smax		600		/* ����ַ������� */
#define pmax		100		/* �������� */
#define ermax		58		/* ��������� */
#define fatmax		6		/* �������������� */
#define omax		63		/* �����Ԫʽ��������� */
#define linleng		132		/* ����ļ����� */
#define setmax		50		/* ������Ԫ�ص������Ŀ */
#define arrmax		500		/* �����±����ֵ */
#define labmax		100		/* ����� */
#define tmpregnum	3		/* ��ʱ�Ĵ������� */
#define glbregnum	3		/* ȫ�ּĴ������� */
#define tmpmax		200		/* �����ʱ�����ĸ��� */		


#define true		1
#define false		0

typedef int index;	/* ��int���������index */
typedef short bool;	/* ���岼���� */

typedef enum{NUL,IDEN,INTCON,CHARCON,STRCON,CONSTTK,INTTK,CHARTK,VARTK,ARRAYTK,OFTK,REPTTK,
			 UNLTK,IFTK,THENTK,ELSETK,DOTK,FORTK,TOTK,DOWNTOTK,PROCETK,FUNCTK,READTK,WRITETK,
			 BEGINTK,ENDTK,PLUS,MINU,MULT,DIV,LSS,LEQ,GRE,GEQ,EQL,NEQ,ASSIGN,SEMICN,COMMA,
			 PERIOD,COLON,QMARK,DQMARK,LPARENT,RPARENT,LBRACK,RBRACK}symbol;	/* ���ʵ������ */

typedef enum{konstant, variable, procedure, function}objecttype;	
/* ��ʶ�����࣬�����������������̻��Ǻ��� */

typedef enum{notyp,ints, chars, arrays}types;
/* ��ʶ�����ͣ����������ַ��������� */

typedef enum{
NOP/*��ָ��*/,ENTER/*�������*/,LEAVE/*�뿪����*/,SETLAB/*���ñ��*/,NEG/*��*/,
MOV/*��ֵ*/, CMP/*�Ƚ�*/,ADD/*�ӷ�*/,SUB/*����*/,IMUL/*�з�������*/,IDIV/*�з�������*/,
JE/*������ת��*/,JNE/*������ת��*/,JG/*������ת��*/,JL/*С����ת��*/,JGE/*���ڵ�����ת��*/,
JLE/*С�ڵ�����ת��*/,JMP/*������ת��*/,CALL/*����*/,RET/*����*/,READ/*��*/,WRITE/*д*/,
INC/*����1*/,DEC/*�Լ�1*/,LOADARRAY/*��ȡ����Ԫ��*/,STOARRAY/*�洢����Ԫ��*/,
PUSH/*��ջ*/,POP/*��ջ*/}fct;		/* ָ�����Ƿ� */

typedef enum {
		NUMOVERFLOW/*��̫�����*/,
		WRONGCHARCON/*�޷�ʶ����ַ�����*/,
		EXPECTQMARK/*ӦΪ������'*/,
		EXPECTDQMARK/*ӦΪ˫����"*/,
		ILEGALCHAR/*�Ƿ��ַ�*/,
		EXPECTSEMICOLON/*Ӧ���Ƿֺ�;*/,
		IDENTREDEFINED/*��ʶ���ض���*/,
		TOOFEWACTPAR/*ʵ�θ���̫��*/,
		ILEGALSYM/*�Ƿ�����*/,
		IDENTUNDEFINED/*��ʶ��δ����*/,
		EXPECTIDEN/* Ӧ�Ǳ�ʶ�� */,
		EXPECTCONST/*Ӧ�ǳ���*/,
		EXPECTUNUM/*Ӧ�����޷�������*/,
		EXPECTRBRACK/*Ӧ�����ҷ�����*/,
		EXPECTOF/*Ӧ����of*/,
		WRONGTYPEHEAD/*��������Ϳ�ʼ����*/,
		EXPECTLBRACK/*Ӧ��������*/,
		EXPECTBASICTYPE/*Ӧ���ǻ�������*/,
		WRONGPARHEAD/*������βο�ʼ����*/,
		EXPECTCOLON/*Ӧ����ð��*/,
		EXPECTRPARENT/*Ӧ������Բ����*/,
		EXPECTEQL/*Ӧ�ǵȺ�*/,
		WRONGACTPARNUM/*�βθ�����ʵ�θ�����ƥ��*/,
		WRONGARRAY/*û������������*/,
		WRONGFACHEAD/*��������ӿ�ʼ����*/,
		WRONGPROCIDENT/*���ʽ�в��ܳ��ֹ��̱�ʶ��*/,
		ARRAYFLOWOVER/*�����±�̫��*/,
		WRONGCMPTYPE/*�Ƚ�����ֻ�������ͻ��ַ���*/,
		EXPECTASSIGN/*Ӧ�Ǹ�ֵ�����*/,
		FUNASSIGNNOTMATCH/*������ֵ������Ͳ�ƥ��*/,
		WRONGASSIGNTYPE/*��ֵ�������ʹ���*/,
		EXPECTEND/*Ӧ��end*/,
		WRONGCONDITIONTYPE/*����ֻ��������*/,
		EXPECTTHEN/*Ӧ��then*/,
		EXPECTUNTIL/*Ӧ����until*/,
		EXPECTVARIBLE/*Ӧ�Ǳ���*/,
		WRONGFORVAR/*for�����ѭ������ֻ����int��char*/,
		WRONGFOREXPR/*for����г�ֵ����ֵ���ʽ������ѭ������������ͬ*/,
		EXPECTTODOWNTO/*Ӧ����to��downto*/,
		EXPECTDO/*Ӧ��do*/,
		WRONGIDEN/*Ӧ�ñ���/����/������ʶ��*/,
		EXPECTBEGIN/*Ӧ��begin*/,
		WRONGREADPAR/*read�Ĳ�������ȷ*/,
		EXPECTLPARENT/*Ӧ������Բ����*/,
		WRONGWRITEPAR/*write��������ȷ*/,
		EXPECTPERIOD/*ȱ�پ��*/,
		DIVZERO/*��������Ϊ0*/,
		WRONGPARTYP/*�����ʵ������*/
}errcode;

typedef enum  {
	IDENTIFIER/*���ű�*/,BLOCK/*�ֳ����*/,ARRAYS/*������Ϣ������*/,
	LEVELS/* �ֳ��������� */,CODE/* �м����� */,STRINGS/* �ַ��������� */
}fatalcode;		/* ����������Ϣ */

typedef enum {
	PARNOTMATCH/*ʵ�����β����Ͳ�ƥ��*/,ASSIGNTYPE,TYPENOTMATCH
}warncode;	/* ������Ϣ */

typedef struct _symset
{
	symbol symbols[setmax];	/* �����е�Ԫ�� */
	bool mark[setmax];	/* mark[i]=true��ʾi�ڼ����mark[i]=0��ʾi���ڼ����� */
	index idx;
}symset;

typedef struct _item	
{
	types typ;
	index ref;
}item;			/* �� */

typedef struct _quadruple	
{
	fct op;
	int src1;
	int src2;
	int dst;
}order;		/* ��Ԫʽ */

int tmptab[tmpmax];

typedef enum {EAX,ECX,EDX}tmpreg;	/* ��ʱ�Ĵ��� */
typedef enum {EBX=3,ESI,EDI}glbreg;
int regpool[tmpregnum+glbregnum];		/* �Ĵ����أ�glbregpool[0]=1����ʾ�Ĵ���EAX��Ŀǰ������Ƿ��ű�1��λ�õı��� */

bool regwrtback[tmpregnum+glbregnum];	/* ��ʶ�Ĵ����Ƿ�ñ�д���ڴ� */
int level;	/* ��ǰ���ڵĲ�� */


char ch;	/* ���һ�δ�Դ�����ļ�����������ַ� */
char id[alng];	/* ���һ�δʷ������õ��ı�ʶ�� */
char line[llng];	/* ��Դ�����ļ��ж�����һ�� */


int inum;	/* ���һ�δʷ������õ����޷�������ֵ */
int sleng;	/* ���һ�δʷ������õ����ַ������� */
int cc;		/* Դ�����ļ���һ�ж������ַ��� */
int ll;		/* ��ǰ�г��� */
int errpos;	/* ����λ�� */
int t,a,b,sx;		/* ���ű�������Ϣ�������ֳ�����ַ���������� */
int display[lmax];	/* �ֳ��������� */
int linecount;/* �к� */
int errcnt;
int labcount;
bool iflag,oflag,skipflag,prtables,execute;
bool errs[ermax];	/* errs[i]=1��ʾ��i�Ŵ��󣬷����ʾû�� */

symbol sy;	/* ���һ�ζ����ĵ������ */

symbol sps[256];	/* ����������Ŷ�Ӧ������� */

symset constbegsys,typebegsys,blockbegsys,facbegsys,statbegsys;	/* �����﷨�ɷֵĿ�ʼ���ż��� */

struct _tab				/* ���ű� */
{
	char name[alng];	/* ������ */
	index link;			/* ָ����һ���ŵ����� */
	objecttype obj;		/* �������ࣨ���������������̡������� */
	types	typ;		/* �������ͣ��������ַ������飩 */
	index	ref;		/* ���������Ļ�����ָ��������Ϣ��������λ�õ����� */
	bool normal;		/* ��ʶ��Ϊ�����β�ʱnormal=false������normal=true */
	int lev;			/* �������ڲ�� */
	int adr;			/* �������βΣ�����ջ�еĴ洢��Ԫ��Ե�ַ������ֵ���ַ���ASCII��ֵ*/
}tab[tmax];

struct _atab			/* ������Ϣ������ */
{
	types eltyp;		/* ����Ԫ������ */
	int size;			/* ����Ԫ�ظ��� */
}atab[amax];

struct _btab			/* �ֳ���� */
{
	index last;			/* �ֳ����е�ǰһ����ʶ����tab���е�λ�� */
	index lastpar;		/* ���̻����е����һ��������tab���е�λ�ã�˳�������ҵ���һ��link=0�ĵط��ǵ�һ�������� */
	index psize;		/* �������÷ֳ���������Ϣ����ռ�Ĵ洢��Ԫ�� */
	index vsize;		/* �ֲ�������������������Ϣ��������ջ��ռ�Ĵ洢��Ԫ�� */
}btab[bmax];

typedef struct _conrec	/* ���� */
{
	types tp;	/* �������� */
	int val;	/* ����ֵ */
}conrec;

struct _contab			/* ������ */
{
	int tx;			/* ������tab���е�λ�� */
	conrec con;		/* ���� */
}contab[bmax];
index cnx;		/* ���������� */
int concount;

char stab[smax][200];		/* �ַ��������� */
order code[pmax][cmax];		/* ��Ԫʽ���� */
int ci,cj;					/* ��Ԫʽ���е��������� */
int codelevel[cmax];		/* ��¼ָ��Ĳ㼶 */

glbreg regalloc[tmax];	/* ������ȫ�ּĴ���������� */

int dx;	/* ���ݷ������� */

FILE *psin,*psout,*pcode,*pasm,*ptab;			/*��������ļ�ָ��*/

void initset(symset *s);	/* ��ʼ��һ���ռ��� */

void insertelem(symbol elem,symset *s);	/* �򼯺��в���Ԫ�� */

bool belongset(symbol elem,symset s);	/* �ж�Ԫ���Ƿ����ڼ��� */

symset elem2set(symbol elem);	/* ��һ��Ԫ��ת���ɼ��� */

symset setsadd(symset s1,symset s2);	/* ������ */

symset elemaddset(symbol elem,symset s1);	/* Ԫ�ز��뼯�ϣ����ؽ������ */

symset elemaddelem(symbol elem1,symbol elem2); /* ����Ԫ�غϲ���һ������ */

#endif
