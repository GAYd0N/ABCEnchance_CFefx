#include "enginedef.h"
#include "r_studioint.h"

#define PLUGIN_VERSION 100020

class CGameStudioModelRenderer;

extern cl_enginefunc_t gEngfuncs;
extern cl_exportfuncs_t gExportfuncs;
extern engine_studio_api_t IEngineStudio;
extern CGameStudioModelRenderer* g_StudioRenderer;

extern const clientdata_t* gClientData;

extern overviewInfo_t* gDevOverview;
extern refdef_t* g_refdef;
extern metaplugins_t g_metaplugins;

//Hook
void AddHook(hook_t* h);
void AddEngineHook(hook_t* h);

void CheckOtherPlugin();
void FillEngineAddress();
void FillAddress();
void InstallEngineHook();
void InstallClientHook();
void UninstallEngineHook();
void UninstallClientHook();
void CheckAsset();

void GL_Init(void);
void HUD_Init(void);
int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s** ppinterface, struct engine_studio_api_s* pstudio);
int HUD_VidInit(void);
void HUD_Shutdown(void);
int HUD_Redraw(float time, int intermission);
void HUD_TxferLocalOverrides(struct entity_state_s* state, const struct clientdata_s* client);
int HUD_UpdateClientData(struct client_data_s* c, float f);
void HUD_ClientMove(struct playermove_s* ppmove, qboolean server);
void V_CalcRefdef(struct ref_params_s* pparams);
void IN_MouseEvent(int mstate);
void CL_CreateMove(float frametime, struct usercmd_s* cmd, int active);
int HUD_KeyEvent(int eventcode, int keynum, const char* pszCurrentBinding);
int HUD_AddEntity(int type, struct cl_entity_s* ent, const char* modelname);
void HUD_VoiceStatus(int entindex, qboolean talking);
void HUD_Frame(double frametime);
void HUD_TxferPredictionData(struct entity_state_s* ps, const struct entity_state_s* pps, struct clientdata_s* pcd, const struct clientdata_s* ppcd, struct weapon_data_s* wd, const struct weapon_data_s* pwd);
void HUD_TempEntUpdate(
	double frametime,   // Simulation time
	double client_time, // Absolute time on client
	double cl_gravity,  // True gravity on client
	TEMPENTITY** ppTempEntFree,   // List of freed temporary ents
	TEMPENTITY** ppTempEntActive, // List 
	int		(*Callback_AddVisibleEntity)(cl_entity_t* pEntity),
	void	(*Callback_TempEntPlaySound)(TEMPENTITY* pTemp, float damp));
void HUD_DrawTransparentTriangles();

void R_NewMap(void);

#define Fill_Sig(sig, base, size, dst) {gHookFuncs.dst = (decltype(gHookFuncs.dst))g_pMetaHookAPI->SearchPattern(base, size, sig, Sig_Length(sig));Sig_FuncNotFound(dst);}
#define GetCallAddress(addr) (addr + (*(int *)((addr)+1)) + 5)
#define Sig_NotFound(name) SYS_ERROR("Could not found: %s\nEngine buildnum��%d", #name, g_dwEngineBuildnum);
#define Sig_FuncNotFound(name) if(!gHookFuncs.name) Sig_NotFound(name)
#define Sig_AddrNotFound(name) if(!addr) Sig_NotFound(name)
#define Sig_AddrFoundOrFill(name) Sig_AddrNotFound(name) else name = (decltype(name))addr;
#define Sig_VarNotFound(name) if(!name) Sig_NotFound(name)

#define Sig_Length(a) (sizeof(a)-1)
#define Search_Pattern(sig) g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, sig, Sig_Length(sig));
#define Search_Pattern_From_Size(fn, size, sig) g_pMetaHookAPI->SearchPattern((void *)(fn), size, sig, Sig_Length(sig))
#define Search_Pattern_From(fn, sig) g_pMetaHookAPI->SearchPattern((void *)fn, ((PUCHAR)g_dwEngineTextBase + g_dwEngineTextSize) - (PUCHAR)fn, sig, Sig_Length(sig))
#define Search_Pattern_Data(sig) g_pMetaHookAPI->SearchPattern(g_dwEngineDataBase, g_dwEngineDataSize, sig, Sig_Length(sig))
#define Search_Pattern_Rdata(sig) g_pMetaHookAPI->SearchPattern(g_dwEngineRdataBase, g_dwEngineRdataSize, sig, Sig_Length(sig))

#define Fill_Func(dest, src) gHookFuncs.dest = *src
#define Fill_EfxFunc(fn) gHookFuncs.fn = *gEngfuncs.pEfxAPI->fn
#define Fill_EngFunc(fn) gHookFuncs.fn = *gEngfuncs.fn
#define Install_InlineHook(fn) AddHook(g_pMetaHookAPI->InlineHook((void *)gHookFuncs.fn, fn, (void **)&gHookFuncs.fn))
#define Install_InlineEngHook(fn) AddEngineHook(g_pMetaHookAPI->InlineHook((void *)*gHookFuncs.fn, fn, (void **)&gHookFuncs.fn))
#define Fill_InlineEfxHook(fn) Fill_EfxFunc(fn);Install_InlineEngHook(fn)
#define Uninstall_Hook(fn) if(##fn){g_pMetaHookAPI->UnHook(##fn);##fn = NULL;}