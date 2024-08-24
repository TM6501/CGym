#pragma once
#include <vector>
#include <string>

namespace CGym
{
	class AbstractGymEnv_1D
	{
	public:
		virtual ~AbstractGymEnv_1D() {}
		
		// Needs to be called between each run. Returns the initial observation
		virtual std::vector<double> reset() = 0;
		
		// 1D only has a single row observation:
		int getObservationRowCount() { return 1; }

		// For 1D, this would be the total observation count:
		virtual int getObservationColumnCount() = 0;

		// Action size of expected action vector. Some environments
		// will use the whole thing, others will take the max index:
		virtual int getActionSize() = 0;

		// Python Gym returns many values. We will return by references:
		virtual void step(
			const std::vector<double>& action,  // Input: The action
			std::vector<double>& nextObservation,  // Output: Next observation
			double& stepReward,  // Output: Reward
			bool& done,  // Output: True if done.
			std::vector<std::string>& info) = 0;  // Output: Extra information

		// Render the current frame of the environment. Default will
		// print that rendering isn't implement.
		virtual void render();

	protected:	


	private:
	};

	class AbstractGymEnv_2D
	{
	public:
		virtual ~AbstractGymEnv_2D() {}

		// See above 1D class for function and argument meanings:
		virtual std::vector<std::vector<double> > reset() = 0;
		virtual int getObservationRowCount() = 0;
		virtual int getObservationColumnCount() = 0;
		virtual int getActionSize() = 0;
		virtual void step(const std::vector<double>& action,
			std::vector<double>& nextObservation, double& stepReward,
			bool& done, std::vector<std::string>& info) = 0;
		virtual void render();

	protected:
	
	private:
	};

};  // End CGym namespace