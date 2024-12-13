//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client-side CINSPlayer.
//
//=============================================================================//

#include "cbase.h"
#include "c_ins_player.h"
#include "c_ins_obj.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"
#include "view.h"
#include "basic_colors.h"
#include "weapondef.h"
#include "insvgui.h"
#include "weapon_ins_base.h"
#include "ins_gamerules.h"
#include "ins_utils.h"
#include "inshud.h"
#include "takedamageinfo.h"
#include "iviewrender_beams.h"
#include "r_efx.h"
#include "dlight.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined( CINSPlayer )
#undef CINSPlayer	
#endif

//=========================================================
//=========================================================

#define MAX_LOCAL_DISTANCE 512
#define MAX_TARGET_DISTANCE 128

//=========================================================
//=========================================================
void __MsgFunc_AngleHack(bf_read &msg)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	QAngle aAngles;

	aAngles[0] = msg.ReadFloat();
	aAngles[1] = msg.ReadFloat();
	aAngles[2] = msg.ReadFloat();

	pPlayer->SnapEyeAngles(aAngles, FIXANGLE_ABSOLUTE);
}

//=========================================================
//=========================================================
void __MsgFunc_ObjOrder( bf_read &msg )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( pPlayer )
		pPlayer->HandleObjOrder( msg );
}

//=========================================================
//=========================================================
void __MsgFunc_UnitOrder( bf_read &msg )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( pPlayer )
		pPlayer->HandleUnitOrder( msg );
}

//=========================================================
//=========================================================
void RecvProxy_CurrentObj(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	C_INSPlayer *pPlayer = (C_INSPlayer*)pStruct;
	Assert(pPlayer);

	int iValue = pData->m_Value.m_Int - 1;

	C_INSObjective *pCurrentObj = NULL;

	if(iValue != INVALID_OBJECTIVE)
		pCurrentObj = C_INSObjective::GetObjective(iValue);

	pPlayer->SetCurrentObj(pCurrentObj);
}

//=========================================================
//=========================================================
void RecvProxy_AllowHeadTurn( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_INSPlayer *pPlayer = ( C_INSPlayer* )pStruct;
	Assert( pPlayer );

	pPlayer->SetAllowHeadTurn( pData->m_Value.m_Int );
}

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(player, C_INSPlayer);

BEGIN_RECV_TABLE_NOBASE(C_INSPlayer, DT_INSPlayerExclusive)

	RecvPropDataTable	( RECVINFO_DT(m_INSLocal),0, &REFERENCE_RECV_TABLE(DT_INSLocal) ),

	RecvPropBool( RECVINFO(m_bCommander)),

	RecvPropBool(RECVINFO(m_bSpawnedViewpoint)),
	RecvPropInt("currentobj", 0, SIZEOF_IGNORE, 0, RecvProxy_CurrentObj),

	RecvPropEHandle(RECVINFO(m_hClippingEntity)),
	RecvPropEHandle(RECVINFO(m_hMantleEntity)),

	RecvPropInt(RECVINFO(m_iStamina)),
	RecvPropTime(RECVINFO(m_flStaminaUpdateThreshold)),

	RecvPropTime	(RECVINFO(m_flStartBunnyHopTime)),
	RecvPropFloat	(RECVINFO(m_flBunnyHopLength)),

	RecvPropBool(RECVINFO(m_bIsCustomized)),
	
	RecvPropInt(RECVINFO(m_iDamageDecay)),
	RecvPropTime(RECVINFO(m_flDamageDecayThreshold)),

	RecvPropFloat( RECVINFO( m_angShakyHands[0] ) ),
	RecvPropFloat( RECVINFO( m_angShakyHands[1] ) ),
	RecvPropFloat(RECVINFO(m_flRadius)),
	RecvPropFloat(RECVINFO(m_flRadiusDesired)),

	RecvPropArray3(RECVINFO_ARRAY(m_iCmdRegister), RecvPropInt(RECVINFO(m_iCmdRegister[0]))),

	RecvPropBool	(RECVINFO(m_bTKPunished)),

	RecvPropTime(RECVINFO( m_flNextStanceThreshold ) ),

	RecvPropTime(RECVINFO(m_flViewTransitionLength)),
	RecvPropTime(RECVINFO(m_flViewTransitionEnd)),
	RecvPropInt(RECVINFO(m_iViewTransitionFrom)),
	RecvPropInt(RECVINFO(m_iViewTransitionTarget)),
	RecvPropInt(RECVINFO(m_iViewOffsetBipod)),

END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT(C_INSPlayer, DT_INSPlayer, CINSPlayer)

	RecvPropDataTable( "ins_localdata", 0, 0, &REFERENCE_RECV_TABLE( DT_INSPlayerExclusive ) ),

	RecvPropInt( RECVINFO( m_iPlayerFlags ) ),
	RecvPropInt( RECVINFO( m_iCurrentStance ) ),
	RecvPropIntWithMinusOneFlag( RECVINFO( m_iLastStance ) ),
	RecvPropIntWithMinusOneFlag( RECVINFO( m_iOldStance ) ),
	RecvPropFloat( RECVINFO( m_flStanceTransMarker ) ),

	RecvPropInt( RECVINFO( m_iLeanType ) ),

	RecvPropInt( "allowht", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_AllowHeadTurn ),

	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),

END_RECV_TABLE()

