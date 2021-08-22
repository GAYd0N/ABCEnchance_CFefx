#include <metahook.h>
#include <parsemsg.h>
#include <math.h>
#include "mathlib.h"
#include "msghook.h"

#include "vguilocal.h"
#include "hud.h"
#include "weapon.h"
#include "CHudDelegate.h"
#include "healthhud.h"

#include "drawElement.h"

#define DAMAGE_NAME "sprites/%d_dmg.spr"
int giDmgHeight, giDmgWidth;
int giDmgFlags[NUM_DMG_TYPES] =
{
	DMG_POISON,
	DMG_ACID,
	DMG_FREEZE | DMG_SLOWFREEZE,
	DMG_DROWN,
	DMG_BURN | DMG_SLOWBURN,
	DMG_NERVEGAS,
	DMG_RADIATION,
	DMG_SHOCK,
	DMG_CALTROP,
	DMG_TRANQ,
	DMG_CONCUSS,
	DMG_HALLUC
};
CHudArmorHealth m_HudArmorHealth;

int __MsgFunc_Health(const char* pszName, int iSize, void* pbuf)
{
	// TODO: update local health data
	BEGIN_READ(pbuf, iSize);
	int x = READ_BYTE();

	m_HudArmorHealth.m_iFlags |= HUD_ACTIVE;

	// Only update the fade if we've changed health
	if (x != m_HudArmorHealth.m_iHealth)
	{
		m_HudArmorHealth.m_fFade = FADE_TIME;
		m_HudArmorHealth.m_iHealth = x;
	}
	return 1;
}
int __MsgFunc_Damage(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int armor = READ_BYTE();
	int damageTaken = READ_BYTE();
	long bitsDamage = READ_LONG();

	vec3_t vecFrom;

	for (int i = 0; i < 3; i++)
		vecFrom[i] = READ_COORD();
	m_HudArmorHealth.UpdateTiles(gEngfuncs.GetClientTime(), bitsDamage);
	if (damageTaken > 0 || armor > 0)
	{
		m_HudArmorHealth.m_takeDamage = damageTaken;
		m_HudArmorHealth.m_takeArmor = armor;
		m_HudArmorHealth.flPainIndicatorKeepTime = gEngfuncs.GetClientTime() + PAIN_INDICAROT_TIME;
		VectorCopy(vecFrom, m_HudArmorHealth.vecDamageFrom);
	}
	return 1;
}
int __MsgFunc_Battery(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int x = READ_SHORT();

	if (x != m_HudArmorHealth.m_iBat)
	{
		m_HudArmorHealth.m_fFade = FADE_TIME;
		m_HudArmorHealth.m_iBat = x;
	}
	return 1;
}

