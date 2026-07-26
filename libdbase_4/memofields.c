/***********************************************
 *
 *
 *
 *
 * (C) Alvaro Cortés. 2004. accsc@arbornet.org
 *
 *
 *
 * Under GPL v2 or above license. NO WARRANTY. Use UNDER YOUR OWN RISK
 *
 * Module for libdollybase. Hanlde DB IV memo fields.
 *
 *
 */

#include "stdio.h"
#include "stdlib.h"

/* If you dont have a enought result pointer reserved ---> memory overflow */
/* use malloc(max+1) and call function with max parameter */
/* if the field is too big nothing happens, only you dont have all the field */

int get_db4_memo_block(char *_fname, int nblock, char *result, int max)
{
	FILE *finput;
	int nbytes,i, nfield;
	char a;
	char magic_block[4];
	
#ifdef DEBUG
	char _name[9];
	char *name = malloc(sizeof(_name));
	
	if(name == NULL)
	{
		fprintf(stderr,"Memofields. Not enought memory.\n");
		fflush(stderr);
		return -1;
		
	}
#endif
	if( (finput = fopen(_fname,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Memofileds. I cant open memo file.\n");
		fflush(stderr);
		free(name);
#endif
		return -1;
	}

	fseek(finput,4,SEEK_SET);
	nbytes=getc(finput)+(getc(finput)*256)+(getc(finput)*65536)+(getc(finput)*16777216);
	
#ifdef DEBUG
	fread(name,1,8,finput);
	fprintf(stderr,"DBT for DBF '%s'.\n",name);
	fflush(stderr);
#endif
	fseek(finput,20,SEEK_SET);
	if ( fgetc(finput) + (fgetc(finput)*256) == 1)
	{
#ifdef DEBUG
		fprintf(stderr,"Warning: Block lenght = 1, prabably DBIII memo field. Use DBIII function instead DB IV function.\n");
		fflush(stderr);
		free(name);

#endif
		return -2;
	}
	
	fseek(finput,nblock*nbytes,SEEK_SET);
	magic_block[0] = getc(finput);
	magic_block[1] = getc(finput);
	magic_block[2] = getc(finput);
	magic_block[3] = getc(finput);

	if(magic_block[0] !=0xFF || magic_block[1] !=0xFF || magic_block[2] != 0x08 || magic_block[3] != 0x00)
	{
#ifdef DEBUG
		fprintf(stderr,"Magic Block not found. Not a DB IV memo field.\n");
		fflush(stderr);
		free(name);
#endif
		return -2;
	}
	nfield = getc(finput)+(getc(finput)*256)+(getc(finput)*65536)+(getc(finput)*16777216);
	
	for( i = 0; i< nfield; ++i)
	{
		if( i+1 >= max)
		{
			result[i] = 0;
			break;
		}
			
		if( i == (nbytes-1))
		{	
			if( getc(finput) != 0xA1 || getc(finput) != 0xA1)
			{
#ifdef DEBUG
				fprintf(stderr,"Corrupt file. No terminator at end of field. Block size %i - Field Size %i\n",nbytes,nfield);
				fflush(stderr);
				free(name);

#endif

				return -1;
			}
		}
		a = fgetc(finput);
		result[i] = a;	
	}
	result[i+1] = 0;

	fclose(finput);

#ifdef DEBUG
	free(name);
#endif

	return nfield;
}

int get_fpt_memo_field(char *_fname, int nBlock, char *res, int max)
{
	FILE *in;
	char *block = (char *) malloc(513); /* Initial size of block */
	char *data_block;
	int next_block,i,o;
	int nSize, nType, nField, _nField[4];
	
	if( block == NULL || data_block == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Memofields. Not enought memory.\n");
		fflush(stderr);
#endif
		return -1;
	}
	if( (in = fopen(_fname,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Memofields. Cant open file.\n");
		fflush(stderr);
#endif
		free(block);
		return -1;
	}

	fread(block,1,512,in);
 	next_block = (block[0]*16777216)+(block[1]*65536)+(block[2]*256)+block[3];
#ifdef DEBUG
	fprintf(stderr,"FTP. Next Block %i.\n",next_block);
	fflush(stderr);
#endif
	if( nBlock > next_block ||  nBlock < 0)
	{
#ifdef DEBUG
		fprintf(stderr,"Memofields. Block out of range, No such block.\n");
		fflush(stderr);
#endif
		fclose(in);
		free(block);
		return -1;
	}
	
	nSize = (block[6]*256) + block[7];
#ifdef DEBUG
	fprintf(stderr,"Memofields. Size of fields %i.\n",nSize);
	fflush(stderr);
#endif
	free(block);

/*	fseek(in,8,SEEK_CUR);*/
/*	fseek(in,nBlock+512-5,SEEK_CUR);*/
#ifdef DEBUG
	fprintf(stderr,"Memofields. Size/Max %i/%i\n",nSize,max);
	fflush(stderr);
#endif
	
	for( i = 0; i< 8000; ++i)
	{
		if( getc(in) == 0x00)
		{
		fprintf(stderr,"%i Este caracter 0x00.\n",i);
		fflush(stderr);
		getchar();
		}
	}	
	fprintf(stderr,"END***\n");
	fflush(stderr);
	fseek(in,512,SEEK_SET);
	
	nType=getc(in)+(getc(in)*256)+(getc(in)*65536)+(getc(in)*16777217);
	fprintf(stderr,"FPT. Record type: %i.\n",nType);
		fflush(stderr);
		_nField[0] = getc(in);
		_nField[1] = getc(in);
		_nField[2] = getc(in);
		_nField[3] = getc(in);
		nField=(_nField[0]*16777216)+(_nField[1]*65536)+(_nField[2]*256)+_nField[3];
#ifdef DEBUG
		fprintf(stderr,"Memofields. Field size: %i.\n",nField);
		fflush(stderr);
#endif
		
		data_block = (char *) malloc(nField+1);
		fread(data_block,1,nField,in);	
		
		for( o = 0; o < nField; ++o)
		{
			if(o == max)
				break;
			res[o] = data_block[o];
		}

	res[o] = '\0';	
	free(data_block);
	fclose(in);
}

