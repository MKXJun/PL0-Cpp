#include "PL0_VirtualMachine.h"

namespace PL0
{
	void VirtualMachine::Initialize(const ProgramInfo& programInfo)
	{
		m_ProgramInfo = programInfo;

		EnvironmentInfo& ri = m_EnvironmentInfo;
		ri.pr = ri.br = 0;
		ri.tr = -1;
		ri.ir = m_ProgramInfo.instructions.front();
		ri.dataStack.clear();
		while (!ri.prStack.empty())
			ri.prStack.pop();
		while (!ri.trStack.empty())
			ri.trStack.pop();
		while (!ri.brStack.empty())
			ri.brStack.pop();
		ri.funcStack.clear();
		ri.funcStack.push_back("__main__");
	}

	void VirtualMachine::Run()
	{
		while (!m_EnvironmentInfo.funcStack.empty())
		{
			RunNextInstruction();
		}
			
	}

	void VirtualMachine::RunNextInstruction()
	{
		EnvironmentInfo& eInfo = m_EnvironmentInfo;
		bool getNextInstruction = true;
		// 取立即数
		if (eInfo.ir.func == Func_LIT)
		{
			if (eInfo.tr == eInfo.dataStack.size() - 1)
			{
				eInfo.dataStack.push_back(eInfo.ir.mix);
				++eInfo.tr;
			}
			else
			{
				eInfo.dataStack[++eInfo.tr] = eInfo.ir.mix;
			}
		}
		// 操作
		else if (eInfo.ir.func == Func_OPR)
		{
			// 过程返回
			if (eInfo.ir.mix == Opr_RET)
			{
				
				eInfo.funcStack.pop_back();
				if (!eInfo.funcStack.empty())
				{
					// 恢复现场
					eInfo.pr = eInfo.prStack.top();
					eInfo.tr = eInfo.trStack.top();
					eInfo.br = eInfo.brStack.top();

					eInfo.prStack.pop();
					eInfo.trStack.pop();
					eInfo.brStack.pop();
				}
			}
			// 栈顶 := -栈顶
			else if (eInfo.ir.mix == Opr_NEG)
			{
				eInfo.dataStack[eInfo.tr] = -eInfo.dataStack[eInfo.tr];
			}
			// 栈顶 := !栈顶
			else if (eInfo.ir.mix == Opr_NOT)
			{
				eInfo.dataStack[eInfo.tr] = !eInfo.dataStack[eInfo.tr];
			}
			// 次栈顶 := 次栈顶 + 栈顶，t减1
			else if (eInfo.ir.mix == Opr_ADD)
			{
				eInfo.dataStack[eInfo.tr - 1] += eInfo.dataStack[eInfo.tr];
				--eInfo.tr;
			}
			// 次栈顶 := 次栈顶 - 栈顶，t减1
			else if (eInfo.ir.mix == Opr_SUB)
			{
				eInfo.dataStack[eInfo.tr - 1] -= eInfo.dataStack[eInfo.tr];
				--eInfo.tr;
			}
			// 次栈顶 := 次栈顶 * 栈顶，t减1
			else if (eInfo.ir.mix == Opr_MUL)
			{
				eInfo.dataStack[eInfo.tr - 1] *= eInfo.dataStack[eInfo.tr];
				--eInfo.tr;
			}
			// 次栈顶 := 次栈顶 / 栈顶，t减1
			else if (eInfo.ir.mix == Opr_DIV)
			{
				eInfo.dataStack[eInfo.tr - 1] /= eInfo.dataStack[eInfo.tr];
				--eInfo.tr;
			}
			// 次栈顶 := 次栈顶 % 栈顶，t减1
			else if (eInfo.ir.mix == Opr_MOD)
			{
				eInfo.dataStack[eInfo.tr - 1] %= eInfo.dataStack[eInfo.tr];
				--eInfo.tr;
			}
			// 次栈顶 := (次栈顶 = 栈顶)，t减1
			else if (eInfo.ir.mix == Opr_EQU)
			{
				eInfo.dataStack[eInfo.tr - 1] = (eInfo.dataStack[eInfo.tr - 1] == eInfo.dataStack[eInfo.tr]);
				--eInfo.tr;
			}
			// 次栈顶 := (次栈顶 != 栈顶)，t减1
			else if (eInfo.ir.mix == Opr_NEQ)
			{
				eInfo.dataStack[eInfo.tr - 1] = (eInfo.dataStack[eInfo.tr - 1] != eInfo.dataStack[eInfo.tr]);
				--eInfo.tr;
			}
			// 次栈顶 := (次栈顶 < 栈顶)，t减1
			else if (eInfo.ir.mix == Opr_LES)
			{
				eInfo.dataStack[eInfo.tr - 1] = (eInfo.dataStack[eInfo.tr - 1] < eInfo.dataStack[eInfo.tr]);
				--eInfo.tr;
			}
			// 次栈顶 := (次栈顶 <= 栈顶)，t减1
			else if (eInfo.ir.mix == Opr_LEQ)
			{
				eInfo.dataStack[eInfo.tr - 1] = (eInfo.dataStack[eInfo.tr - 1] <= eInfo.dataStack[eInfo.tr]);
				--eInfo.tr;
			}
			// 次栈顶 := (次栈顶 > 栈顶)，t减1
			else if (eInfo.ir.mix == Opr_GTR)
			{
				eInfo.dataStack[eInfo.tr - 1] = (eInfo.dataStack[eInfo.tr - 1] > eInfo.dataStack[eInfo.tr]);
				--eInfo.tr;
			}
			// 次栈顶 := (次栈顶 >= 栈顶)，t减1
			else if (eInfo.ir.mix == Opr_GEQ)
			{
				eInfo.dataStack[eInfo.tr - 1] = (eInfo.dataStack[eInfo.tr - 1] >= eInfo.dataStack[eInfo.tr]);
				--eInfo.tr;
			}
			// 栈顶的值输出至控制台屏幕，t减1
			else if (eInfo.ir.mix == Opr_PRT)
			{
				(void)printf("%d\n", eInfo.dataStack[eInfo.tr]);
				--eInfo.tr;
			}
			// 从控制台读取输入，植入栈顶，t加1
			else if (eInfo.ir.mix == Opr_SCN)
			{
				if (eInfo.tr == eInfo.dataStack.size() - 1)
				{
					eInfo.dataStack.push_back(eInfo.ir.mix);
				}
				++eInfo.tr;
				(void)scanf_s("%d", eInfo.dataStack.data() + eInfo.tr);
			}
		}
		// 取值
		else if (eInfo.ir.func == Func_LOD)
		{
			int value;
			if (eInfo.ir.level == 1)
				value = eInfo.dataStack[eInfo.ir.mix];
			else
				value = eInfo.dataStack[eInfo.br + eInfo.ir.mix];

			if (eInfo.tr == eInfo.dataStack.size() - 1)
			{
				eInfo.dataStack.push_back(value);
				++eInfo.tr;
			}
			else
			{
				eInfo.dataStack[++eInfo.tr] = value;
			}

		}
		// 存值
		else if (eInfo.ir.func == Func_STO)
		{
			int value = eInfo.dataStack[eInfo.tr];
			--eInfo.tr;

			if (eInfo.ir.level == 1)
				eInfo.dataStack[eInfo.ir.mix] = value;
			else
				eInfo.dataStack[eInfo.br + eInfo.ir.mix] = value;
		}
		// 调用函数
		else if (eInfo.ir.func == Func_CAL)
		{
			// 保护寄存器现场
			eInfo.prStack.push(eInfo.pr);
			eInfo.trStack.push(eInfo.tr);
			eInfo.brStack.push(eInfo.br);
			// 获取调用的函数名
			for (const auto& id : m_ProgramInfo.identifiers)
			{
				if (id.offset == eInfo.ir.mix && (id.kind & ID_PROCEDURE))
				{
					eInfo.funcStack.push_back(id.name);
					break;
				}
			}
			// 更新寄存器
			eInfo.pr = eInfo.ir.mix;
			eInfo.br = eInfo.tr + 1;

			getNextInstruction = false;
		}
		// 开辟空间
		else if (eInfo.ir.func == Func_INT)
		{
			uint32_t startIdx = GetProcedureIndex(eInfo.funcStack.back()) + 1;
			if (static_cast<uint32_t>(eInfo.tr + eInfo.ir.mix + 1) > m_EnvironmentInfo.dataStack.size())
				eInfo.dataStack.resize(eInfo.tr + eInfo.ir.mix + 1);
			for (int i = 0; i < eInfo.ir.mix; ++i)
			{
				const auto& currID = m_ProgramInfo.identifiers[startIdx + static_cast<uint32_t>(i)];
				// 仅非形参变量赋初值
				if ((currID.kind & ID_VAR) && !(currID.kind & ID_PARAMETER))
				{
					eInfo.dataStack[eInfo.br + currID.offset] = currID.value;
				}
			}
			eInfo.tr += eInfo.ir.mix;
		}
		// 退避
		else if (eInfo.ir.func == Func_POP)
		{
			eInfo.tr -= eInfo.ir.mix;
		}
		// 无条件跳转
		else if (eInfo.ir.func == Func_JMP)
		{
			getNextInstruction = false;
			eInfo.pr = eInfo.ir.mix;
		}
		// 有条件跳转
		else if (eInfo.ir.func == Func_JPC)
		{
			if (eInfo.dataStack[eInfo.tr] == 0)
			{
				getNextInstruction = false;
				eInfo.pr = eInfo.ir.mix;
			}
			--eInfo.tr;
		}


		// 取下一条指令
		if (!eInfo.funcStack.empty())
		{
			if (getNextInstruction)
				++eInfo.pr;
			eInfo.ir = m_ProgramInfo.instructions[eInfo.pr];
		}

	}

