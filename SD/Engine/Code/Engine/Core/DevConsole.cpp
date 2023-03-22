#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Net/RemoteConsole.hpp"

DevConsole* g_theDevConsole;

DevConsole::DevConsole(DevConsoleConfig const& config)
	: m_config(config)
	, m_clock(Clock::GetSystemClock())
	, m_caretStopwatch(&m_clock, config.m_caretBlinkTimer)
{
}


DevConsole::~DevConsole()
{
}


Rgba8 const DevConsole::INFO_ERROR(255, 0, 0, 255);
Rgba8 const DevConsole::INFO_WARNING(255, 255, 0, 255);
Rgba8 const DevConsole::INFO_MAJOR(0, 200, 0, 255);
Rgba8 const DevConsole::INFO_MINOR(0, 100, 200, 255);
Rgba8 const DevConsole::INFO_INPUT(150, 150, 255, 255);
Rgba8 const DevConsole::COLOR_CARET(255, 255, 255, 255);


void DevConsole::Startup()
{
	SubscribeEventCallbackFunction("KeyPressed", Event_KeyPressed);
	SubscribeEventCallbackFunction("CharInput", Event_CharInput);
	SubscribeEventCallbackFunction("Paste", Event_Paste);
	SubscribeEventCallbackFunction("clear", Command_Clear);
	SubscribeEventCallbackFunction("help", Command_Help);
	SubscribeEventCallbackFunction("executeCommandScript", Command_ExecuteCommandFromFile);

	if (m_config.m_hasRemoteConsole)
	{
		RemoteConsoleConfig config;
		config.console = this;
		g_theRemoteConsole = new RemoteConsole(config);
		g_theRemoteConsole->Startup();
	}
}


void DevConsole::BeginFrame()
{
	if (IsOpen())
	{
		if (m_caretStopwatch.CheckDurationElapsedAndDecrement())
		{
			m_caretVisible = !m_caretVisible;
		}
	}

	if (g_theRemoteConsole)
	{
		g_theRemoteConsole->BeginFrame();
	}
}


void DevConsole::EndFrame()
{
	if (g_theRemoteConsole)
	{
		g_theRemoteConsole->EndFrame();
	}
}


void DevConsole::ShutDown()
{
	if (g_theRemoteConsole)
	{
		g_theRemoteConsole->Shutdown();
	}
}


void DevConsole::ExecuteXmlCommandScriptFile(std::string const& XmlFilePath)
{
	XmlDocument doc;
	doc.LoadFile(XmlFilePath.c_str());

	XmlElement const* element = doc.RootElement();

	if (element) ExecuteXmlCommandScriptNode(*element);
}

void DevConsole::ExecuteXmlCommandScriptNode(XmlElement const& commandScriotXmlElement)
{
	XmlElement const* child = commandScriotXmlElement.FirstChildElement();
	while (child)
	{
		std::string command = Stringf("%s", child->Value());
		for (XmlAttribute const* attribute = child->FirstAttribute(); attribute != nullptr; attribute = attribute->Next())
		{
			command += Stringf(" %s=\"%s\"", attribute->Name(), attribute->Value());
		}
		Execute(command);
		child = child->NextSiblingElement();
	}
}

void DevConsole::Execute(std::string const& consoleCommandText)
{
	AddLine(DevConsole::INFO_INPUT, consoleCommandText);

	Strings commands = SplitStringOnDelimiter(consoleCommandText, '\n');
	for (int commandIndex = 0; commandIndex < int(commands.size()); commandIndex++)
	{
		std::string commandLine = commands[commandIndex];
		//Strings tokens = SplitStringOnDelimiter(commandLine, ' ');
		Strings tokens;
		ParseConsoleCommand(commandLine, tokens); 
		std::string commandName = tokens[0];
		EventArgs argumentPairs;
		for (int tokenIndex = 1; tokenIndex < int(tokens.size()); tokenIndex++)
		{
			//Strings argumentPair = SplitStringOnDelimiter(tokens[tokenIndex], '=');
			Strings argumentPair = SplitStringOnFirstDelimiter(tokens[tokenIndex], '=');
			if (argumentPair.size() == 2)
			{
				argumentPairs.SetValue(argumentPair[0], argumentPair[1]);
			}
		}
		FireEvent(commandName, argumentPairs);
		CallHandler(commandName, argumentPairs);
	}
}


void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	DevConsoleLine line;
	line.m_tint = color;
	line.m_text = text;
	m_lines.push_back(line);
}


void DevConsole::ClearLines()
{
	m_lines.clear();
}


