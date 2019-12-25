#pragma once

#include <array>
#include <vector>
#include <string>
#include <set>
#include <forward_list>
#include <stack>

namespace PL0
{

	//
	// 语法单位
	//

	// <程序>          ::= <常量说明部分><变量说明部分><过程说明部分><语句>.

	// <常量说明部分>  ::= {const <值类型><常量定义>{,<常量定义>};}
	// <常量定义>      ::= <id>:=<值>

	// <变量说明部分>  ::= {<值类型><变量定义>{,<变量定义>};}
	// <变量定义>      ::= <id>[:=<值>]

	// <过程说明部分>  ::= <过程首部><分程序>;{<过程说明部分>}
	// <过程首部>      ::= procedure <id>'('[<值类型><id>{,<值类型><id>}]')';
	// <分程序>        ::= <常量说明部分><变量说明部分><语句>

	// <语句>          ::= <空语句>|<赋值语句>|<条件语句>|<while循环语句>|<for循环语句>|<过程调用语句>|<读语句>|<写语句>|<返回语句>|<复合语句>
	// <赋值语句>      ::= <id> :=|+=|-=|*=|/=|%= <表达式>
	// <复合语句>      ::= begin <语句>{;<语句>} end
	// <空语句>        ::= ε
	// <条件语句>      ::= if <条件> then <语句> {else if <条件> then <语句>}[else <语句>]
	// <while循环语句> ::= while <条件> do <语句>
	// <for循环语句>   ::= for <id>:=<表达式> step <表达式> until <条件> do <语句>
	// <读语句>        ::= read '('<id>{,<id>}')'
	// <写语句>        ::= write '('<表达式>{,<表达式>}')'
	// <过程调用语句>  ::= call <id>'('[<表达式>{,<表达式>}]')'
	// <返回语句>      ::= return

	// <条件>          ::= <二级条件>{<逻辑或><二级条件>}
	// <二级条件>      ::= <三级条件>{<逻辑与><三级条件>}
	// <三级条件>      ::= <四级条件>{!=|= <四级条件>}
	// <四级条件>      ::= <表达式>{>|>=|<|<= <表达式>}
	// <表达式>        ::= <二级表达式>{+|- <二级表达式>}
	// <二级表达式>    ::= <三级表达式>{*|/|% <三级表达式>}
	// <三级表达式>    ::= [!|-|+]<四级表达式>
	// <四级表达式>    ::= <id>|<value>|'('<条件>')'




	//
	// 指令集
	//

	// i 指令寄存器
	// p 指令地址寄存器
	// t 栈顶寄存器
	// b 基址寄存器

	// 过程调用相关指令
	// POP 0 N         退避N个存储单元，用于函数形参(仅改变栈顶指针，不移除上面的内容)
	// INT 0 N         在栈顶开辟N个存储单元
	// OPR 0 0         结束被调用过程，返回调用点并退栈
	// CAL L A         调用地址为A的过程，调用过程与被调用过程的层差为L

	// 存取指令
	// LIT 0 A         立即数存入栈顶，t加1
	// LOD L A         将层差为L，偏移量为A的存储单元的值取到栈顶，t加1
	// STO L A         将栈顶的值存入层差为L、偏移量为A的单元，t减1
	
	// 一元运算指令
	// OPR 0 1         栈顶 := -栈顶
	// OPR 0 2         栈顶 := !栈顶

	// 二元运算指令
	// OPR 0 3         次栈顶 := 次栈顶 + 栈顶，t减1
	// OPR 0 4         次栈顶 := 次栈顶 - 栈顶，t减1
	// OPR 0 5         次栈顶 := 次栈顶 * 栈顶，t减1
	// OPR 0 6         次栈顶 := 次栈顶 / 栈顶，t减1
	// OPR 0 7         次栈顶 := 次栈顶 % 栈顶，t减1

	// 二元比较指令
	// OPR 0 8         次栈顶 := (次栈顶 = 栈顶)，t减1
	// OPR 0 9         次栈顶 := (次栈顶 != 栈顶)，t减1
	// OPR 0 10        次栈顶 := (次栈顶 < 栈顶)，t减1
	// OPR 0 11        次栈顶 := (次栈顶 <= 栈顶)，t减1
	// OPR 0 12        次栈顶 := (次栈顶 > 栈顶)，t减1
	// OPR 0 13        次栈顶 := (次栈顶 >= 栈顶)，t减1

