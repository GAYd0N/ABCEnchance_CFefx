#include <metahook.h>
#include "parsemsg.h"
#include "pm_shared.h"
#include "msghook.h"
#include <regex>

#include "hud.h"
#include "vguilocal.h"
#include "drawElement.h"
#include "deathmsg.h"

CHudDeathMsg m_HudDeathMsg;
pfnUserMsgHook m_pfnTextMsg;

#define MSG_BUF_SIZE 64
#define HUD_PRINTNOTIFY 1
#define MSG_SUICIDENOTIFY L" committed suicide."
#define MSG_KILLEDNOTIFY L" was killed by a "
#define MSG_PLAYERKILLNOTIFY L" : (.*) : "

using namespace std;
wregex parttenSuicide(MSG_SUICIDENOTIFY);
wregex parttenKilled(MSG_KILLEDNOTIFY);
wregex parttenPlayer(MSG_PLAYERKILLNOTIFY);

int __MsgFunc_TextMsg(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int type = READ_BYTE();
	if (type == HUD_PRINTNOTIFY)
	{
		char* msg_text = READ_STRING();
		//ʲô���Ͷ���
		if (msg_text == (char*)0)
			return m_pfnTextMsg(pszName, iSize, pbuf);
		wchar_t wideBuf[MSG_BUF_SIZE];
		pLocalize->ConvertANSIToUnicode(msg_text, wideBuf, sizeof(wideBuf));
		wstring stdSzBuf = wideBuf;
		//ʣ�µ����Ӳ�û��Ȥ
		//���򲶻�ƥ��
		wsmatch matched;
		bool found = false;
		found = regex_search(stdSzBuf, matched, parttenSuicide);
		if (found)
		{
			m_HudDeathMsg.InsertNewMsg(matched.prefix().str().c_str(), L"Suicide", L"");
			return 1;
		}
		found = regex_search(stdSzBuf, matched, parttenKilled);
		if (found)
		{
			m_HudDeathMsg.InsertNewMsg(matched.prefix().str().c_str(), L"Killed", matched.suffix().str().c_str());
			return 1;
		}
		found = regex_search(stdSzBuf, matched, parttenPlayer);
		if (found)
		{
			m_HudDeathMsg.InsertNewMsg(matched.prefix().str().c_str(), matched.str(0).c_str(), matched.suffix().str().c_str());
			return 1;
		}
	}
	return 1;
	//Stack overflow
	//return m_pfnTextMsg(pszName, iSize, pbuf);
}

int CHudDeathMsg::Draw(float flTime)
{
	int x = 0;
	int y = 0;
	int w;
	int h;
	for each (deathmsgItem* var in aryKeepMsg)
	{
		if (!var)
			continue;
		DrawVGUI2String(var->killer, x, y, 255, 255, 255, HUDFont);
		GetStringSize(var->killer, &w, NULL, HUDFont);
		x += w;

		DrawVGUI2String(var->executioner, x, y, 255, 255, 255, HUDFont);
		GetStringSize(var->executioner, &w, NULL, HUDFont);
		x += w;

		DrawVGUI2String(var->victim, x, y, 255, 255, 255, HUDFont);
		GetStringSize(var->victim, NULL, &h, HUDFont);
		x = 0;
		y += h;
	}
	return 1;
}
void CHudDeathMsg::Reset(void)
{
	memset(aryKeepMsg, 0, sizeof(aryKeepMsg));
}
void CHudDeathMsg::InsertNewMsg(const wchar_t* v, const wchar_t* e, const wchar_t* k)
{
	wchar_t wideVName[MSG_BUF_SIZE];
	wchar_t wideEName[MSG_BUF_SIZE];
	wchar_t wideKName[MSG_BUF_SIZE];
	wcscpy(wideVName, v);
	wcscpy(wideEName, e);
	wcscpy(wideKName, k);
	for (int i = 0; i < MAX_KEEP_DEATHMSG - 1; i++)
	{
		aryKeepMsg[i + 1] = aryKeepMsg[i];
	}
	deathmsgItem item = { wideEName , wideVName , wideKName };
	aryKeepMsg[0] = &item;
}
int CHudDeathMsg::Init(void)
{
	m_pfnTextMsg = HOOK_MESSAGE(TextMsg);

	HUDFont = pScheme->GetFont("HUDShitFont", true);
	return 1;
}