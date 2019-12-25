#include "PL0_ProgramParser.h"

namespace PL0
{
	bool ProgramParser::Parse(_In_ const std::vector<Symbol>& symbols)
	{
		m_ProgramInfo.identifiers.clear();
		m_ProgramInfo.instructions.clear();

		m_pCurrSymbol = symbols.cbegin();
		m_pEndSymbol = symbols.cend();

		m_ErrorInfos.clear();

		// ***程序的入口点为__main__***
		m_ProgramInfo.identifiers.push_back({"__main__", ID_PROCEDURE, 0, 0, 0});
		m_ProgramInfo.instructions.push_back({Func_JMP, 0, 0});		// 地址待回填

		PraseProgram();

		


		return m_ErrorInfos.empty();
	}

	const std::vector<ErrorInfo>& ProgramParser::GetErrorInfos() const
	{
		return m_ErrorInfos;
	}

	const ProgramInfo& ProgramParser::GetProgramInfo() const
	{
		return m_ProgramInfo;
	}

	bool ProgramParser::PraseProgram()
	{
		//
		// <程序>          ::= <常量说明部分><变量说明部分><过程说明部分><语句>.
		//

		std::vector<Identifier> constants;
		std::vector<Identifier> variables;
		bool isOK = PraseConstDesc(constants);
		isOK = isOK && PraseVarDesc(variables);

		// ***交换常量与变量位置，确定初始栈大小***
		int initStackUnit = 0;
		for (auto& id : variables)
		{
			// ***索引偏移，第1个变量的地址从0开始***
			if (!(m_ProgramInfo.identifiers.back().kind & ID_PROCEDURE))
			{
				id.offset = 1 + m_ProgramInfo.identifiers.back().offset;
			}
			++initStackUnit;
			m_ProgramInfo.identifiers.push_back(id);
		}
			
		for (auto& id : constants)
			m_ProgramInfo.identifiers.push_back(id);

		isOK = isOK && PraseProcedureDesc();

		// ***回填程序地址***
		m_ProgramInfo.identifiers.front().offset = static_cast<int>(m_ProgramInfo.instructions.size());
		m_ProgramInfo.instructions.front().mix = m_ProgramInfo.identifiers.front().offset;
		// ***开辟栈空间***
		m_ProgramInfo.instructions.push_back({Func_INT, 0, initStackUnit });

		isOK = isOK && PraseStatement();
		
		// ***主过程结束***
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_RET });

		if (!isOK)
			return false;

		// .
		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_Period)
		{
			try
			{
				SafeCheck(ST_Error, 39);		// [Error039] '.' Expected.
			}
			catch (const std::exception&)
			{
				return false;
			}	
		}
		++m_pCurrSymbol;


		if (m_pCurrSymbol == m_pEndSymbol)
		{
			return true;
		}
		else
		{
			try
			{
				SafeCheck(ST_Error, 41);		// [Error041] Unexpected content after '.'.
			}
			catch (const std::exception&)
			{
				return false;
			}
		}

	}

	bool ProgramParser::PraseConstDesc(_Inout_ std::vector<Identifier>& constants)
	{
		try
		{
			//
			// <常量说明部分>  ::= {const <值类型><常量定义>{,<常量定义>};}
			//
			
			// const
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Const)
			{
				++m_pCurrSymbol;

				SymbolType valueType = ST_Null;
				// <值类型>
				if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_Int)
				{
					SafeCheck(ST_Error, 31);	// [Error031] Typename expected.
				}
				valueType = m_pCurrSymbol->symbolType;
				++m_pCurrSymbol;
				
				// <常量定义>
				PraseConstDef(valueType, constants);

				//
				// {,<常量定义>};
				//

				while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
				{
					// ,
					++m_pCurrSymbol;
					// <常量定义>
					PraseConstDef(valueType, constants);
				}

				// ;
				SafeCheck(ST_SemiColon, 21);	// [Error021] ';' expected.
				++m_pCurrSymbol;
			}
		}
		catch(std::exception)
		{
			if (m_pCurrSymbol != m_pEndSymbol)
			{
				// 跳过当前语句
				while (m_pCurrSymbol != m_pEndSymbol && 
					(m_pCurrSymbol->symbolType != ST_SemiColon))
				{
					++m_pCurrSymbol;
				}
			}

			return false;
		}

		return true;
	}

	void ProgramParser::PraseConstDef(_In_ SymbolType type, _Inout_ std::vector<Identifier>& constants)
	{
		//
		// <常量定义> ::= <id>:=<值>
		//



		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		// ***id重定义检测***
		int level;
		if (~GetIDIndex(m_pCurrSymbol->word, &level) && level == m_CurrLevel)
		{
			SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
		}

		// ***添加常量id***
		Identifier id = { "", ID_CONST, 0, m_CurrLevel, 0 };
		strcpy_s(id.name, m_pCurrSymbol->word.c_str());
		switch (type)
		{
		case ST_Int: id.kind |= ID_INT; break;
		}

		++m_pCurrSymbol;

		// :=
		SafeCheck(ST_Assign, 23);		// [Error023] ':=' Expected.
		++m_pCurrSymbol;

		// <值>
		if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Integer && type == ST_Int)
		{
			id.value = std::stoi(m_pCurrSymbol->word);
			constants.push_back(id);
			++m_pCurrSymbol;
		}
		else
		{
			SafeCheck(ST_Error, 32);	// [Error032] Value expected.
		}
	}

	bool ProgramParser::PraseVarDesc(_Inout_ std::vector<Identifier>& variables)
	{
		try
		{
			//
			// <变量说明部分>  ::= {<值类型><变量定义>{,<变量定义>};}
			//
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Int)
			{
				SymbolType valueType;

				// <值类型>
				valueType = m_pCurrSymbol->symbolType;
				++m_pCurrSymbol;
				
				// <变量定义>
				PraseVarDef(valueType, variables);

				//
				// {,<变量定义>};
				//

				// ,
				while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
				{
					++m_pCurrSymbol;
					// <变量定义>
					PraseVarDef(valueType, variables);
				}

				// ;
				SafeCheck(ST_SemiColon, 21);	// [Error021] ';' expected.
				++m_pCurrSymbol;

			}
		}
		catch (std::exception)
		{
			if (m_pCurrSymbol != m_pEndSymbol)
			{
				// 跳过当前语句
				while (m_pCurrSymbol != m_pEndSymbol &&
					(m_pCurrSymbol->symbolType != ST_SemiColon))
				{
					++m_pCurrSymbol;
				}
			}

			return false;
		}
		return true;
	}

	void ProgramParser::PraseVarDef(_In_ SymbolType type, _Inout_ std::vector<Identifier>& variables)
	{
		//
		// <变量定义>      ::= <id>[:=<值>]
		//


		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		// ***id重定义检测***
		int level;
		if (~GetIDIndex(m_pCurrSymbol->word, &level) && level == m_CurrLevel)
		{
			SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
		}

		// ***添加变量id***
		Identifier id = { "", ID_VAR, 0, m_CurrLevel, 0 };
		strcpy_s(id.name, m_pCurrSymbol->word.c_str());
		switch (type)
		{
		case ST_Int: id.kind |= ID_INT; break;
		}

		++m_pCurrSymbol;

		//
		// [:=<值>]
		//
		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_Assign)
		{
			variables.push_back(id);
			return;
		}

		// :=
		++m_pCurrSymbol;

		// <值>
		if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Integer && type == ST_Int)
		{ 
			id.value = std::stoi(m_pCurrSymbol->word);
			variables.push_back(id);
			++m_pCurrSymbol;
		}
		else
		{
			SafeCheck(ST_Error, 32);	// [Error032] Value expected.
		}
	}

	bool ProgramParser::PraseProcedureDesc()
	{


		try
		{
			//
			// <过程说明部分>  ::= <过程首部><分程序>;{<过程说明部分>}
			//
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Procedure)
			{
				// <过程首部>
				PraseProcedureHeader();

				// <分程序>

				PraseSubProcedure();

				// ;
				SafeCheck(ST_SemiColon, 21);	// [Error021] ';' expected.
				++m_pCurrSymbol;

				//
				// {<过程说明部分>}
				//

				while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Procedure)
				{
					// <过程说明部分>
					PraseProcedureDesc();
				}

				
			}
		}
		catch (std::exception)
		{
			if (m_pCurrSymbol != m_pEndSymbol)
			{
				// 跳过当前语句
				while (m_pCurrSymbol != m_pEndSymbol &&
					(m_pCurrSymbol->symbolType == ST_SemiColon))
				{
					++m_pCurrSymbol;
				}
			}
			m_CurrLevel = 0;
			return false;
		}
		m_CurrLevel = 0;
		return true;
		
	}

	void ProgramParser::PraseProcedureHeader()
	{
		//
		// <过程首部>      ::= procedure <id>'('[<值类型><id>{,<值类型><id>}]')';
		//

		// procedure
		++m_pCurrSymbol;
		
		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		// ***id重定义检测***
		int level;
		if (~GetIDIndex(m_pCurrSymbol->word, &level))
		{
			SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
		}

		// ***添加过程id***
		Identifier id = { "", ID_PROCEDURE, 0, m_CurrLevel, 
			static_cast<int>(m_ProgramInfo.instructions.size()) };
		strcpy_s(id.name, m_pCurrSymbol->word.c_str());
		m_CurrProcIndex = m_ProgramInfo.identifiers.size();
		m_ProgramInfo.identifiers.push_back(id);
		++m_pCurrSymbol;

		// '('
		SafeCheck(ST_LeftParen, 24);	// [Error024] '(' Expected.
		++m_pCurrSymbol;

		// ***从过程的形参开始为内层***
		++m_CurrLevel;

		//
		// [<值类型><id>{,<值类型><id>}]')';
		//
		if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Int)
		{
			// <值类型>
			SymbolType valueType = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <id>
			SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
			if (~GetIDIndex(m_pCurrSymbol->word, &level) && level == m_CurrLevel)
			{
				SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
			}


			// ***添加过程符号***
			id = { "", ID_PARAMETER | ID_VAR, 0, m_CurrLevel, 0 };
			strcpy_s(id.name, m_pCurrSymbol->word.c_str());
			switch (valueType)
			{
			case ST_Int: id.kind |= ID_INT; break;
			}

			m_ProgramInfo.identifiers.push_back(id);
			++m_pCurrSymbol;

			//
			// {, <值类型><id>}')'
			//

			// ,
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
			{
				++m_pCurrSymbol;
				// <值类型>
				if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_Int)
				{
					SafeCheck(ST_Error, 31);	// [Error031] Typename expected.
				}
				SymbolType valueType = m_pCurrSymbol->symbolType;
				++m_pCurrSymbol;

				// <id>
				SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
				if (~GetIDIndex(m_pCurrSymbol->word, &level) && level == m_CurrLevel)
				{
					SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
				}

				id = { "", ID_PARAMETER | ID_VAR, 0, m_CurrLevel, 0 };
				strcpy_s(id.name, m_pCurrSymbol->word.c_str());
				switch (valueType)
				{
				case ST_Int: id.kind |= ID_INT; break;
				}
				// ***形参索引偏移***
				id.offset = 1 + m_ProgramInfo.identifiers.back().offset;
				m_ProgramInfo.identifiers.push_back(id);
				++m_pCurrSymbol;
			}
		}

		// ')'
		SafeCheck(ST_RightParen, 25);		// [Error025] ')' Expected.
		++m_pCurrSymbol;
		
		// ;
		SafeCheck(ST_SemiColon, 21);		// [Error021] ';' expected.
		++m_pCurrSymbol;
	}

	void ProgramParser::PraseSubProcedure()
	{
		// <分程序>        ::= <常量说明部分><变量说明部分><语句>

		std::vector<Identifier> constants;
		std::vector<Identifier> variables;
		// <常量说明部分>
		PraseConstDesc(constants);
		// <变量说明部分>
		PraseVarDesc(variables);

		// ***交换常量与变量位置***
		int initStackUnit = 0;
		for (auto& id : variables)
		{
			// ***索引偏移，若过程无参，第1个变量的地址从0开始***
			if (!(m_ProgramInfo.identifiers.back().kind & ID_PROCEDURE))
			{
				id.offset = 1 + m_ProgramInfo.identifiers.back().offset;
			}
			++initStackUnit;
			m_ProgramInfo.identifiers.push_back(id);
		}

		// ***获取形参数目***
		int paramCount = 0;
		for (uint32_t i = m_ProgramInfo.identifiers.size() - 1; !(m_ProgramInfo.identifiers[i].kind & ID_PROCEDURE); --i)
		{
			if (m_ProgramInfo.identifiers[i].kind & ID_PARAMETER)
				++paramCount;
		}

		// ***开辟栈空间***
		m_ProgramInfo.instructions.push_back({ Func_INT, 0, paramCount });

		for (auto& id : constants)
			m_ProgramInfo.identifiers.push_back(id);

		// <语句>
		PraseStatement();

		// ***返回，并回收栈空间***
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, 0 });

		// ***过程结束回到上一层***
		--m_CurrLevel;
	}

	bool ProgramParser::PraseStatement()
	{
		try
		{
			//
			// <语句>          ::= <空语句>|<赋值语句>|<条件语句>|<while循环语句>|<for循环语句>|<过程调用语句>|<读语句>|<写语句>|<返回语句>|<复合语句>
			//

			// <空语句>
			if (m_pCurrSymbol == m_pEndSymbol)
				return true;
			// <赋值语句>
			else if (m_pCurrSymbol->symbolType == ST_Identifier)
				PraseAssignmentStat();
			// <复合语句>
			else if (m_pCurrSymbol->symbolType == ST_Begin)
				PraseComplexStat();
			// <条件语句>
			else if (m_pCurrSymbol->symbolType == ST_If)
				PraseConditionalStat();
			// <while循环语句>
			else if (m_pCurrSymbol->symbolType == ST_While)
				PraseWhileLoopStat();
			// <for循环语句>
			else if (m_pCurrSymbol->symbolType == ST_For)
				PraseForLoopStat();
			// <读语句>
			else if (m_pCurrSymbol->symbolType == ST_Read)
				PraseReadStat();
			// <写语句>
			else if (m_pCurrSymbol->symbolType == ST_Write)
				PraseWriteStat();
			// <过程调用语句>
			else if (m_pCurrSymbol->symbolType == ST_Call)
				PraseCallStat();
			// <返回语句>
			else if (m_pCurrSymbol->symbolType == ST_Return)
				PraseReturnStat();
		}
		catch (std::exception)
		{
			if (m_pCurrSymbol != m_pEndSymbol)
			{
				// 跳过当前语句
				while (m_pCurrSymbol != m_pEndSymbol &&
					(m_pCurrSymbol->symbolType != ST_SemiColon))
				{
					++m_pCurrSymbol;
				}
			}

			return false;
		}
		return true;
	}

	void ProgramParser::PraseAssignmentStat()
	{
		//
		// <赋值语句>      ::= <id> :=|+=|-=|*=|/=|%= <表达式>
		//

		// <id>
		const std::string& id = m_pCurrSymbol->word;
		int idLevel;
		uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);
		// ***检验id存在性与是否可以写入***
		if (!~pos)
		{
			SafeCheck(ST_Error, 33);	// [Error033] Unknown identifier.
		}
		else if (!(m_ProgramInfo.identifiers[pos].kind & ID_VAR))
		{
			SafeCheck(ST_Error, 35);	// [Error035] Identifier can't be assigned.
		}
		int idOffset = m_ProgramInfo.identifiers[pos].offset;
		++m_pCurrSymbol;

		// :=|+=|-=|*=|/=|%=
		if (m_pCurrSymbol == m_pEndSymbol || 
			m_pCurrSymbol->symbolType < ST_Assign || m_pCurrSymbol->symbolType >= ST_Equal)
		{
			SafeCheck(ST_Error, 23);	// [Error023] ':=' Expected.
		}
		
		
		SymbolType assignment = m_pCurrSymbol->symbolType;
		if (assignment != ST_Assign)
		{
			m_ProgramInfo.instructions.push_back({ Func_LOD, m_CurrLevel - idLevel, idOffset });
		}
		++m_pCurrSymbol;
		



		// <表达式>
		PraseExpressionL1();

		// 非常规赋值还需要进行一次二元运算
		if (assignment == ST_PlusAndAssign)
		{
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_ADD });
		}
		else if (assignment == ST_MinusAndAssign)
		{
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_SUB });
		}
		else if (assignment == ST_MultiplyAndAssign)
		{
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_MUL });
		}
		else if (assignment == ST_DivideAndAssign)
		{
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_DIV });
		}
		else if (assignment == ST_ModAndAssign)
		{
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_MOD });
		}

		m_ProgramInfo.instructions.push_back({ Func_STO, m_CurrLevel - idLevel, idOffset });
	}

	void ProgramParser::PraseComplexStat()
	{
		//
		// <复合语句>      ::= begin <语句>{;<语句>} end
		//

		// begin
		++m_pCurrSymbol;

		// <语句>
		PraseStatement();

		//
		// {;<语句>}
		//

		// ;
		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_SemiColon)
		{
			++m_pCurrSymbol;

			// <语句>
			PraseStatement();
		}

		// end
		SafeCheck(ST_End, 26);		// [Error026] Keyword 'end' expected.
		++m_pCurrSymbol;
	}

	void ProgramParser::PraseConditionalStat()
	{
		//
		// <条件语句>      ::= if <条件> then <语句> {else if <条件> then <语句>}[else <语句>]
		//

		std::forward_list<int> trueList;	
		std::forward_list<int> falseList;
		int trueLength = 0, falseLength = 0;
		// if
		++m_pCurrSymbol;

		// <条件>
		PraseConditionL1();

		falseList.push_front(m_ProgramInfo.instructions.size());
		m_ProgramInfo.instructions.push_back({Func_JPC, 0, 0});	// 地址待回填
		++falseLength;
		
		// then
		SafeCheck(ST_Then, 27);		// [Error027] Keyword 'then' expected.
		++m_pCurrSymbol;
		
		// <语句>
		PraseStatement();

		//
		// {else if <条件> then <语句>}[else <语句>]
		//


		// else
		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Else)
		{
			++m_pCurrSymbol;

			trueList.push_front(m_ProgramInfo.instructions.size());
			m_ProgramInfo.instructions.push_back({ Func_JMP, 0, 0 }); // 地址待回填
			++trueLength;

			// if
			if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_If)
			{
				++m_pCurrSymbol;

				// <条件>
				PraseConditionL1();

				falseList.push_front(m_ProgramInfo.instructions.size());
				m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });	// 地址待回填
				++falseLength;

				// then
				SafeCheck(ST_Then, 27);		// [Error027] Keyword 'then' expected.
				++m_pCurrSymbol;

				// <语句>
				PraseStatement();
			}
			else
			{
				// <语句>
				PraseStatement();
				break;
			}
		}

		int currOffset = m_ProgramInfo.instructions.size();
		
		// ***真/假链地址回填(假链长度可能比真链大1)***
		if (falseLength > trueLength)
		{
			m_ProgramInfo.instructions[falseList.front()].mix = currOffset;
			falseList.pop_front();
		}
		
		while (falseLength-- && trueLength--)
		{
			int trueNodeOffset = trueList.front();
			m_ProgramInfo.instructions[trueList.front()].mix = currOffset;
			m_ProgramInfo.instructions[falseList.front()].mix = trueNodeOffset + 1;

			falseList.pop_front();
			trueList.pop_front();
		}
	}

	void ProgramParser::PraseWhileLoopStat()
	{
		//
		// <while循环语句> ::= while <条件> do <语句>
		//

		// while
		++m_pCurrSymbol;

		int startLoopOffset = m_ProgramInfo.instructions.size();

		// <条件>
		PraseConditionL1();

		// ***待回填地址***
		int falseIndex = m_ProgramInfo.instructions.size();
		m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });

		// do
		SafeCheck(ST_Do, 28);		// [Error028] Keyword 'do' expected.
		++m_pCurrSymbol;

		// <语句>
		PraseStatement();

		m_ProgramInfo.instructions.push_back({ Func_JMP, 0, startLoopOffset });
		// ***回填地址***
		m_ProgramInfo.instructions[falseIndex].mix = 
			m_ProgramInfo.instructions.size();

	}

	void ProgramParser::PraseForLoopStat()
	{
		//
		// <for循环语句>   ::= for <id>:=<表达式> step <表达式> until <条件> do <语句>
		//

		// for
		++m_pCurrSymbol;

		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		const std::string& id = m_pCurrSymbol->word;
		int idLevel;
		uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);
		if (!(m_ProgramInfo.identifiers[pos].kind & ID_VAR))
		{
			SafeCheck(ST_Error, 35);	// [Error035] Identifier can't be assigned.
		}
		int idOffset = m_ProgramInfo.identifiers[pos].offset;
		++m_pCurrSymbol;

		// :=
		SafeCheck(ST_Assign, 23);		// [Error023] ':=' Expected.
		++m_pCurrSymbol;

		// <表达式>
		PraseExpressionL1();

		m_ProgramInfo.instructions.push_back({ Func_STO, m_CurrLevel - idLevel, idOffset });

		// step
		// ***由于要先判断条件，执行语句块，再进行更新，需要拦截这部分指令***
		SafeCheck(ST_Step, 29);		// [Error029] Keyword 'step' expected.
		++m_pCurrSymbol;

		uint32_t currInstructionCount = m_ProgramInfo.instructions.size();
		std::vector<Instruction> temp;

		// <表达式>
		PraseExpressionL1();

		// ***临时缓存这些表达式***
		temp.insert(temp.cbegin(), m_ProgramInfo.instructions.cbegin() + currInstructionCount,
			m_ProgramInfo.instructions.cend());
		m_ProgramInfo.instructions.erase(m_ProgramInfo.instructions.cbegin() + currInstructionCount);

		// until
		SafeCheck(ST_Until, 30);	// [Error030] Keyword 'until' expected.
		++m_pCurrSymbol;

		int startLoopOffset = m_ProgramInfo.instructions.size();

		// <条件>
		PraseConditionL1();

		// ***由于条件为真时退出循环，在这里需要逻辑取反***
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_NOT });
		// ***待回填地址***
		int falseIndex = m_ProgramInfo.instructions.size();
		m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });

		// do
		SafeCheck(ST_Do, 28);	// [Error028] Keyword 'do' expected.
		++m_pCurrSymbol;

		// <语句>
		PraseStatement();

		// ***执行STEP***
		m_ProgramInfo.instructions.push_back({ Func_LOD, m_CurrLevel - idLevel, idOffset });
		m_ProgramInfo.instructions.insert(m_ProgramInfo.instructions.cend(),
			temp.cbegin(), temp.cend());
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_ADD });
		m_ProgramInfo.instructions.push_back({ Func_STO, m_CurrLevel - idLevel, idOffset });
		m_ProgramInfo.instructions.push_back({ Func_JMP, 0, startLoopOffset });
		// ***回填地址***
		m_ProgramInfo.instructions[falseIndex].mix =
			m_ProgramInfo.instructions.size();
	}

	void ProgramParser::PraseReadStat()
	{
		//
		// <读语句>        ::= read '('<id>{,<id>}')'
		//


		// read
		++m_pCurrSymbol;

		// '('
		SafeCheck(ST_LeftParen, 24);	// [Error024] '(' Expected.
		++m_pCurrSymbol;

		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		const std::string& id = m_pCurrSymbol->word;
		int idLevel;
		uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);
		// ***检验存在性与是否可以写入***
		if (!~pos)
		{
			SafeCheck(ST_Error, 33);	// [Error033] Unknown identifier.
		}
		else if (!(m_ProgramInfo.identifiers[pos].kind & ID_VAR))
		{
			SafeCheck(ST_Error, 35);	// [Error035] Identifier can't be assigned.
		}
		int idOffset = m_ProgramInfo.identifiers[pos].offset;
		++m_pCurrSymbol;

		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_SCN });
		m_ProgramInfo.instructions.push_back({ Func_STO, m_CurrLevel - idLevel , idOffset });

		//
		// {,<id>}
		//
		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
		{
			++m_pCurrSymbol;

			// <id>
			SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
			const std::string& id = m_pCurrSymbol->word;
			int idLevel;
			uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);
			// ***检验存在性与是否可以写入***
			if (!(m_ProgramInfo.identifiers[pos].kind & ID_VAR))
			{
				SafeCheck(ST_Error, 35);	// [Error035] Identifier can't be assigned.
			}
			int idOffset = m_ProgramInfo.identifiers[pos].offset;
			++m_pCurrSymbol;

			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_SCN });
			m_ProgramInfo.instructions.push_back({ Func_STO, m_CurrLevel - idLevel , idOffset });
		}

		// ')'
		SafeCheck(ST_RightParen, 25);		// [Error025] ')' Expected.
		++m_pCurrSymbol;
	}

	void ProgramParser::PraseWriteStat()
	{
		//
		// <写语句>        ::= write '('<表达式>{,<表达式>}')'
		//

		// write
		++m_pCurrSymbol;

		// '('
		SafeCheck(ST_LeftParen, 24);		// [Error024] '(' Expected.
		++m_pCurrSymbol;

		// <表达式>
		PraseExpressionL1();

		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_PRT });

		//
		// {,<表达式>}
		//

		// ,
		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
		{
			++m_pCurrSymbol;

			// <表达式>
			PraseExpressionL1();

			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_PRT });
		}

		// ')'
		SafeCheck(ST_RightParen, 25);		// [Error025] ')' Expected.
		++m_pCurrSymbol;
	}

	void ProgramParser::PraseCallStat()
	{
		//
		// <过程调用语句>  ::= call <id>'('[<表达式>{,<表达式>}]')'
		//

		// call
		++m_pCurrSymbol;

		int paramCount = 0;

		// <id>
		const std::string& id = m_pCurrSymbol->word;
		int idLevel;
		uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);
		// ***检验id存在性与是否为函数***
		if (!~pos)
		{
			SafeCheck(ST_Error, 33);		// [Error033] Unknown identifier.
		}
		else if (!(m_ProgramInfo.identifiers[pos].kind & ID_PROCEDURE))
		{
			SafeCheck(ST_Error, 36);		// [Error036] Identifier is not a procedure.
		}
		int idOffset = m_ProgramInfo.identifiers[pos].offset;
		++m_pCurrSymbol;


		// '('
		SafeCheck(ST_LeftParen, 24);		// [Error024] '(' Expected.
		++m_pCurrSymbol;

		//
		// [<表达式>{,<表达式>}]')'
		//

		// <表达式>
		if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType != ST_RightParen)
		{
			PraseExpressionL1();
			++paramCount;

			// {,<表达式>}

			// ,
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
			{
				++m_pCurrSymbol;

				// <表达式>
				PraseExpressionL1();
				++paramCount;
			}
		}

		// ')'
		SafeCheck(ST_RightParen, 25);		// [Error025] ')' Expected.
		++m_pCurrSymbol;

		// ***查看函数的实际形参数目***
		int actualParamCount = 0;
		uint32_t endPos = m_ProgramInfo.identifiers.size();
		for (uint32_t i = pos + 1; i < endPos; ++i)
		{
			if (m_ProgramInfo.identifiers[i].kind & ID_PARAMETER)
				++actualParamCount;
			else
				break;
		}

		if (actualParamCount != paramCount)
		{
			SafeCheck(ST_Error, 37);		// [Error037] Number of function parameter mismatch.
		}

		// ***退避为形参让出空间(仅改变栈顶指针，不移除上面的内容)***
		m_ProgramInfo.instructions.push_back({ Func_POP, 0, paramCount });

		// ***最后调用函数***
		m_ProgramInfo.instructions.push_back({ Func_CAL, m_CurrLevel - idLevel,
		m_ProgramInfo.identifiers[pos].offset });
	}

	void ProgramParser::PraseReturnStat()
	{
		//
		// <返回语句>      ::= return
		//
		++m_pCurrSymbol;
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_RET });
	}

	void ProgramParser::PraseConditionL1()
	{
		//
		// <条件>          ::= <二级条件>{<逻辑或><二级条件>}
		//

		// <二级条件>
		PraseConditionL2();

		//
		// {<逻辑或><二级条件>}
		//

		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_LogicalOr)
			return;

		std::forward_list<int> trueList;

		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_LogicalOr)
		{
			// <逻辑或>
			++m_pCurrSymbol;

			// ***记录真链***
			
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_NOT });
			trueList.push_front(m_ProgramInfo.instructions.size());
			m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });

			// <二级条件>
			PraseConditionL2();
			
		}

		m_ProgramInfo.instructions.push_back({ Func_JMP, 0, static_cast<int>(m_ProgramInfo.instructions.size() + 2) });

		while (!trueList.empty())
		{
			m_ProgramInfo.instructions[trueList.front()].mix = m_ProgramInfo.instructions.size();
			trueList.pop_front();
		}

		m_ProgramInfo.instructions.push_back({ Func_LIT, 0, 1 });

	}

	void ProgramParser::PraseConditionL2()
	{
		//
		// <二级条件>      ::= <三级条件>{<逻辑与><三级条件>}
		//

		// <三级条件>
		PraseConditionL3();

		//
		// {<逻辑与><三级条件>}
		//

		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_LogicalAnd)
			return;

		std::forward_list<int> falseList;

		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_LogicalAnd)
		{
			// <逻辑与>
			++m_pCurrSymbol;

			// ***记录假链***
			falseList.push_front(m_ProgramInfo.instructions.size());
			m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0});

			// <三级条件>
			PraseConditionL3();
		}

		m_ProgramInfo.instructions.push_back({ Func_JMP, 0, static_cast<int>(m_ProgramInfo.instructions.size() + 2) });

		while (!falseList.empty())
		{
			m_ProgramInfo.instructions[falseList.front()].mix = m_ProgramInfo.instructions.size();
			falseList.pop_front();
		}

		m_ProgramInfo.instructions.push_back({ Func_LIT, 0, 0 });
	}

	void ProgramParser::PraseConditionL3()
	{
		//
		// <三级条件>      ::= <四级条件>{!=|= <四级条件>}
		//

		// <四级条件>
		PraseConditionL4();

		//
		// {!=|= <四级条件>}
		//

		while (m_pCurrSymbol != m_pEndSymbol && 
			(m_pCurrSymbol->symbolType == ST_NotEqual || m_pCurrSymbol->symbolType == ST_Equal))
		{
			// !=|=
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <四级条件>
			PraseConditionL4();

			if (type == ST_NotEqual)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_NEQ });
			}
			else if (type == ST_Equal)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_EQU });
			}
		}
	}

	void ProgramParser::PraseConditionL4()
	{
		//
		// <四级条件>      ::= <表达式>{>|>=|<|<= <表达式>}
		//

		// <表达式>
		PraseExpressionL1();

		//
		// {>|>=|<|<= <表达式>}
		//

		while (m_pCurrSymbol != m_pEndSymbol &&
			(m_pCurrSymbol->symbolType == ST_Greater || m_pCurrSymbol->symbolType == ST_GreaterEqual ||
				m_pCurrSymbol->symbolType == ST_Less || m_pCurrSymbol->symbolType == ST_LessEqual))
		{
			// >|>=|<|<=
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <表达式>
			PraseExpressionL1();


			if (type == ST_Greater)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_GTR });
			}
			else if (type == ST_GreaterEqual)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_GEQ });
			}
			else if (type == ST_Less)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_LES });
			}
			else if (type == ST_LessEqual)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_LEQ });
			}
		}
	}

	void ProgramParser::PraseExpressionL1()
	{
		//
		// <表达式>        ::= <二级表达式>{+|- <二级表达式>}
		//

		// <二级表达式>
		PraseExpressionL2();

		//
		// {+|- <二级表达式>}
		//
		while (m_pCurrSymbol != m_pEndSymbol &&
			(m_pCurrSymbol->symbolType == ST_Plus || m_pCurrSymbol->symbolType == ST_Minus))
		{
			// +|-
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <二级表达式>
			PraseExpressionL2();

			if (type == ST_Plus)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_ADD });
			}
			else if (type == ST_Minus)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_SUB });
			}
		}
	}

	void ProgramParser::PraseExpressionL2()
	{
		//
		// <二级表达式>    ::= <三级表达式>{*|/|% <三级表达式>}
		//

		// <三级表达式>
		PraseExpressionL3();

		//
		// {*|/|% <三级表达式>}
		//
		while (m_pCurrSymbol != m_pEndSymbol &&
			(m_pCurrSymbol->symbolType == ST_Multiply || m_pCurrSymbol->symbolType == ST_Divide ||
				m_pCurrSymbol->symbolType == ST_Mod))
		{
			// *|/|%
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <三级表达式>
			PraseExpressionL3();

			if (type == ST_Multiply)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_MUL });
			}
			else if (type == ST_Divide)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_DIV });
			}
			else if (type == ST_Mod)
			{
				m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_MOD });
			}
		}
	}

	void ProgramParser::PraseExpressionL3()
	{
		//
		// <三级表达式>    ::= [!|-|+]<四级表达式>
		//

		SymbolType prefix = ST_Null;

		// [!|-|+]
		if (m_pCurrSymbol != m_pEndSymbol)
		{
			if (m_pCurrSymbol->symbolType == ST_LogicalNot ||
				m_pCurrSymbol->symbolType == ST_Minus ||
				m_pCurrSymbol->symbolType == ST_Plus)
			{
				prefix = m_pCurrSymbol->symbolType;
				++m_pCurrSymbol;
			}

		}

		// <四级表达式>
		PraseExpressionL4();



		if (prefix == ST_LogicalNot)
		{
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_NOT });
		}
		else if (prefix == ST_Minus)
		{
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_NEG });
		}

	}

	void ProgramParser::PraseExpressionL4()
	{
		//
		// <四级表达式>          ::= <id>|<value>|'('<条件>')'
		//

		if (m_pCurrSymbol == m_pEndSymbol)
		{
			SafeCheck(ST_Error, 38);		// [Error038] Expression/Condition expected.
		}
		// <id>
		else if (m_pCurrSymbol->symbolType == ST_Identifier)
		{
			int idLevel;
			uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);

			if (pos == UINT_MAX)
			{
				SafeCheck(ST_Error, 33);	// [Error033] Unknown identifier.
			}

			int idOffset = m_ProgramInfo.identifiers[pos].offset;
			// ***常量用指令LIT，变量用指令LOD***
			if (m_ProgramInfo.identifiers[pos].kind & ID_CONST)
			{
				m_ProgramInfo.instructions.push_back({ Func_LIT, m_CurrLevel - idLevel, m_ProgramInfo.identifiers[pos].value });
			}
			else
			{
				m_ProgramInfo.instructions.push_back({ Func_LOD, m_CurrLevel - idLevel, idOffset });
			}
			
			++m_pCurrSymbol;
		}
		// <value>
		else if (m_pCurrSymbol->symbolType == ST_Integer)
		{
			int value = std::stoi(m_pCurrSymbol->word);
			m_ProgramInfo.instructions.push_back({ Func_LIT, 0, value });
			++m_pCurrSymbol;
		}
		// '('
		else if (m_pCurrSymbol->symbolType == ST_LeftParen)
		{
			++m_pCurrSymbol;

			// <条件>
			PraseConditionL1();

			// ')'
			SafeCheck(ST_RightParen, 25);		// [Error025] ')' Expected.
			++m_pCurrSymbol;
		}
		else
		{
			SafeCheck(ST_Error, 40);			// [Error040] Invalid expression.
		}
	}

	void ProgramParser::SafeCheck(SymbolType st, ErrorCode errorCode, const char* errorStr)
	{
		// 若当前已经扫完整个符号序列，或者当前期望的符号类型与实际的不一致，则记录错误信息并抛出异常
		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != st)
		{
			ErrorInfo errorInfo; 
			if (m_pCurrSymbol != m_pEndSymbol)
			{
				errorInfo.beg = m_pCurrSymbol->beg;
				errorInfo.end = m_pCurrSymbol->end;
			}
			else
			{
				--m_pCurrSymbol;
				errorInfo.beg = m_pCurrSymbol->beg;
				errorInfo.end = m_pCurrSymbol->end;
				++m_pCurrSymbol;
			}

			errorInfo.errorCode = errorCode;
			m_ErrorInfos.push_back(errorInfo);
			

			throw std::exception(errorStr);
		}
	}

	uint32_t ProgramParser::GetIDIndex(const std::string& str, int* outLevel)
	{
		// 检验当前过程是否有该符号
		uint32_t endIndex = m_ProgramInfo.identifiers.size();
		uint32_t i;
		if (m_CurrLevel > 0)
		{
			for (i = m_CurrProcIndex; i < endIndex; ++i)
			{
				if (m_ProgramInfo.identifiers[i].name == str)
				{
					if (outLevel)
						*outLevel = 1;
					return i;
				}
			}
		}

		// 检验主过程是否有该符号
		i = 1;
		while (i < endIndex && !(m_ProgramInfo.identifiers[i].kind & ID_PROCEDURE))
		{
			if (m_ProgramInfo.identifiers[i].name == str)
			{
				if (outLevel)
					*outLevel = 0;
				return i;
			}
			++i;
		}

		// 检验是否为过程
		i = 1;
		while (i < endIndex)
		{
			if (m_ProgramInfo.identifiers[i].name == str && (m_ProgramInfo.identifiers[i].kind & ID_PROCEDURE))
			{
				if (outLevel)
					*outLevel = 0;
				return i;
			}
			++i;
		}

		return UINT_MAX;
	}

}

