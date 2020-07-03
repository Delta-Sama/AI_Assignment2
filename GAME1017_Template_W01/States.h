#pragma once
#ifndef _STATES_H_
#define _STATES_H_

#include <SDL.h>
#include "Button.h"
#include "TextureManager.h"
#include "CollisionManager.h"
#include "Label.h"
#include "Tile.h"
#include "Player.h"
#include <array>
#include <map>

#define ROWS 24
#define COLS 32

class State // This is the abstract base class for all specific states.
{
protected:
	State() {}
public:
	virtual void Update() = 0; // Having at least one 'pure virtual' method like this, makes a class abtract.
	virtual void Render();     // Meaning we cannot create objects of the class.
	virtual void Enter() = 0;  // Virtual keyword means we can override in derived class.
	virtual void Exit() = 0;
	virtual void Resume();
};

class GameState : public State
{
private:
	SDL_Texture* m_pTileText, * m_pPlayerText;
	std::map<char, Tile*> m_tiles;
	std::array<std::array<Tile*, COLS>, ROWS> m_level; // Fixed-size STL array of Tile pointers.
	std::vector<Label*> labels;
	std::vector<Label*> debugLabels;
	Label* totalPathCostLabel;
	Player* m_pPlayer;
	Sprite* m_pBling;
	bool m_debug = false, m_hEuclid = true;
	bool m_marching = false;

	std::vector<PathConnection*> m_followPath;
	const int m_maxMarchFrame = 5;
	int m_marchPosition = 0;
	int m_marchFrame = 0;
	int m_deltaMoveX, m_deltaMoveY;

public:
	GameState();
	void Update();
	void Render();
	void Enter();
	void Exit();
	void Resume();

	std::array<std::array<Tile*, COLS>, ROWS>& GetLevel() { return m_level; }
};

class TitleState : public State
{
public:
	TitleState();
	void Update();
	void Render();
	void Enter();
	void Exit();
private:
	Button* m_playBtn;
	Label* m_pName;
};

class EndState : public State
{
public:
	EndState();
	~EndState() {}
	void Update();
	void Render();
	void Enter();
	void Exit();
private:

};

#endif