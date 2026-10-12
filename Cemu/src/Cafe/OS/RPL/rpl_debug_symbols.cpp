#include "Cafe/OS/RPL/rpl_debug_symbols.h"

std::map<MPTR, rplDebugSymbolBase*> map_DebugSymbols;

void rplDebugSymbol_createComment(MPTR address, const wchar_t* comment)
{
	auto new_comment = new rplDebugSymbolComment();
	new_comment->type = RplDebugSymbolComment;
	new_comment->comment = comment;
	auto [it, inserted] = map_DebugSymbols.try_emplace(address, new_comment);
	if (!inserted)
	{
		delete it->second;
		it->second = new_comment;
	}
}

rplDebugSymbolBase* rplDebugSymbol_getForAddress(MPTR address)
{
	const auto it = map_DebugSymbols.find(address);
	return it != map_DebugSymbols.end() ? it->second : nullptr;
}

const std::map<MPTR, rplDebugSymbolBase*>& rplDebugSymbol_getSymbols()
{
	return map_DebugSymbols;
}
