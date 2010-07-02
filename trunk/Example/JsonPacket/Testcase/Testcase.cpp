// Testcase.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../JsonPacketLib/JsonNode.h"
#include "assert.h"

class CTestFunctionPrint
{
public:
	CTestFunctionPrint(const char* functionname) : m_functionname(functionname)
	{
		printf("Begin %s\n", m_functionname);
	}

	~CTestFunctionPrint()
	{
		printf("Finsh %s\n", m_functionname);
	}

private:
	const char* m_functionname;
};

void TestNode()
{
	CTestFunctionPrint autoPrint(__FUNCTION__);

	tstring TestName = "alice";
	JsonObjectNode node;
	node.Add("StrValue", TestName);
	tstring name0 = node.GetValue<tstring>("StrValue");
	assert(TestName == name0);

	int TestInteger = 23498;
	node.Add("Integer", TestInteger);
	int number0 = node.GetValue<int>("Integer");
	assert(number0 == TestInteger);
	tstring name1 = node.GetValue<tstring>("StrValue");
	assert(TestName == name1);

	int TestNeInteger = -23498;
	node.Add("SignedInteger", TestNeInteger);
	int number1 = node.GetValue<int>("SignedInteger");
	assert(number1 == TestNeInteger);

	unsigned int TestUinteger = 4294967295;
	node.Add("UInteger", TestUinteger);
	unsigned int uInt0 = node.GetValue<unsigned int>("UInteger");
	assert(uInt0 == TestUinteger);

	float TestFloat = 2113432.5646f;
	node.Add("FloatValue", TestFloat);
	float floatResult = node.GetValue<float>("FloatValue");
	assert(floatResult == TestFloat);

	tstring content = node.ToString();
	JsonObjectNode node1;
	node1.Parse(content);
	unsigned int uInt1 = node1.GetValue<unsigned int>("UInteger");
	assert(uInt1 == TestUinteger);
}

void TestArrayNode()
{
	CTestFunctionPrint autoPrint(__FUNCTION__);
	const int ElementCount = 2;
	tstring TestName[ElementCount] = {"alice", "knight"};
	int TestLevel[ElementCount] = {22, 43};
	float TestExp[ElementCount] = {232254.4456f, 7346565.443f};

	JsonArrayNode arrayNode;
	for (int i=0; i<ElementCount; ++i)
	{
		JsonObjectNode objectNode;
		objectNode.Add("name", TestName[i]);
		objectNode.Add("level", TestLevel[i]);
		objectNode.Add("exp", TestExp[i]);

		arrayNode.Insert(objectNode);
	}
	assert(ElementCount == arrayNode.Size());
	for (int i=0; i<ElementCount; ++i)
	{
		JsonObjectNode objectNode = arrayNode.GetObjectNode(i);
		tstring name = objectNode.GetValue<tstring>("name");
		int level = objectNode.GetValue<int>("level");
		float exp = objectNode.GetValue<float>("exp");
		assert(name == TestName[i]);
		assert(level == TestLevel[i]);
		assert(exp == TestExp[i]);
	}
	tstring content = arrayNode.ToString();
	JsonArrayNode arrayNode1;
	arrayNode1.Parse(content);

	assert(arrayNode == arrayNode1);
}

void TestChildNode()
{
	CTestFunctionPrint autoPrint(__FUNCTION__);
	tstring TestName = "alice";
	JsonObjectNode childNode;
	childNode.Add("name", TestName);
	int TestLevel = 57;
	childNode.Add("level", TestLevel);

	JsonObjectNode parentNode;
	parentNode.Add("npc", &childNode);

	JsonObjectNode chileNode1 = parentNode.GetValue<JsonObjectNode>("npc");
	tstring name = chileNode1.GetValue<tstring>("name");
	assert(name == TestName);
	int level = chileNode1.GetValue<int>("level");
	assert(level == TestLevel);
	assert(childNode == chileNode1);
}

void TestMakeBuffer()
{
	CTestFunctionPrint autoPrint(__FUNCTION__);
	tstring TestName = "alice";
	JsonObjectNode node;
	node.Add("StrValue", TestName);
	int TestInteger = 23498;
	node.Add("Integer", TestInteger);
	int TestNeInteger = -23498;
	node.Add("SignedInteger", TestNeInteger);
	unsigned int TestUinteger = 4294967295;
	node.Add("UInteger", TestUinteger);
	float TestFloat = 2113432.5646f;
	node.Add("FloatValue", TestFloat);

	const int BufferSize = 1024;
	char buffer[BufferSize] = {0,};
	unsigned int writtenSize = node.MakeBuffer(buffer, BufferSize);

	JsonObjectNode dstNode;
	dstNode.Parse(buffer, writtenSize);
	assert(node == dstNode);
}


int _tmain(int argc, _TCHAR* argv[])
{
	TestNode();
	TestArrayNode();
	TestChildNode();
	TestMakeBuffer();
	printf("\nTest success.");
	getchar();
	return 0;
}

