// CGym.cpp : Defines the functions for the static library.

#include "pch.h"
#include "framework.h"
#include "AbstractGymEnv.h"
#include <iostream>

namespace CGym
{
	void AbstractGymEnv_1D::render()
	{
		std::cout << "This environment doesn't allow rendering." << std::endl;
	}

	void AbstractGymEnv_2D::render()
	{
		std::cout << "This environment doesn't allow rendering." << std::endl;
	}
}