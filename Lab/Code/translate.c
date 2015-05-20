#include "translate.h" 

/* 
 * 翻译exp，其中的option表示place的主动、被动。 
 * 0：被动：place上层仅仅提供了位置，可以没有左值。你填写任何，上层会直接使用，比如填写立即数也可以。
 * 1：主动：place是由上层给定的，确定的变量，请将你的结果放入place
 * 如果place==NULL，并且option是被动“0”，则本层不要生成变量返回该exp的结果。
 */
void translate_exp(Node* h,char* place,int option);
//翻译COND，由上层提供跳转label
void translate_cond(Node*h,char* label_true,char* label_false);

void translate(Node* h)
{
	if(h==NULL)return;
	switch(h->type)
	{
		case Specifier:return;
		case FunDec:
		{
			func_d* temp=find_function(h->child[0]->name);
			add_code(3,"FUNCTION",h->child[0]->name,":");
			for(int i=0;i<temp->parameter_count;i++)
				add_code(2,"PARAM",temp->parameter_list[i]->name);
			break;
		}
		case Dec:
		{
			Node* p=h->child[0]->child[0];
			if(p->type!=_ID)
			{
				p=p->child[0];
				if(p->type!=_ID)
				{
					printf("Cannot translate: Code contains variables of multi-dimensional \
					array type or parameters of array type.\n");
					exit(0);
				}
			}
			val_d* temp=find_value(p->name);
			if(temp->kind==USER_DEFINED)
			{
				char length[8];
				itoa(struct_get_size(temp->val_type),length,10);
				add_code(3,"DEC",temp->name,length);
			}
			if(h->child_count==3)
			{
				translate_exp(h->child[2],temp->name,1);
			}
			break;
		}
		case Stmt:
		{
			//根据第一个子结点的不同，分别作不同的处理
			switch(h->child[0]->type)
			{
				case Exp:
				{
					translate_exp(h->child[0],NULL,0);
					break;
				}
				case CompSt:
				{
					translate(h->child[0]);
					break;
				}
				case _RETURN:
				{
					char temp[32];
					translate_exp(h->child[1],temp,0);
					add_code(2,"RETURN",temp);
					break;
				}
				case _IF:
				{
					char label1[32],label2[32],label3[32];
					new_label(label1);
					new_label(label2);
					if(h->child_count==7)
					{
						new_label(label3);
					}
					translate_cond(h->child[2],label1,label2);
					add_code(3,"LABEL",label1,":");
					translate(h->child[4]);
					if(h->child_count==7)
						add_code(2,"GOTO",label3);
					add_code(3,"LABEL",label2,":");
					if(h->child_count==7)
					{
						translate(h->child[6]);
						add_code(3,"LABEL",label3,":");
					}
					break;
				}
				case _WHILE:
				{
					char label1[32],label2[32],label3[32];
					new_label(label1);
					new_label(label2);
					new_label(label3);
					add_code(3,"LABEL",label1,":");
					translate_cond(h->child[2],label2,label3);
					add_code(3,"LABEL",label2,":");
					translate(h->child[4]);
					add_code(2,"GOTO",label1);
					add_code(3,"LABEL",label3,":");
					break;
				}
			}
			break;
		}
		default:
		{
			for(int i=0;i<h->child_count;i++)
				translate(h->child[i]);
			break;
		}
	}
}

void translate_exp(Node* h,char* place,int option)
{
	if(h->child[0]->type==_INT)
	{
		if(place==NULL)return;
		if(option==0)
		{
			sprintf(place,"#%d",h->child[0]->value_i);
		}
		else
		{
			char temp[32];
			sprintf(temp,"#%d",h->child[0]->value_i);
			add_code(3,place,":=",temp);
		}
	}
	if(h->child[0]->type==_FLOAT)
	{
		if(place==NULL)return;
		if(option==0)
		{
			sprintf(place,"#%f",h->child[0]->value_f);
		}
		else
		{
			char temp[32];
			sprintf(temp,"#%f",h->child[0]->value_f);
			add_code(3,place,":=",temp);
		}
	}
	if(h->child[0]->type==_ID && h->child_count==1)
	{
		if(place==NULL)return;
		if(option==0)
		{
			strcpy(place,h->child[0]->name);
		}
		else
		{
			add_code(3,place,":=",h->child[0]->name);
		}
	}
}
void translate_cond(Node*h,char* label_true,char* label_false)
{
	if(h->child[0]->type==_NOT)
		return translate_cond(h->child[1],label_false,label_true);
	if(h->child_count==3 && h->child[1]->type==_RELOP)
	{
		char temp1[32],temp2[32];
		translate_exp(h->child[0],temp1,0);
		translate_exp(h->child[2],temp2,0);
		if(temp1[0]=='#' && temp2[0]=='#')
		{
			int i1=strtol(&temp1[1],NULL,10);
			int i2=strtol(&temp2[1],NULL,10);
			if(strcmp(h->child[1]->name,">")==0)
			{
				if(i1>i2)
					add_code(2,"GOTO",label_true);
				else
					add_code(2,"GOTO",label_false);
			}
			if(strcmp(h->child[1]->name,"<")==0)
			{
				if(i1<i2)
					add_code(2,"GOTO",label_true);
				else
					add_code(2,"GOTO",label_false);
			}
			if(strcmp(h->child[1]->name,">=")==0)
			{
				if(i1>=i2)
					add_code(2,"GOTO",label_true);
				else
					add_code(2,"GOTO",label_false);
			}
			if(strcmp(h->child[1]->name,"<=")==0)
			{
				if(i1<=i2)
					add_code(2,"GOTO",label_true);
				else
					add_code(2,"GOTO",label_false);
			}
			if(strcmp(h->child[1]->name,"==")==0)
			{
				if(i1==i2)
					add_code(2,"GOTO",label_true);
				else
					add_code(2,"GOTO",label_false);
			}
			if(strcmp(h->child[1]->name,"!=")==0)
			{
				if(i1!=i2)
					add_code(2,"GOTO",label_true);
				else
					add_code(2,"GOTO",label_false);
			}
		}
		else
		{
			add_code(6,"IF",temp1,h->child[1]->name,temp2,"GOTO",label_true);
			add_code(2,"GOTO",label_false);
		}
	}
	else if(h->child_count==3 && h->child[1]->type==_AND)
	{
		char label[32];
		new_label(label);
		translate_cond(h->child[0],label,label_false);
		add_code(3,"LABEL",label,":");
		translate_cond(h->child[2],label_true,label_false);
	}
	else if(h->child_count==3 && h->child[1]->type==_OR)
	{
		char label[32];
		new_label(label);
		translate_cond(h->child[0],label_true,label);
		add_code(3,"LABEL",label,":");
		translate_cond(h->child[2],label_true,label_false);
	}
	else
	{
		char temp[32];
		translate_exp(h,temp,0);
		if(temp[0]=='#')
		{
			if(temp[1]=='0')
				add_code(2,"GOTO",label_false);
			else
				add_code(2,"GOTO",label_true);
		}
		else
		{
			add_code(6,"IF",temp,"!=","#0","GOTO",label_true);
			add_code(2,"GOTO",label_false);
		}
	}
}