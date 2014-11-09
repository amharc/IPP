#include "Game.h"
using namespace Upp;

static int diffX[] = {1, 1, 0, -1, -1, -1, 0, 1};
static int diffY[] = {0, 1, 1, 1, 0, -1, -1, -1};

Point DirectionDiff(int direction)
{
	return Point(diffX[direction], diffY[direction]);
}

MovePlayer Opponent(const MovePlayer &player)
{
	switch(player)
	{
	case PLAYER1:
		return PLAYER2;
	case PLAYER2:
		return PLAYER1;
	default:
		return player;	
	}
}

void Segment::Xmlize(XmlIO &xml)
{
	xml
		("position", ballPos)
		("direction", direction)
		("player", player);
}

int GameState::lastDirection() const
{
	if(segments.IsEmpty() || segments.Top().player == FORCE_MAJEURE)
		return -1;
	else
		return segments.Top().direction;
}

bool GameState::HasSegment(Point pos, int direction) const
{
	Segment seg1{pos, direction};
	int reverseDirection = (direction + 4) % 8;
	ASSERT(DirectionDiff(direction) == -DirectionDiff(reverseDirection));
	Segment seg2{pos + DirectionDiff(direction), reverseDirection}; // Reverse
	
	return (segments.Find(seg1) >= 0 || segments.Find(seg2) >= 0);
}

void Game::AssertValidToken(int token)
{
	if(token != this->token)
		throw new CheatingError("Invalid token");
}

void Game::Update()
{
	if(state.ballPos.y < 0)
		state.currentState = PLAYER1_WON;
	if(state.ballPos.y > state.height)
		state.currentState = PLAYER2_WON;

	if(ValidMoves().IsEmpty())
	{
		if(nextPlayer == PLAYER1)
			state.currentState = PLAYER2_WON;
		else
			state.currentState = PLAYER1_WON;
	}
		
	whenUpdated();
	if(state.currentState == ACTIVE || state.currentState == HINT)
		state.currentPlayer = nextPlayer;
}

GameState::GameState(int width, int height)
	: height(height), width(width), ballPos(width/2, height/2),
	  currentPlayer(PLAYER1), currentState(ACTIVE),
	  isHistoryInvalidated(false)
{

}

void GameState::Xmlize(XmlIO &xml)
{
	xml
		("height", height)
		("width", width)
		("ballPosition", ballPos)
		("currentPlayer", currentPlayer)
		("historyInvalidated", isHistoryInvalidated)
		("segments", segments);
	
	currentState = ACTIVE;
}

void Xmlize(Upp::XmlIO &xml, MovePlayer &data)
{
	if(xml.IsStoring())
	{
		String content;
		switch(data)
		{
		case PLAYER1:
			content = String("player1");
			break;
		case PLAYER2:
			content = String("player2");
			break;
		case FORCE_MAJEURE:
			content = String("force_majeure");
		}
		
		xml.Attr("value", content);
	}
	else
	{
		String content;
		xml.Attr("value", content);
	
		if(content == "player1")
			data = PLAYER1;
		else if(content == "player2")
			data = PLAYER2;
		else
			data = FORCE_MAJEURE;	
	}
}

UI::UI(Game &game)
	: game(game)
{
}

GameState& UI::GetState()
{
	return game.state;
}

Game& UI::GetGame()
{
	return game;
}

void UI::Register(Callback whenUpdated)
{
	game.whenUpdated = whenUpdated;
}

Game::Game(int width, int height)
	: gameLoopActive(false), state(GameState(width, height)), nextPlayer(PLAYER1)
{

}

void Game::SetPlayers(Player *player1, Player *player2)
{
	if(!this->player1.IsEmpty())
		throw new CheatingError("Trying to change players mid-game");
	
	this->player1.Attach(player1);
	this->player2.Attach(player2);
	
	player1->Setup(this, true);
	player2->Setup(this, false);
}

const GameState& Game::GetState() const
{
	return state;
}

