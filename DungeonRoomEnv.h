#pragma once
#include "AbstractGymEnv.h"
#include <string>

namespace CGym
{
	enum class DUNGEON_FEATURE {
		NOTHING,
		PLAYER,
		WALL,
		KEY,
		EXIT
	};

    class DungeonRoomEnv :
        public AbstractGymEnv_1D
    {
	public:
		// Need to provide a zero-argument contructor, so all inputs have
		// defaults:
		DungeonRoomEnv(
			int maxCoord = 20,
			bool playerAlwaysOnSouth = true,
			double scoreLossWallHit = 0.01,
			double scoreLossPerTurn = 0.0,
			double scoreLossPerDistanceFromExit = 0.02,
			int maxSteps = 300,
			double keyScore = 20.0,
			double exitScore = 100.0,
			bool requireKeyToExit = true,
			bool multipleRooms = true,
			bool distToWallInObs = true,
			bool allow8Direction = false,
			bool repeatededKeys = false,
			bool useKeys = true);

		std::vector<double> reset();
		int getObservationColumnCount();
		int getActionSize();
		void step(
			const std::vector<double>& action,  // Input: The action
			std::vector<double>& nextObservation,  // Output: Next observation
			double& stepReward,  // Output: Reward
			bool& done,  // Output: True if done.
			std::vector<std::string>& info);  // Output: Extra information
		void render();

	protected:
		// Variables that change the environment:
		int m_observationSize;
		int m_actionSize;
		int m_maxCoordinate;
		bool m_playerAlwaysStartOnSouthWall;
		double m_scoreLossFromHittingWall;
		double m_scoreLossPerTurn;
		double m_scoreLossPerDistanceFromExit; // Lose more score further from exit
		int m_maxSteps;
		double m_getKeyScoreGain;
		double m_exitScoreGain;
		bool m_requireKeyToExit;
		bool m_multipleRooms;  // If false, the environment completes when exiting
		bool m_8DirectionTravel; // If True, 8 directions allowed, else 4.
		bool m_repeatedKeys;  // If True, gathering a key places a new one.
		bool m_useKeys; // If false, ignore all other key inputs and don't use/place keys at all.
				
		bool m_distToWallInObservation;  // If True, the observation will include
		                                 // the distance to walls in each direction.

		// Current locations and status of various objects:
		int m_playerX;
		int m_playerY;
		int m_exitX;
		int m_exitY;
		int m_keyX;
		int m_keyY;
		bool m_playerHasKey;
		int m_currentTurnNumber;

		void resetForNewRoom();
		void placeAKey();
		void placeTheExit();
		char getAsciiFromObservation(DUNGEON_FEATURE obs);
		DUNGEON_FEATURE getAtCoordinate(int X, int Y);
		std::vector<double> getCurrentObservation_goalRadar();
		std::vector<double> getCurrentObservation_surroundings();
		void handleFoundExit(bool& done, int& playerX, int& playerY);
    };

};  // End CGym namespace
