#if !defined( INS_FX_H )
#define INS_FX_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#define FX_INS_ATTACHMENT_MUZZLEFLASH 1
#define FX_INS_ATTACHMENT_SHELLEJECT 2

//=========================================================
//=========================================================
#define FX_FLAGS_FIRSTPERSON	( 1<<0 )
#define FX_FLAGS_NOLIGHT		( 1<<1 )
#define FX_FLAGS_NOFLASH		( 1<<2 )

//=========================================================
//=========================================================
C_BaseCombatWeapon* FX_INS_GetWeapon(ClientEntityHandle_t hViewModelEntity);

//=========================================================
//=========================================================
C_BaseCombatWeapon* FX_INS_GetSource(ClientEntityHandle_t hEntity, Vector& vecSource, QAngle& angSource, int iAttachIndex, bool bFirstPerson);

//=========================================================
//=========================================================
void FX_INS_MuzzleFlash(C_BaseCombatWeapon* pWeapon, const Vector& vecOrigin, const QAngle& vecAngles, bool bFirstPerson);

//=========================================================
//=========================================================
void FX_INS_Default_MuzzleEffect(const Vector& pos, const QAngle& angles, int entindex, float scale, int flags);
void FX_INS_Combine_MuzzleEffect(const Vector& vecOrigin, const QAngle& vecAngles, int iEntIndex);

//=========================================================
//=========================================================
void FX_INS_Default_Tracer(const Vector& start, const Vector& end, int type, float velocity);
void FX_INS_Default_TracerSound(const Vector& start, const Vector& end, int type);

#endif // INS_FX_H