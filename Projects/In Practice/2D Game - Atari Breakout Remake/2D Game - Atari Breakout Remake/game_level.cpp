#include "game_level.h"

#include <fstream>
#include <sstream>

void GameLevel::Load(std::string file, uint32_t levelWidth, uint32_t levelHeight)
{
	// clear old data
	this->Bricks.clear();
	// load from file
	uint32_t tileCode;
	GameLevel level;
	std::string line;
	std::ifstream fstream(file);
	std::vector<std::vector<uint32_t>> tileData;
	if (fstream)
	{
		while (std::getline(fstream, line)) // read each line from level file
		{
			std::istringstream sstream(line);
			std::vector<uint32_t> row;
			while (sstream >> tileCode) // // read each word separated by spaces
			{
				row.push_back(tileCode);
			}
			tileData.push_back(row);
		}
		if (tileData.size() > 0)
		{
			this->init(tileData, levelWidth, levelHeight);
		}
	}
}

void GameLevel::Draw(SpriteRenderer& renderer)
{
	for (GameObject& tile : this->Bricks)
		if (!tile.Destroyed)	
			tile.Draw(renderer);
}

bool GameLevel::IsCompleted()
{
	for (GameObject& tile : this->Bricks)
		if (!tile.IsSolid && !tile.Destroyed)
			return false;
	return true;
}

void GameLevel::init(std::vector<std::vector<uint32_t>> tileData, uint32_t levelWidth, uint32_t levelHeight)
{
	// calculate dimensions
	uint32_t height = tileData.size();
	uint32_t width = tileData[0].size(); // note we can index vector at [0] since this function is only called if height > 0
	float unit_width = levelWidth / static_cast<float>(width);
	float unit_height = levelHeight / static_cast<float>(height);
	//initialize level tile base on tileData
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			// check block type from level data (2D level array)
			if (tileData[y][x] == 1) // solid
			{
				glm::vec2 size(unit_width, unit_height);
				glm::vec2 pos = size * glm::vec2(x,y);
				GameObject obj(pos, size, ResourceManager::GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
				obj.IsSolid = true;
				this->Bricks.push_back(obj);
			}
			else if (tileData[y][x] > 1)
			{
				glm::vec3 color = glm::vec3(1.0f);
				if (tileData[y][x] == 2)
					color = glm::vec3(0.2f, 0.6f, 1.0f);
				else if (tileData[y][x] == 3)
					color = glm::vec3(0.0f, 0.7f, 0.0f);
				else if (tileData[y][x] == 4)
					color = glm::vec3(0.8f, 0.8f, 0.4f);
				else if (tileData[y][x] == 5)
					color = glm::vec3(1.0f, 0.5f, 0.0f);

				glm::vec2 size(unit_width, unit_height);
				glm::vec2 pos = size * glm::vec2(x, y);
				this->Bricks.push_back(GameObject(pos, size, ResourceManager::GetTexture("block"), color));
			}
		}
	}
}
