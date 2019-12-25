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
	// ���������Ϣ
	void OutputErrorInfos(const std::string& content, const std::vector<ErrorInfo>& errorInfos);
	// ������ż�
	void OutputSymbols(const std::vector<Symbol>& symbols);
	// ���������Ϣ
	void OutputProgramInfo(const ProgramInfo& programInfo);
}