	void VirtualMachine::RunProcedure()
	{
		if (m_EnvironmentInfo.ir.func == Func_CAL)
		{
			RunNextInstruction();
			RunToReturn();
		}
		else
		{
			RunNextInstruction();
		}
	}

	void VirtualMachine::RunToReturn()
	{
		uint32_t funcDepth = m_EnvironmentInfo.funcStack.size();
		while (!(m_EnvironmentInfo.ir.func == Func_OPR && m_EnvironmentInfo.ir.mix == Opr_RET
			&& m_EnvironmentInfo.funcStack.size() == funcDepth))
		{
			RunNextInstruction();
		}
		RunNextInstruction();
	}

	bool VirtualMachine::IsFinished() const
	{
		return m_EnvironmentInfo.funcStack.empty();
	}

	void VirtualMachine::PrintInstruction(uint32_t address) const
	{
		std::string func, mix;

		switch (m_ProgramInfo.instructions[address].func)
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

		if (m_ProgramInfo.instructions[address].func == Func_OPR)
		{
			switch (m_ProgramInfo.instructions[address].mix)
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
			mix = std::to_string(m_ProgramInfo.instructions[address].mix);
		}

		printf("%04u    %-8s%-8d%-8s\n", address, func.c_str(), m_ProgramInfo.instructions[address].level, mix.c_str());
	}

