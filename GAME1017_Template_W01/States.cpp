#include "States.h"
#include "StateManager.h" // Make sure this is NOT in "States.h" or circular reference.
#include "Engine.h"
#include "SoundManager.h"
#include "PathManager.h"
#include "EventManager.h"
#include "DebugManager.h"
#include "FontManager.h"
#include <iostream>
#include <fstream>

void State::Render()
{
	SDL_RenderPresent(Engine::Instance().GetRenderer());
}

void State::Resume() {}

void TitleState::Enter()
{
	m_playBtn = new PlayButton({ 0,0,512,200 }, { 312.0f,100.0f,400.0f,100.0f }, Engine::Instance().GetRenderer(), TEMA::GetTexture("play"));
	
	m_pName = new Label("title", 32, 32, "Maxim Dobrivskiy, 101290100");
	
	//FOMA::SetSize("Img/LTYPE.TTF", "tile", 10);
	Engine::Instance().setEngineState(title);
}

TitleState::TitleState()
{
}

void TitleState::Update()
{
	m_playBtn->Update();
}

void TitleState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 255, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	m_playBtn->Render();
	m_pName->Render();
}

void TitleState::Exit()
{
	delete m_playBtn;
	m_playBtn = nullptr;
	delete m_pName;
	m_pName = nullptr;
}

GameState::GameState()
{
}

void GameState::Enter()
{
	SOMA::PlaySound("start", 0, 1);
	Engine::Instance().setEngineState(game);

	totalPathCostLabel = new Label("title", 32, 32, "Total path cost: ...", { 0,0,0,255 });
	labels.push_back(new Label("title", 32, 64, "Press R to reset the Scene", { 0, 0, 0, 255 }));
	labels.push_back(new Label("title", 32, 96, "Press H for Debug Mode", { 0,0,0,255 }));
	debugLabels.push_back(new Label("title", 32, 64, "Press R to reset the Scene", {0, 0, 0, 255}));
	debugLabels.push_back(new Label("title", 32, 96, "Press F to find Shortest Path", { 0, 0, 0, 255 }));
	debugLabels.push_back(new Label("title", 32, 128, "Press H for Game Mode", { 0, 0, 0, 255 }));

	m_pTileText = TEMA::GetTexture("tiles");
	m_pPlayerText = TEMA::GetTexture("maga");
	m_pPlayer = new Player({ 0,0,32,32 }, { (float)(16) * 32, (float)(12) * 32, 32, 32 }, Engine::Instance().GetRenderer(), m_pPlayerText, 0, 0, 0, 4);
	m_pBling = new Sprite({ 224,64,32,32 }, { (float)(16) * 32, (float)(4) * 32, 32, 32 }, Engine::Instance().GetRenderer(), m_pTileText);
	std::ifstream inFile("Dat/Tiledata.txt");
	if (inFile.is_open())
	{ // Create map of Tile prototypes.
		char key;
		int x, y;
		bool obst, haz;
		while (!inFile.eof())
		{
			inFile >> key >> x >> y >> obst >> haz;
			m_tiles.emplace(key, new Tile({ x * 32, y * 32, 32, 32 }, { 0,0,32,32 }, Engine::Instance().GetRenderer(), m_pTileText, obst, haz));
		}
	}
	inFile.close();
	inFile.open("Dat/Level1.txt");
	if (inFile.is_open())
	{ // Build the level from Tile prototypes.
		char key;
		for (int row = 0; row < ROWS; row++)
		{
			for (int col = 0; col < COLS; col++)
			{
				inFile >> key;
				m_level[row][col] = m_tiles[key]->Clone(); // Prototype design pattern used.
				m_level[row][col]->GetDstP()->x = (float)(32 * col);
				m_level[row][col]->GetDstP()->y = (float)(32 * row);
				// Instantiate the labels for each tile.
				m_level[row][col]->m_lCost = new Label("tile", m_level[row][col]->GetDstP()->x + 4, m_level[row][col]->GetDstP()->y + 18, " ", { 0,0,0,255 });
				m_level[row][col]->m_lX = new Label("tile", m_level[row][col]->GetDstP()->x + 18, m_level[row][col]->GetDstP()->y + 2, std::to_string(col).c_str(), { 0,0,0,255 });
				m_level[row][col]->m_lY = new Label("tile", m_level[row][col]->GetDstP()->x + 2, m_level[row][col]->GetDstP()->y + 2, std::to_string(row).c_str(), { 0,0,0,255 });
				// Construct the Node for a valid tile.
				if (!m_level[row][col]->IsObstacle() || m_level[row][col]->IsHazard())
					m_level[row][col]->m_node = new PathNode((int)(m_level[row][col]->GetDstP()->x), (int)(m_level[row][col]->GetDstP()->y));
			}
		}
	}
	inFile.close();
	// Now build the graph from ALL the non-obstacle and non-hazard tiles. Only N-E-W-S compass points.
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			if (m_level[row][col]->Node() == nullptr) // Now we can test for nullptr.
				continue; // An obstacle or hazard tile has no connections.
			// Make valid connections.
			if (row - 1 != -1 && !m_level[row - 1][col]->IsObstacle() && !m_level[row - 1][col]->IsHazard())
				m_level[row][col]->Node()->AddConnection(new PathConnection(m_level[row][col]->Node(), m_level[row - 1][col]->Node(),
					MAMA::Distance(m_level[row][col]->Node()->x, m_level[row - 1][col]->Node()->x, m_level[row][col]->Node()->y, m_level[row - 1][col]->Node()->y)));
			if (row + 1 != ROWS && !m_level[row + 1][col]->IsObstacle() && !m_level[row + 1][col]->IsHazard())
				m_level[row][col]->Node()->AddConnection(new PathConnection(m_level[row][col]->Node(), m_level[row + 1][col]->Node(),
					MAMA::Distance(m_level[row][col]->Node()->x, m_level[row + 1][col]->Node()->x, m_level[row][col]->Node()->y, m_level[row + 1][col]->Node()->y)));
			if (col - 1 != -1 && !m_level[row][col - 1]->IsObstacle() && !m_level[row][col - 1]->IsHazard())
				m_level[row][col]->Node()->AddConnection(new PathConnection(m_level[row][col]->Node(), m_level[row][col - 1]->Node(),
					MAMA::Distance(m_level[row][col]->Node()->x, m_level[row][col - 1]->Node()->x, m_level[row][col]->Node()->y, m_level[row][col - 1]->Node()->y)));
			if (col + 1 != COLS && !m_level[row][col + 1]->IsObstacle() && !m_level[row][col + 1]->IsHazard())
				m_level[row][col]->Node()->AddConnection(new PathConnection(m_level[row][col]->Node(), m_level[row][col + 1]->Node(),
					MAMA::Distance(m_level[row][col]->Node()->x, m_level[row][col + 1]->Node()->x, m_level[row][col]->Node()->y, m_level[row][col + 1]->Node()->y)));
		}
	}
}

