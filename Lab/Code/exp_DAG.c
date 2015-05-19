#include "exp_DAG.h"
//记录的添加，添加一条记录，表示exp的运算式已有结果，结果变脸为name
void DAG_add(exp_list** head,char* name,char* exp)
{
	exp_list* p=(exp_list*)malloc(sizeof(exp_list));
	strcpy(p->name,name);
	strcpy(p->exp,exp);
	p->next=*head;
	*head=p;
}
//记录的查询。返回1查到，0查找失败。成功的结果返回名称于name中。
int DAG_find(exp_list* head,char* name,char* exp)
{
	exp_list* p;
	while(head!=NULL)
	{
		p=head;
		head=head->next;
		if(strcmp(p->exp,exp)==0)
		{
			strcpy(name,p->name);
			return 1;
		}
	}
	return 0;
}
//删除这个表。
void DAG_destroy(exp_list* head)
{
	exp_list* p;
	while(head!=NULL)
	{
		p=head;
		head=head->next;
		free(p);
	}
}