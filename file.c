#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK 100000
#define BLOCK_SZIE 10000

typedef struct directory{
	char name[20];
	int dir_p;
	int data_p;
	struct directory *upper;
	struct directory *deep;
	struct directory *next;
	struct data *data;
}DIR;

typedef struct data{
	char name[20];
	int location;
	struct data *next;
}DATA;

void analysis(char input[50],char command[20],char parameter[30]){
	char i = 0;
	char c = -1;
	char count = 0;
	while(1){
		c = input[i++];
		if (c == 32){
			command[count++] = 0;
			break;
		}
		if (c == 0){
			command[count++] = 0;
			return;
		}
		command[count++] = c;
	}
	count = 0;
	while(1){
		c = input[i++];
		if (c == 32){
			parameter[count++] = 0;
			break;
		}
		if (c == 0){
			parameter[count++] = 0;
			return;
		}
		parameter[count++] = c;
	}
}

void root_init(DIR *root){
	root->upper = NULL;
	root->next = NULL;
	root->data_p = 0;
	root->dir_p = 0;
	root->data = NULL;
	root->deep = NULL;
}

void mkdir(DIR *current,char parameter[30]){
	DIR * new = malloc(sizeof(DIR));
	DIR * c_dir = current;
	root_init(new);
	new->upper = current;
	strcpy(new->name,parameter);
	if(current->deep == NULL){
		current->deep = new;
		current->dir_p++;
		return;
	}
	c_dir = current->deep;
	while(1){
		if(strcmp(c_dir->name,parameter)==0){
			printf("mkdir: cannot create directory ‘%s’: File exists\n",parameter);
			free(new);
			return;
		}
		if(c_dir->next == NULL){
			c_dir->next = new;
			current->dir_p++;
			return;
		}
		c_dir = c_dir->next;
	}
	
}

void ls(DIR *current){
	DIR *c_dir = current->deep;
	printf("directory: ");
	while(c_dir != NULL){
		printf("%s ",c_dir->name);
		c_dir = c_dir->next;
	}
	DATA *c_data = current->data;
	printf("\ndata: ");
	while(c_data !=NULL ){
		printf("%s ",c_data->name);
		c_data = c_data->next;
	}
	printf("\n");
}

void pwd(DIR *current){
	if(current->upper != NULL){
		pwd(current->upper);
		printf("/%s",current->name);
	}
}


DIR *cd(DIR *current,char parameter[30]){
	if(strcmp(parameter,"..") == 0 && current->upper != NULL){
		return current->upper;
	}
	DIR *c_dir = current->deep;
	while(c_dir != NULL){
		if(strcmp(c_dir->name,parameter) == 0){
			return c_dir;
		}
		c_dir = c_dir->next;
	}
	printf("cd: %s: No such file or directory\n",parameter);
	return current;
}

void rm(DIR *current,char parameter[30]){
	DIR *dir_temp = current->deep;
	DIR *delete_dir;
	if(current->deep==NULL) return;
	if(strcmp(dir_temp->name,parameter) == 0){
		current->dir_p--;
		current->deep = dir_temp->next;
		free(dir_temp);
		return ;
	}
	while(dir_temp->next != NULL){
		if(strcmp(dir_temp->next->name,parameter)==0){
			current->dir_p--;
			delete_dir = dir_temp->next;
			dir_temp->next = dir_temp->next->next;
			free(delete_dir);
			return;
		}
		dir_temp = dir_temp->next;
	}
}

void payload(DIR *current){
	printf("data_p: %d dir_p: %d\n",current->data_p,current->dir_p);
}

void init(FILE *fp){
	fp = fopen("file.bin","wb+");
	char block[BLOCK_SZIE] = {0};
	for (int i=0;i<BLOCK;i++){
		fwrite(block,sizeof(char),sizeof(block),fp);
	}
	fclose(fp);
}

int data_size(char parameter[30]){
	FILE *fp;
	fp = fopen(parameter,"rb+");
	fseek(fp,0,SEEK_END);
	return ftell(fp);
}

void input_data(DIR *current,int mark[BLOCK],char parameter[30]){
	FILE *input;
	input = fopen(parameter,"rb+");
	if(input == NULL){
		printf("cannot input '%s' : No such file\n",parameter);
		return ;
	}
	printf("%d\n",data_size(parameter));
	fclose(input);
}




void pr_mark(int mark[BLOCK]){
	for(int i=0;i<BLOCK;i++){
		if(mark[i] != 0){
			printf("%d ",mark[i]);
		}
	}
	printf("\n");
}

int straight(DIR *root){
	int count=0;
	DIR *c_dir = root;
	DIR *d_dir;
	DIR *tail = root;
	while(c_dir != NULL){
		d_dir = c_dir->deep;
		printf("%s dir:%d data:%d\n",c_dir->name,c_dir->dir_p,c_dir->data_p);
		tail->next = d_dir;
		while(tail->next !=NULL){
			tail = tail->next;
		}
		c_dir = c_dir->next;
		count++;
	}
	printf("%d\n",count);
	return count;
}

