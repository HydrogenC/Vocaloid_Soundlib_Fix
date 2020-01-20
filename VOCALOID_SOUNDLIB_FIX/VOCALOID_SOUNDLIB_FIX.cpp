#include <iostream>
#include <atlbase.h>
#include <string>
#include <vector>
#include <functional>

#define _RTN_ERR return false;
#define _RTN_SUC return true;
using namespace std;

constexpr int MAX_KEY_LENGTH = 255;
constexpr int MAX_VALUE_NAME_LENGTH = 16383;

bool enum_keys(const wstring root, vector<wstring>& dest)
{
	CRegKey crk;
	if (crk.Open(HKEY_LOCAL_MACHINE, root.c_str()) == ERROR_SUCCESS) {
		DWORD index = 0;
		TCHAR bfr[MAX_KEY_LENGTH];

		for (;;) {
			DWORD length = MAX_KEY_LENGTH;
			LSTATUS lst = crk.EnumKey(index, bfr, &length);

			switch (lst)
			{
			case ERROR_SUCCESS:
#ifdef DEBUG
				wcout << bfr << L'\n';
#endif // DEBUG

				dest.push_back(root + L'\\' + bfr);
				index += 1;
				break;

			case ERROR_NO_MORE_ITEMS:
				_RTN_SUC

			default:
				_RTN_ERR
			}
		}
	}
	else {
		_RTN_ERR
	}
}

bool copy_keys(const vector<wstring>& keys, function<const wstring(wstring&)> pathModifier)
{
	CRegKey crk;
	for (auto i : keys) {

		wstring dest = pathModifier(i);
		crk.Create(HKEY_LOCAL_MACHINE, dest.c_str());

		DWORD index = 0;
		for (;; index += 1) {
			DWORD length = MAX_VALUE_NAME_LENGTH, sz = MAX_VALUE_NAME_LENGTH, ty = NULL;
			BYTE data[MAX_VALUE_NAME_LENGTH];
			TCHAR valueName[MAX_VALUE_NAME_LENGTH];

			HKEY source = NULL;
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, i.c_str(), 0, 131103UL, &source);

			LSTATUS lst = RegEnumValue(source, index, valueName, &length, NULL, &ty, data, &sz);
			switch (lst)
			{
			case ERROR_SUCCESS:
				break;
			case ERROR_NO_MORE_ITEMS:
				RegCloseKey(source);
				goto END;
			default:
				_RTN_ERR
			}

#ifdef DEBUG
			wcout << valueName << L'\n';
#endif // DEBUG

			if (RegSetValueEx((HKEY)crk, valueName, NULL, ty, data, sz) != ERROR_SUCCESS) {
				_RTN_ERR
			}
		}

	END:crk.Flush();
	}
	_RTN_SUC
}

bool delete_keys(const wstring root)
{
	CRegKey crk;
	if (crk.Open(HKEY_LOCAL_MACHINE, root.c_str()) == ERROR_SUCCESS) {
		if (crk.RecurseDeleteKey(root.c_str()) == ERROR_SUCCESS) {
			crk.Flush();
			_RTN_SUC
		}
		else {
			_RTN_ERR
		}
	}
	else {
		_RTN_ERR
	}
}

int main()
{
	locale::global(locale("en-US.UTF-8"));
	vector<wstring> keys = vector<wstring>();
	const wstring root = L"SOFTWARE\\WOW6432Node\\POCALOID4";
	size_t index = 0;
	bool allSuccess = true;

	allSuccess = allSuccess && enum_keys(root, keys);
	while (index < keys.size())
	{
		allSuccess = allSuccess && enum_keys(keys[index], keys);
		index += 1;
	}

	for (auto i : keys)
	{
		wcout << L"Found key: " << i << L'\n';
	}

	allSuccess = allSuccess && copy_keys(keys, [](const wstring& str)->wstring {
		wstring wstr = str;
		wstr[21] = L'P';
		return wstr;
		});


	if (allSuccess)
	{
		wcout << L"All finished! \n";
#ifndef DEBUG
		delete_keys(root);
#endif // !DEBUG
	}
	else {
		wcout << L"All finished, but with errors. \n";
	}
}