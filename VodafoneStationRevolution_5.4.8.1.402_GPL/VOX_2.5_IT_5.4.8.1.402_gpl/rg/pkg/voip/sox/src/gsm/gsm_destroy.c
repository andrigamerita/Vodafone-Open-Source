/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /arch/cvs/rg/pkg/voip/sox/src/gsm/Attic/gsm_destroy.c,v 1.1.2.1 2006/01/05 10:50:20 tali Exp $ */

#include "gsm.h"

#	include	<stdlib.h>

void gsm_destroy (gsm S)
{
	if (S) free((char *)S);
}