void init_node(DIR *c_dir,char name[20],int dir_p,int data_p){
	strcpy(c_dir->name,name);
	c_dir->dir_p = dir_p;
	c_dir->data_p = data_p;
	c_dir->next = NULL;
	c_dir->deep = NULL;
	c_dir->upper = NULL;
	c_dir->data = NULL;
}

void straight_write(DIR *root,int mark[BLOCK],int *total){
	FILE *fp;
	fp = fopen("dir.bin","wb+");
	fwrite(mark,sizeof(int),BLOCK,fp);
	fwrite(total,sizeof(int),1,fp);
	DIR *c_dir = root;
	while(c_dir != NULL){
		fwrite(c_dir->name,sizeof(char),20,fp);
		fwrite(&c_dir->dir_p,sizeof(int),1,fp);
		fwrite(&c_dir->data_p,sizeof(int),1,fp);
		c_dir = c_dir->next;
	}
	c_dir = root;
	DATA *data;
	while(c_dir !=NULL){
		data = c_dir->data;
		while(data!=NULL){
			fwrite(data->name,sizeof(char),20,fp);
			fwrite(&data->location,sizeof(int),1,fp);
			data = data->next;
		}
		c_dir=c_dir->next;
	}
	fclose(fp);
}

void straight_read(DIR *root,int mark[BLOCK],FILE *dp){
	DIR *c_dir = root;
	int dir_p[10000];
	int data_p;
	char name[20];
	int total;
	FILE *fp;
	fp = fopen("dir.bin","rb+");
	fread(mark,sizeof(int),BLOCK,fp);
	fread(&total,sizeof(int),1,fp);
	fread(name,sizeof(char),20,fp);
	fread(&dir_p[0],sizeof(int),1,fp);
	fread(&data_p,sizeof(int),1,fp);
	init_node(c_dir,name,dir_p[0],data_p);
	for (int i=1;i<total;i++){
		fread(name,sizeof(char),20,fp);
		fread(&dir_p[i],sizeof(int),1,fp);
		fread(&data_p,sizeof(int),1,fp);
		c_dir->next = malloc(sizeof(DIR));
		init_node(c_dir->next,name,dir_p[i],data_p);
		c_dir = c_dir->next;
	}
	c_dir = root;
	char data_name[20];
	int data_location;
	DATA *c_data;

	c_dir = root;
	DIR *a_dir =root->next;
	while(c_dir !=NULL){
		if(c_dir->dir_p != 0){
			c_dir->deep = a_dir;
			for(int j=0;j<c_dir->dir_p;j++){
				a_dir->upper = c_dir;
				a_dir = a_dir->next;
			}
		}
		if(c_dir->data_p != 0){
			c_data = malloc(sizeof(DATA));
			c_dir->data = c_data;
			fread(c_data->name,sizeof(char),20,fp);
			fread(&c_data->location,sizeof(int),1,fp);
			c_data->next=NULL;
			for(int i=1;i<c_dir->data_p;i++){
				c_data->next = malloc(sizeof(DATA));
				c_data = c_data->next;
				fread(c_data->name,sizeof(char),20,fp);
				fread(&c_data->location,sizeof(int),1,fp);
				c_data->next=NULL;
			}
		}
		c_dir = c_dir->next;
	}
	c_dir = root->next;
	for(int i=0;i<total;i++){
		if(dir_p[i] !=0 ){
			for(int j=0;j<dir_p[i]-1;j++){
				c_dir = c_dir->next;
			}
			a_dir = c_dir->next;
			c_dir->next = NULL;
			c_dir = a_dir;
		}
	}
	root->next = NULL;
	fclose(fp);
}

void remove_mark(int mark[BLOCK],int location){
	int next;
	int last = location;
	while(1){
		next = mark[last];
		if(next < 0){
			mark[last] = 0;
			return;
		}
		mark[last] = 0;
		last = next;
	}
}

void delete(DIR *current,int mark[BLOCK],char parameter[20]){
	DATA *data_temp = current->data;
	DATA *delete_data;
	if(current->data == NULL) {
		return;
	}
	if(strcmp(data_temp->name,parameter) == 0){
		remove_mark(mark,data_temp->location);
		current->data_p--;
		current->data = data_temp->next;
		free(data_temp);
		return;
	}
	while(data_temp->next!=NULL){
		if(strcmp(data_temp->next->name,parameter) == 0){
			remove_mark(mark,data_temp->next->location);
			current->data_p--;
			delete_data = data_temp->next;
			data_temp->next = data_temp->next->next;
			free(delete_data);
			return;
		}
		data_temp = data_temp->next;
	}
}

void import_size(int *data_body,int *data_tail,char parameter[30]){
	FILE *fp;
	fp = fopen(parameter,"rb+");
	fseek(fp,0L,SEEK_END);
	int total = ftell(fp);
	*data_body = total/BLOCK_SZIE;
	*data_tail = total%BLOCK_SZIE;
	fclose(fp);
}

