#pragma once

#include "PL0_Common.h"

namespace PL0
{
	//
	// �ʷ�������
	//
	class WordParser
	{
	public:
		WordParser() : m_pCurrContent(), m_CurrCoord() {}
		~WordParser() = default;

		// �����ݽ��������Ĵʷ�����
		bool Parse(_In_ const std::string& content);
		// �����ݽ��������Ĵʷ�����
		bool Parse(_In_ const char* content);

		// ��ȡ������������з���
		const std::vector<Symbol>& GetSymbols() const;

		// ��ȡ�ʷ��������ֵĴ�����Ϣ
		const std::vector<ErrorInfo>& GetErrorInfos() const;

	private:
		// ��ȡ��һ�����ţ����ƽ������е�����
		Symbol GetNextSymbol(_Out_ ErrorInfo * errorInfo);

		// ��ȡ��һ���ַ������ƽ������е�����
		char GetNextChar();
		// ������һ���ַ������ƽ������е�����
		void IgnoreNextChar();
		// ��ȡ��һ���ַ��������ƽ������е�����
		char PeekNextChar();

		// ����һ���ַ����õ��������ĩβλ�ã������ƽ������е�����
		// ��content = "\x12\' + " ����EndPos = "\' + "
		// ��content = "e\"" �� ��EndPos = "\""
		void TryPraseChar(_In_ const char* content, _Out_ const char** pEndCharPos);

	private:
		const char * m_pCurrContent;			// ��ǰ�����Ĵ�����������λ��
		std::vector<Symbol> m_Symbols;			// ���ż�
		CodeCoord m_CurrCoord;					// ��ǰ����
		std::vector<ErrorInfo> m_ErrorInfos;	// ������Ϣ
	};
}

