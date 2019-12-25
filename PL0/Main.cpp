#include "PL0.h"
#include "Console.h"
#include <iostream>
#include <conio.h>
int main()
{
	//
	// ��ʼ������̨�趨
	//
	Console console;
	Console::SetBufferSize(100, 9999);
	Console::SetWindowSize(100, 40);

	Console::WriteLine("PL/0������� ����: X_Jun(MKXJun) �汾: V1.2");
	Console::WriteLine("�����ⲿд�ô����ֱ�Ӹ���ճ�����˴���Ȼ��س���ctrl+z�ٻس�������");

	std::string content;
	std::string str;
	int line = 1;
	Console::WriteFormat("%4d|", line);
	while (std::getline(std::cin, str))
	{
		content += str;
		content += '\n';
		Console::WriteFormat("%4d|", ++line);
		continue;
	}
	Console::WriteLine("===========================================");

	//
	// �ʷ���������
	//
	PL0::WordParser wordParser;
	if (!wordParser.Parse(content))
	{
		PL0::OutputErrorInfos(content, wordParser.GetErrorInfos());
		system("pause");
		return 0;
	}
	Console::WriteLine("���ʷ�������ͨ������");
	PL0::OutputSymbols(wordParser.GetSymbols());
	Console::WriteLine("===========================================");
	
	//
	// �﷨/�����������
	//
	PL0::ProgramParser programParser;
	if (!programParser.Parse(wordParser.GetSymbols()))
	{
		PL0::OutputErrorInfos(content, programParser.GetErrorInfos());
		system("pause");
		return 0;
	}
	Console::WriteLine("���﷨������ͨ������");
	PL0::OutputProgramInfo(programParser.GetProgramInfo());
	Console::WriteLine("===========================================");
	Console::WriteLine("����ѡ������ģʽ(1 ��������|2 ��������)");
	int mode;
	{
		char ch = '\0';
		while (ch != '1' && ch != '2')
			ch = _getch();
		if (ch == '1')
			mode = 1;
		else if (ch == '2')
			mode = 2;
	}

	Console::WriteLine("===========================================");

	//
	// ���������
	//
	PL0::VirtualMachine virtualMachine;
	virtualMachine.Initialize(programParser.GetProgramInfo());
	

	if (mode == 1)
	{
		virtualMachine.Run();
	}
	else
	{
		Console::WriteLine("���ѽ��뵥������ģʽ(�س� ��ָ��|�ո� �����|Esc ����)");
		while (!virtualMachine.IsFinished())
		{
			virtualMachine.PrintRegisterInfo();
			char ch = '\0';
			while (ch != '\r' && ch != 27 && ch != ' ')
				ch = _getch();
			if (ch == '\r')
				virtualMachine.RunNextInstruction();
			else if (ch == ' ')
				virtualMachine.RunProcedure();
			else if (ch == 27)
				virtualMachine.RunToReturn();
		}
	}

	system("pause");

	return 0;
}