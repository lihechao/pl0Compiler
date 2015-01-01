#ifndef __PL0COMPILER_H
#define __PL0COMPILER_H

symbol ksy[nkw]={ARRAYTK,BEGINTK,CHARTK,CONSTTK,DOTK,DOWNTOTK,ELSETK,
ENDTK,FORTK,FUNCTK,IFTK,INTTK,OFTK,PROCETK,READTK,REPTTK,
THENTK,TOTK,UNLTK,VARTK,WRITETK};	/* �ؼ��ֶ�Ӧ������� */
const char key[nkw][alng]={"array","begin","char","const","do","downto","else",
"end","for","function","if","integer","of","procedure",
"read","repeat","then","to","until","var","write"};		/* �ؼ��� */

const char objword[][alng]={"constant","variable","procedure","function"};

const char typword[][alng]={"notyp","integer","char","array"};

const char fctword[][alng]={"NOP     "/*��ָ��*/,"ENTER   ","LEAVE   ","SETLAB  "/*���ñ��*/,
"NEG     "/*��*/,"MOV     ","CMP     ", "ADD     ","SUB     ","IMUL    ","IDIV    ","JE      ","JNE     ",
"JG      ","JL      ","JGE     ","JLE     ","JMP     ","CALL    ","RET     ","READ    ","WRITE   ",
"INC     ","DEC     ","LOADARRAY","STOARRAY","PUSH    ","POP    "};	/* ָ�����Ƿ� */

const char regword[][4]={"eax","ecx","edx","ebx","esi","edi"};

void errormsg();	/* ��ӡ������Ϣ��ժҪ */

void endskip();		/* Դ�����б������Ĳ������滮�߱�� */

void error(errcode n);		/* �ڵ�ǰλ�ã�cc������ӡ����λ�úͳ����� */

void fatal(fatalcode n);	/* ��ӡ�������󣨱�������Ϣ�� */

void skip(symset fsys,errcode n);	/* ����Դ����ֱ��ȡ���ķ������ڸ����ķ��ż�Ϊֹ������ӡ������ */

void test(symset s1,symset s2,errcode n);	/* ���Ե�ǰ�����Ƿ�Ϸ��������Ϸ�����ӡ�����־���������� */

void testsemicolon(symset fsys);	/* ���Ե�ǰ�����Ƿ�Ϊ�ֺ� */

void switchs(bool *b);	/* ��������ѡ���е�'+'��'-'��־ */

void options();	/* �������ʱ�Ŀ�ѡ�� */

void nextch();	/* ��ȡ��һ�ַ��������н�������ӡ���������Դ���� */

void insymbol();	/* ��ȡ��һ���ʷ��ţ�����ע���� */

void enter(char id[],objecttype k);	/* �ֳ����ڣ��ڷ��ű��е�¼�ֳ���˵�����ֳ��ֵı�ʶ�� */

void enterarray(types type,int size);/* ��¼������Ϣ������atab */

void enterblock();/* ��¼�ֳ����btab */

void entercontab();	/* ��¼������ */

void entervariable();	/* ����������¼�����ű��� */


int loc(char id[]);	/* ���ұ�ʶ���ڷ��ű��е�λ�� */

int loccontab(int val,types type);	/* ���ҳ����ڳ������е�λ�� */

void emit(fct op);	/* ����ֻ��ָ�����Ƿ�����Ԫʽ */

void emit1(fct op,int dst);	/* ����ֻ�����Ƿ���dst����Ԫʽ */

void emit2(fct op,int src1,int dst);	/* ����ֻ�����Ƿ���src1��dst����Ԫʽ */

void emit3(fct op,int src1,int src2,int dst);	/* ���������Ƿ���src1��src2��dst����Ԫʽ */

/* ��������г��ֵĳ���������c���س������ͺ�ֵ
<����>	::=	[+| -] <�޷�������>|<�ַ�>*/
void constant(symset fsys,conrec *c);

/* ���������壬�����������������Ϣ��¼�����ű��� 
	<��������>	::=	<��ʶ��>= <����>*/
void constantdef(symset fsys);

/* ������˵�����֣�������������Ӧ��Ϣ������ű�
<����˵������>	::=  const<��������>{,<��������>}; */
void constdec(symset fsys);

void arraytyp(symset fsys,int *rf,types *tp);	/* �����������ͣ�array'['<�޷�������>']' of <��������> */

/* ������������
<��������>	::=   integer | char
<����>	::=	<��������>|array'['<�޷�������>']' of <��������> */
void typ(symset fsys,types *tp,int *rf,types *tp1);

/* �����������
<����˵������>	::=  var <����˵��> ; {<����˵��>;}
<����˵��>		::=  <��ʶ��>{, <��ʶ��>} :  <����>*/
void variabledeclaration(symset fsys);

/* ������̻���˵���е��βα����βμ����й���Ϣ��¼�����ű��� 
<��ʽ������>	::= [var] <��ʶ��>{, <��ʶ��>}: <��������>{; <��ʽ������>}*/
void parameterlist(symset fsys);

/*	������̻���˵�� 
	<����˵������>		::=	<�����ײ�><�ֳ���>{;<�����ײ�><�ֳ���>};
	<����˵������>		::=	<�����ײ�><�ֳ���>{;<�����ײ�><�ֳ���>};
	<�����ײ�>			::=	procedure<��ʶ��>'('[<��ʽ������>]')';
	<�����ײ�>			::=	function <��ʶ��>'('[<��ʽ������>]')': <��������>;
*/
void procdeclaration(symset fsys);

/*<�ֳ���>	::=   [<����˵������>][<����˵������>]{[<����˵������>]| [<����˵������>]}<�������>*/
void block(symset fsys);

void factor(symset fsys,item *x);	

void term(symset fsys,item *x);	/* ������ <��>	::=	<����>{<�˷������><����>} */

void expression(symset fsys,item *x);	/* ������ʽ<���ʽ>	::=	[+|-]<��>{<�ӷ������><��>} */

void condition(symset fsys,item *x,item *y,symbol *op);	/* ��������<����>	::=	<���ʽ><��ϵ�����><���ʽ> */

void assignment(symset fsys,int i);	/* ����ֵ��� */

void compoundstatement(symset fsys);	/* ��������� */

void ifstatement(symset fsys);	/* ����if��� <�������>	::=	if<����>then<���> | if<����>then<���>else<���>*/

void repeatstatement();	/* <repeatѭ�����>  ::=  repeat <���>until<����> */

void forstatement(symset fsys);/*<forѭ�����>::=  for <��ʶ��> := <���ʽ> (to|downto) <���ʽ> do <���>  */

void statement(symset fsys);

void call(symset fsys,int i);	/* ������̻������� i������/������tab�е�����ֵ */

/* ��������
<�����>	::=  read'('<��ʶ��>{,<��ʶ��>}')' */
void readstatement(symset fsys);

/* ����д���
<д���>	::=   write'(' <�ַ���>,<���ʽ> ')'|write'(' <�ַ���>')'|write'('<���ʽ>')' 
�����������䣬����������޶���ʽ����������write'('[<�ַ���>,]<���ʽ>{,<���ʽ>}')'	*/
void writestatement(symset fsys);
#endif

	