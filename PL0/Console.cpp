#include "Console.h"
#include <thread>


// 句柄删除仿函数类
struct HandleCloser
{
	void operator()(HANDLE h) { CloseHandle(h); }
};

// 作用域内句柄
typedef std::unique_ptr<void, HandleCloser> ScopedHandle;





class Console::Impl
{
public:
	Impl(Console* pOwner);
	~Impl();
#pragma region 控制台输出方法(需单例实例化)
	int WriteBuffer(short X, short Y, const char* str);
#pragma endregion

#pragma region 控制台获取方法(需单例实例化)
	BOOL GetBuffer(CHAR_INFO* pBufferOut) { SMALL_RECT rc{0,0,mBufferInfo.dwSize.X-1,mBufferInfo.dwSize.Y-1}; return ReadConsoleOutputA(hOut.get(), pBufferOut, mBufferInfo.dwSize, COORD{}, &rc); }
	COORD GetBufferSize() { return mBufferInfo.dwSize; }
	COORD GetCursorPosition() { return mBufferInfo.dwCursorPosition; }
	DWORD GetCursorSize() { return mCursorInfo.dwSize; }
	BOOL GetCursorVisible() { return mCursorInfo.bVisible; }
	INPUT_RECORD GetInputRecord() { DWORD n = PeekConsoleInputW(hIn.get(), &mInputRecord, sizeof(INPUT_RECORD), &n); return mInputRecord; }
	UINT GetInputCodePage() { return mInputCodePage; }
	UINT GetOutputCodePage() { return mOutputCodePage; }
	errno_t GetWindowName(char* Buffer, size_t BufferCount) { return strcpy_s(Buffer, BufferCount, mTitle); }
#pragma endregion

#pragma region 控制台修改方法(需单例实例化)
	BOOL SetBufferSize(short Width, short Height) 
	{ 
		COORD dwSize{ Width, Height };
		BOOL res = SetConsoleScreenBufferSize(hOut.get(), dwSize);
		if (res)
			mBufferInfo.dwSize = dwSize;
		return res;
	}
	BOOL SetCursorInfo(unsigned Weight, bool Visible)
	{
		CONSOLE_CURSOR_INFO cursorInfo = { Weight, Visible };
		BOOL res = SetConsoleCursorInfo(hOut.get(), &cursorInfo);
		if (res)
			mCursorInfo = cursorInfo;
		return res;
	}
	BOOL SetCursorPos(short X, short Y)
	{
		COORD pos{ X, Y };
		BOOL res = SetConsoleCursorPosition(hOut.get(), pos);
		if (res)
			mBufferInfo.dwCursorPosition = pos;
		return res;
	}
	BOOL SetInputCodePage(unsigned CodePage)
	{
		BOOL res = SetConsoleCP(CodePage);
		if (res)
			mInputCodePage = CodePage;
		return res;
	}
	BOOL SetOutputCodePage(unsigned CodePage)
	{
		BOOL res = SetConsoleOutputCP(CodePage);
		if (res)
			mOutputCodePage = CodePage;
		return res;
	}
	BOOL SetTextColor(Color foreGround, Color backGround)
	{
		WORD att = (WORD)(foreGround + backGround * 16);
		BOOL res = SetConsoleTextAttribute(hOut.get(), att);
		if (res)
			mBufferInfo.wAttributes = att;
		return res;
	}
	BOOL SetTitle(const char* Title)
	{
		BOOL res = SetConsoleTitleA(Title);
		if (res)
			strcpy_s(mTitle, Title);
		return res;
	}
	BOOL SetWindowSize(short Width, short Height)
	{
		SMALL_RECT rc{0, 0, Width - 1, Height - 1};
		return SetConsoleWindowInfo(hOut.get(), TRUE, &rc);
	}

#pragma endregion

	Console * pOwner;
	static Console::Impl * pConsoleImpl;
private:
	
	
	std::thread mMessageProcess;

	ScopedHandle hIn;
	ScopedHandle hOut;
	ScopedHandle hError;

	UINT mInputCodePage, mOutputCodePage;
	char mTitle[MAX_PATH];
	CONSOLE_CURSOR_INFO mCursorInfo;
	CONSOLE_FONT_INFO mFontInfo;
	CONSOLE_SCREEN_BUFFER_INFO mBufferInfo;
	CONSOLE_SELECTION_INFO mSelectionInfo;
	INPUT_RECORD mInputRecord;
};

Console::Impl::Impl(Console* pConsole)
	: pOwner(pConsole),
	hIn(GetStdHandle(STD_INPUT_HANDLE)),
	hOut(GetStdHandle(STD_OUTPUT_HANDLE)),
	hError(GetStdHandle(STD_ERROR_HANDLE)),
	mInputCodePage(GetConsoleCP()),
	mOutputCodePage(GetConsoleOutputCP()),
	mInputRecord()
{
	if (pConsoleImpl != nullptr)
	{
		throw std::exception("Console is a singleton!");
	}
	pConsoleImpl = this;

	// 初步获取信息
	GetConsoleCursorInfo(hOut.get(), &mCursorInfo);
	GetCurrentConsoleFont(hOut.get(), FALSE, &mFontInfo);
	GetConsoleOriginalTitleA(mTitle, MAX_PATH);
	GetConsoleScreenBufferInfo(hOut.get(), &mBufferInfo);
	GetConsoleSelectionInfo(&mSelectionInfo);
	
}

