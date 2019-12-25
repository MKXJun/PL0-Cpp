#pragma once

#include "PL0_Common.h"

namespace PL0
{
	//
	// 虚拟机
	//
	class VirtualMachine
	{
	public:
		// 初始化虚拟机
		void Initialize(const ProgramInfo& programInfo);

		// 完整执行
		void Run();
		// 逐指令执行
		void RunNextInstruction();
		// 逐过程执行
		void RunProcedure();
		// 执行到跳出该过程
		void RunToReturn();
		// 程序是否运行结束
		bool IsFinished() const;
		// 打印当前寄存器信息
		void PrintRegisterInfo() const;
		// 获取当前寄存器信息
		const EnvironmentInfo& GetRegisterInfo() const;

	private:
		void PrintInstruction(uint32_t address) const;
		uint32_t GetProcedureIndex(const std::string& str);


	private:
		EnvironmentInfo m_EnvironmentInfo;	// 寄存器信息
		ProgramInfo m_ProgramInfo;		// 程序信息
	};
}