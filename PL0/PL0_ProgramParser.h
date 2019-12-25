#pragma once

#include "PL0_Common.h"

namespace PL0
{
	//
	// �﷨/���������
	//
	class ProgramParser
	{
	public:
		ProgramParser() : m_CurrLevel(), m_CurrProcIndex(), m_CurrProcAddressOffset() {}
		~ProgramParser() = default;
	
		
		// �﷨��λ

		// <����>          ::= <����˵������><����˵������><����˵������><���>.

		// <����˵������>  ::= {const <ֵ����><��������>{,<��������>};}
		// <��������>      ::= <id>:=<ֵ>

		// <����˵������>  ::= {<ֵ����><��������>{,<��������>};}
		// <��������>      ::= <id>[:=<ֵ>]
		
		// <����˵������>  ::= <�����ײ�><�ֳ���>;{<����˵������>}
		// <�����ײ�>      ::= procedure <id>'('[<ֵ����><id>{,<ֵ����><id>}]')';
		// <�ֳ���>        ::= <����˵������><����˵������><���>
	
		// <���>          ::= <�����>|<��ֵ���>|<�������>|<whileѭ�����>|<forѭ�����>|<���̵������>|<�����>|<д���>|<�������>|<�������>
		// <��ֵ���>      ::= <id> :=|+=|-=|*=|/=|%= <���ʽ>
		// <�������>      ::= begin <���>{;<���>} end
		// <�����>        ::= ��
		// <�������>      ::= if <����> then <���> {else if <����> then <���>}[else <���>]
		// <whileѭ�����> ::= while <����> do <���>
		// <forѭ�����>   ::= for <id>:=<���ʽ> step <���ʽ> until <����> do <���>
		// <�����>        ::= read '('<id>{,<id>}')'
		// <д���>        ::= write '('<���ʽ>{,<���ʽ>}')'
		// <���̵������>  ::= call <id>'('[<���ʽ>{,<���ʽ>}]')'
		// <�������>      ::= return
		
		// <���ʽ>        ::= [+|-]<��>{<�Ӽ������><��>}
		// <��>            ::= <����>{<�˳�ģ�����><����>}
		// <����>          ::= <id>|<value>|'('<���ʽ>')'

		// <����>          ::= <��������>{<�߼���><��������>}
		// <��������>      ::= <��������>{<�߼���><��������>}
		// <��������>      ::= <�ļ�����>{!=|= <�ļ�����>}
		// <�ļ�����>      ::= <���ʽ>{>|>=|<|<= <���ʽ>}
		// <���ʽ>        ::= <�������ʽ>{+|- <�������ʽ>}
		// <�������ʽ>    ::= <�������ʽ>{*|/|% <�������ʽ>}
		// <�������ʽ>    ::= [!|-|+]<�ļ����ʽ>
		// <�ļ����ʽ>    ::= <id>|<value>|'('<����>')'


		// �Է����õĴ�������﷨����
		bool Parse(_In_ const std::vector<Symbol>& symbols);

		// ��ȡ�﷨����������ֵĴ�����Ϣ
		const std::vector<ErrorInfo>& GetErrorInfos() const;

		// ��ȡ������
		const ProgramInfo& GetProgramInfo() const;

	private:

		// <����>          ::= <����˵������><����˵������><����˵������><���>.
		bool PraseProgram();
		
		// <����˵������>  ::= {const <ֵ����><��������>{,<��������>};}
		bool PraseConstDesc(_Inout_ std::vector<Identifier>& constants);
		// <��������>      ::= <id>:=<ֵ>
		void PraseConstDef(_In_ SymbolType type, _Inout_ std::vector<Identifier>& constants);

		// <����˵������>  ::= {<ֵ����><��������>{,<��������>};}
		bool PraseVarDesc(_Inout_ std::vector<Identifier>& variables);
		// <��������>      ::= <id>[:=<ֵ>]
		void PraseVarDef(_In_ SymbolType type, _Inout_ std::vector<Identifier>& variables);

		// <����˵������>  ::= <�����ײ�><�ֳ���>;{<����˵������>}
		bool PraseProcedureDesc();
		// <�����ײ�>      ::= procedure <id>'('[<ֵ����><id>{,<ֵ����><id>}]')';
		void PraseProcedureHeader();
		// <�ֳ���>        ::= <����˵������><����˵������><���>
		void PraseSubProcedure();

		// <���>          ::= <�����>|<��ֵ���>|<�������>|<whileѭ�����>|<forѭ�����>|<���̵������>|<�����>|<д���>|<�������>|<�������>
		bool PraseStatement();
		// <��ֵ���>      ::= <id> :=|+=|-=|*=|/=|%= <���ʽ>
		void PraseAssignmentStat();
		// <�������>      ::= begin <���>{;<���>} end
		void PraseComplexStat();
		// <�������>      ::= if <����> then <���> {else if <����> then <���>}[else <���>]
		void PraseConditionalStat();
		// <whileѭ�����> ::= while <����> do <���>
		void PraseWhileLoopStat();
		// <forѭ�����>   ::= for <id>:=<���ʽ> step <���ʽ> until <����> do <���>
		void PraseForLoopStat();
		// <�����>        ::= read '('<id>{,<id>}')'
		void PraseReadStat();
		// <д���>        ::= write '('<���ʽ>{,<���ʽ>}')'
		void PraseWriteStat();
		// <���̵������>  ::= call <id>'('[<���ʽ>{,<���ʽ>}]')'
		void PraseCallStat();
		// <�������>      ::= return
		void PraseReturnStat();

		// <����>          ::= <��������>{<�߼���><��������>}
		void PraseConditionL1();
		// <��������>      ::= <��������>{<�߼���><��������>}
		void PraseConditionL2();
		// <��������>      ::= <�ļ�����>{!=|= <�ļ�����>}
		void PraseConditionL3();
		// <�ļ�����>      ::= <���ʽ>{>|>=|<|<= <���ʽ>}
		void PraseConditionL4();
		// <���ʽ>        ::= <�������ʽ>{+|- <�������ʽ>}
		void PraseExpressionL1();
		// <�������ʽ>    ::= <�������ʽ>{*|/|% <�������ʽ>}
		void PraseExpressionL2();
		// <�������ʽ>    ::= [!|-|+]<�ļ����ʽ>
		void PraseExpressionL3();
		// <�ļ����ʽ>    ::= <id>|<value>|'('<����>')'
		void PraseExpressionL4();

	private:
		// ��鵱ǰ�����Ƿ�ΪST��������ǣ�����������벢�׳��쳣
		void SafeCheck(SymbolType ST, ErrorCode errorCode, const char* errorStr = "");
		// ��ȡid�ڷ��ű��е������Ͳ㼶
		uint32_t GetIDIndex(const std::string& str, int * outLevel = nullptr);

	private:
		int m_CurrLevel;	// ��ǰ����㼶
		ProgramInfo m_ProgramInfo;	// ������Ϣ

		uint32_t m_CurrProcIndex;			// ��ǰ���̱�ʶ���Ķ�Ӧ����
		uint32_t m_CurrProcAddressOffset;	// ��ǰ���̶�Ӧ��ʼ��ַƫ��ֵ
		std::vector<Symbol>::const_iterator m_pCurrSymbol;	// ��ǰ���ŵĵ�����
		std::vector<Symbol>::const_iterator m_pEndSymbol;	// ���ż�β��ĵ�����
		
		std::vector<ErrorInfo> m_ErrorInfos;	// ������Ϣ
		
	};
}