#include "PL0.h"

static void OutputInstruction(uint32_t address, const PL0::Instruction& instruction);

void PL0::OutputErrorInfos(const std::string& content, const std::vector<ErrorInfo>& errorInfos)
{
	CodeCoord currCoord = { 1, 0 };
	const char* cstr = content.c_str();
	const char* lineEndStr;
	std::string str, spaces;

	if (!errorInfos.empty())
	{
		for (const auto& errorInfo : errorInfos)
		{
			spaces.clear();

			// 跳到错误所在行
			while (currCoord.row < errorInfo.beg.row)
			{
				if (*cstr == '\0')
					break;
				else if (*cstr == '\n')
				{
					++currCoord.row;
					currCoord.col = 1;
				}
				else
				{
					++currCoord.col;
				}
				++cstr;
			}

			// 遍历当前行
			lineEndStr = cstr;
			while (*lineEndStr && *lineEndStr != '\n')
			{
				++lineEndStr;
			}

			// 找出指示起点
			for (size_t i = 0; i < errorInfo.beg.col - 1; ++i)
			{
				if (cstr[i] == '\t')
					spaces += '\t';
				else
					spaces += ' ';
			}

			// 行内容
			str = std::string(cstr, lineEndStr);

			// 输出错误信息
			printf_s("(r%d:c%d) %s\n", errorInfo.beg.row, errorInfo.beg.col,
				g_ErrorMsgs[errorInfo.errorCode]);

			printf_s("%4d|%s\n     %s",errorInfo.beg.row, str.c_str(), spaces.c_str());

			int markTimes;
			if (errorInfo.end.row == errorInfo.beg.row)
				markTimes = errorInfo.end.col - errorInfo.beg.col;
			else
				markTimes = (int)str.size() - errorInfo.beg.col + 1;
			while (markTimes--)
				putchar('^');
			puts("");
		}
	}
		
}

void PL0::OutputSymbols(const std::vector<Symbol>& symbols)
{
	puts("词语集:");
	for (const auto& symbol : symbols)
	{
		printf("%s ▏", symbol.word.c_str());
	}
	puts("");
}

void PL0::OutputProgramInfo(const ProgramInfo& programInfo)
{
	puts("符号表:");
	printf("%-30.28s%-18s%-12s%-10s%-10s\n", "symbol name", "type", "value", "level", "offset");
	for (const auto& id : programInfo.identifiers)
	{
		std::string kind;
		if (id.kind & ID_PARAMETER)
			kind += "prarm ";
		if (id.kind & ID_CONST)
			kind += "const ";
		if (id.kind & ID_VAR)
			kind += "var ";
		if (id.kind & ID_PROCEDURE)
			kind += "proc";
		if (id.kind & ID_INT)
			kind += "int";

		printf("%-30.28s%-18s%-12d%-10d%-10d\n", id.name, kind.c_str(), id.value, id.level, id.offset);
	}

	puts("\n所有指令:");
	printf("%-8s%-8s%-8s%-8s\n", "address", "func", "level", "mix");
	std::string func, mix;


	uint32_t instCount = programInfo.instructions.size();
	for (uint32_t i = 0; i < instCount; ++i)
	{
		OutputInstruction(i, programInfo.instructions[i]);
	}
}

void OutputInstruction(uint32_t address, const PL0::Instruction& instruction)
{
	using namespace PL0;

	std::string func, mix;

	switch (instruction.func)
	{
	case Func_LIT: func = "LIT"; break;
	case Func_OPR: func = "OPR"; break;
	case Func_LOD: func = "LOD"; break;
	case Func_STO: func = "STO"; break;
	case Func_CAL: func = "CAL"; break;
	case Func_INT: func = "INT"; break;
	case Func_POP: func = "POP"; break;
	case Func_JMP: func = "JMP"; break;
	case Func_JPC: func = "JPC"; break;
	}

	if (instruction.func == Func_OPR)
	{
		switch (instruction.mix)
		{
		case Opr_RET: mix = "RET"; break;
		case Opr_NEG: mix = "NEG"; break;
		case Opr_NOT: mix = "NOT"; break;
		case Opr_ADD: mix = "ADD"; break;
		case Opr_SUB: mix = "SUB"; break;
		case Opr_MUL: mix = "MUL"; break;
		case Opr_DIV: mix = "DIV"; break;
		case Opr_MOD: mix = "MOD"; break;
		case Opr_EQU: mix = "EQU"; break;
		case Opr_NEQ: mix = "NEQ"; break;
		case Opr_LES: mix = "LES"; break;
		case Opr_LEQ: mix = "LEQ"; break;
		case Opr_GTR: mix = "GTR"; break;
		case Opr_GEQ: mix = "GEQ"; break;
		case Opr_PRT: mix = "PRT"; break;
		case Opr_PNL: mix = "PNL"; break;
		case Opr_SCN: mix = "SCN"; break;
		}
	}
	else
	{
		mix = std::to_string(instruction.mix);
	}

	printf("%04u    %-8s%-8d%-8s\n", address, func.c_str(), instruction.level, mix.c_str());
}
