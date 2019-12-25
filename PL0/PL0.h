#pragma once

#include "PL0_WordParser.h"
#include "PL0_ProgramParser.h"
#include "PL0_VirtualMachine.h"

#include "PL0_ErrorMsg.h"

#ifdef _WIN64
#error "x64 is unsupported! Switch back to x86."
#endif

namespace PL0
{
	// 输出错误信息
	void OutputErrorInfos(const std::string& content, const std::vector<ErrorInfo>& errorInfos);
	// 输出符号集
	void OutputSymbols(const std::vector<Symbol>& symbols);
	// 输出程序信息
	void OutputProgramInfo(const ProgramInfo& programInfo);
}