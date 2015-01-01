#ifndef __PL0COMPILER_H
#define __PL0COMPILER_H

symbol ksy[nkw]={ARRAYTK,BEGINTK,CHARTK,CONSTTK,DOTK,DOWNTOTK,ELSETK,
ENDTK,FORTK,FUNCTK,IFTK,INTTK,OFTK,PROCETK,READTK,REPTTK,
THENTK,TOTK,UNLTK,VARTK,WRITETK};	/* 关键字对应的类别码 */
const char key[nkw][alng]={"array","begin","char","const","do","downto","else",
"end","for","function","if","integer","of","procedure",
"read","repeat","then","to","until","var","write"};		/* 关键字 */

const char objword[][alng]={"constant","variable","procedure","function"};

const char typword[][alng]={"notyp","integer","char","array"};

const char fctword[][alng]={"NOP     "/*空指令*/,"ENTER   ","LEAVE   ","SETLAB  "/*设置标号*/,
"NEG     "/*求负*/,"MOV     ","CMP     ", "ADD     ","SUB     ","IMUL    ","IDIV    ","JE      ","JNE     ",
"JG      ","JL      ","JGE     ","JLE     ","JMP     ","CALL    ","RET     ","READ    ","WRITE   ",
"INC     ","DEC     ","LOADARRAY","STOARRAY","PUSH    ","POP    "};	/* 指令助记符 */

const char regword[][4]={"eax","ecx","edx","ebx","esi","edi"};

void errormsg();	/* 打印错误信息的摘要 */

void endskip();		/* 源程序中被跳过的部分下面划线标记 */

void error(errcode n);		/* 在当前位置（cc处）打印出错位置和出错编号 */

void fatal(fatalcode n);	/* 打印致命错误（表格溢出信息） */

void skip(symset fsys,errcode n);	/* 跳读源程序，直至取来的符号属于给出的符号集为止，并打印出错标记 */

void test(symset s1,symset s2,errcode n);	/* 测试当前符号是否合法，若不合法，打印出错标志并进行跳读 */

void testsemicolon(symset fsys);	/* 测试当前符号是否为分号 */

void switchs(bool *b);	/* 处理编译可选项中的'+'、'-'标志 */

void options();	/* 处理编译时的可选项 */

void nextch();	/* 读取下一字符，处理行结束符，印出被编译的源程序 */

void insymbol();	/* 读取下一单词符号，处理注释行 */

void enter(char id[],objecttype k);	/* 分程序内，在符号表中登录分程序说明部分出现的标识符 */

void enterarray(types type,int size);/* 登录数组信息向量表atab */

void enterblock();/* 登录分程序表btab */

void entercontab();	/* 登录常量表 */

void entervariable();	/* 将变量名登录到符号表中 */


int loc(char id[]);	/* 查找标识符在符号表中的位置 */

int loccontab(int val,types type);	/* 查找常量在常量表中的位置 */

void emit(fct op);	/* 生成只有指令助记符的四元式 */

void emit1(fct op,int dst);	/* 生成只有助记符与dst的四元式 */

void emit2(fct op,int src1,int dst);	/* 生成只有助记符、src1与dst的四元式 */

void emit3(fct op,int src1,int src2,int dst);	/* 生成有助记符、src1、src2与dst的四元式 */

/* 处理程序中出现的常量，并由c返回常量类型和值
<常量>	::=	[+| -] <无符号整数>|<字符>*/
void constant(symset fsys,conrec *c);

/* 处理常量定义，并将常量名及相关信息登录到符号表中 
	<常量定义>	::=	<标识符>= <常量>*/
void constantdef(symset fsys);

/* 处理常量说明部分，将常量名及相应信息填入符号表
<常量说明部分>	::=  const<常量定义>{,<常量定义>}; */
void constdec(symset fsys);

void arraytyp(symset fsys,int *rf,types *tp);	/* 处理数组类型：array'['<无符号整数>']' of <基本类型> */

/* 处理类型描述
<基本类型>	::=   integer | char
<类型>	::=	<基本类型>|array'['<无符号整数>']' of <基本类型> */
void typ(symset fsys,types *tp,int *rf,types *tp1);

/* 处理变量声明
<变量说明部分>	::=  var <变量说明> ; {<变量说明>;}
<变量说明>		::=  <标识符>{, <标识符>} :  <类型>*/
void variabledeclaration(symset fsys);

/* 处理过程或函数说明中的形参表，将形参及其有关信息登录到符号表中 
<形式参数表>	::= [var] <标识符>{, <标识符>}: <基本类型>{; <形式参数表>}*/
void parameterlist(symset fsys);

/*	处理过程或函数说明 
	<过程说明部分>		::=	<过程首部><分程序>{;<过程首部><分程序>};
	<函数说明部分>		::=	<函数首部><分程序>{;<函数首部><分程序>};
	<过程首部>			::=	procedure<标识符>'('[<形式参数表>]')';
	<函数首部>			::=	function <标识符>'('[<形式参数表>]')': <基本类型>;
*/
void procdeclaration(symset fsys);

/*<分程序>	::=   [<常量说明部分>][<变量说明部分>]{[<过程说明部分>]| [<函数说明部分>]}<复合语句>*/
void block(symset fsys);

void factor(symset fsys,item *x);	

void term(symset fsys,item *x);	/* 处理项 <项>	::=	<因子>{<乘法运算符><因子>} */

void expression(symset fsys,item *x);	/* 处理表达式<表达式>	::=	[+|-]<项>{<加法运算符><项>} */

void condition(symset fsys,item *x,item *y,symbol *op);	/* 处理条件<条件>	::=	<表达式><关系运算符><表达式> */

void assignment(symset fsys,int i);	/* 处理赋值语句 */

void compoundstatement(symset fsys);	/* 处理复合语句 */

void ifstatement(symset fsys);	/* 处理if语句 <条件语句>	::=	if<条件>then<语句> | if<条件>then<语句>else<语句>*/

void repeatstatement();	/* <repeat循环语句>  ::=  repeat <语句>until<条件> */

void forstatement(symset fsys);/*<for循环语句>::=  for <标识符> := <表达式> (to|downto) <表达式> do <语句>  */

void statement(symset fsys);

void call(symset fsys,int i);	/* 处理过程或函数调用 i：函数/过程在tab中的索引值 */

/* 处理读语句
<读语句>	::=  read'('<标识符>{,<标识符>}')' */
void readstatement(symset fsys);

/* 处理写语句
<写语句>	::=   write'(' <字符串>,<表达式> ')'|write'(' <字符串>')'|write'('<表达式>')' 
这里做了扩充，可以输出无限多表达式，即可以有write'('[<字符串>,]<表达式>{,<表达式>}')'	*/
void writestatement(symset fsys);
#endif

	