Console::Impl::~Impl()
{
	pConsoleImpl = nullptr;
}



int Console::Impl::WriteBuffer(short X, short Y, const char * str)
{
	
	int len = strlen(str);
	uint32_t outTotalLen = 0, outLen = 0;
	while (len > 0)
	{
		// 检查越界
		if (X >= mBufferInfo.dwSize.X || Y >= mBufferInfo.dwSize.Y)
			return 0;
		WriteConsoleOutputCharacterA(hOut.get(), str, min(mBufferInfo.dwSize.X - X, (SHORT)len), COORD{ X, Y }, 
			reinterpret_cast<LPDWORD>(&outLen));
		outTotalLen += outLen;
		X = 0, Y++;
		len -= outLen;
	}
	return outTotalLen;
}





// Console::Impl单例
Console::Impl * Console::Impl::pConsoleImpl = nullptr;










Console & Console::operator=(Console && MoveFrom) noexcept
{
	pImpl = std::move(MoveFrom.pImpl);
	pImpl->pOwner = this;
	return *this;
}

Console::Console(Console && MoveFrom) noexcept
	: pImpl(std::move(MoveFrom.pImpl))
{
	pImpl->pOwner = this;
}

// ********************************************
// Console类方法定义
Console::Console()
	: pImpl(std::make_unique<Impl>(this))
{
}

Console::~Console()
{
}



#pragma region 控制台输出方法
int Console::Write(const char * Str) noexcept
{
	return fputs(Str, stdout);
}

int Console::WriteChar(const char Ch) noexcept
{
	return putchar(Ch);
}

int Console::WriteFormat(const char * const Format, ...) noexcept
{
	va_list args;
	va_start(args, Format);
	int res = vprintf_s(Format, args);
	va_end(args);
	return res;
}

int Console::WriteLine(const char * Str) noexcept
{
	return puts(Str);
}

int Console::WriteBuffer(short X, short Y, const char * Str) noexcept
{
	return Impl::pConsoleImpl->WriteBuffer(X, Y, Str);
}
#pragma endregion

#pragma region 控制台输入方法

int Console::ReadFormat(const char * const Format, ...) noexcept
{
	va_list args;
	va_start(args, Format);
	int res = vscanf_s(Format, args);
	va_end(args);
	return res;
}

char * Console::ReadLine(char * Buffer, size_t BufferCount) noexcept
{
	return gets_s(Buffer, BufferCount);
}

char Console::ReadChar() noexcept
{
	return getchar();
}
#pragma endregion

#pragma region 控制台获取方法(需单例实例化)
Console& Console::Get()
{
	if (!Impl::pConsoleImpl || !Impl::pConsoleImpl->pOwner)
	{
		throw std::exception("Console is a singleton");
	}
	// 之后直接返回该单例
	return *Impl::pConsoleImpl->pOwner;
}

BOOL Console::GetBuffer(CHAR_INFO * pBufferOut)
{
	return Impl::pConsoleImpl->GetBuffer(pBufferOut);
}

COORD Console::GetBufferSize()
{
	return Impl::pConsoleImpl->GetBufferSize();
}

UINT Console::GetInputCodePage()
{
	return Impl::pConsoleImpl->GetInputCodePage();
}

UINT Console::GetOutputCodePage()
{
	return Impl::pConsoleImpl->GetOutputCodePage();
}

COORD Console::GetCursorPostion()
{
	return Impl::pConsoleImpl->GetCursorPosition();
}

DWORD Console::GetCursorSize()
{
	return Impl::pConsoleImpl->GetCursorSize();
}

BOOL Console::GetCursorVisible()
{
	return Impl::pConsoleImpl->GetCursorVisible();
}

INPUT_RECORD Console::GetInputRecord()
{
	return Impl::pConsoleImpl->GetInputRecord();
}

int Console::GetWindowName(char * Buffer, size_t BufferCount)
{
	return Impl::pConsoleImpl->GetWindowName(Buffer, BufferCount);
}
#pragma endregion

#pragma region 控制台修改方法(需单例实例化)
BOOL Console::SetBufferSize(short Width, short Height)
{
	return Impl::pConsoleImpl->SetBufferSize(Width, Height);
}

BOOL Console::SetCursorInfo(unsigned Weight, bool Visible)
{
	return Impl::pConsoleImpl->SetCursorInfo(Weight, Visible);
}

BOOL Console::SetCursorPos(short X, short Y)
{
	return Impl::pConsoleImpl->SetCursorPos(X, Y);
}

BOOL Console::SetInputCodePage(unsigned CodePage)
{
	return Impl::pConsoleImpl->SetInputCodePage(CodePage);
}

BOOL Console::SetOutputCodePage(unsigned CodePage)
{
	return Impl::pConsoleImpl->SetOutputCodePage(CodePage);
}

BOOL Console::SetTextColor(Color foreGround, Color backGround)
{
	return Impl::pConsoleImpl->SetTextColor(foreGround, backGround);
}

BOOL Console::SetTitle(const char * Title)
{
	return Impl::pConsoleImpl->SetTitle(Title);
}

BOOL Console::SetWindowSize(short Width, short Height)
{
	return Impl::pConsoleImpl->SetWindowSize(Width, Height);
}

#pragma endregion