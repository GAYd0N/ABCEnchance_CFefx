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
	//gCVars.pCfefxKillTime = CREATE_CVAR("cl_cfefx_time", "1", FCVAR_VALUE, nullptr);
	gCVars.pCfefxSoundVolume = CREATE_CVAR("cl_cfefx_volume", "0.2", FCVAR_VALUE, [](cvar_t* cvar) {cvar->value = std::clamp<float>(cvar->value, 0, 1); });

	m_iDmg = 0;
	m_iDmgTimes = 0;
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

	m_vecScoreEffectPos = { (float)m_pScoreEffect->GetXPos(), (float)m_pScoreEffect->GetYPos(), 0 };
	m_vecScoreEffectSize = { (float)m_pScoreEffect->GetWide(), (float)m_pScoreEffect->GetTall(), 0 };
	m_vecScoreMarkPos = { (float)m_pScoreMark->GetXPos(), (float)m_pScoreMark->GetYPos(), 0 };
	m_vecDmgStarsPos = { (float)m_pDmgStars[0]->GetXPos(), (float)m_pDmgStars[0]->GetYPos(), 0 };
	m_vecDmgStarsSize = { (float)m_pDmgStars[0]->GetWide(), (float)m_pDmgStars[0]->GetTall(), 0 };

	SetVisible(false);
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

//true淡入显示，false淡出隐藏
inline void CCfefxPanel::StartFade(vgui::ImagePanel* panel, bool state, float fadetime, float delaytime) {
	if (!panel->IsVisible())
		panel->SetVisible(true);
	panel->SetAlpha(state ? 0 : 255);
	vgui::GetAnimationController()->RunAnimationCommand(panel, "alpha", state ? 255 : 0, delaytime, fadetime, vgui::AnimationController::INTERPOLATOR_LINEAR);
}
//每次调用占用一个FreeChannel
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
	if (!panel)
		return;
	if (!panel->IsVisible())
		panel->SetVisible(true);
	if (panel->GetAlpha() != 0)
	{
		panel->SetAlpha(255);
		return;
	}
	int p = VecPos(panel);
	auto star = m_pDmgStars[p];
	if (!star->IsVisible())
		star->SetVisible(true);
	vgui::GetAnimationController()->StartAnimationSequence(this, m_szStarAnims[p]);
	StartFade(panel, true, 0.2, 0.3);
}
void CCfefxPanel::ShowScoreEffect() {
	m_pScoreEffect->SetBounds(m_vecScoreEffectPos.x, m_vecScoreEffectPos.y, m_vecScoreEffectSize.x, m_vecScoreEffectSize.y);
	StartFade(m_pScoreEffect, true, 0.2);
	float pos[3], size[3];
	(m_vecScoreMarkPos * 1.05).CopyToArray(pos);
	(m_vecScoreEffectSize * 1.2).CopyToArray(size);
	vgui::GetAnimationController()->RunAnimationCommandEx(m_pScoreEffect, "position", pos, 2, 0.2, 0.5, vgui::AnimationController::INTERPOLATOR_LINEAR);
	vgui::GetAnimationController()->RunAnimationCommandEx(m_pScoreEffect, "size", size, 2, 0.7, 0.2, vgui::AnimationController::INTERPOLATOR_LINEAR);
	StartFade(m_pScoreEffect, false, 0.3, 0.9);
	if (!m_pScoreMark->IsVisible())
		m_pScoreMark->SetVisible(true);
	vgui::GetAnimationController()->StartAnimationSequence(this, "ScoreMarkAnim");
}

void CCfefxPanel::AddDmg(int iDmg)
{
	m_iDmg += iDmg;
	ShowPanel(true);
}
void CCfefxPanel::OnThink()
{
	//伤害小于阈值不触发
	int i = gCVars.pCfefxMaxDmg->value / 10;
	if (m_iDmg < i)
		return;
	//倍数
	int multiples = m_iDmg / i;
	//防止同一倍数重复触发
	if (multiples == m_iDmgTimes || multiples == 0)
		return;
	m_iDmgTimes = multiples;
	//大于9
	if (multiples >= 10)
	{
		PlaySoundByFmod(m_szKillSound, gCVars.pCfefxSoundVolume->value);
		ShowScoreEffect();
		Reset();
	}
	else if (multiples <= 4) //小于5
	{
		if (m_pDmgMarks[0]->GetAlpha() != 0)
			m_pDmgMarks[0]->SetAlpha(0);
		for (auto iter = m_pDmgMarks.begin() + 1; VecPos(*iter) <= multiples; iter++) {
			ShowDmgMark(*iter);
		}
	}
	else if (multiples >= 5 && multiples <= 9) //5-9
	{
		//先隐藏后面的
		if (m_pDmgMarks[0]->GetAlpha() == 0) {
			//上次伤害少于500
			for (auto it = m_pDmgMarks.begin(); it != m_pDmgMarks.end(); it++)
				(*it)->SetAlpha(0);

		}
		else {
			//上次伤害高于500
			for (auto it = m_pDmgMarks.begin() + multiples - 4; it != m_pDmgMarks.end(); it++)
				(*it)->SetAlpha(0);
		}
		//再显示前面的
		for (auto iter = m_pDmgMarks.begin(); VecPos(*iter) < multiples - 4 && iter != m_pDmgMarks.end(); iter++)
			ShowDmgMark(*iter);
	}
	//防止动画执行失败强制显示
	for (auto it = m_pDmgMarks.begin() + 2; it != m_pDmgMarks.end(); it++) {
		if ((*it)->GetAlpha() > (*(it - 1))->GetAlpha())
			(*(it - 1))->SetAlpha(255);
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
	m_iDmgTimes = 0;
	m_iDmg = 0;
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