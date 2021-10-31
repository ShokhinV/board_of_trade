#include "include/rbtree/rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


struct mynode {
  	struct rb_node node;
  	char *string;
	char type;
    int id;
	char operation;
	int count;
    
};

struct buy {
	struct rb_node node;
	char *string;
	char type;
    int id;
	char operation;
	int count;
};

int dealcount = 1;
FILE* output;
struct rb_root mytree = RB_ROOT;

struct rb_root buytree = RB_ROOT;

void my_free(struct mynode *node)
{
	if (node != NULL) {
		if (node->string) {
			free(node->string);
			node->string = NULL;
		}
		free(node);
		node = NULL;
	}
}

void buyFree(struct buy *node)
{
	if (node != NULL) {
		if (node->string) {
			free(node->string);
			node->string = NULL;
		}
		free(node);
		node = NULL;
	}
}

int deleteBuy(struct buy* buy) {
	if (buy) {
		rb_erase(&buy->node,&buytree);
		buyFree(buy);
	}else return 0;
}
int deleteSell(struct mynode* sell) {
	if (sell) {
		rb_erase(&sell->node,&mytree);
		my_free(sell);
		return 1;
	}else return 0;
}

void make_cancel(int id) {
	char string[8];
	sprintf(string,"X,%d\0",id);
	if (output) {
	fputs(string,output);
	fputs("\n",output);
	}
}

void cancel(char *order) {

	int id = atoi(strtok(strchr(order,','),","));
	
	struct rb_node *node;
	for (node = rb_first(&mytree); node; node = rb_next(node))
		if (rb_entry(node, struct mynode, node)->id == id) {
			struct mynode* data = rb_entry(node,struct mynode, node);
			if (data) {
				make_cancel(id);
				deleteSell(data);
				break;
			}
		}
		
	for (node = rb_first(&buytree); node; node = rb_next(node)) {
		if (rb_entry(node, struct buy, node)->id == id) {
			struct buy* data = rb_entry(node,struct buy, node);
			if (data) {
				make_cancel(id);
				deleteBuy(data);
				break;
			}
		}
	}
}

char* clean_prices(char * price) {
	int i = strlen(price) - 1;
	while (i!=0 && price[i] == '0' && price[i-1] != '.')
		strcpy(&price[i],&price[i+1]);
	return price;
}
void make_deal(char type,int seller,int buyer, int count, char* price) {
	char string[1 + sizeof(type) + sizeof(seller) + sizeof(buyer) + sizeof(count) + strlen(price)]; 
	sprintf(string, "T,%d,%c,%d,%d,%d,%s\0",dealcount,type,seller,buyer,count,&price[0]);
	if (output) {
	fputs(string,output);
	fputs("\n",output);
	}
	dealcount++;
}

int isreadyDeal (struct rb_root* sellroot, struct rb_root* buyroot) {
	struct rb_node* sell; 
	struct rb_node* temp;
	struct rb_node* buy;
	struct rb_node **sellnew = &(sellroot->rb_node); 
	struct rb_node **buynew = &(buyroot->rb_node); 
order:
	if (*sellnew)
	sell=rb_first(sellroot);
	else return 0;
	if (*buynew)
	buy=rb_last(buyroot);
	else return 0;
	if (sell != NULL && buy != NULL) {
	
		temp = sell;
		while (temp != NULL) {
			if (strcmp(rb_entry(sell,struct mynode,node)->string,rb_entry(temp,struct mynode,node)->string)==0 && (rb_entry(sell,struct mynode,node)->id > rb_entry(temp,struct mynode,node)->id))
				sell=temp;
			temp=rb_next(temp);	
		}	
		temp = buy;
		while (temp != NULL) {
			if (strcmp(rb_entry(buy,struct buy,node)->string,rb_entry(temp,struct buy,node)->string) == 0 && (rb_entry(buy,struct buy,node)->id > rb_entry(temp,struct buy,node)->id))
				buy=temp;
			temp=rb_prev(temp);	
			}
		if (strcmp(rb_entry(sell,struct mynode,node)->string,rb_entry(buy,struct buy,node)->string) <= 0) {
			struct mynode* seller = rb_entry(sell,struct mynode,node);
			struct buy* buyer = rb_entry(buy,struct buy,node);
			
			if (seller->count > buyer->count) {
				seller->count-=buyer->count;
				make_deal(seller->id < buyer->id ? seller->operation : buyer->operation,
				seller->id < buyer->id ? seller->id : buyer->id,
				seller->id < buyer->id ? buyer->id :seller->id,
				buyer->count,
				seller->id < buyer->id ? seller->string : buyer->string);
				deleteBuy(buyer);
				goto order;
			}else if(buyer->count > seller->count) {
				buyer->count-=seller->count;
				make_deal(seller->id < buyer->id ? seller->operation : buyer->operation,
				seller->id < buyer->id ? seller->id : buyer->id,
				seller->id < buyer->id ? buyer->id :seller->id,
				seller->count,
				seller->id < buyer->id ? seller->string : buyer->string);
				deleteSell(seller);
				goto order;
			} else {
				make_deal(seller->id < buyer->id ? seller->operation : buyer->operation,
				seller->id < buyer->id ? seller->id : buyer->id,
				seller->id < buyer->id ? buyer->id :seller->id,
				buyer->count,
				seller->id < buyer->id ? seller->string : buyer->string);
				deleteSell(seller);
				deleteBuy(buyer);
				goto order;
			}
					
		}else return 0;
	}else return 0;
		
}