void CHudArmorHealth::Init(void)
{
	HOOK_MESSAGE(Health);
	HOOK_MESSAGE(Damage);
	HOOK_MESSAGE(Battery);

	m_iHealth = 100;
	m_fFade = 0;
	m_iFlags = 0;
	m_bitsDamage = 0;
	m_takeDamage = 0;
	m_takeArmor = 0;
	m_iBat = 0;
	giDmgHeight = 0;
	giDmgWidth = 0;
	memset(m_dmg, 0, sizeof(DAMAGE_IMAGE) * NUM_DMG_TYPES);

	iHealthIcon = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-cross1.spr");
	iArmorIconNull = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-shield.spr");
	iArmorIconFull = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-armor-helmet.spr");
	iPainIndicator = gEngfuncs.pfnSPR_Load("abcenchance/spr/pain_indicator.spr");

	StartX = atof(pScheme->GetResourceString("HealthArmor.StartX"));
	IconSize = atof(pScheme->GetResourceString("HealthArmor.IconSize"));
	TextWidth = atof(pScheme->GetResourceString("HealthArmor.TextWidth"));
	BarLength = atof(pScheme->GetResourceString("HealthArmor.BarLength"));
	BarWidth = atof(pScheme->GetResourceString("HealthArmor.BarWidth"));
	ElementGap = atof(pScheme->GetResourceString("HealthArmor.ElementGap"));
	BackGroundY = atof(pScheme->GetResourceString("HealthArmor.BackGroundY"));
	BackGroundLength = atof(pScheme->GetResourceString("HealthArmor.BackGroundLength"));
	BackGroundAlpha = atof(pScheme->GetResourceString("HealthArmor.BackGroundAlpha"));

	HealthIconColor = pScheme->GetColor("HealthArmor.HealthIconColor", gDefaultColor);
	HealthBarColor = pScheme->GetColor("HealthArmor.HealthBarColor", gDefaultColor);
	HealthTextColor = pScheme->GetColor("HealthArmor.HealthTextColor", gDefaultColor);
	ArmorIconColor = pScheme->GetColor("HealthArmor.ArmorIconColor", gDefaultColor);
	ArmorBarColor = pScheme->GetColor("HealthArmor.ArmorBarColor", gDefaultColor);
	ArmorTextColor = pScheme->GetColor("HealthArmor.ArmorTextColor", gDefaultColor);
	BackGroundColor = pScheme->GetColor("HealthArmor.BackGroundColor", gDefaultColor);
	PainIndicatorColor = pScheme->GetColor("HealthArmor.PainIndicatorColor", gDefaultColor);
	PainIndicatorColorA = pScheme->GetColor("HealthArmor.PainIndicatorColorA", gDefaultColor);

	HUDFont = pScheme->GetFont("HUDShitFont", true);
}
int CHudArmorHealth::VidInit(void)
{
	m_hSprite = 0;

	m_HUD_dmg_bio = gHudDelegate->GetSpriteIndex("dmg_bio") + 1;
	m_HUD_cross = gHudDelegate->GetSpriteIndex("cross");

	giDmgHeight = gHudDelegate->GetSpriteRect(m_HUD_dmg_bio).right - gHudDelegate->GetSpriteRect(m_HUD_dmg_bio).left;
	giDmgWidth = gHudDelegate->GetSpriteRect(m_HUD_dmg_bio).bottom - gHudDelegate->GetSpriteRect(m_HUD_dmg_bio).top;
	return 1;
}
void CHudArmorHealth::Reset(void)
{
	iHealthIcon = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-cross1.spr");
	iArmorIconNull = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-shield.spr");
	iArmorIconFull = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-armor-helmet.spr");
	iPainIndicator = gEngfuncs.pfnSPR_Load("abcenchance/spr/pain_indicator.spr");

	m_iHealth = 100;
	m_fFade = 0;
	m_iFlags = 0;
	m_bitsDamage = 0;
	m_takeDamage = 0;
	m_takeArmor = 0;
	m_bitsDamage = 0;
	m_iBat = 0;

	flPainIndicatorKeepTime = 0.0f;
	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		m_dmg[i].fExpire = 0;
	}
}
int CHudArmorHealth::Draw(float flTime)
{
	if (flTime < flPainIndicatorKeepTime)
		DrawPain(flTime);
	int r, g, b, a;
	float iSizeStep = (float)gScreenInfo.iWidth / 3 / BackGroundAlpha;
	float i = 0, nowX = 0;
	float flBackGroundY = gScreenInfo.iHeight * BackGroundY;
	float flBackGroundHeight = gScreenInfo.iHeight - flBackGroundY;
	BackGroundColor.GetColor(r, g, b, a);
	for (i = BackGroundAlpha; i > 0.0f; i--)
	{
		gEngfuncs.pfnFillRGBABlend(nowX, flBackGroundY, iSizeStep, flBackGroundHeight, r, g, b, i);
		nowX += iSizeStep;
	}
	int iStartX = gScreenInfo.iWidth / StartX;
	int iIconSize = flBackGroundHeight * IconSize;
	int iTextWidth = flBackGroundHeight * TextWidth;
	int iTextHeight = flBackGroundHeight * TextHeight;
	int iBarLength = flBackGroundHeight * BarLength;
	int iBarWidth = flBackGroundHeight * BarWidth;
	int iElementGap = flBackGroundHeight * ElementGap;
	int iHealth = m_iHealth;
	int iBattery = m_iBat;
	//HP ICON
	if (!iHealthIcon)
		iHealthIcon = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-cross1.spr");
	HealthIconColor.GetColor(r, g, b, a);
	DrawSPRIcon(iHealthIcon, iStartX, flBackGroundY + (flBackGroundHeight - iIconSize) / 2, iIconSize, iIconSize, r, g, b, a);

	char numberString[16];
	itoa(iHealth, numberString, 10);
	wchar_t wideName[8];
	pLocalize->ConvertANSIToUnicode(numberString, wideName, sizeof(wideName));
	int iSzWidth = 0;
	GetStringSize(wideName, &iSzWidth, NULL, HUDFont);
	iStartX += iIconSize + iElementGap + iTextWidth - iSzWidth;
	HealthTextColor.GetColor(r, g, b, a);
	DrawVGUI2String(wideName, iStartX, flBackGroundY + (flBackGroundHeight - iTextHeight) / 2, r, g, b, HUDFont);
	iStartX += iSzWidth + iElementGap;

	HealthBarColor.GetColor(r, g, b, a);
	gEngfuncs.pfnFillRGBABlend(iStartX, flBackGroundY + (flBackGroundHeight - iBarWidth) / 2, iBarLength, iBarWidth, r / 2, g / 2, b / 2, a);
	gEngfuncs.pfnFillRGBABlend(iStartX, flBackGroundY + (flBackGroundHeight - iBarWidth) / 2, iBarLength * max(0, min(1, (float)iHealth / 100)), iBarWidth, r, g, b, a);
	iStartX += iBarLength + iElementGap * 4;

	//AP
	if (!iArmorIconNull || !iArmorIconFull)
		iArmorIconNull = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-shield.spr");
	if (!iArmorIconFull)
		iArmorIconFull = gEngfuncs.pfnSPR_Load("abcenchance/spr/icon-armor-helmet.spr");
	ArmorIconColor.GetColor(r, g, b, a);
	DrawSPRIcon(iBattery > 0 ? iArmorIconFull : iArmorIconNull, iStartX, flBackGroundY + (flBackGroundHeight - iIconSize) / 2, iIconSize, iIconSize, r, g, b, a);

	itoa(iBattery, numberString, 10);
	pLocalize->ConvertANSIToUnicode(numberString, wideName, sizeof(wideName));
	GetStringSize(wideName, &iSzWidth, NULL, HUDFont);
	iStartX += iIconSize + iElementGap + iTextWidth - iSzWidth;
	ArmorTextColor.GetColor(r, g, b, a);
	DrawVGUI2String(wideName, iStartX, flBackGroundY + (flBackGroundHeight - iTextHeight) / 2, r, g, b, HUDFont);
	iStartX += iSzWidth + iElementGap;

	ArmorBarColor.GetColor(r, g, b, a);
	gEngfuncs.pfnFillRGBABlend(iStartX, flBackGroundY + (flBackGroundHeight - iBarWidth) / 2, iBarLength, iBarWidth, r / 2, g / 2, b / 2, a);
	gEngfuncs.pfnFillRGBABlend(iStartX, flBackGroundY + (flBackGroundHeight - iBarWidth) / 2, iBarLength * max(0, min(1, (float)iBattery / 100)), iBarWidth, r, g, b, a);
	iStartX += iBarLength + iElementGap * 2;
	return DrawDamage(flTime);
}
void CHudArmorHealth::CalcDamageDirection(vec3_t vecFrom)
{
	cl_entity_t* local = gEngfuncs.GetLocalPlayer();
	float yaw = local->curstate.angles[1];
	if (yaw < 0)
		yaw += 360;
	yaw *= (M_PI / 180.0);
	//����Ļ����Ϊ�����������ϵ
	float sprWidth = gScreenInfo.iHeight * 0.1667;
	float y1 = gScreenInfo.iHeight / 4;
	float y2 = y1 + sprWidth;
	/*  ��ת�任
	*                ^
	*                |y
	*           A----------B
	*           |    |     |
	*           C----------D
	*                |
	*  --------------+----------------->x
	*/
	vec2_t vecA, vecB, vecC, vecD;
	vecA[0] = -sprWidth * cos(yaw) + y2 * sin(yaw);
	vecA[1] = sprWidth * sin(yaw) + y2 * cos(yaw);
	vecB[0] = sprWidth * cos(yaw) + y2 * sin(yaw);
	vecB[1] = -sprWidth * sin(yaw) + y2 * cos(yaw);
	vecC[0] = -sprWidth * cos(yaw) + y1 * sin(yaw);
	vecC[1] = sprWidth * sin(yaw) + y1 * cos(yaw);
	vecD[0] = sprWidth * cos(yaw) + y1 * sin(yaw);
	vecD[1] = -sprWidth * sin(yaw) + y1 * cos(yaw);
	//�任ΪOpenGL��Ļ����
	int halfWidth = gScreenInfo.iWidth / 2;
	int halfHeight = gScreenInfo.iHeight / 2;
	vecA[0] += halfWidth;
	vecA[1] = halfHeight - vecA[1];
	vecB[0] += halfWidth;
	vecB[1] = halfHeight - vecB[1];
	vecC[0] += halfWidth;
	vecC[1] = halfHeight - vecC[1];
	vecD[0] += halfWidth;
	vecD[1] = halfHeight - vecD[1];

	Vector2Copy(vecA, vecPainIndicatorA);
	Vector2Copy(vecB, vecPainIndicatorB);
	Vector2Copy(vecC, vecPainIndicatorC);
	Vector2Copy(vecD, vecPainIndicatorD);
}
int CHudArmorHealth::DrawPain(float flTime)
{
	int r, g, b, a;
	if(m_takeDamage <= 0 && m_takeArmor > 0)
		PainIndicatorColorA.GetColor(r, g, b, a);
	else
		PainIndicatorColor.GetColor(r, g, b, a);

	if (!iPainIndicator)
		iPainIndicator = gEngfuncs.pfnSPR_Load("abcenchance/spr/pain_indicator.spr");
	CalcDamageDirection(vecDamageFrom);
	DrawSPRIconPos(iPainIndicator, vecPainIndicatorA, vecPainIndicatorC, vecPainIndicatorD, vecPainIndicatorB, 
		r, g, b, 
		(flPainIndicatorKeepTime - flTime) / PAIN_INDICAROT_TIME * a);
	return 1;
}
int CHudArmorHealth::DrawDamage(float flTime)
{
	int r, g, b, a;
	DAMAGE_IMAGE* pdmg;
	if (!m_bitsDamage)
		return 1;
	BitDamageColor.GetColor(r, g, b, a);
	a = (int)(fabs(sin(flTime * 2)) * 256.0);
	int i;
	for (i = 0; i < NUM_DMG_TYPES; i++)
	{
		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg = &m_dmg[i];
			SPR_Set(gHudDelegate->GetSprite(m_HUD_dmg_bio + i), r, g, b);
			SPR_DrawAdditive(0, pdmg->x, pdmg->y, &gHudDelegate->GetSpriteRect(m_HUD_dmg_bio + i));
		}
	}
	for (i = 0; i < NUM_DMG_TYPES; i++)
	{
		DAMAGE_IMAGE* pdmg = &m_dmg[i];

		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg->fExpire = min(flTime + DMG_IMAGE_LIFE, pdmg->fExpire);

			if (pdmg->fExpire <= flTime	&& a < 40)
			{
				pdmg->fExpire = 0;

				int y = pdmg->y;
				pdmg->x = pdmg->y = 0;
				for (int j = 0; j < NUM_DMG_TYPES; j++)
				{
					pdmg = &m_dmg[j];
					if ((pdmg->y) && (pdmg->y < y))
						pdmg->y += giDmgHeight;
				}
				m_bitsDamage &= ~giDmgFlags[i];
			}
		}
	}
	return 1;
}
void CHudArmorHealth::UpdateTiles(float flTime, long bitsDamage)
{
	DAMAGE_IMAGE* pdmg;
	long bitsOn = ~m_bitsDamage & bitsDamage;
	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		pdmg = &m_dmg[i];
		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg->fExpire = flTime + DMG_IMAGE_LIFE; // extend the duration
			if (!pdmg->fBaseline)
				pdmg->fBaseline = flTime;
		}
		if (bitsOn & giDmgFlags[i])
		{
			pdmg->x = giDmgWidth / 8;
			pdmg->y = ScreenHeight - giDmgHeight * 2;
			pdmg->fExpire = flTime + DMG_IMAGE_LIFE;
			for (int j = 0; j < NUM_DMG_TYPES; j++)
			{
				if (j == i)
					continue;
				pdmg = &m_dmg[j];
				if (pdmg->y)
					pdmg->y -= giDmgHeight;

			}
			pdmg = &m_dmg[i];
		}
	}
	m_bitsDamage |= bitsDamage;
}