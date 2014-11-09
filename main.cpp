#include "Football.h"
#include "AI.h"

#define TFILE <Football/Football.t>
#include <Core/t.h>

GameProperties::GameProperties()
{
	CtrlLayoutOKCancel(*this, t_("Game properties"));
	halfHeight.SetData(Game::DEFAULT_HEIGHT / 2);
	halfWidth.SetData(Game::DEFAULT_WIDTH / 2);
	player1Type.SetData(0);
	player2Type.SetData(0);
}

Football::Football()
	: ui(game), footballCtrl(ui), player1Type(0), player2Type(0)
{
	CtrlLayout(*this, t_("Football"));
	Zoomable().Sizeable();
	
	Add(footballCtrl.HSizePosZ().VSizePosZ());
	AddFrame(menuBar);
	AddFrame(toolBar);
	AddFrame(statusBar);
	
	menuBar.Set(THISBACK(Menu));
	
	menuBar.WhenHelp = toolBar.WhenHelp = statusBar;
	footballCtrl.whenUpdated = THISBACK(RefreshToolbar);
	
	game.state.currentPlayer = FORCE_MAJEURE;
	
	RefreshToolbar();
}

void Football::RefreshToolbar()
{
	toolBar.Set(THISBACK(ToolbarCallback));	
}

void Football::Menu(Bar &menu)
{
	menu.Add(t_("Game"), THISBACK(GameMenu));
	menu.Add(t_("History"), THISBACK(HistoryMenu));
	menu.Add(t_("Hint"), THISBACK(HintMenu));
	menu.Add(t_("Edit"), THISBACK(EditMenu));
}

bool Football::HasAnyGameBegun()
{
	return ui.GetState().currentPlayer != FORCE_MAJEURE;
}

bool Football::CanGoBack()
{
	if(!HasAnyGameBegun())
		return false;
	
	if(IsHinting())
		return ui.GetState().segments.GetCount() > hintPosition;
	else
		return !IsEditing() && !ui.GetState().isHistoryInvalidated &&
	       !ui.GetState().segments.IsEmpty() && ui.GetState().segments.Top().player != FORCE_MAJEURE;	
}

bool Football::CanHint()
{
	return HasAnyGameBegun() &&
	       ui.GetGame().IsGameLoopActive() && ui.GetState().currentState == ACTIVE &&
		   ((ui.GetGame().nextPlayer == PLAYER1 && player1Type == 0)
		   || (ui.GetGame().nextPlayer == PLAYER2 && player2Type == 0));	
}

bool Football::CanEdit()
{
	return HasAnyGameBegun() && ui.GetState().currentState == ACTIVE;
}

bool Football::IsInHistory()
{
	return !futureHistory.IsEmpty();	
}

bool Football::IsHinting()
{
	return ui.GetState().currentState == HINT;	
}

bool Football::IsEditing()
{
	return ui.GetState().currentState == EDIT;
}

void Football::HistoryMenu(Bar &menu)
{
	menu.Add(CanGoBack(),
	         t_("Previous player's move"),
	         CtrlImg::go_back(),
	         THISBACK(HistoryPreviousPlayer))
	        .Help(t_("Go back in history to the previous player's move"));
	        
	menu.Add(CanGoBack(),
	         t_("Previous"),
	         CtrlImg::SmallLeft(),
	         THISBACK(HistoryPrevious))
	        .Help(t_("Go back in history"));
	
	menu.Add(IsInHistory(),
	         t_("Next"),
	         CtrlImg::SmallRight(),
	         THISBACK(HistoryNext))
	         .Help(t_("Go forward in history"));
	         
	menu.Add(IsInHistory(),
	         t_("Next players' move"),
	         CtrlImg::go_forward(),
	         THISBACK(HistoryNextPlayer))
	         .Help(t_("Go forward in history to the next player's move"));
	         
	menu.Add(!IsHinting() && IsInHistory(),
	         t_("Commit"),
	         CtrlImg::check(),
	         THISBACK(HistoryCommit))
	         .Help(t_("Resume play from this position in history"));
	         
	menu.Add(!IsHinting() && IsInHistory(),
	         t_("Abort"),
	         CtrlImg::cross(),
	         THISBACK(HistoryAbort))
	         .Help(t_("Abort browsing history and return to game"));
}

void Football::HintMenu(Bar &menu)
{
	menu.Add(CanHint(),
	         t_("Hint"),
	         CtrlImg::help(),
	         THISBACK(HintStart))
	         .Help("Show hint");

	menu.Add(IsHinting(),
	         t_("Accept hint"),
	         CtrlImg::check(),
	         THISBACK(HintAccept))
	         .Help(t_("Accept shown hint and continue playing"));
	         
	menu.Add(IsHinting(),
	         t_("Reject hint"),
	         CtrlImg::cross(),
	         THISBACK(HintReject))
	         .Help(t_("Reject shown hint and return to game"));
}

