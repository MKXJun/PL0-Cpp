#include "PL0.h"
#include "Console.h"
#include <iostream>
#include <conio.h>
int main()
{
	//
	// 初始化控制台设定
	//
	Console console;
	Console::SetBufferSize(100, 9999);
	Console::SetWindowSize(100, 40);

	Console::WriteLine("PL/0编译程序 作者: X_Jun(MKXJun) 版本: V1.2");
	Console::WriteLine("请在外部写好代码后直接复制粘贴到此处，然后回车按ctrl+z再回车结束。");

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
	// 词法分析部分
	//
	PL0::WordParser wordParser;
	if (!wordParser.Parse(content))
	{
		PL0::OutputErrorInfos(content, wordParser.GetErrorInfos());
		system("pause");
		return 0;
	}
	Console::WriteLine("【词法分析已通过！】");
	PL0::OutputSymbols(wordParser.GetSymbols());
	Console::WriteLine("===========================================");
	
	//
	// 语法/语义分析部分
	//
	PL0::ProgramParser programParser;
	if (!programParser.Parse(wordParser.GetSymbols()))
	{
		PL0::OutputErrorInfos(content, programParser.GetErrorInfos());
		system("pause");
		return 0;
	}
	Console::WriteLine("【语法分析已通过！】");
	PL0::OutputProgramInfo(programParser.GetProgramInfo());
	Console::WriteLine("===========================================");
	Console::WriteLine("按键选择运行模式(1 正常运行|2 单步调试)");
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
	// 虚拟机部分
	//
	PL0::VirtualMachine virtualMachine;
	virtualMachine.Initialize(programParser.GetProgramInfo());
	

	if (mode == 1)
	{
		virtualMachine.Run();
	}
	else
	{
		Console::WriteLine("你已进入单步调试模式(回车 逐指令|空格 逐过程|Esc 跳出)");
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