#include "ir_buffer.h" 

//删除p指向的代码行，并且把p指向其上一行。如果这是惟一的一行，则删除后p=NULL
#define delete_code_node(p) \
do \
{ \
	if((p)->next==(p)) \
	{ \
		free(p); \
		(p)=NULL; \
		head=NULL; \
	} \
	else \
	{ \
		code_node *temp_ptr=(p); \
		(p)=(p)->prev; \
		(p)->next=temp_ptr->next; \
		(p)->next->prev=(p); \
		free(temp_ptr); \
	} \
}while(0)

//label,temp的计数器，用于生成新的label，temp
static int temp_count=0,label_count=0;
//代码buff链表的头指针。
static code_node* head=NULL;
//是否本次有优化
static int changed=0;
//优化选项
static int opt=1;

//中间代码后期优化
void optimize();
//中间代码后期优化:jump
void optimize_jump();
//释放中间代码buffer
void ir_buffer_destroy();
//向文件中打印一行代码
void print_one_line(FILE* fp,code_node* p);
//一个label是否是被使用的，（否则这个label可以删掉了）
int label_is_using(char* name)
{
	code_node* p=head;
	do
	{
		if(p->args_count==6 && strcmp(p->args[5],name)==0)
			return 1;
		else if(strcmp(p->args[0],"GOTO")==0 && strcmp(p->args[1],name)==0)
			return 1;
		p=p->next;
	}while(p!=head);
	return 0;
}
//将所有用到old label的地方替换成new label
void label_change(char* old,char* new)
{
	code_node* p=head;
	do
	{
		if(p->args_count==6 && strcmp(p->args[5],old)==0)
			strcpy(p->args[5],new);
		else if(strcmp(p->args[0],"GOTO")==0 && strcmp(p->args[1],old)==0)
			strcpy(p->args[1],new);
		p=p->next;
	}while(p!=head);
}
//得到realop的反。
void make_oppo_relop(char* op)
{
	if(strcmp(op,">")==0)
		strcpy(op,"<=");
	else if(strcmp(op,"<")==0)
		strcpy(op,">=");
	else if(strcmp(op,">=")==0)
		strcpy(op,"<");
	else if(strcmp(op,"<=")==0)
		strcpy(op,">");
	else if(strcmp(op,"==")==0)
		strcpy(op,"!=");
	else if(strcmp(op,"!=")==0)
		strcpy(op,"==");
}

