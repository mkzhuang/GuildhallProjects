#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventHandler.hpp"

class InputSystem;

using namespace std;

typedef bool (*EventCallbackFunction)(EventArgs& args);

template <class T> using ObjectMemberFunc = bool (T::*)(EventArgs& args);

struct CaseInsensitiveComparator
{
	struct CaseInsensitiveCharCompare
	{
		inline bool operator()(unsigned char const& a, unsigned char const& b) const
		{
			return std::tolower(a) < std::tolower(b);
		}
	};

	inline bool operator()(std::string const& a, std::string const& b) const
	{
		bool result = std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), CaseInsensitiveCharCompare());
		return result;
	}
};

struct EventSubscription
{
	EventCallbackFunction m_function;
};

struct HandlerSubscription
{	
public:
	inline HandlerSubscription(EventHandlerBase* _handler, EventArgs& _args)
		: handler(_handler)
		, args(_args)
	{
	}

public:
	EventHandlerBase* handler = nullptr;
	EventArgs args;
};

typedef std::vector<EventSubscription> SubscriptionList;

typedef std::vector<HandlerSubscription> EventHandleList;

struct EventSystemConfig
{
	InputSystem* input = nullptr;
};

class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config);
	~EventSystem();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	// event function
	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void FireEvent(std::string const& eventName, EventArgs& args);
	void FireEvent(std::string const& eventName);
	void GetRegisteredEventNames(std::vector<std::string>& outNames) const;

	//custom event handle
	template <class T>
	inline void RegisterEventHandler(std::string const& handleName, T* obj, ObjectMemberFunc<T> funcPtr)
	{
		EventArgs args;
		RegisterEventHandler(handleName, obj, funcPtr, args);

	}

	template <class T>
	inline void RegisterEventHandler(std::string const& handleName, T* obj, ObjectMemberFunc<T> funcPtr, EventArgs& args)
	{
		EventHandlerBase* eventHandler = new EventHandler<T>(obj, funcPtr);

		EventHandleList& list = m_handleList[handleName];
		list.emplace_back(eventHandler, args);
	}

	void CallHandler(std::string const& handleName);
	void CallHandler(std::string const& handleName, EventArgs& args);
	void UnregisterEventHandler(std::string const& handleName);
	void UnregisterAllEventsForObject(void* obj);
	template <class T>
	inline void UnregisterEventForObject(void* obj, ObjectMemberFunc<T> funcPtr)
	{
		for (auto& pair : m_handleList)
		{
			EventHandleList& list = pair.second;
			for (auto iter = list.begin(); iter != list.end();)
			{
				auto& item = *iter;
				EventHandler<T>* handler = (EventHandler<T>*)item.handler;
				if (handler && handler->GetHandler() == obj && handler->GetFunction() == funcPtr)
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
	void GetRegisterHandlerNames(std::vector<std::string>& outNames) const;

protected:
	EventSystemConfig m_config;
	std::map<std::string, SubscriptionList, CaseInsensitiveComparator> m_subscriptionListsByEventName;

	std::map<std::string, EventHandleList, CaseInsensitiveComparator> m_handleList;
};


void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void FireEvent(std::string const& eventName, EventArgs& args);
void FireEvent(std::string const& eventName);

template <class T>
inline void RegisterEventHandler(std::string const& handleName, T* obj, ObjectMemberFunc<T> funcPtr)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->RegisterEventHandler(handleName, obj, funcPtr);
}

template <class T>
inline void RegisterEventHandler(std::string const& handleName, T* obj, ObjectMemberFunc<T> funcPtr, EventArgs& args)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->RegisterEventHandler(handleName, obj, funcPtr, args);
}

void CallHandler(std::string const& handleName);
void CallHandler(std::string const& handleName, EventArgs& args);
void UnregisterEventHandler(std::string const& handleName);
void UnregisterAllEventsForObject(void* obj);
template <class T>
inline void UnregisterEventForObject(void* obj, ObjectMemberFunc<T> funcPtr)
{
	if (!g_theEventSystem)
	{
		return;
	}
	g_theEventSystem->UnregisterEventForObject(obj, funcPtr);
}


class EventRecipient
{
public:
	EventRecipient() {}
	virtual ~EventRecipient()
	{
		UnregisterAllEventsForObject(this);
	}

	EventRecipient(EventRecipient const& copy) = delete;
	EventRecipient(EventRecipient&& copy) = delete;

	void operator=(EventRecipient const& copy) = delete;
	void operator=(EventRecipient&& copy) = delete;

	template <class T>
	inline void RegisterHandle(std::string const& handleName, ObjectMemberFunc<T> func)
	{
		RegisterEventHandler(handleName, (T*)(void*)this, func);
	}
};


