#include "global.h"
#include <string.h>
void initset(symset *s)	/* ��ʼ��һ���ռ��� */
{
	s->idx=0;
	memset(s->mark,false,sizeof(s->mark));
}

void insertelem(symbol elem,symset *s)	/* �򼯺��в���Ԫ�� */
{
	if (!s->mark[elem])	/* Ԫ�ز��ڼ�����Ų��� */
	{
		s->symbols[s->idx++]=elem;
		s->mark[elem]=true;
	}
}

bool belongset(symbol elem,symset s)	/* �ж�Ԫ���Ƿ����ڼ��� */
{
	return s.mark[elem];
}

symset elem2set(symbol elem)	/* ��һ��Ԫ��ת���ɼ��� */
{
	symset s;
	initset(&s);
	insertelem(elem,&s);
	return s;
}

symset setsadd(symset s1,symset s2)	/* ������ */
{
	symset s;		/* s=s1+s2 */
	int i;
	initset(&s);	/* ��ʼ������s */
	for (i=0;i<s1.idx;i++)	/* s1��Ԫ�ؼ���s */
	{
		insertelem(s1.symbols[i],&s);
	}
	for (i=0;i<s2.idx;i++)	/* s2��Ԫ�ؼ���s */
	{
		insertelem(s2.symbols[i],&s);
	}
	return s;	/* ����s=s1+s2 */
}

symset elemaddset(symbol elem,symset s1)	/* Ԫ�ز��뼯�ϣ����ؽ������ */
{
	symset s=s1;
	insertelem(elem,&s);
	return s;	/* ����s={elem}+s1 */
}

symset elemaddelem(symbol elem1,symbol elem2) /* ����Ԫ�غϲ���һ������ */
{
	symset s;
	initset(&s);			/* s={} */
	insertelem(elem1,&s);	/* s={elem1} */
	insertelem(elem2,&s);	/* s={elem1,elem2} */
	return s;
}