void DevConsole::Render(AABB2 const& bounds, Renderer* rendererOverride) const
{
	UNUSED(rendererOverride)

	BitmapFont* font = m_config.m_renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	switch (m_mode)
	{
	case DevConsoleMode::HIDDEN:
		break;

	case DevConsoleMode::OPEN_FULL:
		Render_OpenFull(bounds, *m_config.m_renderer , *font);
		if (g_theRemoteConsole)
		{
			g_theRemoteConsole->Render(bounds, *m_config.m_renderer, *font);
		}
		break;
	}
}


DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}


void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;
}


void DevConsole::ToggleMode(DevConsoleMode mode)
{
	if (m_mode == mode)
	{
		m_mode = DevConsoleMode::HIDDEN;
	}
	else
	{
		m_mode = mode;
	}
	ResetCaret();
}


bool DevConsole::IsOpen() const
{
	return GetMode() != DevConsoleMode::HIDDEN;
}


void DevConsole::ClearInput()
{
	m_inputLine = "";
	SetCaretPosition(0);
}


void DevConsole::AddToCommandHistory(std::string input)
{
	m_commandHistory.insert(m_commandHistory.begin(), input);

	if ((int)m_commandHistory.size() > m_config.m_maxCommandHistory)
	{
		m_commandHistory.pop_back();
	}
}


void DevConsole::SetCaretPosition(int position)
{
	m_caretPosition = position;

	if (m_caretPosition < 0)
	{
		m_caretPosition = 0;
	}

	if (m_caretPosition > (int)m_inputLine.size())
	{
		m_caretPosition = (int)m_inputLine.size();
	}

	ResetCaret();
}


void DevConsole::RemoveCharacterOnLine(int direction)
{
	if (direction < 0)
	{
		if (m_caretPosition > 0)
		{
			m_inputLine.erase(m_caretPosition - 1, 1);
			SetCaretPosition(m_caretPosition - 1);
		}
	}
	else if (direction > 0)
	{
		if (m_caretPosition <= (int)m_inputLine.size() - 1)
		{
			m_inputLine.erase(m_caretPosition, 1);
			SetCaretPosition(m_caretPosition);
		}
	}
}


void DevConsole::ResetCaret()
{
	m_caretStopwatch.Restart();
	m_caretVisible = true;
}


std::string const DevConsole::GetPreviousCommand()
{
	if (m_historyIndex < (int)m_commandHistory.size() - 1)
	{

		m_historyIndex++;
		return m_commandHistory[m_historyIndex];
	}

	if ((int)m_commandHistory.size() == 0) 
	{
		return "";
	}

	return m_commandHistory[(int)m_commandHistory.size() - 1];
}


std::string const DevConsole::GetNextCommand()
{
	if (m_historyIndex > 0)
	{
		m_historyIndex--;
		return m_commandHistory[m_historyIndex];
	}
	m_historyIndex = -1;
	return "";
}


Clock const& DevConsole::GetClock() const
{
	return m_clock;
}


