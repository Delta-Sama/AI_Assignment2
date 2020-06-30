#pragma once
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "Tile.h"
#include "Player.h"
#include "Sprite.h"

#include <array>
#include <map>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>

enum engineState { title, game, end };

class Engine
{
public: // Public methods.
	int Run();
	static Engine& Instance(); // This static method creates the static instance that can be accessed 'globally'
	SDL_Renderer* GetRenderer();
	bool& Running();

	void setRunning(bool run) const { Instance().m_running = run; }
	void setEngineState(int state) { m_gameState = static_cast<engineState>(state); }
	SDL_Point* getMouse() const { return &Instance().m_mousePos; }
	engineState getState() const { return m_gameState; }
	SDL_Window* getWindow() const { return m_pWindow; }

private: // Private properties.
	bool m_running; // Loop control flag.
	Uint32 m_start, m_end, m_delta, m_fps; // Fixed timestep variables.
	SDL_Window* m_pWindow; // This represents the SDL window.
	SDL_Renderer* m_pRenderer; // This represents the buffer to draw to.
	// Example-specific properties.
	SDL_Point m_mousePos;
	engineState m_gameState;

private: // Private methods.
	Engine();
	bool Init(const char* title, int xpos, int ypos, int width, int height, int flags);
	void Wake();
	void Sleep();
	void HandleEvents();
	void Update();
	void Render();
	void Clean();

};

#endif