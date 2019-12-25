#pragma once

#include "PL0_Common.h"

namespace PL0
{
	//
	// 词法分析器
	//
	class WordParser
	{
	public:
		WordParser() : m_pCurrContent(), m_CurrCoord() {}
		~WordParser() = default;

		// 对内容进行完整的词法分析
		bool Parse(_In_ const std::string& content);
		// 对内容进行完整的词法分析
		bool Parse(_In_ const char* content);

		// 获取分析处理的所有符号
		const std::vector<Symbol>& GetSymbols() const;

		// 获取词法分析发现的错误信息
		const std::vector<ErrorInfo>& GetErrorInfos() const;

	private:
		// 获取下一个符号，并推进分析中的内容
		Symbol GetNextSymbol(_Out_ ErrorInfo * errorInfo);

		// 获取下一个字符，并推进分析中的内容
		char GetNextChar();
		// 抛弃下一个字符，并推进分析中的内容
		void IgnoreNextChar();
		// 获取下一个字符，但不推进分析中的内容
		char PeekNextChar();

		// 分析一个字符，得到分析完的末尾位置，但不推进分析中的内容
		// 如content = "\x12\' + " ，则EndPos = "\' + "
		// 如content = "e\"" ， 则EndPos = "\""
		void TryPraseChar(_In_ const char* content, _Out_ const char** pEndCharPos);

	private:
		const char * m_pCurrContent;			// 当前遍历的代码内容所处位置
		std::vector<Symbol> m_Symbols;			// 符号集
		CodeCoord m_CurrCoord;					// 当前坐标
		std::vector<ErrorInfo> m_ErrorInfos;	// 错误消息
	};
}

