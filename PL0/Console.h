#pragma once
#include <Windows.h>
#include <cstdio>
#include <memory>




class Console
{
public:
	enum Color
	{
		Black = 0,
		DarkBlue = 1,
		DarkGreen = 2,
		DarkCyan = 3,
		DarkRed = 4,
		DarkMagenta = 5,
		DarkYellow = 6,
		DarkWhite = 7,
		Gray = 8,
		Blue = 9,
		Green = 10,
		Cyan = 11,
		Red = 12,
		Magenta = 13,
		Yellow = 14,
		White = 15
	};


#pragma region 控制台输出方法
	/// <summary>
	/// 输出字符串
	/// </summary>
	static int Write(const char * Str) noexcept;

	/// <summary>
	/// 输出字符
	/// </summary>
	static int WriteChar(const char Ch) noexcept;

	/// <summary>
	/// 格式化输出
	/// </summary>
	static int WriteFormat(const char * const Format, ...) noexcept;

	/// <summary>
	/// 输出字符串后换行
	/// </summary>
	static int WriteLine(const char * Str) noexcept;

	/// <summary>
	/// 指定缓冲区位置输出字符串，不会改变光标位置
	/// </summary>
	/// <param name = "X">缓冲区X坐标位置(0开始)</param>
	/// <param name = "Y">缓冲区Y坐标位置(0开始)</param>
	/// <return>成功输出的字符数目</return>
	static int WriteBuffer(short X, short Y, const char * Str) noexcept;


#pragma endregion

#pragma region 控制台输入方法
	/// <summary>
	/// 格式化输入
	/// </summary>
	static int ReadFormat(const char * const Format, ...) noexcept;

	/// <summary>
	/// 输入字符串
	/// </summary>
	static char * ReadLine(char * const Buffer, size_t BufferCount) noexcept;

	/// <summary>
	/// 输入字符
	/// </summary>
	static char ReadChar() noexcept;
#pragma endregion

#pragma region 控制台获取方法(需单例实例化)
	/// <summary>
	/// 获取控制台的单例(需要先实例化)
	/// </summary>
	static Console& Get();

	/// <summary>
	/// 获取控制台缓冲区内容
	/// </summary>
	static BOOL GetBuffer(CHAR_INFO* pBufferOut);


	/// <summary>
	/// 获取控制台缓冲区的宽度和高度
	/// </summary>
	static COORD GetBufferSize();

	/// <summary>
	/// 获取控制台光标的位置坐标
	/// </summary>
	static COORD GetCursorPostion();

	/// <summary>
	/// 获取控制台光标的大小
	/// </summary>
	static DWORD GetCursorSize();

	/// <summary>
	/// 获取控制台光标的可见性
	/// </summary>
	static BOOL GetCursorVisible();

	/// <summary>
	/// 获取控制台输入记录
	/// </summary>
	static INPUT_RECORD GetInputRecord();

	/// <summary>
	/// 获取控制台输入的代码页
	/// </summary>
	static UINT GetInputCodePage();

	/// <summary>
	/// 获取控制台输出的代码页
	/// </summary>
	static UINT GetOutputCodePage();

	/// <summary>
	/// 获取控制台窗口名
	/// </summary>
	/// <param name = "Buffer">[Out]用于输出窗口名的缓冲区</param>
	/// <param name = "BufferCount">[In]缓冲区元素个数</param>
	/// <return>返回错误码</return>
	static errno_t GetWindowName(char * Buffer, size_t BufferCount);



#pragma endregion

#pragma region 控制台修改方法(需单例实例化)
	/// <summary>
	/// 设置控制台缓冲区的宽度和高度
	/// </summary>
	/// <param name = "Width">缓冲区宽度</param>
	/// <param name = "Height">缓冲区高度</param>
	/// <return>设置是否成功</return>
	static BOOL SetBufferSize(short Width, short Height);

	/// <summary>
	/// 设置光标厚度和可见性
	/// </summary>
	/// <param name = "Weight">光标自底向上的覆盖程度(1-100)</param>
	/// <param name = "Visible">光标可见性</param>
	/// <return>设置是否成功</return>
	static BOOL SetCursorInfo(unsigned Weight, bool Visible);

	/// <summary>
	/// 设置光标位置
	/// </summary>
	/// <param name = "X">光标水平位置(从0开始)</param>
	/// <param name = "Y">光标竖直位置(从0开始</param>
	/// <return>设置是否成功</return>
	static BOOL SetCursorPos(short X, short Y);

	/// <summary>
	/// 设置控制台输入代码页
	/// </summary>
	/// <param name = "CodePage">控制台输入代码页</param>
	/// <return>设置是否成功</return>
	static BOOL SetInputCodePage(unsigned CodePage);

	/// <summary>
	/// 设置控制台输出代码页
	/// </summary>
	/// <param name = "CodePage">控制台输出代码页</param>
	/// <return>设置是否成功</return>
	static BOOL SetOutputCodePage(unsigned CodePage);

	/// <summary>
	/// 设置文本颜色和背景颜色
	/// </summary>
	/// <param name = "foreGround">文本颜色</param>
	/// <param name = "backGround">背景颜色</param>
	/// <return>设置是否成功</return>
	static BOOL SetTextColor(Color foreGround = Color::DarkWhite, Color backGround = Color::Black);

	/// <summary>
	/// 设置控制台标题
	/// </summary>
	/// <param name = "Title">控制台标题</param>
	/// <return>设置是否成功</return>
	static BOOL SetTitle(const char * Title);

	/// <summary>
	/// 设置窗口大小
	/// </summary>
	/// <param name = "Width">控制台窗口宽度</param>
	/// <param name = "Height">控制台窗口高度</param>
	/// <return>设置是否成功</return>
	static BOOL SetWindowSize(short Width, short Height);
#pragma endregion

	Console& operator=(const Console& CopyFrom) = delete;
	Console& operator=(Console&& MoveFrom) noexcept;
	Console(const Console& CopyFrom) = delete;
	Console(Console&& MoveFrom) noexcept;
	Console();
	~Console();

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};


