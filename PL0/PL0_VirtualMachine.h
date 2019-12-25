#pragma once

#include "PL0_Common.h"

namespace PL0
{
	//
	// �����
	//
	class VirtualMachine
	{
	public:
		// ��ʼ�������
		void Initialize(const ProgramInfo& programInfo);

		// ����ִ��
		void Run();
		// ��ָ��ִ��
		void RunNextInstruction();
		// �����ִ��
		void RunProcedure();
		// ִ�е������ù���
		void RunToReturn();
		// �����Ƿ����н���
		bool IsFinished() const;
		// ��ӡ��ǰ�Ĵ�����Ϣ
		void PrintRegisterInfo() const;
		// ��ȡ��ǰ�Ĵ�����Ϣ
		const EnvironmentInfo& GetRegisterInfo() const;

	private:
		void PrintInstruction(uint32_t address) const;
		uint32_t GetProcedureIndex(const std::string& str);


	private:
		EnvironmentInfo m_EnvironmentInfo;	// �Ĵ�����Ϣ
		ProgramInfo m_ProgramInfo;		// ������Ϣ
	};
}