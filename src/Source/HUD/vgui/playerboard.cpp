#pragma once
#include <vector>
#include <metahook.h>

#include <vgui/IImage.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>

#include "local.h"
#include "vguilocal.h"
#include "steamclientpublic.h"
#include "player_info.h"
#include "triangleapi.h"

#include "vgui_controls/ImagePanel.h"
#include "playerboard.h"
#include "Viewport.h"

#include "plugins.h"
#include <mymathlib.h>
#include <CVector.h>

#define VIEWPORT_PLAYERBOARD_NAME "PlayerBoardPanel"

namespace {
	class CBarImage : public vgui::IImage {
	public:
		CBarImage(int tex) {
			m_iTexture = tex;
		}
		void SetTexture(int level) {
			m_iTexture = level;
		}
		virtual void Paint() override {
			if (m_iTexture < 0)
				return;
			vgui::surface()->DrawSetTexture(m_iTexture);
			vgui::surface()->DrawSetColor(m_DrawColor);
			vgui::surface()->DrawTexturedRect(m_iX + m_iOffX, m_iY + m_iOffY,
				m_iX + m_iOffX + m_iWide, m_iY + m_iOffY + m_iTall);
		}
		// Get the size the image will actually draw in (usually defaults to the content size)
		virtual void GetSize(int& wide, int& tall) override {
			GetContentSize(wide, tall);
		}
		// Sets the size of the image
		virtual void SetSize(int wide, int tall) override {
			m_iWide = wide;
			m_iTall = tall;
		}
		// Set the position of the image
		virtual void SetPos(int x, int y) override {
			m_iX = x;
			m_iY = y;
		}
		virtual void SetOffset(int x, int y) {
			m_iOffX = x;
			m_iOffY = y;
		}
		// Gets the size of the content
		virtual void GetContentSize(int& wide, int& tall) override {
			wide = m_iWide;
			tall = m_iTall;
		}
		// Set the draw color
		virtual void SetColor(Color col) override {
			m_DrawColor = col;
		}
	private:
		int m_iTexture = 0;
		int m_iX = 0, m_iY = 0;
		int m_iOffX = 0, m_iOffY = 0;
		int m_iWide = 0, m_iTall = 0;
		Color m_DrawColor = Color(255, 255, 255, 255);
	};
}
CPlayerInfoPanel::CPlayerInfoPanel()
	: BaseClass(nullptr, VIEWPORT_PLAYERBOARD_NAME)
{
	SetCloseButtonVisible(false);
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	vgui::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "PlayerInfoScheme.res", "PlayerInfoScheme");
	SetScheme("PlayerInfoScheme");

	// Header labels
	m_pNameLable = new vgui::Label(this, "PlayerName", "(Anon)");
	m_pBackgroundImagePanel = new vgui::ImagePanel(this, "BackGroundImage");
	m_pHealthImagePanel = new vgui::ImagePanel(this, "HealthImage");
	m_pArmorImagePanel = new vgui::ImagePanel(this, "ArmorImage");
	m_pCourchIconImagePanel = new vgui::ImagePanel(this, "CourchIconImage");
	m_pMedikIconImagePanel = new vgui::ImagePanel(this, "MedikIconImage");
	m_pDeadIconImagePanel = new vgui::ImagePanel(this, "DeadIconImage");

	pPlayerTitle = CREATE_CVAR("cl_playertitle", "1", FCVAR_VALUE, nullptr);

	LoadControlSettings(VGUI2_ROOT_DIR "PlayerBoardPanel.res");
	SetVisible(false);
}

void CPlayerInfoPanel::SetId(int index){
	m_iPlayerIndex = index + 1;
}

void CPlayerInfoPanel::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(GetSchemeColor("PlayerBoardPanel.BgColor", GetSchemeColor("Panel.BgColor", pScheme), pScheme));
	SetTitleBarVisible(false);
}

