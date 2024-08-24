#include "pch.h"
#include "DungeonRoomEnv.h"
#include <random>
#include <map>
#include <iostream>
#include <assert.h>

namespace CGym
{
	// Windows:
	const static std::string CLEAR_COMMAND = "cls";
	
	// Linux:
	// const static std::string CLEAR_COMMAND = "clear";

	DungeonRoomEnv::DungeonRoomEnv(int maxCoord,
		bool playerAlwaysOnSouth, double scoreLossWallHit, double scoreLossPerTurn,
		double scoreLossPerDistanceFromExit, int maxSteps, double keyScore,
		double exitScore, bool requireKeyToExit, bool multipleRooms,
		bool distToWallInObs, bool allow8Direction, bool repeatedKeys, bool useKeys)
		: AbstractGymEnv_1D(),
		m_maxCoordinate(maxCoord),		
		m_playerAlwaysStartOnSouthWall(playerAlwaysOnSouth),
		m_scoreLossFromHittingWall(scoreLossWallHit),
		m_scoreLossPerTurn(scoreLossPerTurn),
		m_scoreLossPerDistanceFromExit(scoreLossPerDistanceFromExit),
		m_maxSteps(maxSteps),
		m_getKeyScoreGain(keyScore),
		m_exitScoreGain(exitScore),
		m_requireKeyToExit(requireKeyToExit),
		m_multipleRooms(multipleRooms),
		m_8DirectionTravel(allow8Direction),
		m_repeatedKeys(repeatedKeys),
		m_useKeys(useKeys),
		m_distToWallInObservation(distToWallInObs),
		m_playerHasKey(false), 
		m_currentTurnNumber(0)
	{
		// Variables that may require calculation will be initialized in here:
		m_observationSize = 2;
		
		// Distance adds 4 inputs:
		if (m_distToWallInObservation)
		{
			m_observationSize += 4;  // One for each direction
		}

		// Distance to key adds 3 inputs (1 for if you're holding the key):
		if (m_useKeys)
		{
			m_observationSize += 3;
		}

		m_actionSize = 4;  // Move in each direction.
		if (m_8DirectionTravel)
		{
			m_actionSize = 8;
		}

		// Item locations are set during reset:
		m_playerX = -1;
		m_playerY = -1;
		m_exitX = -1;
		m_exitY = -1;
		m_keyX = -1;
		m_keyY = -1;
	}
	
	int DungeonRoomEnv::getObservationColumnCount()
	{
		return m_observationSize;
	}

	int DungeonRoomEnv::getActionSize()
	{
		return m_actionSize;
	}

	std::vector<double> DungeonRoomEnv::reset()
	{
		// Do the per-room reset first:
		resetForNewRoom();

		m_currentTurnNumber = 0;

		return getCurrentObservation_goalRadar();
	}

	void DungeonRoomEnv::placeAKey()
	{
		if (!m_useKeys)
			return;

		static std::random_device rd;
		static std::mt19937 rng(rd());		
		static std::uniform_int_distribution<int> 
			coordDistribution(1, m_maxCoordinate - 1);

		// Place the key:
		m_keyX = coordDistribution(rng);
		m_keyY = coordDistribution(rng);

		// Make sure the and key isn't in the same location as the
		// player or exit:
		while ((m_playerX == m_keyX && m_playerY == m_keyY)
			|| (m_exitX == m_keyX && m_exitY == m_keyY))
		{
			m_keyX = coordDistribution(rng);
			m_keyY = coordDistribution(rng);
		}
	}