void Football::EditMenu(Bar &menu)
{
	menu.Add(CanEdit(),
	         t_("Edit board"),
			 CtrlImg::write(),
			 THISBACK(EditBoard))
			 .Help(t_("Open board editor"));	

	menu.Add(IsEditing(),
	         t_("Commit edits"),
			 CtrlImg::check(),
			 THISBACK(EditCommit))
			 .Help(t_("Commit changes"));
			 
	menu.Add(IsEditing(),
	         t_("Abort editing"),
			 CtrlImg::cross(),
			 THISBACK(EditAbort))
			 .Help(t_("Abort editing and return to the game"));
			 
	menu.Add(IsEditing(),
	         t_("Toggle player"),
			 CtrlImg::Toggle(),
			 THISBACK(EditTogglePlayer))
			 .Help(t_("Toggle player"));
}

void Football::GameMenu(Bar &menu)
{
	menu.Add(t_("New"), CtrlImg::new_doc(), THISBACK(NewGame)).Help(t_("Start a new game"));
	menu.Add(t_("Open"), CtrlImg::open(), THISBACK(OpenGame)).Help(t_("Open saved game"));
	
	menu.Add(!IsHinting() && !IsEditing() && !IsInHistory() && HasAnyGameBegun(),
	         t_("Save"),
	         CtrlImg::save(),
	         THISBACK(SaveGame))
	         .Help(t_("Save game"));
	menu.Separator();
	
	menu.Add(!ui.GetGame().IsGameLoopActive() && HasAnyGameBegun()
	         && !IsEditing() && !IsHinting() && !IsInHistory(),
	         t_("Start"),
	         CtrlImg::Plus(),
	         THISBACK(StartGame))
	         .Help(t_("Start game"));
	         
	menu.Add(ui.GetGame().IsGameLoopActive(),
	         t_("Stop"),
	         CtrlImg::Minus(),
	         THISBACK(StopGame))
	         .Help(t_("Stop game"));
	
	menu.Add(t_("Exit"), THISBACK(ExitGame)).Help(t_("Exit game"));
}

void Football::ToolbarCallback(Bar &tool)
{
	GameMenu(tool);
	tool.Separator();
	
	HistoryMenu(tool);
	tool.Separator();
	
	HintMenu(tool);
	tool.Separator();
	
	EditMenu(tool);	
}

void Football::HintStart()
{
	ui.GetState().currentState = HINT;
	hintPosition = ui.GetState().segments.GetCount();
	footballCtrl.RequestHint();
	RefreshToolbar();
}

void Football::HintAccept()
{
	ui.GetState().currentState = ACTIVE;
	HistoryCommit();
}

void Football::HintReject()
{
	while(ui.GetState().segments.GetCount() > hintPosition)
		HistoryMoveBackward();
	
	footballCtrl.AdvanceBall();
	
	ui.GetState().currentState = ACTIVE;
	HistoryCommit();
}

void Football::HistoryMoveBackward()
{
	if(futureHistory.IsEmpty())
		lastPlayer = ui.GetState().currentPlayer;
	
	futureHistory.Add(ui.GetState().segments.Top());
	ui.GetState().currentPlayer = futureHistory.Top().player;
	ui.GetState().ballPos -= DirectionDiff(ui.GetState().segments.Top().direction);
	ui.GetState().segments.Drop();
}

void Football::HistoryPrevious()
{
	if(ui.GetGame().IsGameLoopActive())
		ui.GetGame().Stop();
	
	HistoryMoveBackward();
	footballCtrl.AdvanceBall();
		
	RefreshToolbar();
	Refresh();
}

void Football::HistoryPreviousPlayer()
{
	if(ui.GetGame().IsGameLoopActive())
		ui.GetGame().Stop();
	
	MovePlayer player = ui.GetState().segments.Top().player;
	while(!ui.GetState().segments.IsEmpty() && ui.GetState().segments.Top().player == player)
		HistoryMoveBackward();
	
	footballCtrl.AdvanceBall();
		
	RefreshToolbar();
	Refresh();
}

void Football::HistoryMoveForward()
{
	ui.GetState().segments.Add(futureHistory.Top());
	ui.GetState().ballPos += DirectionDiff(ui.GetState().segments.Top().direction);
	futureHistory.Drop();

	if(futureHistory.IsEmpty())
	{
		ui.GetState().currentPlayer = lastPlayer;
	}
	else
		ui.GetState().currentPlayer = futureHistory.Top().player;
}

void Football::HistoryNext()
{
	HistoryMoveForward();
	
	footballCtrl.AdvanceBall();
	
	RefreshToolbar();
	Refresh();
}

void Football::HistoryNextPlayer()
{
	MovePlayer player = futureHistory.Top().player;
	ASSERT(player != FORCE_MAJEURE);

	while(!futureHistory.IsEmpty() && futureHistory.Top().player == player)
		HistoryMoveForward();
	
	footballCtrl.AdvanceBall();
	
	RefreshToolbar();
	Refresh();
}

