#pragma once
//-----------------------------------------------------------------------------------------------
#include <vector>
#include <string>


enum class StringAlignment
{
	LEFT,
	RIGHT,
	MIDDLE,
};

//-----------------------------------------------------------------------------------------------
typedef std::vector<std::string> Strings;

const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

void IndentString(std::string& input, int maxLength, StringAlignment alignment);

Strings SplitStringOnDelimiter(const std::string& originalString, char delimiterToSplitOn);
Strings SplitStringOnFirstDelimiter(std::string const& originalString, char delimiterToSplitOn);
void ParseConsoleCommand(std::string const& command, Strings& tokens); 
bool ContainsSubstring(const std::string& inputString, const std::string& compareString);


