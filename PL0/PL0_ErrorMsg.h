#pragma once

namespace PL0
{
	const char* const g_ErrorMsgs[] = {
		"",
		// 词法分析
		"[Error001] Integer length exceed.",
		"[Error002] Invalid Value.",
		"[Error003] '=' expected after ':'.",
		"[Error004] Missing */",
		"[Error005] Unknown Character.",
		"[Error006] '\'' expected.",
		"[Error007] Character expected between '\''s.",
		"[Error008] '\"' expected.",
		"[Error009] Extra '|' Expected.",
		"[Error010] Extra '&' Expected.",
		"[Error011] Too much Characters between '\''s.",
		"[Error012] Identifier length exceed.",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		// 语法分析
		"[Error021] ';' expected.",
		"[Error022] Identifier Expected.",
		"[Error023] ':=' Expected.",
		"[Error024] '(' Expected.",
		"[Error025] ')' Expected.",
		"[Error026] Keyword 'end' expected.",
		"[Error027] Keyword 'then' expected.",
		"[Error028] Keyword 'do' expected.",
		"[Error029] Keyword 'step' expected.",
		"[Error030] Keyword 'until' expected.",
		"[Error031] Typename expected.",
		"[Error032] Value expected.",
		"[Error033] Unknown identifier.",
		"[Error034] Identifier redefined.",
		"[Error035] Identifier can't be assigned.",
		"[Error036] Identifier is not a procedure.",
		"[Error037] Number of function parameter mismatch.",
		"[Error038] Expression/Condition expected.",
		"[Error039] '.' Expected.",
		"[Error040] Invalid expression.",
		"[Error041] Unexpected content after '.'."
	};
}