void GameState::Update()
{
	std::string txt;
	PAMA::getPath()->size() == 0 ? txt = "Total path cost: ..." : txt = "Total path cost: " + std::to_string(PAMA::getTotalPathCost());
	totalPathCostLabel->SetText(txt.c_str());

	// m_pPlayer->Update(); // Just stops MagaMan from moving.
	if (EVMA::KeyPressed(SDL_SCANCODE_H)) // Toggle debug mode.
		m_debug = !m_debug;
	if (EVMA::KeyPressed(SDL_SCANCODE_SPACE)) // Toggle the heuristic used for pathfinding.
	{
		m_hEuclid = !m_hEuclid;
		std::cout << "Setting " << (m_hEuclid ? "Euclidian" : "Manhattan") << " heuristic..." << std::endl;
	}
	if ((EVMA::MousePressed(1) or EVMA::MousePressed(3)) and m_debug) // If user has clicked.
	{
		PAMA::ClearPath();
		int xIdx = (EVMA::GetMousePos().x / 32);
		int yIdx = (EVMA::GetMousePos().y / 32);
		if (m_level[yIdx][xIdx]->IsObstacle() || m_level[yIdx][xIdx]->IsHazard())
			return; // We clicked on an invalid tile.
		if (EVMA::MousePressed(1)) // Move the player with left-click.
		{
			if (m_pBling->GetDstP()->x != (float)(xIdx * 32) or m_pBling->GetDstP()->y != (float)(yIdx * 32))
			{
				m_pPlayer->GetDstP()->x = (float)(xIdx * 32);
				m_pPlayer->GetDstP()->y = (float)(yIdx * 32);
			}
		}
		else if (EVMA::MousePressed(3)) // Else move the bling with right-click.
		{
			if (m_pPlayer->GetDstP()->x != (float)(xIdx * 32) or m_pPlayer->GetDstP()->y != (float)(yIdx * 32))
			{
				m_pBling->GetDstP()->x = (float)(xIdx * 32);
				m_pBling->GetDstP()->y = (float)(yIdx * 32);
			}
		}
		for (int row = 0; row < ROWS; row++) // "This is where the fun begins."
		{ // Update each node with the selected heuristic and set the text for debug mode.
			for (int col = 0; col < COLS; col++)
			{
				if (m_level[row][col]->Node() == nullptr)
					continue;
				if (m_hEuclid)
					m_level[row][col]->Node()->SetH(PAMA::HEuclid(m_level[row][col]->Node(), m_level[(int)(m_pBling->GetDstP()->y / 32)][(int)(m_pBling->GetDstP()->x / 32)]->Node()));
				else
					m_level[row][col]->Node()->SetH(PAMA::HManhat(m_level[row][col]->Node(), m_level[(int)(m_pBling->GetDstP()->y / 32)][(int)(m_pBling->GetDstP()->x / 32)]->Node()));
				m_level[row][col]->m_lCost->SetText(std::to_string((int)(m_level[row][col]->Node()->H())).c_str());
			}
		}
	}

	if (EVMA::KeyPressed(SDL_SCANCODE_R)) // Reset scene
		STMA::ChangeState(new GameState);

	if (EVMA::KeyPressed(SDL_SCANCODE_F) and m_debug) // Calculate the path
	{
		PAMA::GetShortestPath(m_level[(int)(m_pPlayer->GetDstP()->y / 32)][(int)(m_pPlayer->GetDstP()->x / 32)]->Node(),
			m_level[(int)(m_pBling->GetDstP()->y / 32)][(int)(m_pBling->GetDstP()->x / 32)]->Node());
	}
}

