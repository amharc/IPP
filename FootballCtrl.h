#ifndef _Football_FootballCtrl_h_
#define _Football_FootballCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include "Game.h"
#include "AI.h"
using namespace Upp;

class FootballCtrl : public Ctrl
{
	private:
		const int ANIMATION_TIME = 200;
		const int ANIMATION_STEPS = ANIMATION_TIME * 20 / 1000; // 20 fps
		typedef FootballCtrl CLASSNAME;
		
		UI &ui;
		void HandleUpdate();
		
		int ballStep;
		Point prevBallPos;
		
		Point fieldCorner;
		int squareSize;
		bool isPlaying;
		One<AI> hinter;
		MovePlayer playerHinted;
		int token;
		
		Point selectedPoint;
		
		void UpdateSize();
		Point GetGamePoint(int x, int y);
		Point GetRealPoint(int x, int y);
		Point GetGamePoint(Point realPoint);
		Point GetRealPoint(Point gamePoint);
		
		bool IsNear(Point from, int direction, Point p);
		
		void PaintField(Draw& w);
		void PaintSegments(Draw& w);
		void PaintValidMoves(Draw& w);
		void PaintBall(Draw& w);
		void PaintOSD(Draw& w);
		void PaintSelectedPoint(Draw& w);

		
	public:
		Callback whenUpdated;
		
		void AdvanceBall();
		class PlayerInterface : public Player
		{
			private:
				FootballCtrl *owner;
			public:
				PlayerInterface(FootballCtrl *owner) : owner(owner) { }
				virtual void GenMove(int token);
		};
		
		FootballCtrl(UI &ui);
		virtual void Paint(Draw& w);
		void GenMove(int token);
		
		virtual void LeftDown(Point p, dword keyflags);

		void ResetGame();
		void RequestHint();
};

#endif
