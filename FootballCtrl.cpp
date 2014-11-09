#include "FootballCtrl.h"
using namespace Upp;

FootballCtrl::FootballCtrl(UI &ui)
	: ui(ui), isPlaying(false), ballStep(0), prevBallPos(Null)
{

}

void FootballCtrl::HandleUpdate()
{
	GuiLock __;
	
	isPlaying = false;
	Refresh();

	if(ballStep == 0)
		SetTimeCallback(ANIMATION_TIME / ANIMATION_STEPS, THISBACK(AdvanceBall));

	if(ui.GetState().currentState == PLAYER1_WON)
	{
		PromptOK(t_("Player 1 has won"));
	}
	else if(ui.GetState().currentState == PLAYER2_WON)
	{
		PromptOK(t_("Player 2 has won"));
	}

	whenUpdated();
}

void FootballCtrl::AdvanceBall()
{
	ballStep += 1;
	Refresh();
	if(ballStep < ANIMATION_STEPS)
		SetTimeCallback(ANIMATION_TIME / ANIMATION_STEPS, THISBACK(AdvanceBall));
	else
	{
		prevBallPos = ui.GetState().ballPos;
		ballStep = 0;
		if(ui.GetGame().IsGameLoopActive())
		{
			if(ui.GetState().currentState == HINT && ui.GetGame().nextPlayer != playerHinted)
				ui.GetGame().Stop();
			else
				ui.GetGame().Continue();
		}
		
	}
}

void FootballCtrl::UpdateSize()
{
	squareSize = min(GetRect().Height() / (ui.GetState().height + 2),
	                 GetRect().Width() / ui.GetState().width);
	
	fieldCorner = GetRect().CenterPos(Size(ui.GetState().width, ui.GetState().height + 2) * squareSize);
	
}

Point FootballCtrl::GetGamePoint(int x, int y)
{
	return GetGamePoint(Point(x, y));
}

Point FootballCtrl::GetRealPoint(int x, int y)
{
	return GetRealPoint(Point(x, y));
}

Point FootballCtrl::GetGamePoint(Point realPoint)
{
	return (realPoint - fieldCorner) / squareSize - Point(0, 1);
}

Point FootballCtrl::GetRealPoint(Point gamePoint)
{
	return (gamePoint + Point(0, 1)) * squareSize + fieldCorner;
}

bool FootballCtrl::IsNear(Point from, int direction, Point p)
{
	Point vector = GetRealPoint(from + DirectionDiff(direction)) - p;
	return vector.x * vector.x + vector.y * vector.y <= (squareSize / 3) * (squareSize / 3);
}

void FootballCtrl::PaintField(Draw& w)
{
	w.DrawRect(GetRect(), Green());
	
	int wi = ui.GetState().width;
	int he = ui.GetState().height;
	
	for(int i = 0; i <= he; ++i)
		w.DrawLine(GetRealPoint(0, i), GetRealPoint(wi, i), PEN_DOT, Gray());
	
	for(int i = 1; i < wi; ++i)
		if(i == wi / 2)
			w.DrawLine(GetRealPoint(i, -1), GetRealPoint(i, he + 1), PEN_DOT, Gray());
		else
			w.DrawLine(GetRealPoint(i, 0), GetRealPoint(i, he), PEN_DOT, Gray());
	
	w.DrawLine(GetRealPoint(0, 0), GetRealPoint(0, he), 1, White());
	w.DrawLine(GetRealPoint(wi, 0), GetRealPoint(wi, he), 1, White());
	           
	w.DrawLine(GetRealPoint(0, 0), GetRealPoint(wi / 2 - 1, 0), 1, White());
	w.DrawLine(GetRealPoint(0, he), GetRealPoint(wi / 2 - 1, he), 1, White());
	           
	w.DrawLine(GetRealPoint(wi / 2 + 1, 0), GetRealPoint(wi, 0), 1, White());
	w.DrawLine(GetRealPoint(wi / 2 + 1, he), GetRealPoint(wi, he), 1, White());
	           
	w.DrawLine(GetRealPoint(wi / 2 - 1, 0), GetRealPoint(wi / 2 - 1, -1), 1, White());
	w.DrawLine(GetRealPoint(wi / 2 + 1, 0), GetRealPoint(wi / 2 + 1, -1), 1, White());
	w.DrawLine(GetRealPoint(wi / 2 - 1, -1), GetRealPoint(wi / 2 + 1, -1), 1, White());
	
	w.DrawLine(GetRealPoint(wi / 2 - 1, he), GetRealPoint(wi / 2 - 1, he + 1), 1, White());
	w.DrawLine(GetRealPoint(wi / 2 + 1, he), GetRealPoint(wi / 2 + 1, he + 1), 1, White());
	w.DrawLine(GetRealPoint(wi / 2 - 1, he + 1), GetRealPoint(wi / 2 + 1, he + 1), 1, White());	
}

void FootballCtrl::PaintSegments(Draw& w)
{
	bool current = ui.GetState().currentState != EDIT;
	MovePlayer lastPlayer = ui.GetState().currentState == HINT ? playerHinted : ui.GetState().currentPlayer;
	
	for(int i = ui.GetState().segments.GetCount() - 1; i >= 0; --i)
	{
		if(ui.GetState().segments.IsUnlinked(i))
			continue;
		
		const Segment &s = ui.GetState().segments[i];
		current &= lastPlayer == s.player;
		
		Point from = GetRealPoint(s.ballPos);
		Point to = GetRealPoint(s.ballPos + DirectionDiff(s.direction));
		w.DrawLine(from, to, 1, current ? Yellow() : White());
	}
}

