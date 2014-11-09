#ifndef _Football_Football_h
#define _Football_Football_h

#include <CtrlLib/CtrlLib.h>

#include "FootballCtrl.h"

#define LAYOUTFILE <Football/Football.lay>
#include <CtrlCore/lay.h>
using namespace Upp;

class GameProperties : public WithGameProperties<TopWindow>
{
public:
	GameProperties();
};

class Football : public WithFootballLayout<TopWindow>
{
private:
	Game game;
	String gameStateBackup;
	UI ui;
	FootballCtrl footballCtrl;
	MenuBar menuBar;
	ToolBar toolBar;
	StatusBar statusBar;
	FileSel fileSel;
	int player1Type, player2Type;
	
	Vector<Segment> futureHistory;
	MovePlayer lastPlayer;
	int hintPosition;
	
	void Menu(Bar &menu);
	void GameMenu(Bar &menu);
	void HistoryMenu(Bar &menu);
	void HintMenu(Bar &menu);
	void EditMenu(Bar &menu);
	void ToolbarCallback(Bar &tool);

	void NewGame();
	void OpenGame();
	void SaveGame();
	
	bool HasAnyGameBegun();
	bool CanGoBack();
	bool CanHint();
	bool CanEdit();
	bool IsInHistory();
	bool IsHinting();
	bool IsEditing();
	
	void HistoryPrevious();
	void HistoryNext();
	void HistoryPreviousPlayer();
	void HistoryNextPlayer();
	void HistoryCommit();
	void HistoryAbort();
	void HistoryMoveForward();
	void HistoryMoveBackward();
	
	void HintStart();
	void HintAccept();
	void HintReject();
	
	void EditBoard();
	void EditCommit();
	void EditAbort();
	void EditTogglePlayer();
	
	void StartGame();
	void StopGame();
	
	void RefreshToolbar();
	Player* PreparePlayer(int type);
	void PrepareGame(int width, int height);
	void FinishPreparingGame();
	void ExitGame();
public:
	typedef Football CLASSNAME;
	Football();
	~Football();
	void Xmlize(XmlIO &xml);
};

#endif