void DevConsole::Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect) const
{
	//draw console box
	Vec2 boxDimensions = bounds.GetDimensions();
	float cellHeight = boxDimensions.y / m_config.m_consoleLines;
	std::vector<Vertex_PCU> pauseVertexArray;
	AABB2 screenBox(bounds.m_mins, bounds.m_maxs);
	AddVertsForAABB2D(pauseVertexArray, screenBox, Rgba8(150, 150, 150, 50));
	AABB2 inputLineBox(Vec2(0.f, 0.f), Vec2(boxDimensions.x, cellHeight));
	AddVertsForAABB2D(pauseVertexArray, inputLineBox, Rgba8(100, 100, 100, 150));
	renderer.BindTexture(nullptr);
	renderer.DrawVertexArray(int(pauseVertexArray.size()), pauseVertexArray.data());

	//draw each console line   
	std::vector<Vertex_PCU> consoleVerts;
	for (int lineIndex = int(m_lines.size()) - 1; lineIndex >= 0; lineIndex--)
	{
		DevConsoleLine line = m_lines[lineIndex];
		AABB2 lineBox(Vec2(0.f, cellHeight * static_cast<float>(m_lines.size() - lineIndex)), Vec2(boxDimensions.x, cellHeight * static_cast<float>(m_lines.size() - lineIndex + 1)));
		font.AddVertsForTextInBox2D(consoleVerts, lineBox, cellHeight, line.m_text, line.m_tint, fontAspect, Vec2(0.f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
	}
	AABB2 inputBox(Vec2(0.f, 0.f), Vec2(boxDimensions.x, cellHeight));
	font.AddVertsForTextInBox2D(consoleVerts, inputBox, cellHeight, m_inputLine, DevConsole::INFO_INPUT, fontAspect, Vec2(0.f, 0.5f), TextBoxMode::SHRINK_TO_FIT);

	renderer.BindTexture(&font.GetTexture());
	renderer.DrawVertexArray(int(consoleVerts.size()), consoleVerts.data());

	if (m_caretVisible)
	{
		std::vector<Vertex_PCU> caretVerts;
		float caretPosX = font.GetTextWidth(cellHeight, m_inputLine.substr(0, m_caretPosition), fontAspect);
		//float caretPosX = cellHeight * fontAspect * m_caretPosition;
		LineSegment2 caretLine(Vec2(caretPosX + .5f, 0.f), Vec2(caretPosX + .5f, cellHeight));
		AddVertsForLineSegment2D(caretVerts, caretLine, 1.f, DevConsole::COLOR_CARET);
		renderer.BindTexture(nullptr);
		renderer.DrawVertexArray(int(caretVerts.size()), caretVerts.data());
	}
}


bool DevConsole::Event_KeyPressed(EventArgs& args)
{
	if (!g_theDevConsole->IsOpen()) return false;

	unsigned char key = args.GetValue("key", (unsigned char)0);
	switch (key)
	{
		case 27: //escape
			g_theDevConsole->ClearInput();
			break;
		case 13: //enter
			g_theDevConsole->Execute(g_theDevConsole->m_inputLine);
			g_theDevConsole->AddToCommandHistory(g_theDevConsole->m_inputLine);
			g_theDevConsole->ClearInput();
			g_theDevConsole->m_historyIndex = -1;
			break;
		case 37: //left arrow
			g_theDevConsole->SetCaretPosition(g_theDevConsole->m_caretPosition - 1);
			break;
		case 39: //right arrow
			g_theDevConsole->SetCaretPosition(g_theDevConsole->m_caretPosition + 1);
			break;
		case 38: //up arrow
			g_theDevConsole->m_inputLine = g_theDevConsole->GetPreviousCommand();
			g_theDevConsole->SetCaretPosition((int)(g_theDevConsole->m_inputLine).size());
			break;
		case 40: //down arrow
			g_theDevConsole->m_inputLine = g_theDevConsole->GetNextCommand();
			g_theDevConsole->SetCaretPosition((int)(g_theDevConsole->m_inputLine).size());
			break;
		case 36: //home
			g_theDevConsole->SetCaretPosition(0);
			break;
		case 35: //end
			g_theDevConsole->SetCaretPosition((int)(g_theDevConsole->m_inputLine).size());
			break;
		case 46: //delete
			g_theDevConsole->RemoveCharacterOnLine(1);
			break;
		case 8: //back space
			g_theDevConsole->RemoveCharacterOnLine(-1);
			break;
	}
	return false;
}


bool DevConsole::Event_CharInput(EventArgs& args)
{
	if (!g_theDevConsole->IsOpen()) return false;

	int charCode = args.GetValue("key", 0);
	if ((charCode >= 32) && (charCode <= 126) && (charCode != '`') && (charCode != '~'))
	{
		g_theDevConsole->m_inputLine.insert((g_theDevConsole->m_caretPosition), 1, static_cast<unsigned char>(charCode));
		g_theDevConsole->m_caretPosition++;
		g_theDevConsole->ResetCaret();
	}
	return false;
}


bool DevConsole::Event_Paste(EventArgs& args)
{
	if (!g_theDevConsole->IsOpen()) return false;

	std::string clipboard = args.GetValue("clipboard", "");
	g_theDevConsole->m_inputLine.append(clipboard);
	g_theDevConsole->m_caretPosition += static_cast<int>(clipboard.size());
	g_theDevConsole->ResetCaret();

	return false;
}


bool DevConsole::Command_Clear(EventArgs& args)
{
	UNUSED(args)

	if (!g_theDevConsole->IsOpen()) return false;

	g_theDevConsole->ClearLines();
	return false;
}


bool DevConsole::Command_Help(EventArgs& args)
{
	if (!g_theDevConsole->IsOpen()) return false;

	std::vector<std::string> commands;
	g_theEventSystem->GetRegisteredEventNames(commands);
	std::string filter = args.GetValue("Filter", "");

	std::string numCommands = Stringf("## Registered Commands [%d] ##", commands.size());
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, numCommands);
	if (!filter.empty())
	{
		std::string filterLine = Stringf("-- Filtering to '%s' --", filter.c_str());
		g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, filterLine);
	}

	for (int index = 0; index < (int)commands.size(); index++)
	{
		if (ContainsSubstring(commands[index], filter))
		{
			g_theDevConsole->AddLine(DevConsole::INFO_MINOR, commands[index]);
		}
	}

	return false;
}


bool DevConsole::Command_ExecuteCommandFromFile(EventArgs& args)
{
	std::string const& filePath = args.GetValue("filePath", "");

	g_theDevConsole->ExecuteXmlCommandScriptFile(filePath);

	return false;
}


