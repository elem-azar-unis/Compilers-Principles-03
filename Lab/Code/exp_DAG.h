#include <stdlib.h>
#include <string.h>

#ifndef _EXP_DAG_H_
#define _EXP_DAG_H_
typedef struct exp_list
{
	char name[32];			//结果名称
	char exp[96];			//部分结果的运算式
	struct exp_list* next;	//以链表形式存放。
}exp_list;
#endif

//记录的添加，添加一条记录，表示exp的运算式已有结果，结果变量为name，注意采用头部添加，需要传入head指针的指针。
void DAG_add(exp_list** head,char* name,char* exp);
//记录的查询。返回1查到，0查找失败。成功的结果返回名称于name中。
int DAG_find(exp_list* head,char* name,char* exp);
//删除这个表。
void DAG_destroy(exp_list* head);