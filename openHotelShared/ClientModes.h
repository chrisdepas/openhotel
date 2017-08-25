#ifndef __CLIENTMODES_H__
#define __CLIENTMODES_H__

enum EClientMode {
	MODE_FIRST = 0,

	/* Valid modes */
	MODE_WALK,				/* default - client can walk around etc. */
	MODE_OPERATOREDIT,		/* Can edit non-resident rooms */
	MODE_FROZEN,			/* SERVER->CLIENT ONLY! Client can't move */
	/* */

	MODE_LAST,
};

#endif