bool Game::IsMovePossible(Point from, int direction) const
{
	Point newPoint = from + DirectionDiff(direction);
	
	if(newPoint.x < 0 || newPoint.x > state.width)
		return false;
	
	auto inGoal = [&](Point pt) -> bool
	{
		return state.width / 2 - 1 <= pt.x && pt.x <= state.width / 2 + 1
	           && (pt.y <= 0 || pt.y >= state.height);
	};
	
	if(newPoint.y < 0)
		return from.y == 0 && (from.x == state.width / 2 
		       || from.x == state.width / 2 - 1 && direction == 7
		       || from.x == state.width / 2 + 1 && direction == 5);
	
	if(newPoint.y > state.height)
		return from.y == state.height && (from.x == state.width / 2 
		       || from.x == state.width / 2 - 1 && direction == 1
		       || from.x == state.width / 2 + 1 && direction == 3);
	
	if(IsBorder(from) && IsBorder(newPoint) && direction % 2 == 0)
		return false;
	
	if(inGoal(from) && inGoal(newPoint))
		return true;
	
	if((from.y < 0 || from.y > state.height) && !inGoal(newPoint))
		return false;
	
	if(from.x == 0 && newPoint.x == 0
	   || from.x == state.width && newPoint.x == state.width
	   || from.y == 0 && newPoint.y == 0
	   || from.y == state.height && newPoint.y == state.height)
	   return false;
	
	return true;
}

bool Game::IsBorder(Point pt) const
{
	if(pt.x == 0 || pt.x == state.width)
		return true;
	
	if(pt.y < 0 || pt.y > state.height)
		return true;
	
	if(pt.x != state.width / 2 && (pt.y == 0 || pt.y == state.height))
		return true;
	
	return false;
}

MoveValidResponse Game::IsMoveValid(int direction) const
{
	if(state.HasSegment(direction))
		return MOVE_INVALID;
	
	if(!IsMovePossible(state.ballPos, direction))
		return MOVE_INVALID;
	
	Point target = state.ballPos + DirectionDiff(direction);
	if(IsBorder(target))
		return MOVE_VALID_WITH_CONTINUATION;
	
	for(int cdir = 0; cdir < 8; ++cdir)
		if(state.HasSegment(target, cdir))
			return MOVE_VALID_WITH_CONTINUATION;
		
	return MOVE_VALID;
}

Vector<int> Game::ValidMoves() const
{
	Vector<int> moves;
	for(int i = 0; i < 8; ++i)
		if(IsMoveValid(i) != MOVE_INVALID)
			moves.Add(i);
	
	return moves;
}

void Game::Loop()
{
	if(state.currentState != HINT)
		state.currentState = ACTIVE;

	nextPlayer = state.currentPlayer;
	Update();		
	semaphore->Wait();
	if(!gameLoopActive)
		return;
	
	while((state.currentState == ACTIVE || state.currentState == HINT) && gameLoopActive) {
		token = Random();
	
		if(state.currentPlayer == PLAYER1)
			player1->GenMove(token);
		else
			player2->GenMove(token);
		
		semaphore->Wait();
	
		if(!gameLoopActive)
			return;
		
		Update();		
		semaphore->Wait();
	}
}

void Game::Move(int token, int direction)
{
	AssertValidToken(token);
	MoveValidResponse response = IsMoveValid(direction);
	if(response == MOVE_INVALID)
		throw new CheatingError("Tried to perform invalid move");
	
	state.segments.Add(Segment{state.ballPos, direction, state.currentPlayer});
	if(response == MOVE_VALID)
		nextPlayer = Opponent(state.currentPlayer);
	else
		nextPlayer = state.currentPlayer;
	
	state.ballPos += DirectionDiff(direction);
	semaphore->Release();
}

void Game::Start()
{
	semaphore.Create();
	gameLoop.Create();
	
	gameLoopActive = true;
	gameLoop->Run(THISBACK(Loop));
}

void Game::Continue()
{
	semaphore->Release();
}

void Game::Stop()
{
	gameLoopActive = false;
	if(!gameLoop.IsEmpty())
	{
		semaphore->Release();
		gameLoop->Wait();
		semaphore.Clear();
		gameLoop.Clear();
	}
}

bool Game::IsGameLoopActive() const
{
	return gameLoopActive;
}

void Player::Setup(Game *game, bool starts)
{
	this->game = game;
	this->starts = starts;
}