int find_empty(int mark[BLOCK]){
	for (int i=0;i<BLOCK;i++){
		if(mark[i] == 0){
			return i;
		}
	}
	printf("full\n");
	return -1;
}

void out_load(int mark[BLOCK],int location,char parameter[20]){
	FILE *fp;
	int c_loc = location;
	char buffer[BLOCK];
	fp = fopen(parameter,"wb+");
	FILE *file;
	file = fopen("file.bin","rb+");
	while(1){
		if(mark[c_loc] < 0){
			fseek(file,sizeof(char)*BLOCK_SZIE*c_loc,SEEK_SET);
			fread(buffer,sizeof(char),mark[c_loc]*-1,file);
			fwrite(buffer,sizeof(char),mark[c_loc]*-1,fp);
			fclose(file);
			fclose(fp);
			return;
		}
		fseek(file,sizeof(char)*BLOCK_SZIE*c_loc,SEEK_SET);
		fread(buffer,sizeof(char),BLOCK_SZIE,file);
		fwrite(buffer,sizeof(char),BLOCK_SZIE,fp);
		c_loc = mark[c_loc];
	}
}

void output(DIR *current,int mark[BLOCK],char parameter[20]){
	int data_tail=0;
	DATA *c_data = current->data;
	while(c_data != NULL){
		if(strcmp(c_data->name,parameter)==0){
			out_load(mark,c_data->location,parameter);
			return;
		}
		c_data = c_data->next;
	}
}

void import(DIR *current,int mark[BLOCK],char parameter[20]){
	int data_body;
	int data_tail;
	import_size(&data_body,&data_tail,parameter);
	char buffer[BLOCK_SZIE];
	FILE *in;
	FILE *out;
	in = fopen(parameter,"rb+");
	if(in == NULL){
		printf("No %s such data\n",parameter);
		return;
	}
	out = fopen("file.bin","rb+");
	DATA *new_data = malloc(sizeof(DATA));
	new_data->next=NULL;
	int last = find_empty(mark);
	mark[last] = last;
	int next;
	new_data->location = last;
	
	strcpy(new_data->name,parameter);
	current->data_p++;
	DATA *c_data;
	if(current->data == NULL){
		current->data = new_data;
	}else{
		c_data = current->data;
		while(c_data->next!=NULL){
			c_data=c_data->next;
		}
		c_data->next = new_data;
	}
	for(int i=0;i<data_body;i++){
		mark[last] = (BLOCK+1) * -1;
		next = find_empty(mark);
		mark[last] = next;
		fread(buffer,sizeof(char),BLOCK_SZIE,in);
		fseek(out,sizeof(char)*last*BLOCK_SZIE,SEEK_SET);
		fwrite(buffer,sizeof(char),BLOCK_SZIE,out);
		last = next;
	}
	mark[last] = data_tail * -1;
	fread(buffer,sizeof(char),data_tail,in);
	fseek(out,sizeof(char)*last*BLOCK_SZIE,SEEK_SET);
	fwrite(buffer,sizeof(char),data_tail,out);
	fclose(in);
	fclose(out);
}

void main(void){
	int total = 0;
	int mark[BLOCK] = {0};
	char input[50];
	char command[20];
	char parameter[30];
	DIR *root = malloc(sizeof(DIR));
	strcpy(root->name,"root");
	root_init(root);
	DIR *current = root;
	FILE *fp;
	FILE *file;
	FILE *dp;
	dp = fopen("dir.bin","rb+");
	if (dp == NULL){
		dp = fopen("dir.bin","wb+");
		fclose(dp);
	}else{
		straight_read(root,mark,dp);
	}
	fp = fopen("file.bin","rb+");
	if (fp==NULL){
		printf("initialization\n");
		init(fp);
	}
	while(1){
		pwd(current);
		printf(" # ");
		gets(input);
		analysis(input,command,parameter);
		if(strcmp(command,"exit") == 0){
			break;
		}else
		if(strcmp(command,"mkdir") == 0){
			mkdir(current,parameter);
		}else
		if(strcmp(command,"ls") == 0){
			ls(current);
		}else
		if(strcmp(command,"cd") == 0){
			current = cd(current,parameter);
		}else
		if(strcmp(command,"payload") == 0){
			payload(current);
		}else
		if(strcmp(command,"rm") == 0){
			rm(current,parameter);
		}else
		if(strcmp(command,"import") == 0 ){
			import(current,mark,parameter);
		}else
		if(strcmp(command,"mark") == 0){
			pr_mark(mark);
		}else
		if(strcmp(command,"delete") == 0){
			delete(current,mark,parameter);
		}else
		if(strcmp(command,"output") == 0){
			output(current,mark,parameter);
		}else
		{
			printf("%s: command not found\n",command);
		}
	}
	total = straight(root);
	straight_write(root,mark,&total);
}