//=========================================================
//=========================================================
BEGIN_PREDICTION_DATA(C_INSPlayer)

	DEFINE_PRED_TYPEDESCRIPTION( m_INSLocal, CINSPlayerLocalData ),

	DEFINE_PRED_FIELD( m_iPlayerFlags, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD(m_iCurrentStance, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_iLastStance, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_flStanceTransMarker, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),

	DEFINE_PRED_FIELD(m_iStamina, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD_TOL(m_flStaminaUpdateThreshold, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),

	DEFINE_PRED_FIELD_TOL( m_flStartBunnyHopTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
	DEFINE_PRED_FIELD( m_flBunnyHopLength, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD(m_iDamageDecay, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_flDamageDecayThreshold, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),

	DEFINE_PRED_FIELD_TOL( m_angShakyHands, FIELD_VECTOR, FTYPEDESC_INSENDTABLE, 0.125f ),
	DEFINE_PRED_FIELD_TOL( m_flRadius, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.125f ),
	DEFINE_PRED_FIELD_TOL( m_flRadiusDesired, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.125f ),

	DEFINE_PRED_FIELD(m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK),

	DEFINE_PRED_ARRAY(m_iCmdRegister, FIELD_INTEGER, CMDREGISTER_COUNT, FTYPEDESC_INSENDTABLE),

	DEFINE_PRED_FIELD_TOL( m_flNextStanceThreshold, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),

END_PREDICTION_DATA()

//=========================================================
//=========================================================
C_INSPlayer::C_INSPlayer()
{
	m_pPlayerAnimState = CreatePlayerAnimState(this, this, LEGANIM_9WAY, true);

    ResetBreathing();

	m_angFreeAimAngles.Init();

	m_iIDEntIndex = 0;

	HOOK_MESSAGE(AngleHack);
	HOOK_MESSAGE( ObjOrder );
	HOOK_MESSAGE( UnitOrder );

	m_pCurrentObj = NULL;

	m_iDamageDecay = 0;

	m_iPerferredFireMode = 0;

	m_CCFadedHandle = m_CCDeathHandle = 0;

	m_hClippingEntity	= NULL;
	m_hMantleEntity		= NULL;

	m_flHeadLookThreshold = 0.0f;

	// zero [
	m_pFlashlightBeam = NULL;
	// zero ]
}

//=========================================================
//=========================================================
C_INSPlayer::~C_INSPlayer()
{
	m_pPlayerAnimState->Release();
}

//=========================================================
//=========================================================
void C_INSPlayer::PreThink(void)
{
	BaseClass::PreThink();

	if( m_lifeState >= LIFE_DYING )
		return;

	// simulate player processes
	HandleWalking( );
	SimulateStamina();
    SimulateBreathing();
	SimulateDamageDecay();

	// handle bandaging
	HandleBandaging();

	// handle leaning
	HandleLeaning();
}

//=========================================================
//=========================================================
void C_INSPlayer::ClientThink( void )
{
	// work out muzzle positions etc
	HandleMuzzle( );

	// find ID target for local player
	UpdateIDTarget( );

	// update head position
	UpdateHeadPosition( );

	// update color correction
	UpdateColorCorrection( );
}

//=========================================================
//=========================================================
void C_INSPlayer::PostThink( void )
{
	BaseClass::PostThink( );

	UpdateLookAt( );

	if( GetPlayerFlags( ) & FL_PLAYER_BIPOD && IsMoving( ) )
	{
		QAngle eyeAngles = EyeAngles( );
		m_angSupportedFireAngles[ YAW ] = eyeAngles[ YAW ];
	}
}

//=========================================================
//=========================================================
const QAngle &C_INSPlayer::GetRenderAngles( void )
{
	if( IsRagdoll( ) )
		return vec3_angle;

    return m_pPlayerAnimState->GetRenderAngles( );
}

//=========================================================
//=========================================================
void C_INSPlayer::UpdateClientSideAnimation(void)
{
	m_pPlayerAnimState->EnsureValidLayers( );

	// update the animation data. it does the local check here so this works when using
	// a third-person camera (and we don't have valid player angles).
	m_pPlayerAnimState->Update( EyeAngles( )[ YAW ], m_angEyeAngles[ PITCH ] );

	BaseClass::UpdateClientSideAnimation( );
}

//=========================================================
//=========================================================
bool C_INSPlayer::ShouldDraw(void)
{
	// need tocheck this before players are drawn before all entities are created
	if (!g_PR)
		return false;

	// PNOTE: check logic here

	// if we're dead, our ragdoll will be drawn for us instead.
	if( !IsAlive( ) )
		return false;

	// never draw when player resource isn't active or not on playteam
	if( !OnPlayTeam( ) )
		return false;

	// see above
	if( IsLocalPlayer( ) && IsRagdoll( ) )
		return true;

	return BaseClass::ShouldDraw();
}

//=========================================================
//=========================================================
void C_INSPlayer::AddEntity( void )
{
	BaseClass::AddEntity( );

	UpdateLookAt( );

    UpdateClientSideAnimation( );

	SetupColorCorrection( );

	// zero out model pitch, blending takes care of all of it.
	SetLocalAnglesDim( X_INDEX, 0 );

	HandleFlashlight( );
}

//=========================================================
//=========================================================
CStudioHdr *C_INSPlayer::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();
	
	Initialize();

	return hdr;
}

//=========================================================
//=========================================================
void C_INSPlayer::Initialize( void )
{
	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );

	CStudioHdr *hdr = GetModelPtr();
	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}
}

//=========================================================
//=========================================================
bool C_INSPlayer::CreateMove(float flInputSampleTime, CUserCmd* pCmd, bool bFakeInput)
{
	pCmd->vmuzzle = m_vecMuzzle;
	pCmd->amuzzle = m_angMuzzle;

	return BaseClass::CreateMove(flInputSampleTime, pCmd, bFakeInput);
}

//=========================================================
//=========================================================
extern Vector WALL_MIN;
extern Vector WALL_MAX;

void C_INSPlayer::CalcDeathCamView( Vector& eyeOrigin, QAngle& eyeAngles, float &fov )
{
	// set FOV
	fov = GetFOV( );

	// when the player hasn't finished dying and hasn't had their soul-stolen, show the eyes
	const DeathInfoData_t &DeathInfo = GetINSVGUIHelper( )->GetDeathData( );

	if( m_lifeState != LIFE_RESPAWNABLE && DeathInfo.m_iType != PDEATHTYPE_SOULSTOLEN )
	{
		CBaseAnimating *pRagdoll = GetRagdollEntity( );

		if( pRagdoll && m_iEyeAttachment != -1 )
		{
			matrix3x4_t AttachmentToWorld;
			pRagdoll->GetAttachment( m_iEyeAttachment, AttachmentToWorld );

			Vector vecForward;
			MatrixGetColumn( AttachmentToWorld, 0, vecForward );

			pRagdoll->GetAttachment( m_iEyeAttachment, eyeOrigin, eyeAngles );
			eyeOrigin -= ( vecForward * 4.0f );

			return;
		}
	}

	// otherwise show outside the ragdoll
	CBaseEntity	* killer = GetObserverTarget( );

	float interpolation = ( gpGlobals->curtime - m_flDeathTime ) / DEATH_ANIMATION_TIME;
	interpolation = clamp( interpolation, 0.0f, 1.0f );

	m_flObserverChaseDistance += gpGlobals->frametime * 48.0f;
	m_flObserverChaseDistance = clamp( m_flObserverChaseDistance, 16, CHASE_CAM_DISTANCE );

	QAngle aForward = eyeAngles = EyeAngles( );
	Vector origin = EyePosition( );			

	IRagdoll *pRagdoll = GetRepresentativeRagdoll( );

	if( pRagdoll )
	{
		origin = pRagdoll->GetRagdollOrigin( );
		origin.z += VEC_DEAD_VIEWHEIGHT.z; // look over ragdoll, not through
	}
	
	if( killer && killer->IsPlayer( ) && ( killer != this ) ) 
	{
		Vector vKiller = killer->EyePosition() - origin;
		QAngle aKiller; VectorAngles( vKiller, aKiller );
		InterpolateAngles( aForward, aKiller, eyeAngles, interpolation );
	}

	Vector vForward;
	AngleVectors( eyeAngles, &vForward );

	VectorNormalize( vForward );

	VectorMA( origin, -m_flObserverChaseDistance, vForward, eyeOrigin );

	// clip against world
	trace_t trace; 
	C_BaseEntity::PushEnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
	UTIL_TraceHull( origin, eyeOrigin, WALL_MIN, WALL_MAX, MASK_SOLID, this, COLLISION_GROUP_NONE, &trace );
	C_BaseEntity::PopEnableAbsRecomputations();

	if( trace.fraction < 1.0 )
	{
		eyeOrigin = trace.endpos;
		m_flObserverChaseDistance = VectorLength( origin - eyeOrigin );
	}
}

//=========================================================
//=========================================================
float C_INSPlayer::GetViewmodelFOV( void )
{
	return BaseClass::GetViewmodelFOV();
}

//=========================================================
//=========================================================
const QAngle &C_INSPlayer::EyeAngles( void )
{
	if( IsLocalPlayer( ) )
		return BaseClass::EyeAngles( );
	else
		return m_angEyeAngles;
}

//=========================================================
//=========================================================
C_BaseAnimating *C_INSPlayer::BecomeRagdollOnClient(bool bCopyEntity)
{
	return NULL;
}

//=========================================================
//=========================================================
IRagdoll *C_INSPlayer::GetRepresentativeRagdoll(void) const
{
	if(m_hRagdoll.Get())
	{
		C_INSRagdoll *pRagdoll = (C_INSRagdoll*)m_hRagdoll.Get();
		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}

//=========================================================
//=========================================================
CBaseAnimating *C_INSPlayer::GetRagdollEntity(void) const
{
	return (C_INSRagdoll*)m_hRagdoll.Get();
}

//=========================================================
//=========================================================
float C_INSPlayer::GetFreeaimDistance( void ) const
{
	C_WeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( !pWeapon )
		return 0.0f;

	return pWeapon->GetFreeaimDistance( );
}

//=========================================================
//=========================================================
bool C_INSPlayer::IsFreeaimEnabled( void ) const
{
	C_WeaponINSBase *pWeapon = GetActiveINSWeapon( );
	return ( pWeapon && pWeapon->UseFreeaim( ) && pWeapon->AllowFreeaim( ) );
}

//=========================================================
//=========================================================
ConVar cl_freeaimswr( "cl_freeaimswr", "-1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "set freeaim screen weapon relation" );

float C_INSPlayer::GetFreeaimScreenWeaponRelation(void) const
{
	if( cl_freeaimswr.GetInt( ) != -1 )
		return clamp( cl_freeaimswr.GetFloat( ), 0.0f, 1.0f );

	return GetWeaponDef()->flFreeaimScreenWeaponRelation;
}

//=========================================================
//=========================================================
ConVar cl_freeaimswrf( "cl_freeaimswrf", "-1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "use freeaim screen weapon relation fraction" );

bool C_INSPlayer::UseFreeaimSWRFraction(void) const
{
	if( cl_freeaimswrf.GetInt( ) != -1 )
		return cl_freeaimswrf.GetBool( );

	return GetWeaponDef()->bFreeaimSWRFraction;
}

//=========================================================
//=========================================================
const QAngle &C_INSPlayer::GetFreeAimAngles(void) const
{
	return m_angFreeAimAngles;
}

//=========================================================
//=========================================================
void C_INSPlayer::SetFreeAimAngles(const QAngle &angAngles)
{
	m_angFreeAimAngles = angAngles;	
}

//=========================================================
//=========================================================
int C_INSPlayer::GetTeamID( void ) const
{
	return g_PR->GetTeamID( entindex( ) );
}

//=========================================================
//=========================================================
bool C_INSPlayer::OnPlayTeam( void ) const
{
	return OnPlayTeam( entindex( ) );
}

//=========================================================
//=========================================================
C_PlayTeam *C_INSPlayer::GetPlayTeam( int iPlayerID )
{
	return ( OnPlayTeam( iPlayerID ) ? GetGlobalPlayTeam( g_PR->GetTeamID( iPlayerID ) ) : NULL );
}

//=========================================================
//=========================================================
bool C_INSPlayer::OnPlayTeam( int iPlayerID )
{
	return IsPlayTeam(g_PR->GetTeamID( iPlayerID ) );
}

//=========================================================
//=========================================================
int C_INSPlayer::GetSquadID(void) const
{
	SquadData_t SquadData;
	GetSquadData(SquadData);

	return SquadData.GetSquadID();
}

//=========================================================
//=========================================================
int C_INSPlayer::GetSlotID( void ) const
{
	SquadData_t SquadData;
	GetSquadData( SquadData );

	return SquadData.GetSlotID( );
}

//=========================================================
//=========================================================
bool C_INSPlayer::IsValidSquad( void ) const
{
	SquadData_t SquadData;
	GetSquadData( SquadData );

	return SquadData.IsValid( );
}

//=========================================================
//=========================================================
C_INSSquad *C_INSPlayer::GetSquad( void ) const
{
	if( !OnPlayTeam( ) || !IsValidSquad( ) )
		return NULL;

	C_PlayTeam *pTeam = ( C_PlayTeam* )GetTeam( );
	return pTeam->GetSquad( GetSquadID( ) );
}

//=========================================================
//=========================================================
void C_INSPlayer::GetSquadData( SquadData_t &SquadData ) const
{
	C_PlayTeam *pTeam = GetPlayTeam( );

	if( pTeam )
		pTeam->GetSquadData( entindex( ), SquadData );
}

//=========================================================
//=========================================================
const char *C_INSPlayer::CanSpawn(void) const
{
	C_INSSquad *pSquad = GetSquad();

	if(!pSquad)
		return NULL;

	return pSquad->CanSpawn(this);
}

//=========================================================
//=========================================================
bool C_INSPlayer::IsDeveloper( void )
{
	return false;
}

//=========================================================
//=========================================================
bool C_INSPlayer::ParsePlayerName( int iPlayerID, bool bIncludeTitle, char *pszBuffer, int iLength )
{
	const char *pszName = g_PR->GetPlayerName(iPlayerID);

	if( bIncludeTitle )
	{
		static char szFullName[ MAX_PLAYER_TITLE_LENGTH ];

		C_PlayTeam *pTeam = C_INSPlayer::GetPlayTeam( iPlayerID );

		if( !pTeam )
			return false;

		int iRankID = pTeam->GetRank( iPlayerID );

		if( iRankID == INVALID_RANK )
			return false;

		CTeamLookup *pTeamLookup = pTeam->GetTeamLookup( );

		if( !pTeamLookup )
			return false;

		const char *pszRank = pTeamLookup->GetRankName( iRankID );

		if( !pszRank )
			return false;

		Q_snprintf( szFullName, MAX_PLAYER_TITLE_LENGTH, "%s. %s", pszRank, pszName );

		pszName = szFullName;
	}

	UTIL_MakeSafeName( pszName, pszBuffer, iLength );

	return true;
}

//=========================================================
//=========================================================
bool C_INSPlayer::HasZoom( void )
{
	if( GetCmdValue( CMDREGISTER_3DSCOPE ) == 0 )
		return false;

	C_WeaponINSBase *pWeapon = GetActiveINSWeapon( );
	return ( pWeapon && pWeapon->HasScope( ) );
}

//=========================================================
//=========================================================
bool C_INSPlayer::IsZoomed( void )
{
	return ( HasZoom( ) && GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS );
}

//=========================================================
//=========================================================
bool C_INSPlayer::ActiveZoom( void )
{
	return ( HasZoom( ) && GetPlayerFlags( ) & ( FL_PLAYER_IRONSIGHTS | FL_PLAYER_BIPOD ) );
}

//=========================================================
//=========================================================
int C_INSPlayer::GetRank(void) const
{
	if(!OnPlayTeam())
		return 0;

	C_PlayTeam *pTeam = (C_PlayTeam*)GetTeam();
	return pTeam->GetRank(entindex());
}

//=========================================================
//=========================================================
const char *C_INSPlayer::GetFullNextRankName(void) const
{
	// PNOTE: can anyone say redundant?
	if( IsCommander( ) )
		return NULL;

	CTeamLookup *pTeam = GetTeamLookup( );
	int iRankID = GetRank( );

	if( iRankID == INVALID_RANK || iRankID == ( RANK_COMMANDER - 1 ) )
		return NULL;

	return ( pTeam ? pTeam->GetFullRankName( iRankID + 1 ) : NULL );
}

//=========================================================
//=========================================================
void C_INSPlayer::HandleObjOrder( bf_read &msg )
{
	m_ObjOrders.Init( msg );

	GetINSHUDHelper( )->SendObjOrderUpdate( );
}

//=========================================================
//=========================================================
const CObjOrder *C_INSPlayer::GetObjOrders( void ) const
{
	return &m_ObjOrders;
}

//=========================================================
//=========================================================
bool C_INSPlayer::HasObjOrders( void ) const
{
	return m_ObjOrders.HasOrders( );
}

//=========================================================
//=========================================================
void C_INSPlayer::HandleUnitOrder( bf_read &msg )
{
	m_UnitOrders.Init( msg );

	GetINSHUDHelper( )->SendUnitOrderUpdate( );
}

//=========================================================
//=========================================================
const CUnitOrder *C_INSPlayer::GetUnitOrders( void ) const
{
	return &m_UnitOrders;
}

//=========================================================
//=========================================================
bool C_INSPlayer::HasUnitOrders( void ) const
{
	return m_UnitOrders.HasOrders( );
}

//=========================================================
//=========================================================
int C_INSPlayer::GetPlayerOrder( void ) const
{
	if( !OnPlayTeam( ) )
		return INVALID_PORDER;

	C_PlayTeam *pTeam = GetPlayTeam( );
	return pTeam->GetOrder( entindex( ) );
}

//=========================================================
//=========================================================
int C_INSPlayer::GetStatusType( void ) const
{
	if( !OnPlayTeam( ) )
		return INVALID_PSTATUSTYPE;

	C_PlayTeam *pTeam = GetPlayTeam( );
	return pTeam->GetStatusType( entindex( ) );
}

//=========================================================
//=========================================================
int C_INSPlayer::GetStatusID( void ) const
{
	if( !OnPlayTeam( ) )
		return INVALID_PSTATUSID;

	C_PlayTeam *pTeam = GetPlayTeam( );
	return pTeam->GetStatusID( entindex( ) );
}

//=========================================================
//=========================================================
bool C_INSPlayer::IsCapturing( void ) const
{
	return ( m_pCurrentObj != NULL );
}

//=========================================================
//=========================================================
int C_INSPlayer::GetIDTarget() const
{
	return m_iIDEntIndex;
}

//=========================================================
//=========================================================
void C_INSPlayer::UpdateIDTarget(void)
{
	// clear old target and find a new one
	m_iIDEntIndex = 0;

	// don't update if a remote or not not a play team
	// or they're a certain spectator
	if( !IsLocalPlayer( ) || !OnPlayTeam() ||
		GetObserverMode() == OBS_MODE_CHASE || GetObserverMode() == OBS_MODE_DEATHCAM )
		return;

	// find them!
	trace_t tr;
	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin(), 1500, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && ( pEntity != this ) && pEntity->IsPlayer() )
		{
			C_INSPlayer *pTargetPlayer = ToINSPlayer( pEntity );

			if( pTargetPlayer->IsAlive() && pTargetPlayer->OnPlayTeam() && OnSameTeam( pTargetPlayer, this ) )
				m_iIDEntIndex = pEntity->entindex( );
		}
	}
}

//=========================================================
//=========================================================
const Vector &C_INSPlayer::GetChaseCamOrigin(void)
{
	if( IsAlive() || !m_hRagdoll )
		return GetRenderOrigin( );

	return m_hRagdoll->GetRenderOrigin( );
}

//=========================================================
//=========================================================
int	C_INSPlayer::GetChaseCamDistance(void)
{
	return CHASE_CAM_DISTANCE;
}

//=========================================================
//=========================================================
int C_INSPlayer::GetHealthType( void ) const
{
	CPlayTeam *pTeam = GetPlayTeam( );
	return ( pTeam ? pTeam->GetHealthType( entindex( ) ) : 0 );
}

//=========================================================
//=========================================================
C_Team *C_INSPlayer::GetTeam( void ) const
{
	return GetGlobalTeam( GetTeamID( ) );
}

void C_INSPlayer::HandleMuzzle( void )
{
	if (!IsLocalPlayer())
		return;

	QAngle angMuzzle;
	Vector vecMuzzle;

	C_BaseAnimating* pRenderedWeaponMode = GetRenderedWeaponModel();
	if (pRenderedWeaponMode != NULL)
		pRenderedWeaponMode->GetAttachment(1, vecMuzzle, angMuzzle);

	m_angMuzzle = angMuzzle;
	m_vecMuzzle = vecMuzzle;
}

//=========================================================
//=========================================================

// TODO: put this constant somewhere

float C_INSPlayer::GetLookOver( void ) const
{
	return 0.0f;
}

//=========================================================
//=========================================================
void C_INSPlayer::UpdateLookAt( void )
{
	if( IsLocalPlayer( ) )
		return;

	// head yaw
	if( m_headYawPoseParam < 0 || m_headPitchPoseParam < 0 )
		return;

	// blinking
	if( m_blinkTimer.IsElapsed( ) )
	{
		m_blinktoggle = !m_blinktoggle;
		m_blinkTimer.Start( RandomFloat( 1.5f, 4.0f ) );
	}

	// work out where the look
	QAngle desiredAngles;

	// orient eyes
	m_viewtarget = m_vecLookAtTarget;

	// figure out where we want to look in world space.
	Vector to = m_vecLookAtTarget - EyePosition();
	VectorAngles(to, desiredAngles);

	// figure out where our body is facing in world space.
	QAngle bodyAngles( 0, 0, 0 );
	bodyAngles[YAW] = GetLocalAngles( )[ YAW ];

	float flBodyYawDiff = bodyAngles[ YAW ] - m_flLastBodyYaw;
	m_flLastBodyYaw = bodyAngles[ YAW ];

	// set the head's yaw.
	float desired = AngleNormalize( desiredAngles[ YAW ] - bodyAngles[ YAW ] );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
	m_flCurrentHeadYaw = ApproachAngle( desired, m_flCurrentHeadYaw, 130 * gpGlobals->frametime );

	// counterrotate the head from the body rotation so it doesn't rotate past its target.
	m_flCurrentHeadYaw = AngleNormalize( m_flCurrentHeadYaw - flBodyYawDiff );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
		
	SetPoseParameter( m_headYawPoseParam, m_flCurrentHeadYaw );
		
	// set the head's yaw.
	desired = AngleNormalize( desiredAngles[ PITCH ] );
	desired = clamp( desired, m_headPitchMin, m_headPitchMax );
		
	m_flCurrentHeadPitch = ApproachAngle( desired, m_flCurrentHeadPitch, 130 * gpGlobals->frametime );
	m_flCurrentHeadPitch = AngleNormalize( m_flCurrentHeadPitch );
	SetPoseParameter( m_headPitchPoseParam, m_flCurrentHeadPitch );
}

//=========================================================
//=========================================================
const QAngle &C_INSPlayer::GetSupportedFireAngles(void) const
{
	return m_angSupportedFireAngles;
}

//=========================================================
//=========================================================
void C_INSPlayer::SetSupportedFireAngles(const QAngle &angAngles)
{
	m_angSupportedFireAngles = angAngles;
}

//=========================================================
//=========================================================
#define MIN_LOOKFORWARD 20

void C_INSPlayer::UpdateHeadPosition( void )
{
	if( IsLocalPlayer( ) )
		return;

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	Vector vForward;
	AngleVectors( GetLocalAngles( ), &vForward );

	// find someone to look at
	if( CanLookAt( ) )
	{
		Vector vLocalOrigin, vMyOrigin, vDir;

		vLocalOrigin = pPlayer->GetAbsOrigin( );
		vMyOrigin = GetAbsOrigin( );

		vDir = vLocalOrigin - vMyOrigin;

		if( vDir.Length( ) <= MAX_LOCAL_DISTANCE )
		{
			for( int iClient = 0; iClient < gpGlobals->maxClients; ++iClient )
			{
				CBaseEntity *pEnt = UTIL_PlayerByIndex( iClient );

				if( !pEnt || !pEnt->IsPlayer( ) || pEnt->entindex( ) == entindex( ) )
					continue;

				C_BasePlayer *pPlayer = ToBasePlayer( pEnt );

				if( pPlayer->IsObserver( ) )
					continue;

				Vector vTargetOrigin = pEnt->GetAbsOrigin( );
				vDir = vTargetOrigin - vMyOrigin;
			
				if( vDir.Length( ) > MAX_TARGET_DISTANCE ) 
					continue;

				VectorNormalize( vDir );

				if( DotProduct( vForward, vDir ) < 0.0f )
					continue;

				m_vecLookAtTarget = pEnt->EyePosition( );
				return;
			}
		}
	}
	
	// otherwise make them look forward
	m_vecLookAtTarget = GetAbsOrigin( ) + vForward * 512;
}

//=========================================================
//=========================================================
bool C_INSPlayer::CanLookAt( void )
{
	// don't when not running
	if( !INSRules( )->IsModeRunning( ) )
		return false;

	// always when endgame
	if( INSRules( )->RunningMode( )->GetStatus( GAMERUNNING_ENDGAME ) )
		return true;

	// never when bipod or ironsights
	if( ( GetPlayerFlags( ) & ( FL_PLAYER_IRONSIGHTS | FL_PLAYER_BIPOD ) ) != 0 )
		return false;

	// never when moving
	if( UTIL_IsMoving( GetAbsVelocity( ), MIN_LOOKFORWARD ) )
		return false;

	// server needs to allow
	if( !m_bAllowHeadTurn )
		return false;

	// not when inside threshold
	if( m_flHeadLookThreshold >= gpGlobals->curtime )
		return false;

	return true;
}

//=========================================================
//=========================================================
#define COLORCORRECTION_FADED "materials/correction/faded.raw"
#define COLORCORRECTION_DEATH "materials/correction/death.raw"

void C_INSPlayer::SetupColorCorrection( void )
{
	if( !IsLocalPlayer( ) )
		return;

	if( m_CCFadedHandle == 0 )
	{
		m_CCFadedHandle = colorcorrection->AddLookup( COLORCORRECTION_FADED );

		colorcorrection->LockLookup( m_CCFadedHandle );
		colorcorrection->LoadLookup( m_CCFadedHandle, COLORCORRECTION_FADED );
		colorcorrection->UnlockLookup( m_CCFadedHandle );
	}

	if( m_CCDeathHandle == 0 )
	{
		m_CCDeathHandle = colorcorrection->AddLookup( COLORCORRECTION_DEATH );

		colorcorrection->LockLookup( m_CCDeathHandle );
		colorcorrection->LoadLookup( m_CCDeathHandle, COLORCORRECTION_DEATH );
		colorcorrection->UnlockLookup( m_CCDeathHandle );
	}
}

//=========================================================
//=========================================================
bool C_INSPlayer::IsValidColorCorrection( void ) const
{
	return ( m_CCFadedHandle != 0 && m_CCDeathHandle != 0 );
}

//=========================================================
//=========================================================
void C_INSPlayer::ResetColorCorrection( void )
{
	if( !IsValidColorCorrection( ) )
		return;

	colorcorrection->SetLookupWeight( m_CCFadedHandle, 0.0f );
	colorcorrection->SetLookupWeight( m_CCDeathHandle, 0.0f );
}

//=========================================================
//=========================================================
void C_INSPlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
{
	CTakeDamageInfo info = inputInfo;

	if( m_takedamage )
	{
		if( !INSRules( )->PlayerAdjustDamage( this, info.GetAttacker( ), info ) )
			return;
	}

	BaseClass::TraceAttack( info, vecDir, ptr );
}

//=========================================================
//=========================================================
enum PlayerCCType_t
{
	PLAYERCCTYPE_FULL = 0,
	PLAYERCCTYPE_NOSPEC,
	PLAYERCCTYPE_NONE,
	PLAYERCCTYPE_COUNT
};

void NewPlayerCCType(IConVar* var, const char* pOldValue, float flOldValue)
{
	ConVar* pVar = (ConVar*)var;

	if (pVar->GetInt() != PLAYERCCTYPE_NONE)
		return;

	C_INSPlayer* pPlayer = C_INSPlayer::GetLocalPlayer();
	if (pPlayer)
		pPlayer->ResetColorCorrection();
}

ConVar playercctype( "cl_playercctype", "0", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "Player Color Correction Type", true, 0, true, PLAYERCCTYPE_COUNT - 1, NewPlayerCCType );

void C_INSPlayer::UpdateColorCorrection( void )
{
	if( !IsLocalPlayer( ) )
		return;

	// don't update when  disabled
	if( playercctype.GetInt( ) == PLAYERCCTYPE_NONE )
		return;

	// handle gamerules
	const ColorCorrectionHandle_t &CCHandleMap = INSRules( )->GetColorCorrection( );

	if( CCHandleMap != 0 )
		colorcorrection->SetLookupWeight( CCHandleMap, 1.0f );

	// don't update when invalid
	if( !IsValidColorCorrection( ) )
		return;

	static float flFadeTime = PLAYER_DEATHFADEOUT * 0.35f;

	// handle it
	bool bDying = ( m_flDeathTime + PLAYER_DEATHFADEOUT ) >= gpGlobals->curtime;

	ColorCorrectionHandle_t &CCHandleActive = bDying ? m_CCDeathHandle : m_CCFadedHandle;
	ColorCorrectionHandle_t &CCHandleOther = bDying ? m_CCFadedHandle : m_CCDeathHandle;
	float flActiveWeight;

	if( bDying )
		flActiveWeight = min( ( ( gpGlobals->curtime - m_flDeathTime ) / flFadeTime ), 1.0f );
	else
		flActiveWeight = ( IsRunningAround( ) || ( ( playercctype.GetInt( ) == PLAYERCCTYPE_NOSPEC ) && INSRules( )->IsModeRunning( ) && IsObserver( ) ) ) ? 0.0f : 1.0f;

	colorcorrection->SetLookupWeight( CCHandleActive, flActiveWeight );
	colorcorrection->SetLookupWeight( CCHandleOther, 0.0f );
}

//=========================================================
//=========================================================
C_INSSquad *GetLocalSquad(void)
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( pPlayer )
		return pPlayer->GetSquad( );

	return NULL;
}

//=========================================================
//=========================================================
void C_INSPlayer::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	if( state == SHOULDTRANSMIT_END )
		ReleaseFlashlight( );

	BaseClass::NotifyShouldTransmit( state );
}

