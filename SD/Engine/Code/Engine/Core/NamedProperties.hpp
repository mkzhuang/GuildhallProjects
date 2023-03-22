#pragma once
#include "Engine/Core/StringUtils.hpp"

#include <string>
#include <map>

class NamedPropertyBase
{
	friend class NamedProperties;

protected:
	NamedPropertyBase() {}
	virtual ~NamedPropertyBase() {}

	virtual NamedPropertyBase* Clone() const = 0;
};


template <typename T>
class NamedProperty : public NamedPropertyBase
{
	friend class NamedProperties;

	NamedProperty(T value)
		: m_value(value)
	{
	}
	NamedProperty(NamedProperty<T> const& prop)
	{
		m_value = prop.m_value;
	}

	virtual ~NamedProperty() {}

	virtual NamedPropertyBase* Clone() const override
	{
		return new NamedProperty<T>(*this);
	}

	T m_value;
};


class NamedProperties
{
public:
	NamedProperties() {}
	~NamedProperties()
	{
		for (auto item : m_properties)
		{
			delete item.second;
		}
		m_properties.clear();
	}

	
	NamedProperties(NamedProperties const& properties)
	{
		for (auto p : properties.m_properties)
		{
			m_properties[p.first] = p.second->Clone();
		}
	}

	NamedProperties(NamedProperties&& properties) noexcept
	{
		m_properties = std::move(properties.m_properties);
	}

	void operator=(NamedProperties const& properties)
	{
		for (auto item : m_properties)
		{
			delete item.second;
		}
		m_properties.clear();

		for (auto p : properties.m_properties)
		{
			m_properties[p.first] = p.second->Clone();
		}
	}

	void operator=(NamedProperties&& properties) noexcept
	{
		for (auto item : m_properties)
		{
			delete item.second;
		}
		m_properties.clear();

		m_properties = std::move(properties.m_properties);
	}

	template <typename T>
	inline void SetValue(std::string const& key, T value)
	{
		NamedPropertyBase* namedProp = new NamedProperty<T>(value);
		std::map<std::string, NamedPropertyBase*>::iterator iter = m_properties.find(key);

		if (iter == m_properties.end())
		{
			m_properties[key] = namedProp;
			return;
		}

		// static is faster, but dynamic ensure types are same
		NamedProperty<T>* prop = dynamic_cast<NamedProperty<T>*>(iter->second);
		if (prop)
		{
			iter->second = namedProp;
			return;
		}

		m_properties.erase(iter);
		m_properties[key] = namedProp;
	}

	inline void SetValue(std::string const& key, char const* value)
	{
		std::string const& string = Stringf(value);
		SetValue(key, string);
	}

	template <typename T>
	inline T GetValue(std::string const& key, T defaultValue) const
	{
		std::map<std::string, NamedPropertyBase*>::const_iterator iter = m_properties.find(key);
		if (iter == m_properties.end())
		{
			return defaultValue;
		}

		// static is faster, but dynamic ensure types are same
		NamedProperty<T>* prop = dynamic_cast<NamedProperty<T>*>(iter->second);
		if (prop)
		{
			return prop->m_value;
		}

		return defaultValue;
	}

	inline std::string GetValue(std::string const& key, char const* defaultValue) const
	{
		std::string const& string = Stringf(defaultValue);
		return GetValue(key, string);
	}

public:
	std::map<std::string, NamedPropertyBase*> m_properties;
};