	void DungeonRoomEnv::placeTheExit()
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());
		static std::uniform_int_distribution<int>
			coordDistribution(1, m_maxCoordinate - 1);

		// Place the exit:
		m_exitX = coordDistribution(rng);
		m_exitY = coordDistribution(rng);

		// Make sure it doesn't overlap with any other objects:
		while ((m_exitX == m_keyX && m_exitY == m_keyY)
			|| (m_exitX == m_playerX && m_exitY == m_playerY))
		{
			m_exitX = coordDistribution(rng);
			m_exitY = coordDistribution(rng);
		}
	}

	void DungeonRoomEnv::resetForNewRoom()
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());
		
		// Place the player:
		static std::uniform_int_distribution<int> 
			coordDistribution(1, m_maxCoordinate-1);

		m_playerX = coordDistribution(rng);
		m_playerY = 1;
		if (!m_playerAlwaysStartOnSouthWall)
		{
			m_playerY = coordDistribution(rng);
		}				
		
		placeAKey();
		placeTheExit();
		
		m_playerHasKey = false;		
	}

	char DungeonRoomEnv::getAsciiFromObservation(DUNGEON_FEATURE obs)
	{
		// Convert what was found to the the ascii that will be rendered:
		static std::map<DUNGEON_FEATURE, char> obsMapping = {
			{DUNGEON_FEATURE::NOTHING, '-'},
			{DUNGEON_FEATURE::PLAYER, '@'},
			{DUNGEON_FEATURE::WALL, 'W'},
			{DUNGEON_FEATURE::KEY, 'K'},
			{DUNGEON_FEATURE::EXIT, 'E'}
		};
		
		return obsMapping[obs];
	}

	// This is the observation output to the user if we're using an
	// 8-output immediate-surroundings observation:
	double getOutputFeatureFromObservation(DUNGEON_FEATURE obs)
	{
		static std::map<DUNGEON_FEATURE, double> obsMapping = {
			{DUNGEON_FEATURE::NOTHING, -1.0},
			{DUNGEON_FEATURE::PLAYER, 0.0},  // Duplicated because never output
			{DUNGEON_FEATURE::WALL, 0.0},
			{DUNGEON_FEATURE::KEY, 0.5},
			{DUNGEON_FEATURE::EXIT, 1.0}
		};

		return obsMapping[obs];
	}

	DUNGEON_FEATURE DungeonRoomEnv::getAtCoordinate(int X, int Y)
	{
		DUNGEON_FEATURE retVal = DUNGEON_FEATURE::NOTHING;

		// Wall will be overridden if we find the exit:
		if (X == 0 || Y == 0 || X == m_maxCoordinate || Y == m_maxCoordinate)
		{
			retVal = DUNGEON_FEATURE::WALL;
		}
		else if (X == m_playerX && Y == m_playerY)
		{
			retVal = DUNGEON_FEATURE::PLAYER;
		}
		else if (X == m_keyX && Y == m_keyY)
		{
			retVal = DUNGEON_FEATURE::KEY;
		}

		if (X == m_exitX && Y == m_exitY)
		{
			retVal = DUNGEON_FEATURE::EXIT;
		}

		return retVal;
	}

	void DungeonRoomEnv::render()
	{
		system(CLEAR_COMMAND.c_str());		
		for (int y = m_maxCoordinate; y >= 0; y -= 1)
		{
			for (int x = 0; x <= m_maxCoordinate; ++x)
			{
				std::cout << getAsciiFromObservation(getAtCoordinate(x, y));
			}
			std::cout << std::endl;
		}
	}

	std::vector<double> DungeonRoomEnv::getCurrentObservation_goalRadar()
	{
		std::vector<double> retVal;

		// First two values of the observation are distances from the key:
		if (m_useKeys)
		{
			if (m_playerHasKey && !m_repeatedKeys)
			{
				retVal.push_back(0.0);
				retVal.push_back(0.0);
			}
			else  // There is a key in the room:
			{
				// Actual play-size is 2 less than the max coordinate:
				retVal.push_back((double(m_keyX) - m_playerX) / (m_maxCoordinate - 2.0));
				retVal.push_back((double(m_keyY) - m_playerY) / (m_maxCoordinate - 2.0));
			}
		}

		// Distance from the player to the exit:
		retVal.push_back((double(m_exitX) - m_playerX) / (m_maxCoordinate - 2.0));
		retVal.push_back((double(m_exitY) - m_playerY) / (m_maxCoordinate - 2.0));

		// If the player has the key, mark as such:
		if (m_useKeys)
		{
			if (m_playerHasKey)
			{
				retVal.push_back(1.0);
			}
			else
			{
				retVal.push_back(0.0);
			}
		}

		// If we're also providing the distance to the nearest wall, add it here.
		// With the square room, these could be condensed to a single value effectively providing
		// the agent with its coordinates, but we're leaving room for expansion with the idea that
		// the nearest wall isn't always the boundary of the room.
		if (m_distToWallInObservation)
		{
			// Left, Up, Right, Down:
			retVal.push_back(double(m_playerX - 1.0) / (m_maxCoordinate - 2.0));
			retVal.push_back(double(m_maxCoordinate - m_playerY - 1.0) / (m_maxCoordinate - 2.0));
			retVal.push_back(double(m_maxCoordinate - m_playerX - 1.0) / (m_maxCoordinate - 2.0));
			retVal.push_back(double(m_playerY - 1.0) / (m_maxCoordinate - 2.0));
		}

		return retVal;
	}

	void DungeonRoomEnv::step(const std::vector<double>& action,
		std::vector<double>& nextObservation, double& stepReward,
		bool& done, std::vector<std::string>& info)
	{
		// Actions have to be the right size:
		assert(static_cast<int>(action.size()) == getActionSize());

		// Reward starts at 0:
		stepReward = -m_scoreLossPerTurn;
		done = false;
		info.clear();

		// If we're taking away score for being far from the exit, calculate it here:
		if (m_scoreLossPerDistanceFromExit > 0.0)
		{
			// sqrt[(X1-X2)^2 + (Y1-Y2)^2]
			stepReward -=
				(std::sqrt(((m_playerX - m_exitX) * (m_playerX - m_exitX)) +
					       ((m_playerY - m_exitY) * (m_playerY - m_exitY))) *
				  m_scoreLossPerDistanceFromExit);
		}

		int newX = m_playerX;
		int newY = m_playerY;

		// Get the agent's selected action:
		int maxIdx = int(std::max_element(action.begin(), action.end()) - action.begin());		

		if (m_8DirectionTravel)
		{
			// Change player coordinates based on the action:
			if (maxIdx == 0 || maxIdx >= 6)
				newX -= 1;

			if (maxIdx >= 2 && maxIdx <= 4)
				newX += 1;

			if (maxIdx >= 4 && maxIdx <= 6)
				newY -= 1;

			if (maxIdx <= 2)
				newY += 1;
		}
		else // 4 direction travel: up, right, down, left
		{
			if (maxIdx == 0)
				newY += 1;
			else if (maxIdx == 1)
				newX += 1;
			else if (maxIdx == 2)
				newY -= 1;
			else
				newX -= 1;
		}

		// Check if we just hit a wall:
		if (getAtCoordinate(newX, newY) == DUNGEON_FEATURE::WALL)
		{
			// Go back to where we started:
			newX = m_playerX;
			newY = m_playerY;

			stepReward -= m_scoreLossFromHittingWall;
		}

		// Acquired the key:
		if (m_useKeys)
		{
			if (getAtCoordinate(newX, newY) == DUNGEON_FEATURE::KEY)
			{
				m_playerHasKey = true;

				// If more than one key is allowed, place another:
				if (m_repeatedKeys)
					placeAKey();
				else
				{
					m_keyX = -1;
					m_keyY = -1;
				}

				stepReward += m_getKeyScoreGain;
				info.push_back("Found the key!");
			}
		}

		// Found the exit:
		if (getAtCoordinate(newX, newY) == DUNGEON_FEATURE::EXIT)
		{
			// If we have the key or don't need it:
			if (!m_useKeys || m_playerHasKey || !m_requireKeyToExit)
			{
				stepReward += m_exitScoreGain;
				handleFoundExit(done, newX, newY);
				info.push_back("Found the exit!");
			}
			else  // Exit is treated as a wall:
			{
				newX = m_playerX;
				newY = m_playerY;
				stepReward -= m_scoreLossFromHittingWall;
			}
		}

		// Turn number forward:
		m_currentTurnNumber += 1;
		if (m_currentTurnNumber >= m_maxSteps)
			done = true;

		// Finish moving the player 
		m_playerX = newX;
		m_playerY = newY;

		nextObservation = getCurrentObservation_goalRadar();
	}

	void DungeonRoomEnv::handleFoundExit(bool& done, int& playerX, int& playerY)
	{
		if (m_multipleRooms)  // Finding the exit earns you another dungeon room:
		{
			resetForNewRoom();
			done = false;
			playerX = m_playerX;
			playerY = m_playerY;
		}
		else
		{
			done = true;
		}
	}

	std::vector<double> DungeonRoomEnv::getCurrentObservation_surroundings()
	{
		// Observation is upLeft, up, upRight, right, downRight, down, downLeft, left:
		std::vector<double> retVal{
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX - 1, m_playerY + 1)),
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX, m_playerY + 1)),
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX + 1, m_playerY + 1)),
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX + 1, m_playerY)),
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX + 1, m_playerY - 1)),
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX, m_playerY - 1)),
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX - 1, m_playerY - 1)),
			getOutputFeatureFromObservation(getAtCoordinate(m_playerX - 1, m_playerY))
		};

		return retVal;
	}
	
};  // End namespace CGym