void FootballCtrl::PaintValidMoves(Draw& w)
{
	for(int direction : ui.GetGame().ValidMoves())
	{
		Point from = GetRealPoint(ui.GetState().ballPos);
		Point to = GetRealPoint(ui.GetState().ballPos + DirectionDiff(direction));
		Color color = ui.GetGame().IsGameLoopActive() ? Red() : Gray();
		w.DrawLine(from, to, 1, color);
		w.DrawEllipse(Rect(to, Size(0, 0)).Inflated(squareSize / 6), color);
	}	
}

void FootballCtrl::PaintBall(Draw& w)
{
	Point ballPos = GetRealPoint(prevBallPos) + 
		(GetRealPoint(ui.GetState().ballPos) - GetRealPoint(prevBallPos)) * ballStep / ANIMATION_STEPS; // animation
	Rect ballRect = Rect(ballPos, Size(0, 0)).Inflated(squareSize / 3);
	
	MovePlayer player = ui.GetState().currentState == HINT ? playerHinted : ui.GetState().currentPlayer;
	Color color = player == PLAYER2 ? Black() : White();
		
	w.DrawEllipse(ballRect, color); 	
}

void FootballCtrl::PaintOSD(Draw& w)
{
	String message;
	
	switch(ui.GetState().currentState)
	{
	case ACTIVE:
	case EDIT:
		message = AsString(t_("Player ")) + (ui.GetState().currentPlayer == PLAYER1 ? "1" : "2");
		break;
	case PLAYER1_WON:
		message = t_("Player 1 won");
		break;
	case PLAYER2_WON:
		message = t_("Player 2 won");
		break;
	}
	
	w.DrawText(0, 0, message, SerifZ(10), White());	
}

void FootballCtrl::PaintSelectedPoint(Draw& w)
{
	Rect rect = Rect(GetRealPoint(selectedPoint), Size(0, 0)).Inflated(squareSize / 5);
	w.DrawEllipse(rect, Yellow());
	
	for(int direction = 0; direction < 8; ++direction)
		if(ui.GetGame().IsMovePossible(selectedPoint, direction))
		{
			Rect rect = Rect(GetRealPoint(selectedPoint + DirectionDiff(direction)), Size(0, 0)).Inflated(squareSize / 5);
			w.DrawEllipse(rect, Red());
		}
}

void FootballCtrl::Paint(Draw& w)
{
	UpdateSize();
	
	if(ui.GetState().currentPlayer == FORCE_MAJEURE)
		return;
	
	PaintField(w);
	PaintSegments(w);
	
	if((ui.GetState().currentState == ACTIVE || ui.GetState().currentState == HINT)
	   && (isPlaying || !ui.GetGame().IsGameLoopActive()))
	{
		PaintValidMoves(w);
	}
	
	PaintBall(w);
	
	if(ui.GetState().currentState == EDIT)
	{
		if(selectedPoint != Null)
			PaintSelectedPoint(w);
	}
	
	PaintOSD(w);
}

void FootballCtrl::LeftDown(Point p, dword keyflags)
{
	if(ui.GetState().currentState == EDIT)
	{
		if(keyflags & K_SHIFT)
		{
			Point cand = GetGamePoint(p + Point(squareSize / 2, squareSize / 2));
			
			if(cand.x >= 0 && cand.x <= ui.GetState().width
			   && cand.y >= 0 && cand.y <= ui.GetState().height)
			   ui.GetState().ballPos = prevBallPos = cand;	
			
			selectedPoint = Null;
		}
		else if(selectedPoint == Null)
		{
			selectedPoint = GetGamePoint(p + Point(squareSize / 2, squareSize / 2));
			
			if(selectedPoint.x < 0 || selectedPoint.x > ui.GetState().width
			   || (selectedPoint.y < 0 || selectedPoint.y > ui.GetState().height) && !ui.GetGame().IsBorder(selectedPoint))
			   selectedPoint = Null;
		}
		else
		{
			for(int direction = 0; direction < 8; ++direction)
			{
				if(IsNear(selectedPoint, direction, p) && ui.GetGame().IsMovePossible(selectedPoint, direction))
				{
					if(!ui.GetState().HasSegment(selectedPoint, direction))
						ui.GetState().segments.Add(Segment{selectedPoint, direction, FORCE_MAJEURE});
					else
					{
						ui.GetState().segments.UnlinkKey(Segment{selectedPoint, direction});
						ui.GetState().segments.UnlinkKey(Segment{selectedPoint + DirectionDiff(direction), (direction + 4) % 8}); // opposite
					}
				}
			}
				
			selectedPoint = Null;
		}

		Refresh();
	}
	else
	{
		if(isPlaying && ui.GetGame().IsGameLoopActive())
			for(int direction : ui.GetGame().ValidMoves())
				if(IsNear(ui.GetState().ballPos, direction, p))
				{
					isPlaying = false;
					ui.GetGame().Move(token, direction);
				}
	}
}

void FootballCtrl::GenMove(int token)
{
	if(ui.GetState().currentState == HINT)
	{
		ASSERT(ui.GetState().currentPlayer == playerHinted);
		hinter->GenMove(token);
	}
	else
	{
		GuiLock __;
		isPlaying = true;
		this->token = token;
		Refresh();
	}
}

void FootballCtrl::PlayerInterface::GenMove(int token)
{
	owner->GenMove(token);
}

void FootballCtrl::ResetGame()
{
	isPlaying = false;
	selectedPoint = Null;
	playerHinted = FORCE_MAJEURE;
	ui.Register(THISBACK(HandleUpdate));
	prevBallPos = ui.GetState().ballPos;
}

void FootballCtrl::RequestHint()
{
	hinter.Create();
	hinter->Setup(&ui.GetGame(), ui.GetState().currentPlayer == PLAYER1);
	hinter->GenMove(token);
	playerHinted = ui.GetState().currentPlayer;
}