void Football::HistoryCommit()
{
	futureHistory.Clear();
	StartGame();
}

void Football::HistoryAbort()
{
	while(!futureHistory.IsEmpty())
		HistoryMoveForward();
	
	StartGame();
}

void Football::EditBoard()
{
	if(ui.GetGame().IsGameLoopActive())
		ui.GetGame().Stop();
	
	PromptOK(t_("You are now in edit mode.&&To draw a segment, click on its begin and then on its end.&"
	            "To move ball click with the Shift key pressed.&To toggle player, use button in the toolbar."));
	gameStateBackup = StoreAsXML(ui.GetState(), "backup");
	
	ui.GetState().currentState = EDIT;
	
	RefreshToolbar();
	Refresh();
}

void Football::EditCommit()
{
	if(!PromptYesNo(t_("Are you sure? This action will invalidate history.")))
		return;
	
	ui.GetState().isHistoryInvalidated = true;
	ui.GetState().currentState = ACTIVE;
	ui.GetState().segments.Sweep();
	gameStateBackup.Clear();
	
	RefreshToolbar();
	Refresh();
}

void Football::EditAbort()
{
	LoadFromXML(ui.GetState(), gameStateBackup);
	gameStateBackup.Clear();
	ui.GetState().currentState = ACTIVE;
	ui.GetState().segments.Sweep();
	
	RefreshToolbar();
	Refresh();
}

void Football::EditTogglePlayer()
{
	ui.GetState().currentPlayer = Opponent(ui.GetState().currentPlayer);
	
	RefreshToolbar();
	Refresh();
}

void Football::StartGame()
{
	footballCtrl.ResetGame();
	ui.GetGame().Start();
		
	RefreshToolbar();
	Refresh();
}

void Football::StopGame()
{
	ui.GetGame().Stop();
	RefreshToolbar();
	Refresh();
}

void Football::NewGame()
{
	game.Stop();
	GameProperties properties;

	{
		GuiLock __;
		if(!properties.ExecuteOK()
		   && HasAnyGameBegun()) // check if any game has been started before
		{
			StartGame();
			return;
		}
	}
	
	player1Type = ~properties.player1Type;
	player2Type = ~properties.player2Type;
	PrepareGame(2 * static_cast<int>(~properties.halfWidth), 2 * static_cast<int>(~properties.halfHeight));
	FinishPreparingGame();
	
	Refresh();
	RefreshToolbar();
}

Player* Football::PreparePlayer(int type)
{
	if(type == 0)
		return new FootballCtrl::PlayerInterface(&footballCtrl);
	else
		return new AI();	
}

void Football::PrepareGame(int width, int height)
{
	futureHistory.Clear();
	game.Stop();
	game = Game(width, height);
}

void Football::FinishPreparingGame()
{
	game.SetPlayers(PreparePlayer(player1Type), PreparePlayer(player2Type));
	
	if(game.state.currentPlayer == PLAYER1 && player1Type == 0
	   || game.state.currentPlayer == PLAYER2 && player2Type == 0) // Player is a human
	   StartGame();
	else
	{
		footballCtrl.ResetGame();
		RefreshToolbar();
		Refresh();
	}
}

void Football::OpenGame()
{
	GuiLock __;
	
	game.Stop();
	
	if(fileSel.ExecuteOpen())
	{
		PrepareGame(Game::DEFAULT_WIDTH, Game::DEFAULT_HEIGHT);
		if(!LoadFromXMLFile(*this, ~fileSel))
		{
			Exclamation(t_("Invalid game file"));
		}
		FinishPreparingGame();
	}
	else if(HasAnyGameBegun()) // check if any game has been started before
		StartGame();
}


void Football::SaveGame()
{
	GuiLock __;
	switch(ui.GetState().currentState)
	{
	case ACTIVE:
	case PLAYER1_WON:
	case PLAYER2_WON:
		game.Stop();
		
		if(fileSel.ExecuteSaveAs())	
			StoreAsXMLFile(*this, "game", ~fileSel);
		
		footballCtrl.ResetGame();
		game.Start();
		Refresh();
		break;
	default:
		Exclamation(t_("Can't save game while giving a hint, browsing history or editing board"));
	}
}

void Football::ExitGame()
{
	GuiLock __;
	game.Stop();
	
	if(PromptYesNo(t_("Are you sure?")))
		Close();
	else if(HasAnyGameBegun())
		StartGame();
}

Football::~Football()
{
	game.Stop();
}

void Football::Xmlize(XmlIO &xml)
{
	xml
		("player1", player1Type)
		("player2", player2Type)
		("state", ui.GetState());
}

GUI_APP_MAIN
{
	SetLanguage(GetSystemLNG());
	Football().Run();
}