//=========================================================
//=========================================================
void C_INSPlayer::HandleFlashlight( void )
{
	if( IsLocalPlayer( ) )
		return;

	if( IsEffectActive( EF_DIMLIGHT ) )
	{
		int iAttachment = LookupAttachment( "anim_attachment_RH" );

		if ( iAttachment < 0 )
			return;

		Vector vecOrigin;
		QAngle eyeAngles = m_angEyeAngles;

		GetAttachment( iAttachment, vecOrigin, eyeAngles );

		Vector vForward;
		AngleVectors( eyeAngles, &vForward );

		trace_t tr;
		UTIL_TraceLine( vecOrigin, vecOrigin + ( vForward * 200 ), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		if( !m_pFlashlightBeam )
		{
			BeamInfo_t beamInfo;
			beamInfo.m_nType = TE_BEAMPOINTS;
			beamInfo.m_vecStart = tr.startpos;
			beamInfo.m_vecEnd = tr.endpos;
			beamInfo.m_pszModelName = "sprites/glow01.vmt";
			beamInfo.m_pszHaloName = "sprites/glow01.vmt";
			beamInfo.m_flHaloScale = 3.0;
			beamInfo.m_flWidth = 8.0f;
			beamInfo.m_flEndWidth = 35.0f;
			beamInfo.m_flFadeLength = 300.0f;
			beamInfo.m_flAmplitude = 0;
			beamInfo.m_flBrightness = 60.0;
			beamInfo.m_flSpeed = 0.0f;
			beamInfo.m_nStartFrame = 0.0;
			beamInfo.m_flFrameRate = 0.0;
			beamInfo.m_flRed = 255.0;
			beamInfo.m_flGreen = 255.0;
			beamInfo.m_flBlue = 255.0;
			beamInfo.m_nSegments = 8;
			beamInfo.m_bRenderable = true;
			beamInfo.m_flLife = 0.5;
			beamInfo.m_nFlags = FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

			m_pFlashlightBeam = beams->CreateBeamPoints( beamInfo );
		}

		if( m_pFlashlightBeam )
		{
			BeamInfo_t beamInfo;
			beamInfo.m_vecStart = tr.startpos;
			beamInfo.m_vecEnd = tr.endpos;
			beamInfo.m_flRed = 255.0;
			beamInfo.m_flGreen = 255.0;
			beamInfo.m_flBlue = 255.0;

			beams->UpdateBeamInfo( m_pFlashlightBeam, beamInfo );

			dlight_t *el = effects->CL_AllocDlight( 0 );
			el->origin = tr.endpos;
			el->radius = 50; 
			el->color.r = 200;
			el->color.g = 200;
			el->color.b = 200;
			el->die = gpGlobals->curtime + 0.1;
		}
	}
	else if( m_pFlashlightBeam )
	{
		ReleaseFlashlight( );
	}
}

//=========================================================
//=========================================================
void C_INSPlayer::ReleaseFlashlight( void )
{
	if( m_pFlashlightBeam )
	{
		m_pFlashlightBeam->flags = 0;
		m_pFlashlightBeam->die = gpGlobals->curtime - 1;

		m_pFlashlightBeam = NULL;
	}
}

//=========================================================
//=========================================================
void C_INSPlayer::InitalSpawn( void )
{
	engine->ServerCmd( PCMD_INITALSPAWN );
}

//=========================================================
//=========================================================
int GetLocalTeamID( void ) 
{ 
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
	Assert( pPlayer );

	if( pPlayer )
		return pPlayer->GetTeamID( ); 

	return INVALID_TEAM;
}


//=========================================================
//=========================================================
C_Team *GetLocalTeam( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );
	Assert( pPlayer );

	if( !pPlayer )
		return NULL;

	return GetPlayersTeam( engine->GetLocalPlayer( ) );
}

