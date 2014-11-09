#ifndef _Football_AI_h_
#define _Football_AI_h_

#include "Game.h"

class AI : public Player
{
	public:
		AI();
		virtual void GenMove(int token);
};

#endif
