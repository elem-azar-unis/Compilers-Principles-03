#include "ir_buffer.h" 

//label,temp的计数器，用于生成新的label，temp
static int temp_count=0,label_count=0;
//代码buff链表的头指针。
static code_node* head=NULL;

//中间代码后期优化
void optimize();
//释放中间代码buffer
void ir_buffer_destroy();
//向文件中打印一行代码
void print_one_line(FILE* fp,code_node* p);

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
	optimize();
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
//中间代码后期优化
void optimize()
{
	
}