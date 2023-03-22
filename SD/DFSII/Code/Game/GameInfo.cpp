#include "Game/GameInfo.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/GUI/GUI_Canvas.hpp"
#include "Engine/GUI/GUI_Element.hpp"
#include "Engine/GUI/GUI_Text.hpp"
#include "Engine/GUI/GUI_Scrollable.hpp"
#include "Engine/Core/EventSystem.hpp"

GameInfo::GameInfo(Renderer* renderer, AABB2 const& infoBound)
	: m_renderer(renderer)
{
	m_canvas = new GUI_Canvas(infoBound);
	m_canvas->LoadLayout("Data/GameInfo.xml", m_renderer);
	RegisterEventHandler("addLog", this, &GameInfo::AddLog);
	RegisterEventHandler("updatePlayerName", this, &GameInfo::UpdatePlayerName);
	RegisterEventHandler("updateStatus", this, &GameInfo::UpdateStatus);
	RegisterEventHandler("updateItems", this, &GameInfo::UpdateItems);
	m_canvas->ToggleOpen();
}


GameInfo::~GameInfo()
{
	delete m_canvas;
	UnregisterEventHandler("addLog");
	UnregisterEventHandler("updatePlayerName");
	UnregisterEventHandler("updateStatus");
	UnregisterEventHandler("updateItems");
}


void GameInfo::Update()
{
	m_canvas->Update();
}


void GameInfo::Render() const
{
	m_canvas->Render(m_renderer);
}


bool GameInfo::AddLog(EventArgs& args)
{
	BitmapFont* font = m_renderer->CreateOrGetBitmapFont("Data/Fonts/InputMonoBold_44x_64p_aa");
	std::string log = args.GetValue("log", "");
	IndentString(log, 30, StringAlignment::LEFT);
	GUI_Text* logElement = new GUI_Text();
	logElement->SetCanvas(m_canvas);
	logElement->SetAnchorType(GUI_AnchorType::NONE);
	logElement->SetText(log);
	logElement->SetFont(font);
	logElement->SetHasDropShadow(true);
	logElement->SetTextHeight(16.f);
	logElement->SetTextColor(Rgba8(200, 200, 200, 255));
	logElement->SetTextAlignment(Vec2(0.f, 0.5f));
	logElement->SetCanHover(false);

	GUI_Element* scrollable = m_canvas->FindElementByName("logScrollable");
	static_cast<GUI_Scrollable*>(scrollable)->AddEntry(logElement);

	return false;
}


bool GameInfo::UpdatePlayerName(EventArgs& args)
{
	std::string const& name = args.GetValue("name", "");
	GUI_Element* playerName = m_canvas->FindElementByName("nameText");
	static_cast<GUI_Text*>(playerName)->SetText(name);

	return false;
}


bool GameInfo::UpdateStatus(EventArgs& args)
{
	float worldDay = args.GetValue("worldDay", 0.f);
	float currentHealth = args.GetValue("currentHealth", 0.f);
	float maxHealth = args.GetValue("maxHealth", 0.f);
	std::string const& action = "Action: " + args.GetValue("action", "");
	std::string const& goal = "Goal: " + args.GetValue("goal", "") + " " + action;
	// world day
	float dayF = worldDay / 1.f;
	int dayI = RoundDownToInt(dayF);
	float hourF = fmodf(worldDay, 1.f) * 24.f;
	int hourI = RoundDownToInt(hourF);
	std::string worldDayInfo = Stringf("Day:%d, Hour:%d", dayI, hourI);

	GUI_Element* worldDayText = m_canvas->FindElementByName("worldDayText");
	static_cast<GUI_Text*>(worldDayText)->SetText(worldDayInfo);

	// health
	float healthRatio = currentHealth / maxHealth;
	std::string healthInfo = Stringf("%.f/%.f", currentHealth, maxHealth);

	GUI_Element* healthText = m_canvas->FindElementByName("healthBarText");
	healthText->SetBackgroundUVs(AABB2(Vec2::ZERO, Vec2(healthRatio, 1.f)));
	static_cast<GUI_Text*>(healthText)->SetText(healthInfo);

	// action
	GUI_Element* actionText = m_canvas->FindElementByName("actionText");
	static_cast<GUI_Text*>(actionText)->SetText(goal);

	return false;
}


bool GameInfo::UpdateItems(EventArgs& args)
{
	int gold = args.GetValue("gold", 0);
	std::string const& itemString = args.GetValue("items", "");
	Strings items = SplitStringOnDelimiter(itemString, ' ');

	std::map<std::string, int> counts;
	for (std::string const& item : items)
	{
		std::map<std::string, int>::iterator iter = counts.find(item);

		if (iter == counts.end())
		{
			counts[item] = 1;
		}
		else
		{
			iter->second++;
		}
	}

	BitmapFont* font = m_renderer->CreateOrGetBitmapFont("Data/Fonts/InputMonoBold_44x_64p_aa");

	GUI_Element* scrollable = m_canvas->FindElementByName("itemScrollable");
	static_cast<GUI_Scrollable*>(scrollable)->ClearEntries();

	std::vector<GUI_Element*> texts;
	if (gold > 0)
	{
		std::string goldInfo = Stringf("gold x%d", gold);
		GUI_Text* goldText = new GUI_Text();
		goldText->SetCanvas(m_canvas);
		goldText->SetAnchorType(GUI_AnchorType::NONE);
		goldText->SetText(goldInfo);
		goldText->SetHasDropShadow(true);
		goldText->SetFont(font);
		goldText->SetTextHeight(24.f);
		goldText->SetTextColor(Rgba8(200, 200, 200, 255));
		goldText->SetTextAlignment(Vec2(0.f, 0.5f));
		goldText->SetCanHover(false);
		texts.push_back(goldText);
	}

	for (auto count : counts)
	{
		std::string info = Stringf("%s x%d", count.first.c_str(), count.second);
		GUI_Text* text = new GUI_Text();
		text->SetCanvas(m_canvas);
		text->SetAnchorType(GUI_AnchorType::NONE);
		text->SetText(info);
		text->SetHasDropShadow(true);
		text->SetFont(font);
		text->SetTextHeight(24.f);
		text->SetTextColor(Rgba8(200, 200, 200, 255));
		text->SetTextAlignment(Vec2(0.f, 0.5f));
		text->SetCanHover(false);
		texts.push_back(text);
	}

	static_cast<GUI_Scrollable*>(scrollable)->SetEntries(texts);

	return false;
}


