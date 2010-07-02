#pragma once

#include "DataTypeConvertor.h"

namespace json
{
	class Object;
	class Array;
}

class JsonObjectNode
{
public:
	JsonObjectNode();
	JsonObjectNode(const json::Object& obj);
	JsonObjectNode(const JsonObjectNode& other);
	~JsonObjectNode();

public:
	template<typename T>
	void Add(const char* key, T paramValue)
	{
		const int MaxStrLen = 128;
		char strValue[MaxStrLen] = {0,};
		Datatype::ToString(strValue, paramValue, MaxStrLen);
		AddImpl(key, strValue);
	}

	template<>
	void Add(const char* key, JsonObjectNode* node) 
	{ 
		AppendChild(key, node);
	}

	template<typename T>
	T		GetValue(const char* key) const
	{
		tstring result = GetValueImpl(key);
		T returnValue;
		Datatype::ToOwnValue(result.c_str(), returnValue);
		return returnValue;
	}

	template<>
	JsonObjectNode GetValue(const char* key) const
	{
		return GetChileNode(key);
	}

	tstring ToString();
	void Parse(const char* content, size_t len);
	void Parse(const tstring& content);

	bool operator == (const JsonObjectNode& object) const;
	
protected:
	void AddImpl(const char* key, const tstring& value);
	tstring GetValueImpl(const char* key) const;
	void AppendChild(const char* key, JsonObjectNode* child);
	JsonObjectNode GetChileNode(const char* key) const;

	json::Object* GetRawNode();

private:
	json::Object* m_jsonObj;
	
	friend class JsonArrayNode;
};

//////////////////////////////////////////////////////////////////////////

class JsonArrayNode
{
public:
	JsonArrayNode();
	virtual ~JsonArrayNode();

public:
	void Insert(JsonObjectNode& node);
	size_t Size();
	void Clear();
	JsonObjectNode GetObjectNode(int index);

	tstring ToString();
	void Parse(const char* content, size_t len);
	void Parse(const tstring& content);

	bool operator == (const JsonArrayNode& object) const;

private:
	json::Array* m_jsonArray;
};