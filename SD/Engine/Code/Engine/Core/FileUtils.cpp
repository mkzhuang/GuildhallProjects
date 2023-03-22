#include "Engine/Core/FileUtils.hpp"

#include <iostream>
#include <fstream>

bool FileExists(const std::string& filename)
{
	struct stat buffer;
	int status = stat(filename.c_str(), &buffer);
	return status == 0 ? true : false;
}


int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename)
{
	std::ofstream file(filename, std::ofstream::out | std::ofstream::binary);
	if (file.is_open())
	{
		file.write((char*)inBuffer.data(), inBuffer.size());
		file.close();
		return 0;
	}
	return -1;
}


int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	std::ifstream file(filename, std::ifstream::in | std::ifstream::binary);
	if (file.is_open())
	{
		file.seekg(0, file.end);
		int size = (int)file.tellg();
		file.seekg(0, file.beg);

		outBuffer.resize(size);
		file.read(reinterpret_cast<char*>(outBuffer.data()), outBuffer.size());
		file.close();
		return 0;
	}
	return -1;
}


int FileReadToString(std::string& outString, const std::string& filename)
{
	std::vector<uint8_t> outBuffer;
	std::ifstream file(filename, std::ifstream::in);
	if (file.is_open())
	{
		file.seekg(0, file.end);
		int size = (int)file.tellg();
		file.seekg(0, file.beg);

		outBuffer.resize(size);
		file.read(reinterpret_cast<char*>(outBuffer.data()), outBuffer.size());
		file.close();
		outString = (char*)outBuffer.data() + '\0';
		return 0;
	}
	return -1;
}


