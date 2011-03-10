/*
Copyright ESIEE (2009) 

m.couprie@esiee.fr

This software is an image processing library whose purpose is to be
used primarily for research and teaching.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software. You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/
/*
 *
 * simple screen I/O
 *
 * for the Level Set Tool Box
 *
 * Hugues Talbot	 9 Jul 2009
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "pde_toolbox_LSTB.h"

/* print the messages from the toolbox */
int lreadLSTBmsgs(void)
{
        int i;
        char ** Msgs;
        int NumMsgs;

        /* Get a handle on LSTB's message list */
        Msgs = LSTB_get_messages();
        NumMsgs = LSTB_get_num_messages();

        /* Dump across to console  */
        for (i = 0; i < NumMsgs; i++) {
            fprintf(stderr, "%s", Msgs[i]);
        }

        /* Clear the message list */
        LSTB_clear_messages();

        return 0;
}




/* 1 = Default to printing debugging info */
static int debugLSTB = 1;

/* LSTB_error:
	Add an error message to the list
*/
void LSTB_error(char *msg, ...)
{
    char strarg[1000],buf[1000];
    va_list args;

    va_start(args, msg);
    sprintf(strarg, "*** ERROR: %s", msg);
    vsprintf(buf,strarg,args);
    LSTB_add_message(buf);
    va_end(args);
    return;
}

void LSTB_enable_debug(void)
{
    debugLSTB = 1;
    return;
}

void LSTB_disable_debug(void)
{
    debugLSTB = 0;
    return;
}

int LSTB_is_debug_enabled(void)
{
    return debugLSTB;
}

/* LSTB_debug:
	Add a debugging message to the list
*/
void LSTB_debug(char *msg, ...)
{
	char strarg[1000],buf[1000];
	va_list args;

	if (debugLSTB) {
		va_start(args, msg);
		sprintf(strarg, "%s", msg);
		vsprintf(buf,strarg,args);
		LSTB_add_message(buf);
		va_end(args);
	}
	return;
}


static int NumMsgs = 0;
static char **Msgs = NULL;
/* Combine sequential messages and tokenize on \n's */
static char currentMessageComplete = 1; /* TRUE */

void LSTB_show_messages(void)
{
    int i;
    for(i=0;i<NumMsgs;++i)
	puts(Msgs[i]);
}

void LSTB_clear_messages(void)
{
    if(Msgs != NULL) {
	int i;
	for(i=0;i<NumMsgs;++i)
	    free(Msgs[i]);
	free(Msgs);
	Msgs = NULL;
	NumMsgs = 0;
	currentMessageComplete = 1;
    }
}

/* Tokenise a message on '\n's and add each component */
int LSTB_add_message(const char * msg,...)
{
    int last_index, this_index;
	char buff[1000];
	char tokenbuff[1000];
    va_list args;

    va_start(args,msg);

    /* Dump input to character buffer */
	vsprintf(buff,msg,args);

	/* Tokenize on \n, include \n in each substring */
	last_index = 0;
	while(1) {
		/* Locate \n or \0 */
		for (this_index = last_index; buff[this_index] != '\n' && buff[this_index] != '\0'; ) {
			this_index++;
		}

		/* Copy the subset from last_index to this_index (inclusive) to token buffer */
		memcpy(tokenbuff, buff + last_index, (this_index - last_index + 1)*sizeof(char));
		tokenbuff[this_index - last_index + 1] = '\0';

		/* Pass to LSTB_append_line */
		LSTB_append_line(tokenbuff);

		/* Finish when we reach the end of the string */
		if (buff[this_index] == '\0') break;

		/* Update state for next loop */
		last_index = this_index + 1;
	}

    va_end(args);
    return 0;
}

/* Add a single line to the string list.  May or may not 'end' in \n,
must not have \n in interior of string.  Must end in '\0' as a string */
int LSTB_append_line(const char * buff)
{
    static int const mod = 5;

	/* Check if we need more string pointers in our list */
	if(currentMessageComplete && !(NumMsgs%mod)) {
		if(NumMsgs == 0) {
	    	Msgs = (char**)malloc(sizeof(char*) * mod);
		} else {
		    Msgs = (char**)realloc(Msgs, (NumMsgs +mod)*sizeof(char*));
    	}
	}

    /* Add a new list element or append as appropriate */
	if (currentMessageComplete) {
		/* Allocate memory for next list element and copy buffer */
		Msgs[NumMsgs] = (char*)malloc((strlen(buff)+1)
				  	*sizeof(char));
    	strcpy(Msgs[NumMsgs],buff);

		NumMsgs++;
	} else {
		/* Reallocate existing string */
		Msgs[NumMsgs-1] = (char *)realloc(Msgs[NumMsgs-1], (strlen(Msgs[NumMsgs-1]) + strlen(buff) + 1) * sizeof(char));

		/* Append buff to existing string */
		strcat(Msgs[NumMsgs-1], buff);
	}

	/* Check if the last list element is complete (ends in \n) */
	currentMessageComplete = (Msgs[NumMsgs-1][strlen(Msgs[NumMsgs-1]) - 1] == '\n');

	return 0;
}

char ** LSTB_get_messages(void)
{
    return Msgs;
}

int LSTB_get_num_messages(void)
{
    return NumMsgs;
}







