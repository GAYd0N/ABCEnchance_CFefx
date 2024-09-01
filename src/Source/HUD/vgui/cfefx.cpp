#include <algorithm>
#include "metahook.h"

#include "soundengine.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/AnimationController.h"

#include "local.h"
#include "vguilocal.h"

#include "Viewport.h"
#include "cfefx.h"

#define VIEWPORT_CFEFXPANEL_NAME "CfefxPanel"

CCfefxPanel::CCfefxPanel() : BaseClass(nullptr, VIEWPORT_CFEFXPANEL_NAME) {
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	gCVars.pCfefxEnable = CREATE_CVAR("cl_cfefx", "1", FCVAR_VALUE, nullptr);
	gCVars.pCfefxMaxDmg = CREATE_CVAR("cl_cfefx_max", "1000", FCVAR_VALUE, [](cvar_t* cvar) {cvar->value = std::clamp<int>(cvar->value, 10, 10000); });
	gCVars.pCfefxKillTime = CREATE_CVAR("cl_cfefx_time", "1", FCVAR_VALUE, nullptr);
	gCVars.pCfefxSoundVolume = CREATE_CVAR("cl_cfefx_volume", "0.2", FCVAR_VALUE, [](cvar_t* cvar) {cvar->value = std::clamp<float>(cvar->value, 0, 1); });

	m_pScoreMark = new vgui::ImagePanel(this, "ScoreMark");
	m_pScoreEffect = new vgui::ImagePanel(this, "ScoreEffect");
	m_pDmgMarks = {
		new vgui::ImagePanel(this, "DmgMarkFive"),
		new vgui::ImagePanel(this, "DmgMarkOne"),
		new vgui::ImagePanel(this, "DmgMarkTwo"),
		new vgui::ImagePanel(this, "DmgMarkThree"),
		new vgui::ImagePanel(this, "DmgMarkFour")
	};
	m_pDmgStars = {
		new vgui::ImagePanel(this, "StarFive"),
		new vgui::ImagePanel(this, "StarOne"),
		new vgui::ImagePanel(this, "StarTwo"),
		new vgui::ImagePanel(this, "StarThree"),
		new vgui::ImagePanel(this, "StarFour")
	};

	LoadControlSettings(VGUI2_ROOT_DIR "CfefxPanel.res");
	vgui::GetAnimationController()->SetScriptFile(GetVPanel(), VGUI2_ROOT_DIR "hudanimations.txt");

	m_flScoreEffectPos = { (float)m_pScoreEffect->GetXPos(), (float)m_pScoreEffect->GetYPos(), 0 };
	m_flScoreEffectSize = { (float)m_pScoreEffect->GetWide(), (float)m_pScoreEffect->GetTall(), 0 };
	m_flScoreMarkPos = { (float)m_pScoreMark->GetXPos(), (float)m_pScoreMark->GetYPos(), 0 };
	m_flDmgStarsPos = { (float)m_pDmgStars[0]->GetXPos(), (float)m_pDmgStars[0]->GetYPos(), 0 };
	m_flDmgStarsSize = { (float)m_pDmgStars[0]->GetWide(), (float)m_pDmgStars[0]->GetTall(), 0 };

	m_pScoreMark->SetVisible(false);
	m_pScoreEffect->SetVisible(false);
	for (auto iter = m_pDmgMarks.begin(); iter != m_pDmgMarks.end(); iter++) {
		(*iter)->SetVisible(false);
		(*iter)->SetAlpha(0);
	}
	for (auto iter = m_pDmgStars.begin(); iter != m_pDmgStars.end(); iter++) {
		(*iter)->SetVisible(false);
		(*iter)->SetAlpha(0);
	}
}

void CCfefxPanel::ApplySchemeSettings(vgui::IScheme* pScheme) {
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(Color(0, 0, 0, 0));
}

