#include "StdAfx.h"
#include "JsonNode.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"

//////////////////////////////////////////////////////////////////////////
// JsonObjectNode
//////////////////////////////////////////////////////////////////////////

JsonObjectNode::JsonObjectNode() 
	: m_jsonObj(new json::Object())
{
}

//////////////////////////////////////////////////////////////////////////

JsonObjectNode::JsonObjectNode( const json::Object& obj )
	: m_jsonObj(new json::Object())
{
	(*m_jsonObj) = obj;
}

//////////////////////////////////////////////////////////////////////////

JsonObjectNode::JsonObjectNode( const JsonObjectNode& other )
{
	(*m_jsonObj) = (*other.m_jsonObj);
}
//////////////////////////////////////////////////////////////////////////

JsonObjectNode::~JsonObjectNode()
{
	if (NULL != m_jsonObj)
	{
		delete m_jsonObj;
		m_jsonObj = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

void JsonObjectNode::AddImpl( const char* key, const tstring& value )
{
	(*m_jsonObj)[key] = json::String(value);
}

//////////////////////////////////////////////////////////////////////////

tstring JsonObjectNode::GetValueImpl( const char* key ) const
{
	json::String& value = (*m_jsonObj)[key];
	return value;
}
//////////////////////////////////////////////////////////////////////////

json::Object* JsonObjectNode::GetRawNode() const
{
	return m_jsonObj;
}

//////////////////////////////////////////////////////////////////////////

void JsonObjectNode::AppendChild( const char* key, JsonObjectNode* child )
{
	(*m_jsonObj)[key] = (*child->GetRawNode());
}

//////////////////////////////////////////////////////////////////////////

JsonObjectNode JsonObjectNode::GetChileNode( const char* key ) const
{
	return JsonObjectNode((*m_jsonObj)[key]);
}

//////////////////////////////////////////////////////////////////////////

tstring JsonObjectNode::ToString() const
{
	std::stringstream stream;
	json::Writer::Write(*m_jsonObj, stream);
	return stream.str();
}

//////////////////////////////////////////////////////////////////////////

unsigned int JsonObjectNode::Parse( const char* content, size_t len )
{
	std::stringstream stream;
	stream.str(tstring(content, len));
	try
	{
		json::Reader::Read(*m_jsonObj, stream);
	}
	catch (const json::Exception& )
	{
		return 0;
	}

	return ToString().length();
}

//////////////////////////////////////////////////////////////////////////

unsigned int JsonObjectNode::Parse(const tstring& content)
{
	return Parse(content.c_str(), content.length());
}

//////////////////////////////////////////////////////////////////////////

bool JsonObjectNode::operator == (const JsonObjectNode& object) const 
{
	return (*m_jsonObj) == (*object.m_jsonObj);
}


//////////////////////////////////////////////////////////////////////////
// JsonArrayNode
//////////////////////////////////////////////////////////////////////////

JsonArrayNode::JsonArrayNode()
	: m_jsonArray(new json::Array())
{
}

//////////////////////////////////////////////////////////////////////////

JsonArrayNode::~JsonArrayNode()
{
	if (NULL != m_jsonArray)
	{
		delete m_jsonArray;
		m_jsonArray = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

void JsonArrayNode::Insert( JsonObjectNode& node )
{
	m_jsonArray->Insert(*node.GetRawNode());
}

//////////////////////////////////////////////////////////////////////////

size_t JsonArrayNode::Size() const
{
	return m_jsonArray->Size();
}

//////////////////////////////////////////////////////////////////////////

void JsonArrayNode::Clear()
{
	m_jsonArray->Clear();
}

//////////////////////////////////////////////////////////////////////////

JsonObjectNode JsonArrayNode::GetObjectNode( int index )
{
	json::Array::const_iterator curIter(m_jsonArray->Begin());
	for(int i=0;i<index; ++i)
		++curIter;
	const json::Object& returnObj = *curIter;

	return JsonObjectNode(returnObj);
}

//////////////////////////////////////////////////////////////////////////

tstring JsonArrayNode::ToString() const
{
	std::stringstream stream;
	json::Writer::Write(*m_jsonArray, stream);
	return stream.str();
}

//////////////////////////////////////////////////////////////////////////

void JsonArrayNode::Parse( const char* content, size_t len )
{
	std::stringstream stream;
	stream.str(tstring(content, len));
	json::Reader::Read(*m_jsonArray, stream);
}

//////////////////////////////////////////////////////////////////////////

void JsonArrayNode::Parse( const tstring& content )
{
	Parse(content.c_str(), content.length());
}

//////////////////////////////////////////////////////////////////////////

bool JsonArrayNode::operator == (const JsonArrayNode& object) const 
{
	return (*m_jsonArray) == (*object.m_jsonArray);
}

//////////////////////////////////////////////////////////////////////////

