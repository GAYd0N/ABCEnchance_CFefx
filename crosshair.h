#pragma once
class CHudCustomCrosshair : public CHudBase
{
public:
	int Init(void);
	int Draw(float flTime);
	void Reset(void);
private:
	cvar_t* pCvarDefaultFOV = NULL;
	cvar_t* pCvarDefaultCrosshair = NULL;
	void DrawCrosshairSPR(int x, int y, int hPic, wrect_t hRc);
	void DrawDefaultCrosshair(float flTime, int x, int y);
};
extern CHudCustomCrosshair m_HudCrosshair;