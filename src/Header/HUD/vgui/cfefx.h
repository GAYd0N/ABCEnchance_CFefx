#pragma once
#include <array>
#include "soundengine.h"
#include <vgui_controls/EditablePanel.h>
#include "IViewportPanel.h"

#pragma region Class CDmgMarkItem
class CDmgMarkItem : public vgui::EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE(CDmgMarkItem, vgui::EditablePanel);
	CDmgMarkItem(Panel* parent, const char* name, int index);
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	virtual void ApplySettings(KeyValues* inResourceData) override;
	virtual void ShowPanel(bool state);
	virtual void SetImage(const char *imageName);
	void SetDmgMultiples(int multiples);

private:
	enum DmgMarkType
	{
		ONE_HUNDRED,
		FIVE_HUNDRED
	};

	int m_iDmgMarkType;
	int m_iDmgMultiples = 0;
	vgui::ImagePanel* m_pDmgMark = nullptr;
};
#pragma endregion

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
	virtual void OnThink() override;

	void AddDmg(int iDmg);

private:
	//void ShowKillMark(wchar_t* killer);
	void StartFade(vgui::Panel* panel, bool state, float fadetime, float = 0);
	void PlaySoundByFmod(const char* name, float volume);
	void ShowDmgMark(CDmgMarkItem* panel);
	void ShowScoreEffect();
	int VecPos(CDmgMarkItem* panel);

	int m_iDmgTimes;
	int m_iDmg;
	Vector m_vecScoreEffectPos;
	Vector m_vecScoreEffectSize;
	Vector m_vecScoreMarkPos;
	Vector m_vecDmgStarsPos;
	Vector m_vecDmgStarsSize;

	vgui::ImagePanel* m_pScoreMark = nullptr;
	vgui::ImagePanel* m_pScoreEffect = nullptr;
	//vgui::ImagePanel* m_pKillMark = nullptr;

	std::array<CDmgMarkItem*, 5> m_pDmgMarks;
	std::array<vgui::ImagePanel*, 5> m_pDmgStars;
	//std::array<const char*, 6> m_szKillMarks = {
	// "abcenchance/tga/cfefx/badge_multi1",
	// "abcenchance/tga/cfefx/badge_multi2",
	// "abcenchance/tga/cfefx/badge_multi3",
	// "abcenchance/tga/cfefx/badge_multi4",
	// "abcenchance/tga/cfefx/badge_multi5",
	// "abcenchance/tga/cfefx/badge_multi6"
	// };

	const char* m_szKillSound = { "abcenchance/sound/UI_SPECIALKILL2.wav" };
	std::array<const char*, 5> m_szStarAnims = {
		"StarFiveAnim",
		"StarOneAnim",
		"StarTwoAnim",
		"StarThreeAnim",
		"StarFourAnim"
	};
	//std::array<const char*, 5> m_szMarkAnims = {
	//	"DmgMarkFiveFade",
	//	"DmgMarkOneFade",
	//	"DmgMarkTwoFade",
	//	"DmgMarkThreeFade",
	//	"DmgMarkFourFade"
	//};

	FModEngine::CFModSound m_pSound;
	FModEngine::CFModChannel m_pChannel;
};
#pragma endregion