//true������ʾ��false��������
inline void CCfefxPanel::StartFade(vgui::ImagePanel* panel, bool state, float fadetime, float delaytime) {
	if (!panel->IsVisible())
		panel->SetVisible(true);
	panel->SetAlpha(state ? 0 : 255);
	vgui::GetAnimationController()->RunAnimationCommand(panel, "alpha", state ? 255 : 0, delaytime, fadetime, vgui::AnimationController::INTERPOLATOR_LINEAR);
}
//ÿ�ε���ռ��һ��FreeChannel
void CCfefxPanel::PlaySoundByFmod(const char* name, float volume) {
	if (m_pChannel.Valid())
		m_pChannel.Stop();
	if (m_pSound.Valid())
		m_pSound.Release();
	char FullPath[MAX_PATH] = { 0 };
	g_pFullFileSystem->GetLocalPath(name, FullPath, MAX_PATH);
	if (!FullPath) {
		Warning("Couldn't load sound:'%s'\n", name);
		return;
	}
	FModEngine::CFModSystem* soundSystem = FModEngine::GetSystem();
	soundSystem->CreateSound(FullPath, FMOD_DEFAULT, nullptr, m_pSound);
	soundSystem->PlaySound(FMOD_CHANNEL_FREE, m_pSound, false, m_pChannel);
	m_pChannel.SetVolume(volume);
}
void CCfefxPanel::ShowDmgMark(vgui::ImagePanel* panel) {
	if (!panel || panel->GetAlpha() > 0)
		return;
	if (!panel->IsVisible())
		panel->SetVisible(true);
	int p = VecPos(panel);
	auto star = m_pDmgStars[p];
	if (!star->IsVisible())
		star->SetVisible(true);
	vgui::GetAnimationController()->StartAnimationSequence(this, m_szStarAnims[p]);
	StartFade(panel, true, 0.2, 0.3);
}
void CCfefxPanel::ShowScoreEffect() {
	m_pScoreEffect->SetBounds(m_flScoreEffectPos.x, m_flScoreEffectPos.y, m_flScoreEffectSize.x, m_flScoreEffectSize.y);
	StartFade(m_pScoreEffect, true, 0.2);
	float pos[3], size[3];
	(m_flScoreMarkPos * 1.05).CopyToArray(pos);
	(m_flScoreEffectSize * 1.2).CopyToArray(size);
	vgui::GetAnimationController()->RunAnimationCommandEx(m_pScoreEffect, "position", pos, 2, 0.2, 0.5, vgui::AnimationController::INTERPOLATOR_LINEAR);
	vgui::GetAnimationController()->RunAnimationCommandEx(m_pScoreEffect, "size", size, 2, 0.7, 0.2, vgui::AnimationController::INTERPOLATOR_LINEAR);
	StartFade(m_pScoreEffect, false, 0.3, 0.9);
	if (!m_pScoreMark->IsVisible())
		m_pScoreMark->SetVisible(true);
	vgui::GetAnimationController()->StartAnimationSequence(this, "ScoreMarkAnim");
}
void CCfefxPanel::ShowScoreMark(int& iDmg) {
	int i = gCVars.pCfefxMaxDmg->value / 10;
	if (iDmg > i) {
		int a = iDmg / i;
		if (a == iDmgTimes || a == 0)
			return;
		if (a > 9) {
			PlaySoundByFmod(m_szKillSound, gCVars.pCfefxSoundVolume->value);
			ShowScoreEffect();
			Reset();
			iDmg = 0;
			return;
		}
		if (a < 5) {
			if (m_pDmgMarks[0]->GetAlpha() != 0)
				m_pDmgMarks[0]->SetAlpha(0);
			for (auto iter = m_pDmgMarks.begin() + a; iter != m_pDmgMarks.end(); iter++) {
				(*iter)->SetAlpha(0);
			}
			for (auto iter = m_pDmgMarks.begin() + a; VecPos(*iter) <= a; iter++) {
				ShowDmgMark(*iter);
			}
		}
		else {
			//�����غ����
			if (m_pDmgMarks[0]->GetAlpha() == 0) {
				//�ϴ��˺�����500
				for (auto it = m_pDmgMarks.begin(); it != m_pDmgMarks.end(); it++)
					(*it)->SetAlpha(0);
			}
			else { 
				//�ϴ��˺�����500
				for (auto it = m_pDmgMarks.begin() + a - 4; it != m_pDmgMarks.end(); it++)
					(*it)->SetAlpha(0);
			}
			//����ʾǰ���
			for (auto iter = m_pDmgMarks.begin(); VecPos(*iter) < a - 4 && iter != m_pDmgMarks.end(); iter++)
				ShowDmgMark(*iter);
		}
		iDmgTimes = a;
	}
}

//TODO
//void CCfefxPanel::ShowKillMark(wchar_t* killer)
//{
//	char buffer[32];
//	Q_UnicodeToUTF8(killer, buffer, 32);
//	if (!Q_strcmp(buffer, CPlayerInfo::GetPlayerInfo(gEngfuncs.GetLocalPlayer()->index)->GetName()))
//	{
//		if (gEngfuncs.GetClientTime() - m_flRestoredTime > gCVars.pCfefxKillTime->value || iKill != -1) {
//			iKill = -1;
//			m_pKillMark->SetAlpha(0);
//			return;
//		}
//		else
//		{
//			m_flRestoredTime = gEngfuncs.GetClientTime();
//			iKill++;
//		}
//		m_pKillMark->SetAlpha(255);
//		m_pKillMark->SetImage(m_szKillMarks[std::clamp<int>(iKill, 0, 5)]);
//		PlaySoundByFmod(m_szKillSound[std::clamp<int>(iKill, 0, 7)], gCVars.pCfefxSoundVolume->value);
//	}
//}

void CCfefxPanel::Reset() {
	ShowPanel(true);
	for (auto iter = m_pDmgMarks.begin(); iter != m_pDmgMarks.end(); iter++)
		(*iter)->SetAlpha(0);
	for (auto iter = m_pDmgStars.begin(); iter != m_pDmgStars.end(); iter++)
		(*iter)->SetAlpha(0);
	iDmgTimes = 0;
}

void CCfefxPanel::ShowPanel(bool state) {
	if (state == IsVisible())
		return;
	SetVisible(state);
}
bool CCfefxPanel::IsVisible() {
	return BaseClass::IsVisible();
}
vgui::VPANEL CCfefxPanel::GetVPanel() {
	return BaseClass::GetVPanel();
}
void CCfefxPanel::SetParent(vgui::VPANEL parent) {
	BaseClass::SetParent(parent);
}
const char* CCfefxPanel::GetName() {
	return VIEWPORT_CFEFXPANEL_NAME;
}
inline int CCfefxPanel::VecPos(vgui::ImagePanel* panel) {
	return panel ? std::distance(m_pDmgMarks.begin(), std::find(m_pDmgMarks.begin(), m_pDmgMarks.end(), panel)) : 0;
}