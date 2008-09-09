#ifndef _HIPPOLIB_HTTPHEADER_H
#define _HIPPOLIB_HTTPHEADER_H

#include <stdexcept>
#include <boost/algorithm/string.hpp>

namespace hippolib
{
	class httpheader
	{
	public:
		httpheader() : m_name(""), m_value("")
		{
		}

		httpheader(const std::string & name, const std::string & value)
			: m_name(name), m_value(value)
		{
		}

		httpheader(const httpheader & header)
			: m_name(header.m_name), m_value(header.m_value)
		{
		}

		httpheader& operator=(const httpheader & header)
		{
			if(this != &header)
			{
				m_name = header.m_name;
				m_value = header.m_value;
			}
			return *this;
		}

		std::string getName() const
		{
			return m_name;
		}

		void setName(const std::string & name)
		{
			m_name = name;
		}

		std::string getValue() const
		{
			return m_value;
		}

		void setValue(const std::string & value)
		{
			m_value = value;
		}

		std::string getString() const
		{
			std::string ret = m_name + ": " + m_value;
			return ret;
		}

		static httpheader parseString(const std::string & str)
		{
			std::string key, value;
			std::string::size_type index = str.find_first_of(':');

			if(index == std::string::npos)
				throw std::invalid_argument(str + " - No colon");
			if(index+1 >= str.size())
				throw std::invalid_argument(str + " - Format error");

			key = str.substr(0, index);
			if(index+2 < str.size())
				value = str.substr(index+2);
			else
				value = "";

			if(value.size() > 0 && value.at(value.size()-1) == '\r')
				value = value.substr(0, value.size()-1);

			return httpheader(key, value);
		}
	private:
		std::string m_name;
		std::string m_value;
	};

	class HeaderKeyComparer
	{
	public:
		bool operator()(std::string s1, std::string s2) const
		{
			return boost::algorithm::ilexicographical_compare(s1, s2);
		}
	};
}

#endif

