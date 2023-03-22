#include "Engine/Core/EventSystem.hpp"

EventSystem* g_theEventSystem;

EventSystem::EventSystem(EventSystemConfig const& config)
	:m_config(config)
{
}

EventSystem::~EventSystem()
{
}


void EventSystem::Startup()
{

}


void EventSystem::BeginFrame()
{

}


void EventSystem::EndFrame()
{

}


void EventSystem::ShutDown()
{

}


void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	EventSubscription subscription;
	subscription.m_function = functionPtr;
	std::map<std::string, SubscriptionList>::iterator iter = m_subscriptionListsByEventName.find(eventName);

	if (iter == m_subscriptionListsByEventName.end()) 
	{
		SubscriptionList list;
		list.push_back(subscription);
		m_subscriptionListsByEventName[eventName] = list;
	}
	else
	{
		SubscriptionList& list = iter->second;
		for (int subscriber = 0; subscriber < int(list.size()); subscriber++)
		{
			if (list[subscriber].m_function == functionPtr)
			{
				return;
			}
		}
		list.push_back(subscription);
	}
}


void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	std::map<std::string, SubscriptionList>::iterator iter = m_subscriptionListsByEventName.find(eventName);

	if (iter == m_subscriptionListsByEventName.end())
	{
		//ERROR_AND_DIE("No such event exist");
	}
	else
	{
		SubscriptionList& list = iter->second;
		for (int subscriber = 0; subscriber < int(list.size()); subscriber++)
		{
			if (list[subscriber].m_function == functionPtr)
			{
				list.erase(list.begin() + subscriber);
				//list[subscriber].m_function = nullptr;
				return;
			}
		}
	}

	//ERROR_AND_DIE("No such function pointer");
}


void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	std::map<std::string, SubscriptionList>::iterator iter = m_subscriptionListsByEventName.find(eventName);

	if (iter == m_subscriptionListsByEventName.end())
	{
		//ERROR_AND_DIE("No such event exist");
	}
	else
	{
		SubscriptionList& list = iter->second;
		for (int subscriber = 0; subscriber < int(list.size()); subscriber++)
		{
			list[subscriber].m_function(args);
		}
	}
}


void EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs args;
	FireEvent(eventName, args);
}


void EventSystem::GetRegisteredEventNames(std::vector<std::string>& outNames) const
{
	for (std::map<std::string, SubscriptionList>::const_iterator iter = m_subscriptionListsByEventName.begin(); iter != m_subscriptionListsByEventName.end(); iter++)
	{
		SubscriptionList const& list = iter->second;
		if (list.size() > 0)
		{
			outNames.push_back(iter->first);
		}
	}
}


void EventSystem::CallHandler(std::string const& handleName)
{
	std::map<std::string, EventHandleList>::iterator iter = m_handleList.find(handleName);
	if (iter == m_handleList.end())
	{
		//ERROR_AND_DIE("No such event exist");
	}
	else
	{
		EventHandleList& list = iter->second;
		for (int subscriber = 0; subscriber < int(list.size()); subscriber++)
		{
			list[subscriber].handler->CallFunction(list[subscriber].args);
		}
	}
}


void EventSystem::CallHandler(std::string const& handleName, EventArgs& args)
{
	std::map<std::string, EventHandleList>::iterator iter = m_handleList.find(handleName);
	if (iter == m_handleList.end())
	{
		//ERROR_AND_DIE("No such event exist");
	}
	else
	{
		EventHandleList& list = iter->second;
		for (int subscriber = 0; subscriber < int(list.size()); subscriber++)
		{
			list[subscriber].handler->CallFunction(args);
		}
	}
}


void EventSystem::UnregisterEventHandler(std::string const& handleName)
{
	std::map<std::string, EventHandleList>::iterator iter = m_handleList.find(handleName);
	if (iter == m_handleList.end())
	{
		//ERROR_AND_DIE("No such event exist");
	}
	else
	{
		EventHandleList& list = iter->second;
		for (auto itemIter = list.begin(); itemIter != list.end();)
		{
			auto& item = *itemIter;
			delete item.handler;
			item.handler = nullptr;
			itemIter = list.erase(itemIter);
		}
		m_handleList.erase(handleName);
	}
}


void EventSystem::UnregisterAllEventsForObject(void* obj)
{
	for (auto& pair : m_handleList)
	{
		EventHandleList& list = pair.second;
		for (auto iter = list.begin(); iter != list.end();)
		{
			auto& item = *iter;
			if (item.handler && item.handler->GetHandler() == obj)
			{
				delete item.handler;
				item.handler = nullptr;
				iter = list.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
}


void EventSystem::GetRegisterHandlerNames(std::vector<std::string>& outNames) const
{
	for (std::map<std::string, EventHandleList>::const_iterator iter = m_handleList.begin(); iter != m_handleList.end(); iter++)
	{
		EventHandleList const& list = iter->second;
		if (list.size() > 0)
		{
			outNames.push_back(iter->first);
		}
	}
}


void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->SubscribeEventCallbackFunction(eventName, functionPtr);
}


void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, functionPtr);
}


void FireEvent(std::string const& eventName, EventArgs& args)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->FireEvent(eventName, args);
}


void FireEvent(std::string const& eventName)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->FireEvent(eventName);
}


void CallHandler(std::string const& handleName)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->CallHandler(handleName);
}


void CallHandler(std::string const& handleName, EventArgs& args)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->CallHandler(handleName, args);
}


void UnregisterEventHandler(std::string const& handleName)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->UnregisterEventHandler(handleName);
}


void UnregisterAllEventsForObject(void* obj)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->UnregisterAllEventsForObject(obj);
}