void GameState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 60, 60, 180, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	// Draw anew.
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			m_level[row][col]->Render(); // Render each tile.
			// Render the debug data...
			if (m_debug && m_level[row][col]->Node() != nullptr)
			{
				m_level[row][col]->m_lCost->Render();
				m_level[row][col]->m_lX->Render();
				m_level[row][col]->m_lY->Render();
				// I am also rendering out each connection in blue. If this is a bit much for you, comment out the for loop below.
				for (unsigned i = 0; i < m_level[row][col]->Node()->GetConnections().size(); i++)
				{
					DEMA::QueueLine({ m_level[row][col]->Node()->GetConnections()[i]->GetFromNode()->x + 16, m_level[row][col]->Node()->GetConnections()[i]->GetFromNode()->y + 16 },
						{ m_level[row][col]->Node()->GetConnections()[i]->GetToNode()->x + 16, m_level[row][col]->Node()->GetConnections()[i]->GetToNode()->y + 16 }, { 255,255,255,255 });
				}
			}

		}
	}

	if (m_debug)
	{
		PAMA::DrawPath();
		DEMA::FlushLines();
	}

	m_pPlayer->Render();
	m_pBling->Render();

	if (m_debug)
	{
		for (Label* label : debugLabels)
		{
			label->Render();
		}
	}
	else
	{
		for (Label* label : labels)
		{
			label->Render();
		}
	}

	totalPathCostLabel->Render();
}

void GameState::Exit()
{
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			delete m_level[row][col];
			m_level[row][col] = nullptr; // Wrangle your dangle.
		}
	}

	for (auto const& i : m_tiles)
	{
		delete m_tiles[i.first];
	}
	m_tiles.clear();

	for (Label* label : labels)
	{
		delete label;
	}
	labels.clear();

	delete totalPathCostLabel;

	PAMA::ClearPath();
	DEMA::Quit();
}

void GameState::Resume()
{
}

EndState::EndState()
{
}

void EndState::Update()
{
}

void EndState::Render()
{
}

void EndState::Enter()
{
	Engine::Instance().setEngineState(end);
}

void EndState::Exit()
{
}