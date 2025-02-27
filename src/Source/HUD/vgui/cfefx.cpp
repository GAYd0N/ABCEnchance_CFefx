#include <algorithm>
#include "metahook.h"

#include <vgui/IScheme2.h>
#include "soundengine.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/AnimationController.h"

#include "local.h"
#include "vguilocal.h"
#include "hud.h"
#include "Viewport.h"
#include "cfefx.h"

#define VIEWPORT_CFEFXPANEL_NAME "CfefxPanel"

#pragma region Class CfefxPanel
CCfefxPanel::CCfefxPanel() : BaseClass(nullptr, VIEWPORT_CFEFXPANEL_NAME) {
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	gCVars.pCfefxEnable = CREATE_CVAR("cl_cfefx", "1", FCVAR_VALUE, nullptr);
	pCfefxMaxDmg = CREATE_CVAR("cl_cfefx_max", "1000", FCVAR_VALUE, [](cvar_t* cvar) {cvar->value = std::clamp<int>(cvar->value, 10, 10000000); });
	pCfefxSoundVolume = CREATE_CVAR("cl_cfefx_volume", "20", FCVAR_VALUE, [](cvar_t* cvar) {cvar->value = std::clamp<float>(cvar->value, 0, 100); });
	//gCVars.pCfefxKillTime = CREATE_CVAR("cl_cfefx_time", "1", FCVAR_VALUE, nullptr);

	m_iDmg = 0;
	m_iDmgMultiples = 0;

	m_pScoreMark = new vgui::ImagePanel(this, "ScoreMark");
	m_pScoreEffect = new vgui::ImagePanel(this, "ScoreEffect");
	m_pDmgMarks = {
		new vgui::ImagePanel(this, "DmgMarkOne"),
		new vgui::ImagePanel(this, "DmgMarkTwo"),
		new vgui::ImagePanel(this, "DmgMarkThree"),
		new vgui::ImagePanel(this, "DmgMarkFour"),
		new vgui::ImagePanel(this, "DmgMarkFive"),
		new vgui::ImagePanel(this, "DmgMarkSix"),
		new vgui::ImagePanel(this, "DmgMarkSeven"),
		new vgui::ImagePanel(this, "DmgMarkEight"),
		new vgui::ImagePanel(this, "DmgMarkNine")
	};
	m_pDmgStars = {
		new vgui::ImagePanel(this, "StarOne"),
		new vgui::ImagePanel(this, "StarTwo"),
		new vgui::ImagePanel(this, "StarThree"),
		new vgui::ImagePanel(this, "StarFour"),
		new vgui::ImagePanel(this, "StarFive"),
		new vgui::ImagePanel(this, "StarSix"),
		new vgui::ImagePanel(this, "StarSeven"),
		new vgui::ImagePanel(this, "StarEight"),
		new vgui::ImagePanel(this, "StarNine"),
	};

	LoadControlSettings(VGUI2_ROOT_DIR "CfefxPanel.res");
	vgui::GetAnimationController()->SetScriptFile(GetVPanel(), VGUI2_ROOT_DIR "hudanimations.txt");

	m_vecScoreEffectPos = { (float)m_pScoreEffect->GetXPos(), (float)m_pScoreEffect->GetYPos(), 0 };
	m_vecScoreEffectSize = { (float)m_pScoreEffect->GetWide(), (float)m_pScoreEffect->GetTall(), 0 };
	m_vecScoreMarkPos = { (float)m_pScoreMark->GetXPos(), (float)m_pScoreMark->GetYPos(), 0 };
	m_vecScoreMarkSize = { (float)m_pScoreMark->GetWide(), (float)m_pScoreMark->GetTall(), 0 };
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

void CCfefxPanel::ApplySettings(KeyValues* inResourceData) {
	BaseClass::ApplySettings(inResourceData);

}
//true淡入显示，false淡出隐藏
inline void CCfefxPanel::StartFade(vgui::Panel* panel, bool state, float fadetime, float delaytime) {
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

void CCfefxPanel::UpdateDmgMark(int index) {
	int p = index;
	if (p > DMGMARK_ARRAY_ENDINDEX || !m_pDmgMarks[p])
		return;

	auto panel = m_pDmgMarks[p];
	if (!panel->IsVisible())
		panel->SetVisible(true);

	if (panel->GetAlpha() != 0)
		return;

	auto star = m_pDmgStars[p];
	if (!star->IsVisible())
		star->SetVisible(true);

	ResetDmgMark(p);

	vgui::GetAnimationController()->StartAnimationSequence(this, m_szStarAnims[p]);
	vgui::GetAnimationController()->StartAnimationSequence(this, m_szMarkAnims[p]);
}

void CCfefxPanel::UpdateScoreEffect() {
	ResetScoreEffect();
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
	if (iDmg > 0)
		m_iDmg += iDmg;
	ShowPanel(true);
}

void CCfefxPanel::OnThink()
{
	if (!gClientData)
	{
		ShowPanel(false);
		return;
	}
	if (GetBaseViewPort()->IsInSpectate() ||
		GetBaseViewPort()->IsHudHide(HUD_HIDEALL | HUD_HIDEWEAPONS) ||
		gClientData->health <= 0)
	{
		Reset();
		return;
	}
	//伤害小于阈值不触发
	int i = static_cast<int>(pCfefxMaxDmg->value) / 10;
	if (m_iDmg < i)
		return;

	//倍数
	uint multiples = m_iDmg / i;

	//防止同一倍数重复触发
	if (multiples == m_iDmgMultiples || multiples == 0)
		return;

	m_iDmgMultiples = multiples;

	//大于9
	if (multiples >= 10)
	{
		m_iDmgMultiples = 0;
		m_iDmg = 0;

		for (uint i = 0; i < m_pDmgMarks.size(); i++)
			ResetDmgMark(i);

		PlaySoundByFmod(m_szKillSound, pCfefxSoundVolume->value);
		UpdateScoreEffect();

		return;
	}

	uint x = 0;
	uint y = 0;
	uint z = 0;

	if (multiples < 5)
	{
		x = 4;
		y = DMGMARK_ARRAY_ENDINDEX;
		z = 0;
	}
	else
	{
		x = 0;
		y = 4;
		z = 4;
	}

	for (uint i = x; i < y; i++)
		ResetDmgMark(i);
	
	for (uint i = z; i < multiples; i++)
		UpdateDmgMark(i);
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

void CCfefxPanel::Reset() 
{
	for (uint i = 0; i < DMGMARK_ARRAY_ENDINDEX; i++)
		ResetDmgMark(i);

	ResetScoreEffect();

	m_iDmgMultiples = 0;
	m_iDmg = 0;
}

void CCfefxPanel::ResetDmgMark(int index)
{
	// 停止动画并还原状态
	vgui::GetAnimationController()->StopAnimationSequence(this, m_szMarkAnims[index]);
	m_pDmgMarks[index]->SetAlpha(0);

	vgui::GetAnimationController()->StopAnimationSequence(this, m_szStarAnims[index]);
	m_pDmgStars[index]->SetAlpha(0);
	m_pDmgStars[index]->SetPos(m_vecDmgStarsPos.x, m_vecDmgStarsPos.y);
	m_pDmgStars[index]->SetSize(m_vecDmgStarsSize.x, m_vecDmgStarsSize.y);
}

void CCfefxPanel::ResetScoreEffect()
{
	if (m_pScoreEffect->GetAlpha() != 0 || 
		m_pScoreEffect->GetXPos() != m_vecScoreEffectPos.x ||
		m_pScoreMark->GetAlpha() != 0 ||
		m_pScoreMark->GetXPos() != m_vecScoreMarkPos.x)
	{
		m_pScoreMark->SetAlpha(0);
		m_pScoreMark->SetBounds(m_vecScoreMarkPos.x, m_vecScoreMarkPos.y, m_vecScoreMarkSize.x, m_vecScoreMarkSize.y);

		m_pScoreEffect->SetAlpha(0);
		m_pScoreEffect->SetBounds(m_vecScoreEffectPos.x, m_vecScoreEffectPos.y, m_vecScoreEffectSize.x, m_vecScoreEffectSize.y);

		vgui::GetAnimationController()->CancelAnimationsForPanel(m_pScoreEffect);
		vgui::GetAnimationController()->StopAnimationSequence(this, "ScoreMarkAnim");

	}
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

#pragma endregion


