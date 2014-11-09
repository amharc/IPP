#include "AI.h"
using namespace Upp;

AI::AI()
{
	
}

void AI::GenMove(int token)
{
	Vector<int> moves = game->ValidMoves();
	int best = moves.Top();
	int target = starts ? 6 : 2;
	
	for(int move : moves)
	{
		if(abs(move - target) < abs(best - target))
			best = move;
	}
	
	game->Move(token, best);
}
