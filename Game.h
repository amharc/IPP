#ifndef _Football_Game_h_
#define _Football_Game_h_

#include <Core/Core.h>
#include <stdexcept>
using namespace Upp;

Point DirectionDiff(int direction);

enum MovePlayer
{
	FORCE_MAJEURE = 0,
	PLAYER1 = 1,
	PLAYER2 = 2,	
};

void Xmlize(XmlIO &xml, MovePlayer &data);

MovePlayer Opponent(const MovePlayer &player);

struct Segment : Moveable<Segment> 
{
	Point ballPos;
	int direction;
	MovePlayer player;
	
	rval_default(Segment);
	
	Segment(const Segment&) = default;
	Segment(Point ballPos = Point(), int direction = 0, MovePlayer player = FORCE_MAJEURE) : ballPos(ballPos), direction(direction), player(player) { }
	unsigned GetHashValue() const { return CombineHash(ballPos, direction); }
	bool operator==(const Segment &that) const { return ballPos == that.ballPos && direction == that.direction; }
	String ToString() const { return "Segment[" + AsString(ballPos) + "->" + AsString(direction) + " by " + ((player == PLAYER1) ? "PLAYER1" : (player == PLAYER2) ? "PLAYER2" : "FORCE_MAJEURE") + " ]"; }
	void Xmlize(XmlIO &xml);
};

class Game;

enum GameResult {
	EDIT = -2,
	HINT = -1, // Used internally during hints
	ACTIVE = 0,
	PLAYER1_WON = 1,
	PLAYER2_WON = 2
};

struct GameState
{
	int height, width;
	Point ballPos;
	MovePlayer currentPlayer;
	Index<Segment> segments;
	GameResult currentState;
	bool isHistoryInvalidated;
	
	GameState(int width, int height);
	int lastDirection() const;
	bool HasSegment(int direction) const { return HasSegment(ballPos, direction); }
	bool HasSegment(Point pos, int direction) const;
	void Xmlize(XmlIO &xml);
};

class Player
{
	protected:
		Game *game;
		bool starts;
	public:
		void Setup(Game *game, bool starts);
		virtual void GenMove(int token) = 0;
		virtual ~Player() = default;
};

enum MoveValidResponse
{
	MOVE_INVALID,
	MOVE_VALID,
	MOVE_VALID_WITH_CONTINUATION
};

class CheatingError : public std::logic_error
{
	using std::logic_error::logic_error;
};

class UI {
	private:
		Game &game;
	public:
		UI(Game &game);
		void Register(Callback whenUpdated);
		GameState& GetState();
		Game& GetGame();
};

class FootballCtrl;

class Game
{
	private:
		bool gameLoopActive;
		One<Thread> gameLoop;
		One<Semaphore> semaphore;
		
		GameState state;
		int token;

		Callback whenUpdated;
		One<Player> player1, player2;
		MovePlayer nextPlayer;
		
		void AssertValidToken(int token);
		void Update();
		void RequestMove();
		
		void Loop();
		void Start();
		void Continue();
		void Stop();
		
		typedef Game CLASSNAME;
	public:
		static const int DEFAULT_WIDTH = 8;
		static const int DEFAULT_HEIGHT = 10;
		
		Game(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);

		bool IsGameLoopActive() const;
		
		void SetPlayers(Player *player1, Player *player2);

		const GameState& GetState() const;
		
		bool IsMovePossible(Point from, int direction) const;
		MoveValidResponse IsMoveValid(int direction) const;
		bool IsBorder(Point pt) const;
		Vector<int> ValidMoves() const;
		
		void Move(int token, int direction);
		
	friend class UI;
	friend class FootballCtrl;
	friend class Football;
};

#endif