struct mynode *parse(char* order) {
char type;
int id;
char string;
int count;
char* price;
int res;
struct mynode *tmp;
struct buy* temp;
switch (order[0])
{   
    case 'C':   
        cancel(order);      
        return NULL;
        
    case 'O':
		type = order[0];
		id = atoi(strtok(strchr(order,','),","));
		string=*(strtok(NULL,","));
		count=atoi(strtok(NULL,","));
		price=clean_prices(strtok(NULL,"\n"));
		switch (string) 
		{	
			case 'S' :
				tmp = (struct mynode*)malloc(sizeof(struct mynode));
				tmp->string=(char*)malloc(strlen(price));
				tmp->type=order[0];
        		tmp->id=id;
				memcpy(tmp->string,price,sizeof(tmp->string));
				tmp->count=count;
				tmp->operation=string;
				return tmp;
			case 'B' :	
				temp = (struct buy*)malloc(sizeof(struct buy));
				temp->string=(char*)malloc(strlen(price));
				temp->type=order[0];
        		temp->id=id;
				memcpy(temp->string,price,sizeof(temp->string));
				temp->count=count;
				temp->operation=string;
				return temp;
			default:
				return NULL;
		}
	default:
		return NULL;
}

}


int buyInsert(struct rb_root *root, struct buy *data)
{
  	struct rb_node **new = &(root->rb_node), *parent = NULL;

  	
  	while (*new) {
  		struct buy *this = container_of(*new, struct buy, node);
  		int result = strcmp(data->string, this->string);
		  

		parent = *new;
	
  			if (result < 0)
  				new = &((*new)->rb_left);
  			else if (result >= 0)
  				new = &((*new)->rb_right);
  			else
  				{return 0;}
  	
			  
			  
}
  	
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, root);

	return 1;
}

int my_insert(struct rb_root *root, struct mynode *data)
{
  	struct rb_node **new = &(root->rb_node), *parent = NULL;

  	
  	while (*new) {
  		struct mynode *this = container_of(*new, struct mynode, node);
  		int result = strcmp(data->string, this->string);
		

		parent = *new;
		
  			if (result <= 0)
  				new = &((*new)->rb_left);
  			else if (result > 0)
  				new = &((*new)->rb_right);
  			else
  				return 0;
  	
			  

	
	}
  	
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, root);

	return 1;
}

int main(int argc, char* argv[])
{
	char *path = (char *)malloc(256 * sizeof(char));
	int size = 256;
	char order[size];
    FILE *fp;
	size_t read = 1;
	output = fopen("./MyOutput.txt","r+");

	if (!argv[1]) {
		printf("Path to orders file\n");
    	scanf("%s",path);
    	printf("Your input file: %s\n",path);
    	fp = fopen(path, "r");
	} else {
		fp = fopen(argv[1],"r");
	}


    if (fp == NULL ) {
        printf("Bad input file path. Exiting...\n");
		exit(EXIT_FAILURE);
    
	}
	if(output == NULL) {
		printf("File MyOutput.txt does not exits! Creating....\n");
		sleep(1);
		output = fopen("./MyOutput.txt","a+");
	}
	 if (!output) {
		 printf ("Can't create output file! Check your user rights!\n");
		 exit(EXIT_FAILURE);
	 }	
	
	while (read = fgets(order,size,fp) != NULL) {	

		struct mynode* temp = parse(order);
		
		
		if (temp != NULL) {
			
				if (temp->operation == 'S') {
						my_insert(&mytree,temp);
						isreadyDeal(&mytree,&buytree);
				} else if (temp->operation == 'B') {
						buyInsert(&buytree,temp);
						isreadyDeal(&mytree,&buytree);
				}else {
					printf("File with wrong order. Simulation canceled");
					exit(EXIT_FAILURE);
				}		
		}
	}	
		
	fclose(fp);
	fclose(output);
	struct rb_node *node;
	int count = 0;
	printf("Count of active sellers on board after simulation: ");
	for (node = rb_first(&mytree); node; node = rb_next(node))
		count++;

	printf("%d\n",count);	
	
	count = 0;
	printf("Count of active buyers on board after simulation: ");
	for (node = rb_first(&buytree); node; node = rb_next(node))
		count++;
	printf("%d\n",count);


	return 0;
}