//=========================================================
//=========================================================
C_PlayTeam *GetLocalPlayTeam( void )
{
	if( !engine )
		return NULL;

	return GetPlayersPlayTeam( engine->GetLocalPlayer( ) );
}

//=========================================================
//=========================================================
IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_INSRagdoll, DT_INSRagdoll, CINSRagdoll )
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) ),
END_RECV_TABLE()

//=========================================================
//=========================================================
C_INSRagdoll::C_INSRagdoll()
{
}

//=========================================================
//=========================================================
C_INSRagdoll::~C_INSRagdoll()
{
	PhysCleanupFrictionSounds( this );

	if ( m_hPlayer )
		m_hPlayer->CreateModelInstance();
}

//=========================================================
//=========================================================
void C_INSRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;
	
	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();
    	
	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		const char *pszName = pDestEntry->watcher->GetDebugName();
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(), pszName ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

//=========================================================
//=========================================================
void C_INSRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
{
	if( iDamageType & DMG_RICOCHET )
		return;

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject( );

	if( !pPhysicsObject )
		return;

	Vector dir, dir2, hitpos;
	dir = dir2 = pTrace->endpos - pTrace->startpos;

	VectorMA( pTrace->startpos, pTrace->fraction, dir2, hitpos );
	VectorNormalize( dir2 );

	Vector vecBloodStart, vecBloodEnd;
	VectorMA( hitpos, 172, dir, vecBloodEnd );
	VectorMA( hitpos, 10,  dir, vecBloodStart );

	trace_t tr;
	UTIL_TraceLine( vecBloodStart, vecBloodEnd,  MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
	UTIL_BloodDecalTrace( &tr, BloodColor( ) );

	UTIL_BloodDrips( hitpos, dir2, BLOOD_COLOR_RED, random->RandomInt( 25, 100 ) );

	if( iDamageType & DMG_BLAST )
		dir *= 500;  // adjust impact strenght
	else
		dir *= 30;

	if( iDamageType & DMG_BLAST )
	{
		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );
	}
}

