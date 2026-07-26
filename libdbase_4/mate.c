#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "libdbase.h"

int average_campo(DATABASEDBF *asp, char *campo, int nexts)
{
int u,i,o;
char *f1;

if( (f1 = (char *) malloc(2048)) == NULL)
{
return -1;
}
int total = 0;
u = field_to_number(asp, campo);
if( dfield_type(asp, u) != 'N')
return -1;
if( nexts == 0)
o = reccount(asp);
else
o = nexts+1;
for(i = 0; i< o; ++i)
{
	get_field(asp,u,&f1);
total = total + atoi(f1); 
skip(&asp);
}
/*printf("%i registros promediados\n",o);
printf("%s\n",campo);
printf("%6i\n",(total/o));*/
fflush(stdout);
free(f1);
return (total/o);
}


int sum_campo(DATABASEDBF *asp, char *campo, int nexts)
{
int u,i,o;
char *f1;

if (( f1= (char *) malloc(2048)) == NULL)
{
return -1;
}
int total = 0;
if (( u = field_to_number(asp, campo)) == -2)
{
return -3;
}
if( dfield_type(asp, u) != 'N')
return -2;
if( nexts == 0)
o = reccount(asp);
else
o = nexts+1;
for(i = 0; i< o; ++i)
{
get_field(asp,u,&f1);
total = total + atoi(f1);
skip(&asp);
}
free(f1);
return (total);
}

int min(int x,int y)
{
if( x > y)
return y;
else
return x;
}

int max(int x, int y)
{
if(x > y)
return x;
else
return y;
}


int potencia(int num, int veces)
{
int y;
for(y = 0; y<veces; ++y)
num = num * num;
return num;
}

