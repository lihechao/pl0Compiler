#include "global.h"
#include <string.h>
void initset(symset *s)	/* 初始化一个空集合 */
{
	s->idx=0;
	memset(s->mark,false,sizeof(s->mark));
}

void insertelem(symbol elem,symset *s)	/* 向集合中插入元素 */
{
	if (!s->mark[elem])	/* 元素不在集合里才插入 */
	{
		s->symbols[s->idx++]=elem;
		s->mark[elem]=true;
	}
}

bool belongset(symbol elem,symset s)	/* 判断元素是否属于集合 */
{
	return s.mark[elem];
}

symset elem2set(symbol elem)	/* 把一个元素转化成集合 */
{
	symset s;
	initset(&s);
	insertelem(elem,&s);
	return s;
}

symset setsadd(symset s1,symset s2)	/* 集合求并 */
{
	symset s;		/* s=s1+s2 */
	int i;
	initset(&s);	/* 初始化集合s */
	for (i=0;i<s1.idx;i++)	/* s1的元素加入s */
	{
		insertelem(s1.symbols[i],&s);
	}
	for (i=0;i<s2.idx;i++)	/* s2的元素加入s */
	{
		insertelem(s2.symbols[i],&s);
	}
	return s;	/* 返回s=s1+s2 */
}

symset elemaddset(symbol elem,symset s1)	/* 元素并入集合，返回结果集合 */
{
	symset s=s1;
	insertelem(elem,&s);
	return s;	/* 返回s={elem}+s1 */
}

symset elemaddelem(symbol elem1,symbol elem2) /* 两个元素合并成一个集合 */
{
	symset s;
	initset(&s);			/* s={} */
	insertelem(elem1,&s);	/* s={elem1} */
	insertelem(elem2,&s);	/* s={elem1,elem2} */
	return s;
}
