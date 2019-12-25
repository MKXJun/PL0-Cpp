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

		// ***�������ڵ�Ϊ__main__***
		m_ProgramInfo.identifiers.push_back({"__main__", ID_PROCEDURE, 0, 0, 0});
		m_ProgramInfo.instructions.push_back({Func_JMP, 0, 0});		// ��ַ������

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
		// <����>          ::= <����˵������><����˵������><����˵������><���>.
		//

		std::vector<Identifier> constants;
		std::vector<Identifier> variables;
		bool isOK = PraseConstDesc(constants);
		isOK = isOK && PraseVarDesc(variables);

		// ***�������������λ�ã�ȷ����ʼջ��С***
		int initStackUnit = 0;
		for (auto& id : variables)
		{
			// ***����ƫ�ƣ���1�������ĵ�ַ��0��ʼ***
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

		// ***��������ַ***
		m_ProgramInfo.identifiers.front().offset = static_cast<int>(m_ProgramInfo.instructions.size());
		m_ProgramInfo.instructions.front().mix = m_ProgramInfo.identifiers.front().offset;
		// ***����ջ�ռ�***
		m_ProgramInfo.instructions.push_back({Func_INT, 0, initStackUnit });

		isOK = isOK && PraseStatement();
		
		// ***�����̽���***
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
			// <����˵������>  ::= {const <ֵ����><��������>{,<��������>};}
			//
			
			// const
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Const)
			{
				++m_pCurrSymbol;

				SymbolType valueType = ST_Null;
				// <ֵ����>
				if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_Int)
				{
					SafeCheck(ST_Error, 31);	// [Error031] Typename expected.
				}
				valueType = m_pCurrSymbol->symbolType;
				++m_pCurrSymbol;
				
				// <��������>
				PraseConstDef(valueType, constants);

				//
				// {,<��������>};
				//

				while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
				{
					// ,
					++m_pCurrSymbol;
					// <��������>
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
				// ������ǰ���
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
		// <��������> ::= <id>:=<ֵ>
		//



		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		// ***id�ض�����***
		int level;
		if (~GetIDIndex(m_pCurrSymbol->word, &level) && level == m_CurrLevel)
		{
			SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
		}

		// ***��ӳ���id***
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

		// <ֵ>
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
			// <����˵������>  ::= {<ֵ����><��������>{,<��������>};}
			//
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Int)
			{
				SymbolType valueType;

				// <ֵ����>
				valueType = m_pCurrSymbol->symbolType;
				++m_pCurrSymbol;
				
				// <��������>
				PraseVarDef(valueType, variables);

				//
				// {,<��������>};
				//

				// ,
				while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
				{
					++m_pCurrSymbol;
					// <��������>
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
				// ������ǰ���
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
		// <��������>      ::= <id>[:=<ֵ>]
		//


		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		// ***id�ض�����***
		int level;
		if (~GetIDIndex(m_pCurrSymbol->word, &level) && level == m_CurrLevel)
		{
			SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
		}

		// ***��ӱ���id***
		Identifier id = { "", ID_VAR, 0, m_CurrLevel, 0 };
		strcpy_s(id.name, m_pCurrSymbol->word.c_str());
		switch (type)
		{
		case ST_Int: id.kind |= ID_INT; break;
		}

		++m_pCurrSymbol;

		//
		// [:=<ֵ>]
		//
		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_Assign)
		{
			variables.push_back(id);
			return;
		}

		// :=
		++m_pCurrSymbol;

		// <ֵ>
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
			// <����˵������>  ::= <�����ײ�><�ֳ���>;{<����˵������>}
			//
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Procedure)
			{
				// <�����ײ�>
				PraseProcedureHeader();

				// <�ֳ���>

				PraseSubProcedure();

				// ;
				SafeCheck(ST_SemiColon, 21);	// [Error021] ';' expected.
				++m_pCurrSymbol;

				//
				// {<����˵������>}
				//

				while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Procedure)
				{
					// <����˵������>
					PraseProcedureDesc();
				}

				
			}
		}
		catch (std::exception)
		{
			if (m_pCurrSymbol != m_pEndSymbol)
			{
				// ������ǰ���
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
		// <�����ײ�>      ::= procedure <id>'('[<ֵ����><id>{,<ֵ����><id>}]')';
		//

		// procedure
		++m_pCurrSymbol;
		
		// <id>
		SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
		// ***id�ض�����***
		int level;
		if (~GetIDIndex(m_pCurrSymbol->word, &level))
		{
			SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
		}

		// ***��ӹ���id***
		Identifier id = { "", ID_PROCEDURE, 0, m_CurrLevel, 
			static_cast<int>(m_ProgramInfo.instructions.size()) };
		strcpy_s(id.name, m_pCurrSymbol->word.c_str());
		m_CurrProcIndex = m_ProgramInfo.identifiers.size();
		m_ProgramInfo.identifiers.push_back(id);
		++m_pCurrSymbol;

		// '('
		SafeCheck(ST_LeftParen, 24);	// [Error024] '(' Expected.
		++m_pCurrSymbol;

		// ***�ӹ��̵��βο�ʼΪ�ڲ�***
		++m_CurrLevel;

		//
		// [<ֵ����><id>{,<ֵ����><id>}]')';
		//
		if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Int)
		{
			// <ֵ����>
			SymbolType valueType = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <id>
			SafeCheck(ST_Identifier, 22);	// [Error022] Identifier Expected.
			if (~GetIDIndex(m_pCurrSymbol->word, &level) && level == m_CurrLevel)
			{
				SafeCheck(ST_Error, 34);	// [Error034] Identifier redefined.
			}


			// ***��ӹ��̷���***
			id = { "", ID_PARAMETER | ID_VAR, 0, m_CurrLevel, 0 };
			strcpy_s(id.name, m_pCurrSymbol->word.c_str());
			switch (valueType)
			{
			case ST_Int: id.kind |= ID_INT; break;
			}

			m_ProgramInfo.identifiers.push_back(id);
			++m_pCurrSymbol;

			//
			// {, <ֵ����><id>}')'
			//

			// ,
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
			{
				++m_pCurrSymbol;
				// <ֵ����>
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
				// ***�β�����ƫ��***
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
		// <�ֳ���>        ::= <����˵������><����˵������><���>

		std::vector<Identifier> constants;
		std::vector<Identifier> variables;
		// <����˵������>
		PraseConstDesc(constants);
		// <����˵������>
		PraseVarDesc(variables);

		// ***�������������λ��***
		int initStackUnit = 0;
		for (auto& id : variables)
		{
			// ***����ƫ�ƣ��������޲Σ���1�������ĵ�ַ��0��ʼ***
			if (!(m_ProgramInfo.identifiers.back().kind & ID_PROCEDURE))
			{
				id.offset = 1 + m_ProgramInfo.identifiers.back().offset;
			}
			++initStackUnit;
			m_ProgramInfo.identifiers.push_back(id);
		}

		// ***��ȡ�β���Ŀ***
		int paramCount = 0;
		for (uint32_t i = m_ProgramInfo.identifiers.size() - 1; !(m_ProgramInfo.identifiers[i].kind & ID_PROCEDURE); --i)
		{
			if (m_ProgramInfo.identifiers[i].kind & ID_PARAMETER)
				++paramCount;
		}

		// ***����ջ�ռ�***
		m_ProgramInfo.instructions.push_back({ Func_INT, 0, paramCount });

		for (auto& id : constants)
			m_ProgramInfo.identifiers.push_back(id);

		// <���>
		PraseStatement();

		// ***���أ�������ջ�ռ�***
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, 0 });

		// ***���̽����ص���һ��***
		--m_CurrLevel;
	}

	bool ProgramParser::PraseStatement()
	{
		try
		{
			//
			// <���>          ::= <�����>|<��ֵ���>|<�������>|<whileѭ�����>|<forѭ�����>|<���̵������>|<�����>|<д���>|<�������>|<�������>
			//

			// <�����>
			if (m_pCurrSymbol == m_pEndSymbol)
				return true;
			// <��ֵ���>
			else if (m_pCurrSymbol->symbolType == ST_Identifier)
				PraseAssignmentStat();
			// <�������>
			else if (m_pCurrSymbol->symbolType == ST_Begin)
				PraseComplexStat();
			// <�������>
			else if (m_pCurrSymbol->symbolType == ST_If)
				PraseConditionalStat();
			// <whileѭ�����>
			else if (m_pCurrSymbol->symbolType == ST_While)
				PraseWhileLoopStat();
			// <forѭ�����>
			else if (m_pCurrSymbol->symbolType == ST_For)
				PraseForLoopStat();
			// <�����>
			else if (m_pCurrSymbol->symbolType == ST_Read)
				PraseReadStat();
			// <д���>
			else if (m_pCurrSymbol->symbolType == ST_Write)
				PraseWriteStat();
			// <���̵������>
			else if (m_pCurrSymbol->symbolType == ST_Call)
				PraseCallStat();
			// <�������>
			else if (m_pCurrSymbol->symbolType == ST_Return)
				PraseReturnStat();
		}
		catch (std::exception)
		{
			if (m_pCurrSymbol != m_pEndSymbol)
			{
				// ������ǰ���
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
		// <��ֵ���>      ::= <id> :=|+=|-=|*=|/=|%= <���ʽ>
		//

		// <id>
		const std::string& id = m_pCurrSymbol->word;
		int idLevel;
		uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);
		// ***����id���������Ƿ����д��***
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
		



		// <���ʽ>
		PraseExpressionL1();

		// �ǳ��渳ֵ����Ҫ����һ�ζ�Ԫ����
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
		// <�������>      ::= begin <���>{;<���>} end
		//

		// begin
		++m_pCurrSymbol;

		// <���>
		PraseStatement();

		//
		// {;<���>}
		//

		// ;
		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_SemiColon)
		{
			++m_pCurrSymbol;

			// <���>
			PraseStatement();
		}

		// end
		SafeCheck(ST_End, 26);		// [Error026] Keyword 'end' expected.
		++m_pCurrSymbol;
	}

	void ProgramParser::PraseConditionalStat()
	{
		//
		// <�������>      ::= if <����> then <���> {else if <����> then <���>}[else <���>]
		//

		std::forward_list<int> trueList;	
		std::forward_list<int> falseList;
		int trueLength = 0, falseLength = 0;
		// if
		++m_pCurrSymbol;

		// <����>
		PraseConditionL1();

		falseList.push_front(m_ProgramInfo.instructions.size());
		m_ProgramInfo.instructions.push_back({Func_JPC, 0, 0});	// ��ַ������
		++falseLength;
		
		// then
		SafeCheck(ST_Then, 27);		// [Error027] Keyword 'then' expected.
		++m_pCurrSymbol;
		
		// <���>
		PraseStatement();

		//
		// {else if <����> then <���>}[else <���>]
		//


		// else
		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Else)
		{
			++m_pCurrSymbol;

			trueList.push_front(m_ProgramInfo.instructions.size());
			m_ProgramInfo.instructions.push_back({ Func_JMP, 0, 0 }); // ��ַ������
			++trueLength;

			// if
			if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_If)
			{
				++m_pCurrSymbol;

				// <����>
				PraseConditionL1();

				falseList.push_front(m_ProgramInfo.instructions.size());
				m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });	// ��ַ������
				++falseLength;

				// then
				SafeCheck(ST_Then, 27);		// [Error027] Keyword 'then' expected.
				++m_pCurrSymbol;

				// <���>
				PraseStatement();
			}
			else
			{
				// <���>
				PraseStatement();
				break;
			}
		}

		int currOffset = m_ProgramInfo.instructions.size();
		
		// ***��/������ַ����(�������ȿ��ܱ�������1)***
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
		// <whileѭ�����> ::= while <����> do <���>
		//

		// while
		++m_pCurrSymbol;

		int startLoopOffset = m_ProgramInfo.instructions.size();

		// <����>
		PraseConditionL1();

		// ***�������ַ***
		int falseIndex = m_ProgramInfo.instructions.size();
		m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });

		// do
		SafeCheck(ST_Do, 28);		// [Error028] Keyword 'do' expected.
		++m_pCurrSymbol;

		// <���>
		PraseStatement();

		m_ProgramInfo.instructions.push_back({ Func_JMP, 0, startLoopOffset });
		// ***�����ַ***
		m_ProgramInfo.instructions[falseIndex].mix = 
			m_ProgramInfo.instructions.size();

	}

	void ProgramParser::PraseForLoopStat()
	{
		//
		// <forѭ�����>   ::= for <id>:=<���ʽ> step <���ʽ> until <����> do <���>
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

		// <���ʽ>
		PraseExpressionL1();

		m_ProgramInfo.instructions.push_back({ Func_STO, m_CurrLevel - idLevel, idOffset });

		// step
		// ***����Ҫ���ж�������ִ�����飬�ٽ��и��£���Ҫ�����ⲿ��ָ��***
		SafeCheck(ST_Step, 29);		// [Error029] Keyword 'step' expected.
		++m_pCurrSymbol;

		uint32_t currInstructionCount = m_ProgramInfo.instructions.size();
		std::vector<Instruction> temp;

		// <���ʽ>
		PraseExpressionL1();

		// ***��ʱ������Щ���ʽ***
		temp.insert(temp.cbegin(), m_ProgramInfo.instructions.cbegin() + currInstructionCount,
			m_ProgramInfo.instructions.cend());
		m_ProgramInfo.instructions.erase(m_ProgramInfo.instructions.cbegin() + currInstructionCount);

		// until
		SafeCheck(ST_Until, 30);	// [Error030] Keyword 'until' expected.
		++m_pCurrSymbol;

		int startLoopOffset = m_ProgramInfo.instructions.size();

		// <����>
		PraseConditionL1();

		// ***��������Ϊ��ʱ�˳�ѭ������������Ҫ�߼�ȡ��***
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_NOT });
		// ***�������ַ***
		int falseIndex = m_ProgramInfo.instructions.size();
		m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });

		// do
		SafeCheck(ST_Do, 28);	// [Error028] Keyword 'do' expected.
		++m_pCurrSymbol;

		// <���>
		PraseStatement();

		// ***ִ��STEP***
		m_ProgramInfo.instructions.push_back({ Func_LOD, m_CurrLevel - idLevel, idOffset });
		m_ProgramInfo.instructions.insert(m_ProgramInfo.instructions.cend(),
			temp.cbegin(), temp.cend());
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_ADD });
		m_ProgramInfo.instructions.push_back({ Func_STO, m_CurrLevel - idLevel, idOffset });
		m_ProgramInfo.instructions.push_back({ Func_JMP, 0, startLoopOffset });
		// ***�����ַ***
		m_ProgramInfo.instructions[falseIndex].mix =
			m_ProgramInfo.instructions.size();
	}

	void ProgramParser::PraseReadStat()
	{
		//
		// <�����>        ::= read '('<id>{,<id>}')'
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
		// ***������������Ƿ����д��***
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
			// ***������������Ƿ����д��***
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
		// <д���>        ::= write '('<���ʽ>{,<���ʽ>}')'
		//

		// write
		++m_pCurrSymbol;

		// '('
		SafeCheck(ST_LeftParen, 24);		// [Error024] '(' Expected.
		++m_pCurrSymbol;

		// <���ʽ>
		PraseExpressionL1();

		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_PRT });

		//
		// {,<���ʽ>}
		//

		// ,
		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
		{
			++m_pCurrSymbol;

			// <���ʽ>
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
		// <���̵������>  ::= call <id>'('[<���ʽ>{,<���ʽ>}]')'
		//

		// call
		++m_pCurrSymbol;

		int paramCount = 0;

		// <id>
		const std::string& id = m_pCurrSymbol->word;
		int idLevel;
		uint32_t pos = GetIDIndex(m_pCurrSymbol->word, &idLevel);
		// ***����id���������Ƿ�Ϊ����***
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
		// [<���ʽ>{,<���ʽ>}]')'
		//

		// <���ʽ>
		if (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType != ST_RightParen)
		{
			PraseExpressionL1();
			++paramCount;

			// {,<���ʽ>}

			// ,
			while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_Comma)
			{
				++m_pCurrSymbol;

				// <���ʽ>
				PraseExpressionL1();
				++paramCount;
			}
		}

		// ')'
		SafeCheck(ST_RightParen, 25);		// [Error025] ')' Expected.
		++m_pCurrSymbol;

		// ***�鿴������ʵ���β���Ŀ***
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

		// ***�˱�Ϊ�β��ó��ռ�(���ı�ջ��ָ�룬���Ƴ����������)***
		m_ProgramInfo.instructions.push_back({ Func_POP, 0, paramCount });

		// ***�����ú���***
		m_ProgramInfo.instructions.push_back({ Func_CAL, m_CurrLevel - idLevel,
		m_ProgramInfo.identifiers[pos].offset });
	}

	void ProgramParser::PraseReturnStat()
	{
		//
		// <�������>      ::= return
		//
		++m_pCurrSymbol;
		m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_RET });
	}

	void ProgramParser::PraseConditionL1()
	{
		//
		// <����>          ::= <��������>{<�߼���><��������>}
		//

		// <��������>
		PraseConditionL2();

		//
		// {<�߼���><��������>}
		//

		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_LogicalOr)
			return;

		std::forward_list<int> trueList;

		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_LogicalOr)
		{
			// <�߼���>
			++m_pCurrSymbol;

			// ***��¼����***
			
			m_ProgramInfo.instructions.push_back({ Func_OPR, 0, Opr_NOT });
			trueList.push_front(m_ProgramInfo.instructions.size());
			m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0 });

			// <��������>
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
		// <��������>      ::= <��������>{<�߼���><��������>}
		//

		// <��������>
		PraseConditionL3();

		//
		// {<�߼���><��������>}
		//

		if (m_pCurrSymbol == m_pEndSymbol || m_pCurrSymbol->symbolType != ST_LogicalAnd)
			return;

		std::forward_list<int> falseList;

		while (m_pCurrSymbol != m_pEndSymbol && m_pCurrSymbol->symbolType == ST_LogicalAnd)
		{
			// <�߼���>
			++m_pCurrSymbol;

			// ***��¼����***
			falseList.push_front(m_ProgramInfo.instructions.size());
			m_ProgramInfo.instructions.push_back({ Func_JPC, 0, 0});

			// <��������>
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
		// <��������>      ::= <�ļ�����>{!=|= <�ļ�����>}
		//

		// <�ļ�����>
		PraseConditionL4();

		//
		// {!=|= <�ļ�����>}
		//

		while (m_pCurrSymbol != m_pEndSymbol && 
			(m_pCurrSymbol->symbolType == ST_NotEqual || m_pCurrSymbol->symbolType == ST_Equal))
		{
			// !=|=
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <�ļ�����>
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
		// <�ļ�����>      ::= <���ʽ>{>|>=|<|<= <���ʽ>}
		//

		// <���ʽ>
		PraseExpressionL1();

		//
		// {>|>=|<|<= <���ʽ>}
		//

		while (m_pCurrSymbol != m_pEndSymbol &&
			(m_pCurrSymbol->symbolType == ST_Greater || m_pCurrSymbol->symbolType == ST_GreaterEqual ||
				m_pCurrSymbol->symbolType == ST_Less || m_pCurrSymbol->symbolType == ST_LessEqual))
		{
			// >|>=|<|<=
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <���ʽ>
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
		// <���ʽ>        ::= <�������ʽ>{+|- <�������ʽ>}
		//

		// <�������ʽ>
		PraseExpressionL2();

		//
		// {+|- <�������ʽ>}
		//
		while (m_pCurrSymbol != m_pEndSymbol &&
			(m_pCurrSymbol->symbolType == ST_Plus || m_pCurrSymbol->symbolType == ST_Minus))
		{
			// +|-
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <�������ʽ>
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
		// <�������ʽ>    ::= <�������ʽ>{*|/|% <�������ʽ>}
		//

		// <�������ʽ>
		PraseExpressionL3();

		//
		// {*|/|% <�������ʽ>}
		//
		while (m_pCurrSymbol != m_pEndSymbol &&
			(m_pCurrSymbol->symbolType == ST_Multiply || m_pCurrSymbol->symbolType == ST_Divide ||
				m_pCurrSymbol->symbolType == ST_Mod))
		{
			// *|/|%
			SymbolType type = m_pCurrSymbol->symbolType;
			++m_pCurrSymbol;

			// <�������ʽ>
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
		// <�������ʽ>    ::= [!|-|+]<�ļ����ʽ>
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

		// <�ļ����ʽ>
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
		// <�ļ����ʽ>          ::= <id>|<value>|'('<����>')'
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
			// ***������ָ��LIT��������ָ��LOD***
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

			// <����>
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
		// ����ǰ�Ѿ�ɨ�������������У����ߵ�ǰ�����ķ���������ʵ�ʵĲ�һ�£����¼������Ϣ���׳��쳣
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
		// ���鵱ǰ�����Ƿ��и÷���
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

		// �����������Ƿ��и÷���
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

		// �����Ƿ�Ϊ����
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

