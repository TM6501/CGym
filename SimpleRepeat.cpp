#include "pch.h"
#include "SimpleRepeat.h"
#include <random>
#include <assert.h>

namespace CGym
{
	SimpleRepeat::SimpleRepeat(int valueCount, int testSteps)
		: m_inAndOutSize(valueCount), m_testSteps(testSteps), m_currentTestStep(0)
	{
	}

	std::vector<double> SimpleRepeat::reset()
	{
		m_currentTestStep = 0;
		m_previousObservation = getSingleObservation();
		return m_previousObservation;
	}

	int SimpleRepeat::getObservationColumnCount()
	{
		return m_inAndOutSize;
	}

	// Action size of expected action vector. Some environments
	// will use the whole thing, others will take the max index:
	int SimpleRepeat::getActionSize()
	{
		return m_inAndOutSize;
	}

	// Python Gym returns many values. We will return by references:
	void SimpleRepeat::step(
		const std::vector<double>& action,
		std::vector<double>& nextObservation,
		double& stepReward,
		bool& done,
		std::vector<std::string>& info)
	{
		assert(action.size() == static_cast<unsigned int>(getActionSize()));

		done = false;
		stepReward = 0.0;
		for (unsigned int i = 0; i < action.size(); ++i)
			stepReward -= abs(action[i] - m_previousObservation[i]);			

		nextObservation = getSingleObservation();
		m_previousObservation = nextObservation;

		m_currentTestStep += 1;
		if (m_currentTestStep >= m_testSteps)
			done = true;
	}

	std::vector<double> SimpleRepeat::getSingleObservation()
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());
		static std::uniform_real_distribution<double> obsDistribution(-1.0, 1.0);

		std::vector<double> obs;
		for (auto i = 0; i < m_inAndOutSize; ++i)
			obs.push_back(obsDistribution(rng));

		return obs;
	}
}