	// 转移指令
	// JMP 0 A         无条件转移至地址A
	// JPC 0 A         若栈顶为0，转移至地址A，t减1

	// 输入输出指令
	// OPR 0 14        栈顶的值输出至控制台屏幕，t减1
	// OPR 0 15        控制台屏幕输出一个换行
	// OPR 0 16        从控制台读入一行输入，植入栈顶，t加1


	// 常量

	constexpr size_t g_KeywordOffset = 100;		// 关键字相对于符号类型枚举值的偏移
	constexpr size_t g_KeywordCount = 20;		// 关键字数目
	constexpr size_t g_MaxIdentifierLength = 47;// 标识符最大长度
	constexpr size_t g_MaxIntegerLength = 10;	// 整数最大长度

	// 符号类型
	enum SymbolType
	{
		ST_Null = 0,			// 空
		ST_Error,				// 错误类型
		// 值类型
		ST_Identifier = 10,		// 标识符
		ST_Integer,				// 整型
		ST_Real,				// 实型
		ST_Character,			// 字符型
		ST_StringConst,			// 字符串常量

		// 算术运算符
		ST_Plus = 20,			// 加 +
		ST_Minus,				// 减 -
		ST_Multiply,			// 乘 *
		ST_Divide,				// 除 /
		ST_Mod,					// 取余 %

		// 赋值
		ST_Assign = 30,			// 赋值语句 :=
		ST_PlusAndAssign,		// 加后赋值 +=
		ST_MinusAndAssign,		// 减后赋值 -=
		ST_MultiplyAndAssign,	// 乘后赋值 *=
		ST_DivideAndAssign,		// 除后赋值 /=
		ST_ModAndAssign,		// 取余后赋值 %=

		// 比较运算符
		ST_Equal = 40,			// 判断lhs == rhs
		ST_NotEqual,			// 判断lhs != rhs
		ST_Less,				// 判断lhs < rhs
		ST_LessEqual,			// 判断lhs <= rhs
		ST_Greater,			    // 判断lhs > rhs
		ST_GreaterEqual,		// 判断lhs <= rhs

		// 逻辑运算符
		ST_LogicalAnd = 50,		// 逻辑与 &&
		ST_LogicalOr,			// 逻辑或 ||
		ST_LogicalNot,			// 逻辑非 !

		// 括号
		ST_LeftParen = 60,		// 小括号 左 (
		ST_RightParen,			// 小括号 右 )
		ST_LeftBracket,			// 中括号 左 [
		ST_RightBracket,		// 中括号 右 ]
		ST_LeftBrace,			// 大括号 左 {
		ST_RightBrace,			// 大括号 右 }

		// 分割符
		ST_Comma = 70,			// 逗号 ,
		ST_SemiColon,			// 分号 ;
		ST_Period,				// 句号 .


		// 注释
		ST_Comment = 80,		// 注释

		// 关键字
		ST_Begin = 100,			// 代码片段开始 begin
		ST_Call,				// 函数调用
		ST_Char,				// char声明语句
		ST_Const,				// 常量声明修饰符
		ST_Do,					// do语句
		ST_Else,				// else语句
		ST_End,					// 代码片段结束 end
		ST_Float,				// float声明语句
		ST_For,					// for语句
		ST_If,					// if语句	
		ST_Int,					// int声明语句
		ST_Procedure,			// 程序声明修饰符
		ST_Read,				// read语句
		ST_Return,				// return语句
		ST_Step,				// step语句
		ST_String,				// string语句
		ST_Then,				// then语句
		ST_Until,				// until语句
		ST_While,				// while语句
		ST_Write,				// write语句
	};

	// 功能类型
	enum FuncType
	{
		Func_LIT,	// 取立即数
		Func_OPR,	// 操作
		Func_LOD,	// 读取
		Func_STO,	// 保存
		Func_CAL,	// 调用
		Func_INT,	// 初始化空间
		Func_POP,	// 退栈
		Func_JMP,	// 无条件跳转
		Func_JPC	// 有条件跳转
	};

