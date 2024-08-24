#pragma once
#include "AbstractGymEnv.h"

/***
This environment will select a series of random numbers between
-1.0 and 1.0 at every step.  The agent's goal is to simple repeat
that observation back to the environment.  A perfect score (0.0)
occurs only when every single environmental output is mirrored in
the agent's outputs.
***/

namespace CGym
{
    class SimpleRepeat :
        public AbstractGymEnv_1D
    {
	public:

		SimpleRepeat(int valueCount = 6, int testSteps = 100);

		std::vector<double> reset();

		// 1D only has a single row observation:
		int getObservationRowCount() { return 1; }

		// For 1D, this would be the total observation count:
		int getObservationColumnCount();

		// Action size of expected action vector. Some environments
		// will use the whole thing, others will take the max index:
		int getActionSize();

		// Python Gym returns many values. We will return by references:
		void step(
			const std::vector<double>& action,  // Input: The action
			std::vector<double>& nextObservation,  // Output: Next observation
			double& stepReward,  // Output: Reward
			bool& done,  // Output: True if done.
			std::vector<std::string>& info);  // Output: Extra information

	protected:
		int m_inAndOutSize;
		int m_testSteps;
		int m_currentTestStep;
		std::vector<double> m_previousObservation;
		
		std::vector<double> getSingleObservation();
    };
}

