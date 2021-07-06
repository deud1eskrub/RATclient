#pragma once
#include <cstdlib>

#include <iostream>

namespace sEncryption {

	void __cdecl zeroBuffer(char* inputBuffer, int bufferSize)
	{

		std::memset(inputBuffer, NULL, bufferSize);
	};
};