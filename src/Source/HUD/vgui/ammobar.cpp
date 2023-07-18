#pragma once
#include <map>
#include <metahook.h>

#include <vgui/IImage.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/ILocalize.h>
#include <vgui2/spr_image.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui_controls/ImageSprPanel.h"
#include <vgui_controls/AnimationController.h>

#include <hud.h>
#include "local.h"
#include <vguilocal.h>
#include "weapon.h"
#include <weaponbank.h>

#include "Viewport.h"
#include "ammobar.h"

#define VIEWPORT_AMMO_NAME "AmmoPanel"

using namespace vgui;

CAmmoPanel::CAmmoPanel()
	: BaseClass(nullptr, VIEWPORT_AMMO_NAME){
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	vgui::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "AmmoScheme.res", "AmmoScheme");
	SetScheme("AmmoScheme");

	m_pBackground = new ImagePanel(this, "Background");

	m_Ammo1Icon = new ImageSprPanel(this, "Ammo1Icon");
	m_pAmmo1Label = new Label(this, "Ammo1", "0");
	m_pSpr1 = new CSPRImage();

	m_pSlashLabel = new Label(this, "Slash", "|");

	m_Ammo2Icon = new ImageSprPanel(this, "Ammo2Icon");
	m_pAmmo2Label = new Label(this, "Ammo2", "0");
	m_pSpr2 = new CSPRImage();
	LoadControlSettings(VGUI2_ROOT_DIR "AmmoPanel.res");
}
const char* CAmmoPanel::GetName(){
	return VIEWPORT_AMMO_NAME;
}
void CAmmoPanel::Reset(){
	if (!IsVisible())
		ShowPanel(true);
}
void CAmmoPanel::ApplySchemeSettings(vgui::IScheme* pScheme){
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(GetSchemeColor("AmmoBar.BgColor", GetSchemeColor("Panel.BgColor", pScheme), pScheme));
	m_pAmmo1Label->SetFgColor(GetSchemeColor("AmmoBar.Ammo1FgColor", GetSchemeColor("Label.FgColor", pScheme), pScheme));
	m_pAmmo2Label->SetFgColor(GetSchemeColor("AmmoBar.Ammo2FgColor", GetSchemeColor("Label.FgColor", pScheme), pScheme));
	m_pSlashLabel->SetFgColor(GetSchemeColor("AmmoBar.SlashFgColor", GetSchemeColor("Label.FgColor", pScheme), pScheme));
	m_Ammo1Icon->SetDrawColor(GetSchemeColor("AmmoBar.Ammo1IconColor", GetSchemeColor("Panel.FgColor", pScheme), pScheme));
	m_Ammo2Icon->SetDrawColor(GetSchemeColor("AmmoBar.Ammo2IconColor", GetSchemeColor("Panel.FgColor", pScheme), pScheme));
}
void CAmmoPanel::ShowPanel(bool state){
	if (state == IsVisible())
		return;
	SetVisible(state);
}
bool CAmmoPanel::IsVisible(){
	return BaseClass::IsVisible();
}
VPANEL CAmmoPanel::GetVPanel(){
	return BaseClass::GetVPanel();
}
void CAmmoPanel::SetParent(vgui::VPANEL parent){
	BaseClass::SetParent(parent);
}

void CAmmoPanel::SetWeapon(WEAPON* weapon){
	if (!weapon)
		return;
	char buf[64];
	if (weapon->iAmmoType > 0) {
		m_Ammo1Icon->SetVisible(true);
		m_pAmmo1Label->SetVisible(true);
		m_pSpr1->SetTextureID(weapon->hAmmo);
		m_pSpr1->SetRect(weapon->rcAmmo.left, weapon->rcAmmo.right, weapon->rcAmmo.top, weapon->rcAmmo.bottom);
		m_pSpr1->SetRenderMode(kRenderTransAdd);
		m_Ammo1Icon->SetImage(m_pSpr1);
		int iAmmoLeft = gWR.CountAmmo(weapon->iAmmoType);
		if (weapon->iClip >= 0) {
			if (weapon->iFlags & 32 && weapon->iClip2 >= 0 && weapon->iClip2 != 255)
				Q_snprintf(buf, "%dx%d/%d", weapon->iClip2, weapon->iClip, iAmmoLeft);
			else
				Q_snprintf(buf, "%d/%d", weapon->iClip, iAmmoLeft);
		}
		else
			Q_snprintf(buf, "%d", iAmmoLeft);
		m_pAmmo1Label->SetText(buf);
	}
	else {
		m_Ammo1Icon->SetVisible(false);
		m_pAmmo1Label->SetVisible(false);
	}
	if (weapon->iAmmo2Type > 0) {
		m_Ammo2Icon->SetVisible(true);
		m_pAmmo2Label->SetVisible(true);
		m_pSlashLabel->SetVisible(true);
		m_pSpr2->SetTextureID(weapon->hAmmo2);
		m_pSpr2->SetRect(weapon->rcAmmo2.left, weapon->rcAmmo2.right, weapon->rcAmmo2.top, weapon->rcAmmo2.bottom);
		m_pSpr2->SetRenderMode(kRenderTransAdd);
		m_Ammo2Icon->SetImage(m_pSpr2);
		int iAmmoLeft2 = gWR.CountAmmo(weapon->iAmmo2Type);
		if (weapon->iClip2 >= 0)
			Q_snprintf(buf, "%d/%d", weapon->iClip2, iAmmoLeft2);
		else
			Q_snprintf(buf, "%d", iAmmoLeft2);
		m_pAmmo2Label->SetText(buf);
	}
	else {
		m_Ammo2Icon->SetVisible(false);
		m_pAmmo2Label->SetVisible(false);
		m_pSlashLabel->SetVisible(false);
	}
}