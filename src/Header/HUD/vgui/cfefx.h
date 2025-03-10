#pragma once
#include <array>
#include "soundengine.h"
#include <vgui_controls/EditablePanel.h>
#include "IViewportPanel.h"

#pragma region Class CfefxPanel

extern const clientdata_t* gClientData;
class CCfefxPanel : public vgui::EditablePanel, public IViewportPanel {
public:
	DECLARE_CLASS_SIMPLE(CCfefxPanel, vgui::EditablePanel);
	CCfefxPanel();
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	virtual void ApplySettings(KeyValues* inResourceData) override;
	// IViewportPanel overrides
	virtual const char* GetName() override;
	virtual void Reset() override;
	virtual void ShowPanel(bool state) override;
	virtual bool IsVisible() override;
	virtual vgui::VPANEL GetVPanel() override;
	virtual void SetParent(vgui::VPANEL parent) override;

	void AddDmg(int iDmg);

private:
	void UpdateAnimations();
	//void ShowKillMark(wchar_t* killer);
	void StartFade(vgui::Panel* panel, bool state, float fadetime, float = 0);
	void PlaySoundByFmod(const char* name, float volume);
	void UpdateDmgMark(uint index);
	void UpdateScoreEffect();
	void ResetDmgMark(int index);
	void ResetScoreEffect();

	//cvar_t*	pCfefxKillTime;
	cvar_t* pCfefxMaxDmg = nullptr;
	cvar_t* pCfefxSoundVolume = nullptr;

	uint m_iDmgMultiples;
	int m_iDmg;

	Vector m_vecScoreEffectPos;
	Vector m_vecScoreEffectSize;
	Vector m_vecScoreMarkPos;
	Vector m_vecScoreMarkSize;
	Vector m_vecDmgStarsPos;
	Vector m_vecDmgStarsSize;

	vgui::ImagePanel* m_pScoreMark = nullptr;
	vgui::ImagePanel* m_pScoreEffect = nullptr;
	//vgui::ImagePanel* m_pKillMark = nullptr;

	std::array<vgui::ImagePanel*, 9> m_aryDmgMarks;
	std::array<vgui::ImagePanel*, 9> m_aryDmgStars;
	//std::array<const char*, 6> m_szKillMarks = {
	// "abcenchance/tga/cfefx/badge_multi1",
	// "abcenchance/tga/cfefx/badge_multi2",
	// "abcenchance/tga/cfefx/badge_multi3",
	// "abcenchance/tga/cfefx/badge_multi4",
	// "abcenchance/tga/cfefx/badge_multi5",
	// "abcenchance/tga/cfefx/badge_multi6"
	// };

	const char* m_szKillSound = { "abcenchance/sound/UI_SPECIALKILL2.wav" };
	std::array<const char*, 9> m_aryStarAnims = {
		"StarOneAnim",
		"StarTwoAnim",
		"StarThreeAnim",
		"StarFourAnim",
		"StarFiveAnim",
		"StarSixAnim",
		"StarSevenAnim",
		"StarEightAnim",
		"StarNineAnim"
	};
	std::array<const char*, 9> m_aryMarkAnims = {
		"DmgMarkOneFade",
		"DmgMarkTwoFade",
		"DmgMarkThreeFade",
		"DmgMarkFourFade",
		"DmgMarkFiveFade",
		"DmgMarkSixFade",
		"DmgMarkSevenFade",
		"DmgMarkEightFade",
		"DmgMarkNineFade"
	};

	FModEngine::CFModSound m_pSound;
	FModEngine::CFModChannel m_pChannel;
};
#pragma endregion

