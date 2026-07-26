/************************************************************************



	memo.c

	(C) Alvaro Cortés. 2004. accsc@arbornet.org

	Memory vars stuff. Under GPL licence. NO WARRANTY. 
	USE UNDER YOUR OWN RISK.


	Probably multiple buffer overflows in this function. Check with 
	ccmalloc.


**************************************************************************/



#include "stdio.h"
/*#include "malloc.h"*/
#include "stdlib.h"
#include "string.h"

struct __var_mem
{
char name[12];    	        /* Name of variable */
int type;               /* Type of variable */
char *content;        	/* Content of var */

struct __var_mem* next_var;  /* pointer to next var */
int pos;
};


struct __var_mem* first_var = NULL;
struct __var_mem* last_var = NULL;

int store_new_var(char *name, char *content, int type)
{
	struct __var_mem *new_var = (struct __var_mem *)  malloc(sizeof(struct __var_mem));
	if( new_var == NULL)
	{
		fprintf(stderr,"Memory Error: Not enought memory for allocate a new var!!!\n");
		fflush(stderr);
		return -1;
	}
	strncpy(new_var->name,name,11);
	new_var->content = content;
	new_var->type = type;
	new_var->next_var = NULL;

	if( first_var == NULL) 
	{
		first_var = new_var;
		last_var = new_var;
		first_var->pos = 1;
	}else{
		new_var->pos = last_var->pos+1;
		last_var->next_var = new_var;
		last_var = new_var;	
	}
	return 0;
}

int show_all_vars()
{
	struct __var_mem *una;
	int i;
	if( first_var == NULL)
	{
		fprintf(stderr,"No vars declared\n");
		return -1;
	}
	una = first_var;
	fprintf(stderr,"Name - Content - Type\n");
	while(una != NULL)
	{	
	fprintf(stderr,"%s - %s - %i\n",una->name, una->content,una->type);
		una = una->next_var;
	}
	fprintf(stderr,"Total - %i Vars declared\n",una->pos);
return 0;

}


int free_all()
{
	struct __var_mem *dos,*tres;
	int num_var;
	num_var = 0;
	dos = first_var;
	while( dos != NULL)
	{
		++num_var;
		if( dos->next_var != NULL)
		{
		tres = dos->next_var;
		free(dos);	
		dos = tres;
		}
		else{
		free(dos);
		break;
		}
	}
first_var = NULL;
fprintf(stderr,"All var freed. Total %i\n",num_var);
fflush(stderr);
return 0;
}

int free_var(int nVar)
{
	struct __var_mem *uno,*dos;
	int i;

	if( nVar == last_var->pos)
	{
		uno = first_var;
		--nVar;
		for( i = 1; i<nVar; ++i)
		{
		uno = uno->next_var;
		}
		dos = uno->next_var;
		uno->next_var = NULL; 
		free(dos);
	}else if( nVar == 1){
		uno= first_var->next_var;
		dos = first_var;
		free(dos);
		first_var = uno;
	}else{
	uno = first_var;
	--nVar;
	for( i = 1; i<nVar; ++i)
	{
		uno = uno->next_var;
	}
	dos = uno->next_var;
	uno->next_var = dos->next_var;
	free(dos);

	}
	return 0;
}