//=========================================================
//=========================================================
void C_INSRagdoll::CreateRagdoll(void)
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_INSPlayer *pPlayer = ToINSPlayer(m_hPlayer);

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.
		if ( !pPlayer->IsLocalPlayer( ) )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( m_vecRagdollOrigin );

			SetAbsAngles( pPlayer->GetRenderAngles() );

			SetAbsVelocity( m_vecRagdollVelocity );

			int iSeq = pPlayer->GetSequence();
			if ( iSeq == -1 )
			{
				Assert( false );
				iSeq = 0;
			}

			SetSequence( iSeq );
			SetCycle( 0.0 );

			Interp_Reset( varMap );
		}		

		m_nSkin=pPlayer->m_nSkin;
		m_nBody=pPlayer->m_nBody;
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );

	}

	SetModelIndex( m_nModelIndex );

	// Turn it into a ragdoll.
	// Make us a ragdoll..
	m_nRenderFX = kRenderFxRagdoll;

	BecomeRagdollOnClient();

}

//=========================================================
//=========================================================
void C_INSRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		CreateRagdoll();
	
		IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

		if( pPhysicsObject )
		{
			AngularImpulse aVelocity(0,0,0);

			Vector vecExaggeratedVelocity = 3 * m_vecRagdollVelocity;

			pPhysicsObject->AddVelocity( &vecExaggeratedVelocity, &aVelocity );
		}
	}
}

//=========================================================
//=========================================================
int C_INSRagdoll::GetPlayerEntIndex(void) const
{
	C_INSPlayer *pPlayer = dynamic_cast<C_INSPlayer*>(m_hPlayer.Get());

	if(!pPlayer)
		return 0;

	return pPlayer->entindex();
}

//=========================================================
//=========================================================
IRagdoll* C_INSRagdoll::GetIRagdoll(void) const
{
	return m_pRagdoll;
}

//=========================================================
//=========================================================
void C_INSRagdoll::UpdateOnRemove(void)
{
	VPhysicsSetObject( NULL );

	BaseClass::UpdateOnRemove();
}