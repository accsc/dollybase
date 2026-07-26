/***************************************************
 *
 *
 *
 *
 * (C) Alvaro Cortés. 2004. accsc@arbornet.org
 *
 * Under GPL licence v2 or above. NO WARRANTY. Use ONDER YOUR OWN RISK
 *
 *
 * 
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libdbase.h"


LBL * use_label(char *_fname)
{
	FILE *a;
	LBL *test = NULL;
	int i;
	char *block;
		
	test->sig = 0x00;

	if( (block  = (char *) malloc(1035)) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Label. Not enought memory.\n");
		fflush(stderr);
#endif
		return test;
	}


        if( (test  = (LBL *) calloc(sizeof(LBL),1)) == NULL)
        {
#ifdef DEBUG
                fprintf(stderr,"Label. Not enought memory.\n");
                fflush(stderr);
#endif
                free(block);
                return test;
        }


	if( (a = fopen(_fname,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Label. Cant open label file.\n");
		fflush(stderr);
#endif

		free(block);
		free(test); 
                test = NULL;
		return test;
	}
	if (fread((char *) block,1, 1033 ,a) == 0)
	{
#ifdef DEBUG
	fprintf(stderr,"Error. Cant read label file.\n");
	fflush(stderr);
#endif
	free(block);
	free(test); 
        test = NULL;
	return test;
	}
	
#ifdef DEBUG
	fprintf(stderr,"File open and memory allocated.\n");
	fflush(stderr);
#endif
	test->sig = block[0];

	for( i = 0; i< 60; ++i)
	{
	test->Remarks[i] = block[i+1];
	}
#ifdef DEBUG
	fprintf(stderr,"Remarks loaded.\n");
	fflush(stderr);
#endif
	test->Remarks[60] = 0;
	test->nLines = block[61] + (block[62]*256);	
	test->nCols = block[63] + (block[64]*256);
	test->lMargin = block[65] + (block[66]*256);
	test->spaceh = block[67] + (block[68]*256);
	test->spacev = block[69] + (block[70]*256);
	test->spacea = block[71] + (block[72]*256);
#ifdef DEBUG
	fprintf(stderr,"Rest Info loaded.\n");
	fflush(stderr);
#endif
	if( strlen(block) > 961)
	{
		free(block);
		return test;
	}
		
	for( i = 0; i<960; ++i)
	{
	test->text[i] = block[i+73];
	}
#ifdef DEBUG
	fprintf(stderr,"Text loaded.\n");
	fflush(stderr);
#endif
	test->text[960] = '\0';
#ifdef DEBUG
	fprintf(stderr,"%i\n",block[1032]);
	if( block[1032] != 0x20)
	{
		fprintf(stderr,"End of file different 0x2 value\n");
		fflush(stderr);
	}
#endif
	free(block);
	#ifdef DEBUG
		fprintf(stderr,"BLOCK freed.\n");
		fflush(stderr);
	#endif
	fclose(a);
#ifdef DEBUG
	fprintf(stderr,"File closed.\n");
	fflush(stderr);
#endif 	
	return test;
}
void print_label_info(LBL *test)
{
	
	printf("Signature 0x%x\n",test->sig);
	printf("Remarks %s\n",test->Remarks);
	printf("Height: %i\n",test->nLines);
	printf("Width: %i\n",test->nCols);
	printf("Left Margin: %i\n",test->lMargin);
	printf("Label line: %i\n",test->spaceh);
	printf("Label space: %i\n",test->spacev);
	printf("Labels across: %i\n",test->spacea);
	printf("Label Text: $%s$\n",test->text);
	fflush(stdout);
}

int create_label(LBL *label1, char *_fname)
{
	FILE *o;
	char *lab;
	int i;
	
	if( (lab = (char *) malloc(1034)) == NULL)
	{
#ifdef DEBUG
	fprintf(stderr,"Label. Not enought memory.\n");
	fflush(stderr);
#endif
		return -1;
	}
	if( (o = fopen(_fname,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Label. Cant open file.\n");
		fflush(stderr);
#endif
		free(lab);
		return -2;
	}
	lab[0] = 0x02; /* LBL signature */
	lab[1033] = 0x02; /* End signature */
	for( i = 1; i <= 60; ++i)
		lab[i] = ' ';

	/* nLines conversion */
	if( label1->nLines > 65536)
		label1->nLines = label1->nLines - 65536;
	if( label1->nCols > 65536)
		label1->nCols = label1->nCols - 65536;
	if( label1->lMargin > 65536)
		label1->lMargin = label1->lMargin - 65536;
	if( label1->spaceh > 65536)
		label1->spaceh = label1->spaceh - 65536;
	if( label1->spacev > 65536)
		label1->spacev = label1->spacev - 65536;
	if( label1->spacea > 65536)
		label1->spacea = label1->spacea - 65536;
	
	lab[62] = label1->nLines/256;
	lab[61] = label1->nLines - (lab[62]*256); 

	lab[64] = label1->nCols/256;
	lab[63] = label1->nCols - (lab[64]*256);

	lab[66] = label1->lMargin/256;
	lab[65] = label1->lMargin - (lab[66]*256);

	lab[68] = label1->spaceh/256;
	lab[67] = label1->spaceh - (lab[68]*256);

	lab[70] = label1->spacev/256;
	lab[69] = label1->spacev - (lab[70]*256);

	lab[72] = label1->spacea/256;
	lab[71] = label1->spacea - (lab[72]*256);
	
	for( i = 73; i<=(73+strlen(label1->text)); ++i)
		lab[i] = label1->text[i-73];

	for( i = strlen(label1->text)+74; i<=1033; ++i)
		lab[i] = ' ';

	lab[1032] = 0x2;
	fwrite(lab,1,1033,o);
	fflush(o);
	free(lab);
	fclose(o);
	return 0;
}