	// 操作类型
	enum OprType
	{
		Opr_RET,		// 过程返回
		Opr_NEG,		// 栈顶 := -栈顶
		Opr_NOT,		// 栈顶 := !栈顶
		Opr_ADD,		// 次栈顶 := 次栈顶 + 栈顶，t减1
		Opr_SUB,		// 次栈顶 := 次栈顶 - 栈顶，t减1
		Opr_MUL,		// 次栈顶 := 次栈顶 * 栈顶，t减1
		Opr_DIV,		// 次栈顶 := 次栈顶 / 栈顶，t减1
		Opr_MOD,		// 次栈顶 := 次栈顶 % 栈顶，t减1
		Opr_EQU,		// 次栈顶 := (次栈顶 = 栈顶)，t减1
		Opr_NEQ,		// 次栈顶 := (次栈顶 != 栈顶)，t减1
		Opr_LES,		// 次栈顶 := (次栈顶 < 栈顶)，t减1
		Opr_LEQ,		// 次栈顶 := (次栈顶 <= 栈顶)，t减1
		Opr_GTR,		// 次栈顶 := (次栈顶 > 栈顶)，t减1
		Opr_GEQ,		// 次栈顶 := (次栈顶 >= 栈顶)，t减1
		Opr_PRT,		// 栈顶的值输出至控制台屏幕，t减1
		Opr_PNL,		// 控制台屏幕输出一个换行
		Opr_SCN			// 从控制台读取输入，植入栈顶，t加1
	};

	// 标识符类型
	enum IDType
	{
		ID_INT = 0x1,
		ID_FLOAT = 0x2,
		ID_PROCEDURE = 0x4,

		ID_CONST = 0x10,
		ID_VAR = 0x20,
		ID_PARAMETER = 0x40
	};

	// 关键字字符串
	const char* const g_KeyWords[g_KeywordCount] = {
		"begin",
		"call",
		"char",
		"const",
		"do",
		"else",
		"end",
		"float",
		"for",
		"if",
		"int",
		"procedure",
		"read",
		"return",
		"step",
		"string",
		"then",
		"until",
		"while",
		"write"
	};

	// 错误码
	using ErrorCode = uint32_t;

	// 代码坐标(从1开始)
	struct CodeCoord
	{
		size_t row;
		size_t col;
	};

	// 符号(完成词法分析)
	struct Symbol
	{
		std::string word;
		CodeCoord beg, end;
		SymbolType symbolType;
	};

	// 标识符(完成语法分析)
	struct Identifier
	{
		char name[g_MaxIdentifierLength + 1];	// 名称
		uint32_t kind;		// 符号类型
		int value;			// 值
		int level;			// 层级
		int offset;			// 地址偏移
	};

	// 指令
	struct Instruction
	{
		uint32_t func;		// 功能
		int level;			// 层级差
		int mix;			// 地址偏移/立即数/指令
		int reserved;		// 保留字(暂时无用)
	};

	// 程序信息
	struct ProgramInfo
	{
		// [4字节]标识符起始位置 [4字节]标识符数目 [4字节]指令起始位置 [4字节]指令数目
		// {
		//   [64字节] 过程标识符
		//   { [64字节] 该过程的形参}
		//   { [64字节] 该过程的变量}
		// }
		// 
		// {[16字节] 指令}

		std::vector<Identifier> identifiers;	// 标识符
		std::vector<Instruction> instructions;	// 指令
	};

	// 环境信息
	struct EnvironmentInfo
	{
		int pr;								// 指令地址寄存器
		int tr;								// 栈顶寄存器
		int br;								// 基址寄存器
		Instruction ir;						// 指令寄存器
		std::vector<int> dataStack;			// 数据栈区
		std::stack<int> prStack;			// 指令地址寄存栈
		std::stack<int> trStack;			// 栈顶寄存栈
		std::stack<int> brStack;			// 基址寄存栈
		std::vector<std::string> funcStack;	// 函数调用栈
	};

	// 错误信息
	struct ErrorInfo
	{
		CodeCoord beg, end;
		ErrorCode errorCode;
	};

}