	void VirtualMachine::PrintRegisterInfo() const
	{
		printf("===========================\n");
		printf("address func    level   mix\n");
		PrintInstruction(m_EnvironmentInfo.pr);
		printf("pr=%d br=%d tr=%d\n", m_EnvironmentInfo.pr, m_EnvironmentInfo.br, m_EnvironmentInfo.tr);
		if (m_EnvironmentInfo.tr >= m_EnvironmentInfo.br)
		{
			printf("dataStack[br...tr]: ");
			for (int i = m_EnvironmentInfo.br; i <= m_EnvironmentInfo.tr; ++i)
			{
				printf("%d ", m_EnvironmentInfo.dataStack[i]);
			}
			puts("");
		}
		printf("funcDepth: %u\ncurrFunc: %s\n", m_EnvironmentInfo.funcStack.size(), m_EnvironmentInfo.funcStack.back().c_str());
		printf("========program I/O========\n");
	}

	const EnvironmentInfo& VirtualMachine::GetRegisterInfo() const
	{
		return m_EnvironmentInfo;
	}

	uint32_t VirtualMachine::GetProcedureIndex(const std::string& str)
	{
		uint32_t endIndex = m_ProgramInfo.identifiers.size();
		for (uint32_t i = 0; i < endIndex; ++i)
		{
			if (str == m_ProgramInfo.identifiers[i].name &&
				(m_ProgramInfo.identifiers[i].kind & ID_PROCEDURE))
			{
				return i;
			}
		}

		return UINT_MAX;
	}

}

