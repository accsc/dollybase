/**********************************************************
 *
 *
 *  (C) Alvaro Cortés. 2004.
 *  accsc@arbornet.org  
 *
 *
 *  Under GPL licence. NO WARRANTY. Use UNDER YOUR OWN RISK.
 *
 *
 *
 *
 *********************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* trims with two arguments */
void trim(const char *origen,char *destino);
void ltrim(const char *orgien, char *destino);
void rtrim(const char *origen, char *destino);

/* trims with only one argument */
void ltrim_only(char *str);
void rtrim_only(char *str);
void trim_only(char *str);

/* trim with ingnored sections inside */
/* Ex: trim_ignore(' ? "fdf fdfd fdf " ','"'); 
/* Return: '?"fdf fdfd fdf "'   */
void trim_ignore(char *str, char ign);

/* mode -> 0 - case sensitive, mode -> 1 case insensitive */
/* mode -> 2 - exact case sensitive, mode -> 3 exact case insensitive */
int search( char *str, char *cri, int mode);

void trim(const char *origen, char *destino)
{
	while (*origen != '\0')
	{
		if( *origen != ' ')
		*destino++ = *origen;
		origen++;
	}
	*destino++ = '\0';
}
	
void ltrim( const char *origen, char *destino)
{
	while( *origen == ' ')
	{
		origen++;
	}
	while( *origen != '\0')
	{
	*destino++ = *origen;
	origen++;
	}
	*destino++ = '\0';
}
void rtrim( const char *origen, char *destino)
{
	int i,o,size;
	size = strlen(origen); /* Size + (NULL char) */

	for( i = (size-1); i>= 0; --i)
	{
		if( *(origen+i) != ' ')
			break;
	}
	for( o = 0; o<=i; ++o)
	{
	*destino++ = *origen;
	origen++;
	}
	*destino++ = '\0';
}

void ltrim_only( char *str)
{
	char *q,*p;
	
	q = p = str;
	while(*q == ' ' || *q == '\t')
	q++;
	while( *q != '\0')
	{
	 *p = *q;
	 p++;
	 q++;
	}
	*p = '\0';
}

void rtrim_only( char *str)
{
	char *p;
	int size,i;
	
	size = strlen(str);
	p = str;
	/* size -1, because NULL char is included */
	for( i = (size-1); i>=0; --i){
	if( *(p+i) == ' ' || *(p+i) == '\t')
		*(p+i) = '\0';
	else
		break;
	}	

}

void trim_only( char *str)
{
	char *p;

	while( *str != '\0')
	{
		if( *str != ' ' && *str != '\t')
		*p++ = *str;
		str++;
	}
	*p++ = '\0';
	str = p;
}

void trim_ignore(char *str, char ign)
{
	int ignore = 0;
	char *p;
	while( *str != '\0')
	{
		if( *str == ign)
			++ignore;
		else if( *str != ' ' || ignore == 1)
			*p++ = *str;

		str++;
	}
	*p++ = '\0';
	str = p;
}

int search(char *str, char *cri, int mode)
{
	int ssize,csize,i;
	ssize = strlen(str);
	csize = strlen(cri);
	if( mode > 3 || mode <0)
	return -1;
	if( ssize < csize)
	return -1;
	if( csize <= 0 || ssize <= 0)
	return -1;
	
	for( i = 0; i<= (ssize-csize); ++i)
	{
		if( mode == 0)
		{
			if( strncmp( str+i,cri,csize) == 0 )
			return 0;
		}else if( mode == 1){
			if( strncasecmp( str+i,cri,csize) == 0)
				return 0;
		}else if( mode == 2){
			if( strncmp( str+i,cri,csize) == 0  && ( *(str+i+csize) == ' ' || *(str+i+csize) == '\0' || *(str+i+csize) == '\t' || *(str+i+csize) == '\n') )
			return 0;
		}else if( mode == 3){
			if( strncasecmp( str+i,cri,csize) == 0 && ( *(str+i+csize) == ' ' || *(str+i+csize) == '\0' || *(str+i+csize) == '\t' || *(str+i+csize) == '\n') )
			return 0;
		}
		
	}
return -1;
}