void CPlayerInfoPanel::Think(){
	cl_entity_t* local = gEngfuncs.GetLocalPlayer();
	if(!local)
		return;
	if (local->index == m_iPlayerIndex)
		return;
	//�ӽǽǶ�
	vec3_t vecView;
	CVector vecLength;
	float angledotResult;
	CVector vecEntityOrigin;
	CVector vecHUD;
	gEngfuncs.GetViewAngles(vecView);
	mathlib::AngleVectors(vecView, vecView, nullptr, nullptr);
	cl_entity_t* entity = gEngfuncs.GetEntityByIndex(m_iPlayerIndex);
	if (!entity ||
		entity->curstate.messagenum != local->curstate.messagenum ||
		!entity->player || !entity->model) {
		SetVisible(false);
		return;
	}
	//�����Һ�Ŀ������ƫ��
	mathlib::VectorSubtract(entity->curstate.origin, local->curstate.origin, vecLength);
	if (vecLength.FLength() >= 1024) {
		SetVisible(false);
		return;
	}
	vecLength = vecLength.Normalize();
	angledotResult = mathlib::DotProduct(vecLength, vecView);
	//cos 60
	if (angledotResult > 0.5) {
		vecEntityOrigin = entity->curstate.origin;
		vecEntityOrigin.z += 45;
		VEC_WorldToScreen(vecEntityOrigin, vecHUD);
		this->SetPos(vecHUD.x - this->GetWide() / 2, vecHUD.y - this->GetCaptionHeight());
	}
	else{
		SetVisible(false);
		return;
	}
	UpdateClientInfo();
}

const char* CPlayerInfoPanel::GetName()
{
	return VIEWPORT_PLAYERBOARD_NAME;
}

void CPlayerInfoPanel::Reset()
{
	if (IsVisible())
	{
		ShowPanel(false);
	}
}

void CPlayerInfoPanel::ShowPanel(bool state)
{
	if (state == IsVisible())
		return;

	if (state)
	{
		Activate();
	}
	else
	{
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
	}
	SetVisible(state);
}

vgui::VPANEL CPlayerInfoPanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CPlayerInfoPanel::IsVisible()
{
	return BaseClass::IsVisible();
}

void CPlayerInfoPanel::SetParent(vgui::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CPlayerInfoPanel::UpdateClientInfo(){
	CPlayerInfo* pi = GetPlayerInfo(m_iPlayerIndex);
	if (pi->IsValid()) {
		if (pi->IsSpectator() || pi->GetTeamNumber() != GetThisPlayerInfo()->GetTeamNumber()) {
			ShowPanel(false);
			return;
		}	
		int iHealth = pi->GetHealth();
		float flArmorRatio = mathlib::clamp((float)pi->GetArmor() / 100.0f, 0.0f, 1.0f);
		float flHealthRatio = mathlib::clamp((float)iHealth / 100.0f, 0.0f, 1.0f);
		m_pNameLable->SetText(pi->GetName());
		m_pNameLable->SetFgColor(g_pViewPort->GetPlayerColor(m_iPlayerIndex));
		m_pArmorImagePanel->SetWide(m_pBackgroundImagePanel->GetWide() * flArmorRatio);
		m_pHealthImagePanel->SetWide(m_pBackgroundImagePanel->GetWide() * flHealthRatio);
		cl_entity_t* ent = gEngfuncs.GetEntityByIndex(m_iPlayerIndex);
		if (iHealth < m_iDangerHealth) {
			if (iHealth <= 0){
				m_pCourchIconImagePanel->SetVisible(false);
				m_pMedikIconImagePanel->SetVisible(false);
				m_pDeadIconImagePanel->SetVisible(true);
			}
			else {
				m_pCourchIconImagePanel->SetVisible(false);
				m_pMedikIconImagePanel->SetVisible(true);
				m_pDeadIconImagePanel->SetVisible(false);
			}
		}
		else if (fabs(ent->curstate.maxs[2] - ent->curstate.mins[2]) < 64) {
			m_pCourchIconImagePanel->SetVisible(true);
			m_pMedikIconImagePanel->SetVisible(false);
			m_pDeadIconImagePanel->SetVisible(false);
		}
		else {
			m_pCourchIconImagePanel->SetVisible(false);
			m_pMedikIconImagePanel->SetVisible(false);
			m_pDeadIconImagePanel->SetVisible(false);
		}
		ShowPanel(true);
	}
	else {
		ShowPanel(false);
	}
	
}