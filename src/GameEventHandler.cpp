#include "GameEventHandler.h"
#include <detours/detours.h>
#pragma warning(disable: 4091)
#pragma warning(disable: 4189)

typedef struct FormEditorIDEntry
{
	RE::FormID formId = 0;
	RE::BSFixedString editorId;
	bool operator==(const FormEditorIDEntry& other) const noexcept
	{
		return other.formId == this->formId;
	};
	bool operator==(RE::FormID otherId) const noexcept
	{
		return otherId == this->formId;
	}
	std::weak_ordering operator<=>(const FormEditorIDEntry& other) const noexcept
	{
		return this->formId <=> other.formId;
	}
	std::weak_ordering operator<=>(RE::FormID otherId) const noexcept
	{
		return this->formId <=> otherId;
	}
} FormEditorIDEntry;
std::mutex table_mutex;
std::vector<std::unordered_map<RE::FormID, FormEditorIDEntry>> EditorIdLookupTables;
static bool NewSetEditorID(RE::TESForm& form, const char* name)
{
	if (!name || name[0] == '\x00') {
		return true;
	}
	if (strnlen(name, 127) != 127) {
		std::lock_guard l(table_mutex);
		auto& table = EditorIdLookupTables[form.formType.underlying()];
		RE::BSFixedString editorId(name);
		FormEditorIDEntry entry{ form.formID, editorId };
		table.insert_or_assign(form.formID, entry);

	} else {
		std::lock_guard l(table_mutex);
		auto& table = EditorIdLookupTables[form.formType.underlying()];
		std::string cutName;
		cutName.assign(name, 127);
		RE::BSFixedString editorId(cutName.c_str());
		FormEditorIDEntry entry{ form.formID, editorId };
		table.insert_or_assign(form.formID, entry);
	}
	return true;
}
static const char* NewGetEditorID(RE::TESForm& form)
{
	std::lock_guard l(table_mutex);
	auto& table = EditorIdLookupTables[form.formType.underlying()];
	if (!table.contains(form.formID)) {
		return "";
	}
	return table.find(form.formID)->second.editorId.c_str();
}
uintptr_t OriginalGetEditorID = 0x0;
uintptr_t OriginalSetEditorID = 0x0;
void GameEventHandler::onLoad()
{
	logger::info("onLoad()");
	EditorIdLookupTables.resize(0x100);
	OriginalGetEditorID = REL::Offset(0x107430).address();
	OriginalSetEditorID = REL::Offset(0x107890).address();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)OriginalGetEditorID, NewGetEditorID);
	DetourTransactionCommit();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)OriginalSetEditorID, NewSetEditorID);
	DetourTransactionCommit();
}