void close_opt()
{
	opt=0;
}
//生成新的label，名称放在提供好的name里面。
void new_label(char* name)
{
	strcpy(name,"label");
	char temp[20];
	itoa(label_count,temp,10);
	label_count++;
	strcat(name,temp);
}
//生成新的临时变量，名称放在提供好的name里面。
void new_temp(char* name)
{
	strcpy(name,"_t");
	char temp[20];
	itoa(temp_count,temp,10);
	temp_count++;
	strcat(name,temp);
}
//添加一条代码，指明这条代码的词数，然后传入各个词语，各个词语都是char*，即传入多个字符串
void add_code(int args_count,...)
{
	code_node* p=(code_node*)malloc(sizeof(code_node));
	p->args_count=args_count;
	va_list ap;
	va_start(ap,args_count);
	for(int i=0;i<args_count;i++)
		strcpy(p->args[i],va_arg(ap,char*));
	if(head==NULL)
	{
		p->prev=p;
		p->next=p;
		head=p;
	}
	else
	{
		p->next=head;
		p->prev=head->prev;
		head->prev->next=p;
		head->prev=p;
	}
}
//将内存中的代码打印到文件中，传入新文件路径
void print_code(char* name)
{
	if(head==NULL)return;
	FILE* fp;
	fp=fopen(name,"w");
	assert(fp!=NULL);
	if(opt)optimize();
	code_node* p=head;
	print_one_line(fp,p);
	p=p->next;
	while(p!=head)
	{
		print_one_line(fp,p);
		p=p->next;
	}
	ir_buffer_destroy();
	fclose(fp);
}
//释放中间代码buffer
void ir_buffer_destroy()
{
	if(head==NULL)return;
	code_node* p=head->next,*q;
	free(head);
	while(p!=head)
	{
		q=p;
		p=p->next;
		free(q);
	}
}
//向文件中打印一行代码
void print_one_line(FILE* fp,code_node* p)
{
	fprintf(fp,"%s",p->args[0]);
	for(int i=1;i<p->args_count;i++)
		fprintf(fp," %s",p->args[i]);
	fprintf(fp,"\n");
}
//中间代码后期优化:jump
void optimize_jump()
{
	/* 首先将可能优化的if xxx goto a 优化掉,从后向前。
	 * 1、if xxx goto a; (若干个label定义); LABEL a; ==> (若干个label定义); LABEL a;
	 * 2、if xxx goto a; goto b; (若干个label定义); LABEL a; ==> if (NOT)xxx goto b; (若干个label定义); LABEL a;
	 */
	code_node* p=head->prev;
	//1
	do
	{
		if(p->args_count==6)
		{
			code_node* q=p->next;
			while(strcmp("LABEL",q->args[0])==0)
			{	
				if(strcmp(q->args[1],p->args[5])==0)
					break;
				q=q->next;
			}
			if(strcmp("LABEL",q->args[0])==0)
			{	
				changed=1;
				delete_code_node(p);
				p=p->next;
			}
		}
		p=p->prev;
	}while(p!=head->prev);
	//2
	p=head;
	do
	{
		if(p->args_count==6)
		{
			code_node* q=p->next;
			while(strcmp(q->args[0],"LABEL")==0)
				q=q->next;
			if(strcmp("GOTO",q->args[0])==0)
			{
				code_node* r=q;
				q=q->next;
				while(strcmp("LABEL",q->args[0])==0)
				{	
					if(strcmp(q->args[1],p->args[5])==0)
						break;
					q=q->next;
				}
				if(strcmp("LABEL",q->args[0])==0)
				{	
					make_oppo_relop(p->args[2]);
					strcpy(p->args[5],r->args[1]);
					delete_code_node(r);
					changed=1;
				}
			}
		}
		p=p->next;
	}while(p!=head);
	// 优化 goto a; (若干个label定义); LABEL a; ==> (若干个label定义); LABEL a; 从后向前
	p=head->prev;
	do
	{
		if(strcmp("GOTO",p->args[0])==0)
		{
			code_node* q=p->next;
			while(strcmp("LABEL",q->args[0])==0)
			{	
				if(strcmp(q->args[1],p->args[1])==0)
					break;
				q=q->next;
			}
			if(strcmp("LABEL",q->args[0])==0)
			{	
				delete_code_node(p);
				changed=1;
				p=p->next;
			}
		}
		p=p->prev;
	}while(p!=head->prev);
	/* 优化 goto a； (语句块S，没有label)；label *；表示其实中间的语句永远是不可达的，
	 * 即语句块S只能从goto开始顺序执行到，而不能从其他地方跳转进来。故语句块S可以删掉。
	*/
	p=head;
	do
	{
		if(strcmp("GOTO",p->args[0])==0)
		{
			code_node* q=p->next;
			while(q!=head && strcmp("LABEL",q->args[0])!=0)
				q=q->next;
			if(q==head)
			{
				p=p->next;
				continue;
			}
			while(p->next!=q)
			{
				changed=1;
				p=p->next;
				delete_code_node(p);
			}
		}
		p=p->next;
	}while(p!=head);
	/* goto a;...;LABEL a;(LABELs);goto b; ==> goto b;...;LABEL a;(LABELs);goto b;
	 * goto a;...;LABEL a;(LABELs);return *; ==> return *;...;LABEL a;(LABELs);return *;
	 * LABEL a;(LABELs);goto a; ==>LABEL a;(LABELs);
	 */
	p=head;
	do
	{
		if(strcmp("GOTO",p->args[0])==0)
		{
			code_node* q=p->next;
			while(strcmp("LABEL",q->args[0])!=0 || strcmp(q->args[1],p->args[1])!=0)
				q=q->next;
			while(strcmp("LABEL",q->args[0])==0)
				q=q->next;
			if(p==q)
			{
				changed=1;
				delete_code_node(p);
			}
			else if(strcmp(q->args[0],"GOTO")==0)
			{
				changed=1;
				strcpy(p->args[1],q->args[1]);
			}
			else if(strcmp(q->args[0],"RETURN")==0)
			{
				changed=1;
				strcpy(p->args[0],q->args[0]);
				strcpy(p->args[1],q->args[1]);
			}
		}
		else if(p->args_count==6)
		{
			code_node* q=p->next;
			while(strcmp("LABEL",q->args[0])!=0 || strcmp(q->args[1],p->args[5])!=0)
				q=q->next;
			while(strcmp("LABEL",q->args[0])==0)
				q=q->next;
			if(p==q)
			{
				changed=1;
				delete_code_node(p);
			}
			else if(strcmp(q->args[0],"GOTO")==0)
			{
				changed=1;
				strcpy(p->args[5],q->args[1]);
			}
		}
		p=p->next;
	}while(p!=head);
	//在return后面的语句，直到下一个label/function，都是永远不可达的。删掉他们。
	p=head;
	do
	{
		if(strcmp(p->args[0],"RETURN")==0)
			while(p->next!=head && strcmp(p->next->args[0],"LABEL")!=0 && strcmp(p->next->args[0],"FUNCTION")!=0)
			{
				p=p->next;
				changed=1;
				delete_code_node(p);
			}
		p=p->next;
	}while(p!=head);
	//label 去重。 LABEL a； LABEL b，则把所有用到b的地方都变成a，删掉b。
	p=head;
	do
	{
		if(strcmp(p->args[0],"LABEL")==0)
			while(strcmp(p->next->args[0],"LABEL")==0)
			{
				label_change(p->next->args[1],p->args[1]);
				p=p->next;
				changed=1;
				delete_code_node(p);
			}
		p=p->next;
	}while(p!=head);
	//去掉不被使用的label
	p=head;
	do
	{
		if(strcmp(p->args[0],"LABEL")==0)
			if(!label_is_using(p->args[1]))
			{	
				delete_code_node(p);
				changed=1;
			}
		p=p->next;
	}while(p!=head);
}
void optimize()
{
	do
	{
		changed=0;
		optimize_jump();
	}while(changed);
}