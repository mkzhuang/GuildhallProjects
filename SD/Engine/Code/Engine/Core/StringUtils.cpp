#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


void IndentString(std::string& input, int maxLength, StringAlignment alignment)
{
	int spaces = maxLength - (int)input.size();
	if (spaces <= 0) return;

	switch (alignment)
	{
		case StringAlignment::LEFT:
			input.insert(input.size(), spaces, ' ');
			break;
		case StringAlignment::RIGHT:
			input.insert(0, spaces, ' ');
			break;
		case StringAlignment::MIDDLE:
		{
			int leftHalf = spaces / 2;
			int rightHalf = spaces - leftHalf;
			input.insert(0, leftHalf, ' ');
			input.insert(input.size(), rightHalf, ' ');
			break;
		}
		default:
			break;
	}
}


Strings SplitStringOnDelimiter(const std::string& originalString, char delimiterToSplitOn)
{
	Strings output;
	int index = 0;
	std::string token = "";

	while (index != int(originalString.size()))
	{
		if (originalString[index] == delimiterToSplitOn)
		{
			if (token.empty()) 
			{
				index++;
				continue;
			}
			output.push_back(token);
			token = "";
		}
		else
		{
			token.push_back(originalString[index]);
		}
		index++;
	}
	if (!token.empty()) output.push_back(token);

	return output;
}


Strings SplitStringOnFirstDelimiter(std::string const& originalString, char delimiterToSplitOn)
{
	Strings output;
	std::string token = "";
	for (int charIndex = 0; charIndex < (int)originalString.size(); charIndex++)
	{
		char currentChar = originalString[charIndex];
		if (currentChar == delimiterToSplitOn && charIndex < (int)originalString.size() - 1)
		{
			output.push_back(token);
			std::string second(&originalString[charIndex + 1], originalString.size() - charIndex - 1);
			output.push_back(second);
			return output;
		}
		else
		{
			token.push_back(currentChar);
		}
	}
	if (!token.empty()) output.push_back(token);
	return output;
}

void ParseConsoleCommand(std::string const& command, Strings& tokens)
{
	bool inQuotes = false;
	std::string token = "";
	for (int charIndex = 0; charIndex < (int)command.size(); charIndex++)
	{
		char currentChar = command[charIndex];
		if (currentChar == ' ' && !inQuotes)
		{
			if (token.empty()) continue;
			tokens.push_back(token);
			token = "";
		}
		else if (currentChar == '\"')
		{
			inQuotes = !inQuotes;
		}
		else
		{
			token.push_back(currentChar);
		}
	}

	if (!token.empty()) tokens.push_back(token);
}


bool ContainsSubstring(const std::string& inputString, const std::string& compareString)
{
	if (compareString.empty()) return true;

	int pos = (int)inputString.find(compareString);
	if (pos >= 0)
	{
		return true;
	}

	return false;
}


