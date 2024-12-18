//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Activities that are available to all NPCs.
//
//=============================================================================//

#include "cbase.h"
#include "ai_activity.h"
#include "activitylist.h"
#include "stringregistry.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// Init static variables
//=============================================================================
static CStringRegistry* m_pActivitySR = NULL;
static int				m_iNumActivities = 0;

//-----------------------------------------------------------------------------
// Purpose: Add an activity to the activity string registry and increment
//			the acitivty counter
//-----------------------------------------------------------------------------
static void AddActivityToSR(const char* actName, int actID)
{
	Assert(m_pActivitySR);
	if (!m_pActivitySR)
		return;

	// technically order isn't dependent, but it's too damn easy to forget to add new ACT_'s to all three lists.

	// NOTE: This assertion generally means that the activity enums are out of order or that new enums were not added to all
	//		 relevant tables.  Make sure that you have included all new enums in:
	//			game_shared/ai_activity.h
	//			game_shared/activitylist.cpp
	//			dlls/ai_activity.cpp
	MEM_ALLOC_CREDIT();

	static int lastActID = -2;
	Assert(actID >= LAST_SHARED_ACTIVITY || actID == lastActID + 1 || actID == ACT_INVALID);
	lastActID = actID;

	m_pActivitySR->AddString(actName, actID);
	m_iNumActivities++;
}

//-----------------------------------------------------------------------------
// Purpose: Given and activity ID, return the activity name
//-----------------------------------------------------------------------------
const char* GetActivityName(int actID)
{
	if (actID == -1)
		return "ACT_INVALID";

	// m_pActivitySR only contains public activities, ActivityList_NameForIndex() has them all
	const char* name = ActivityList_NameForIndex(actID);

	if (!name)
	{
		AssertOnce(!"GetActivityName() returning NULL!");
	}

	return name;
}

//-----------------------------------------------------------------------------
// Purpose: Given and activity name, return the activity ID
//-----------------------------------------------------------------------------
int GetActivityID(const char* actName)
{
	Assert(m_pActivitySR);
	if (!m_pActivitySR)
		return ACT_INVALID;

	return m_pActivitySR->GetStringID(actName);
}

#define ADD_ACTIVITY_TO_SR(activityname) AddActivityToSR(#activityname,activityname)

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
static void InitDefaultActivitySR(void)
{
	ADD_ACTIVITY_TO_SR(ACT_INVALID);
	ADD_ACTIVITY_TO_SR(ACT_RESET);
	ADD_ACTIVITY_TO_SR(ACT_IDLE);
	ADD_ACTIVITY_TO_SR(ACT_TRANSITION);
	ADD_ACTIVITY_TO_SR(ACT_COVER);
	ADD_ACTIVITY_TO_SR(ACT_COVER_MED);
	ADD_ACTIVITY_TO_SR(ACT_COVER_LOW);
	ADD_ACTIVITY_TO_SR(ACT_WALK);
	ADD_ACTIVITY_TO_SR(ACT_WALK_AIM);
	ADD_ACTIVITY_TO_SR(ACT_WALK_CROUCH);
	ADD_ACTIVITY_TO_SR(ACT_WALK_CROUCH_AIM);
	ADD_ACTIVITY_TO_SR(ACT_RUN);
	ADD_ACTIVITY_TO_SR(ACT_RUN_AIM);
	ADD_ACTIVITY_TO_SR(ACT_RUN_CROUCH);
	ADD_ACTIVITY_TO_SR(ACT_RUN_CROUCH_AIM);
	ADD_ACTIVITY_TO_SR(ACT_RUN_PROTECTED);
	ADD_ACTIVITY_TO_SR(ACT_SCRIPT_CUSTOM_MOVE);
	ADD_ACTIVITY_TO_SR(ACT_RANGE_ATTACK1);
	ADD_ACTIVITY_TO_SR(ACT_RANGE_ATTACK2);
	ADD_ACTIVITY_TO_SR(ACT_RANGE_ATTACK1_LOW);
	ADD_ACTIVITY_TO_SR(ACT_RANGE_ATTACK2_LOW);
	ADD_ACTIVITY_TO_SR(ACT_DIESIMPLE);
	ADD_ACTIVITY_TO_SR(ACT_DIEBACKWARD);
	ADD_ACTIVITY_TO_SR(ACT_DIEFORWARD);
	ADD_ACTIVITY_TO_SR(ACT_DIEVIOLENT);
	ADD_ACTIVITY_TO_SR(ACT_DIERAGDOLL);
	ADD_ACTIVITY_TO_SR(ACT_FLY);
	ADD_ACTIVITY_TO_SR(ACT_HOVER);
	ADD_ACTIVITY_TO_SR(ACT_GLIDE);
	ADD_ACTIVITY_TO_SR(ACT_SWIM);
	ADD_ACTIVITY_TO_SR(ACT_JUMP);
	ADD_ACTIVITY_TO_SR(ACT_HOP);
	ADD_ACTIVITY_TO_SR(ACT_LEAP);
	ADD_ACTIVITY_TO_SR(ACT_LAND);
	ADD_ACTIVITY_TO_SR(ACT_CLIMB_UP);
	ADD_ACTIVITY_TO_SR(ACT_CLIMB_DOWN);
	ADD_ACTIVITY_TO_SR(ACT_CLIMB_DISMOUNT);
	ADD_ACTIVITY_TO_SR(ACT_SHIPLADDER_UP);
	ADD_ACTIVITY_TO_SR(ACT_SHIPLADDER_DOWN);
	ADD_ACTIVITY_TO_SR(ACT_STRAFE_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_STRAFE_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_ROLL_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_ROLL_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_TURN_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_TURN_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_CROUCH);
	ADD_ACTIVITY_TO_SR(ACT_CROUCHIDLE);
	ADD_ACTIVITY_TO_SR(ACT_STAND);
	ADD_ACTIVITY_TO_SR(ACT_USE);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL1);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL2);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL3);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL_ADVANCE);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL_FORWARD);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL_GROUP);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL_HALT);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_SIGNAL_TAKECOVER);
	ADD_ACTIVITY_TO_SR(ACT_LOOKBACK_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_LOOKBACK_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_COWER);
	ADD_ACTIVITY_TO_SR(ACT_SMALL_FLINCH);
	ADD_ACTIVITY_TO_SR(ACT_BIG_FLINCH);
	ADD_ACTIVITY_TO_SR(ACT_MELEE_ATTACK1);
	ADD_ACTIVITY_TO_SR(ACT_MELEE_ATTACK2);
	ADD_ACTIVITY_TO_SR(ACT_RELOAD);
	ADD_ACTIVITY_TO_SR(ACT_RELOAD_LOW);
	ADD_ACTIVITY_TO_SR(ACT_ARM);
	ADD_ACTIVITY_TO_SR(ACT_DISARM);
	ADD_ACTIVITY_TO_SR(ACT_PICKUP_GROUND);
	ADD_ACTIVITY_TO_SR(ACT_PICKUP_RACK);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_ANGRY);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_RELAXED);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_STIMULATED);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_AGITATED);
	ADD_ACTIVITY_TO_SR(ACT_WALK_RELAXED);
	ADD_ACTIVITY_TO_SR(ACT_WALK_STIMULATED);
	ADD_ACTIVITY_TO_SR(ACT_WALK_AGITATED);
	ADD_ACTIVITY_TO_SR(ACT_RUN_RELAXED);
	ADD_ACTIVITY_TO_SR(ACT_RUN_STIMULATED);
	ADD_ACTIVITY_TO_SR(ACT_RUN_AGITATED);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_AIM_RELAXED);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_AIM_STIMULATED);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_AIM_AGITATED);
	ADD_ACTIVITY_TO_SR(ACT_WALK_AIM_RELAXED);
	ADD_ACTIVITY_TO_SR(ACT_WALK_AIM_STIMULATED);
	ADD_ACTIVITY_TO_SR(ACT_WALK_AIM_AGITATED);
	ADD_ACTIVITY_TO_SR(ACT_RUN_AIM_RELAXED);
	ADD_ACTIVITY_TO_SR(ACT_RUN_AIM_STIMULATED);
	ADD_ACTIVITY_TO_SR(ACT_RUN_AIM_AGITATED);
	ADD_ACTIVITY_TO_SR(ACT_WALK_HURT);
	ADD_ACTIVITY_TO_SR(ACT_RUN_HURT);
	ADD_ACTIVITY_TO_SR(ACT_SPECIAL_ATTACK1);
	ADD_ACTIVITY_TO_SR(ACT_SPECIAL_ATTACK2);
	ADD_ACTIVITY_TO_SR(ACT_COMBAT_IDLE);
	ADD_ACTIVITY_TO_SR(ACT_WALK_SCARED);
	ADD_ACTIVITY_TO_SR(ACT_RUN_SCARED);
	ADD_ACTIVITY_TO_SR(ACT_VICTORY_DANCE);
	ADD_ACTIVITY_TO_SR(ACT_DIE_HEADSHOT);
	ADD_ACTIVITY_TO_SR(ACT_DIE_CHESTSHOT);
	ADD_ACTIVITY_TO_SR(ACT_DIE_GUTSHOT);
	ADD_ACTIVITY_TO_SR(ACT_DIE_BACKSHOT);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_HEAD);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_CHEST);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_STOMACH);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_LEFTARM);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_RIGHTARM);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_LEFTLEG);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_RIGHTLEG);
	ADD_ACTIVITY_TO_SR(ACT_FLINCH_PHYSICS);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_ON_FIRE);
	ADD_ACTIVITY_TO_SR(ACT_WALK_ON_FIRE);
	ADD_ACTIVITY_TO_SR(ACT_RUN_ON_FIRE);
	ADD_ACTIVITY_TO_SR(ACT_RAPPEL_LOOP);
	ADD_ACTIVITY_TO_SR(ACT_180_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_180_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_90_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_90_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_STEP_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_STEP_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_STEP_BACK);
	ADD_ACTIVITY_TO_SR(ACT_STEP_FORE);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_RANGE_ATTACK1);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_RANGE_ATTACK2);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_MELEE_ATTACK1);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_MELEE_ATTACK2);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_RANGE_ATTACK1_LOW);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_RANGE_ATTACK2_LOW);
	ADD_ACTIVITY_TO_SR(ACT_MELEE_ATTACK_SWING_GESTURE);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_SMALL_FLINCH);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_BIG_FLINCH);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_BLAST);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_HEAD);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_CHEST);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_STOMACH);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_LEFTARM);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_RIGHTARM);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_LEFTLEG);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_FLINCH_RIGHTLEG);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_LEFT);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_RIGHT);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_LEFT45);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_RIGHT45);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_LEFT90);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_RIGHT90);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_LEFT45_FLAT);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_RIGHT45_FLAT);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_LEFT90_FLAT);
	ADD_ACTIVITY_TO_SR(ACT_GESTURE_TURN_RIGHT90_FLAT);
	ADD_ACTIVITY_TO_SR(ACT_DO_NOT_DISTURB);
	ADD_ACTIVITY_TO_SR(ACT_VM_DRAW);
	ADD_ACTIVITY_TO_SR(ACT_VM_HOLSTER);
	ADD_ACTIVITY_TO_SR(ACT_VM_IDLE);
	ADD_ACTIVITY_TO_SR(ACT_VM_FIDGET);
	ADD_ACTIVITY_TO_SR(ACT_VM_PULLBACK);
	ADD_ACTIVITY_TO_SR(ACT_VM_PULLBACK_HIGH);
	ADD_ACTIVITY_TO_SR(ACT_VM_PULLBACK_LOW);
	ADD_ACTIVITY_TO_SR(ACT_VM_THROW);
	ADD_ACTIVITY_TO_SR(ACT_VM_PULLPIN);
	ADD_ACTIVITY_TO_SR(ACT_VM_PRIMARYATTACK);
	ADD_ACTIVITY_TO_SR(ACT_VM_SECONDARYATTACK);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOAD);
	ADD_ACTIVITY_TO_SR(ACT_VM_DRYFIRE);
	ADD_ACTIVITY_TO_SR(ACT_VM_HITLEFT);
	ADD_ACTIVITY_TO_SR(ACT_VM_HITLEFT2);
	ADD_ACTIVITY_TO_SR(ACT_VM_HITRIGHT);
	ADD_ACTIVITY_TO_SR(ACT_VM_HITRIGHT2);
	ADD_ACTIVITY_TO_SR(ACT_VM_HITCENTER);
	ADD_ACTIVITY_TO_SR(ACT_VM_HITCENTER2);
	ADD_ACTIVITY_TO_SR(ACT_VM_MISSLEFT);
	ADD_ACTIVITY_TO_SR(ACT_VM_MISSLEFT2);
	ADD_ACTIVITY_TO_SR(ACT_VM_MISSRIGHT);
	ADD_ACTIVITY_TO_SR(ACT_VM_MISSRIGHT2);
	ADD_ACTIVITY_TO_SR(ACT_VM_MISSCENTER);
	ADD_ACTIVITY_TO_SR(ACT_VM_MISSCENTER2);
	ADD_ACTIVITY_TO_SR(ACT_VM_HAULBACK);
	ADD_ACTIVITY_TO_SR(ACT_VM_SWINGHARD);
	ADD_ACTIVITY_TO_SR(ACT_VM_SWINGMISS);
	ADD_ACTIVITY_TO_SR(ACT_VM_SWINGHIT);
	ADD_ACTIVITY_TO_SR(ACT_VM_IDLE_TO_LOWERED);
	ADD_ACTIVITY_TO_SR(ACT_VM_IDLE_LOWERED);
	ADD_ACTIVITY_TO_SR(ACT_VM_LOWERED_TO_IDLE);
	ADD_ACTIVITY_TO_SR(ACT_VM_RECOIL1);
	ADD_ACTIVITY_TO_SR(ACT_VM_RECOIL2);
	ADD_ACTIVITY_TO_SR(ACT_VM_RECOIL3);
	ADD_ACTIVITY_TO_SR(ACT_RUNTOIDLE);
	ADD_ACTIVITY_TO_SR(ACT_PHYSCANNON_DETACH);
	ADD_ACTIVITY_TO_SR(ACT_PHYSCANNON_ANIMATE);
	ADD_ACTIVITY_TO_SR(ACT_PHYSCANNON_ANIMATE_PRE);
	ADD_ACTIVITY_TO_SR(ACT_PHYSCANNON_ANIMATE_POST);
	ADD_ACTIVITY_TO_SR(ACT_VM_FIREMODE);
	ADD_ACTIVITY_TO_SR(ACT_VM_IFIREMODE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DFIREMODE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DIFIREMODE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DRAW_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_IDLE_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOAD_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_TIRED);
	ADD_ACTIVITY_TO_SR(ACT_VM_TIRED_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_DOWN);
	ADD_ACTIVITY_TO_SR(ACT_VM_DOWN_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_CRAWL);
	ADD_ACTIVITY_TO_SR(ACT_VM_CRAWL_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_IIN);
	ADD_ACTIVITY_TO_SR(ACT_VM_IOUT);
	ADD_ACTIVITY_TO_SR(ACT_VM_IIN_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_IOUT_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_ISHOOT);
	ADD_ACTIVITY_TO_SR(ACT_VM_IIDLE);
	ADD_ACTIVITY_TO_SR(ACT_VM_IRECOIL1);
	ADD_ACTIVITY_TO_SR(ACT_VM_IRECOIL2);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOADEMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_IRON_IDLE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_LIFTED_IDLE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_IDLE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_IN);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_OUT);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_IRON_IN);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_IRON_OUT);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_FIRE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_IRON_FIRE);
	ADD_ACTIVITY_TO_SR(ACT_VM_SHOOTLAST);
	ADD_ACTIVITY_TO_SR(ACT_VM_ISHOOT_LAST);
	ADD_ACTIVITY_TO_SR(ACT_VM_ISHOOT_DRY);
	ADD_ACTIVITY_TO_SR(ACT_VM_ISHOOTDRY);
	ADD_ACTIVITY_TO_SR(ACT_VM_READY);
	ADD_ACTIVITY_TO_SR(ACT_VM_HOLSTER_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_IIDLE_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_ROF_UP);
	ADD_ACTIVITY_TO_SR(ACT_VM_ROF_DOWN);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_RELOAD);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_RELOAD_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_DRYFIRE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DEPLOYED_IRON_DRYFIRE);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOAD_INSERT);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOAD_INSERT_PULL);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOAD_END);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOAD_END_EMPTY);
	ADD_ACTIVITY_TO_SR(ACT_VM_PULLBACK_HIGH_BAKE);
	ADD_ACTIVITY_TO_SR(ACT_VM_DRAW_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_DRAWFULL_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_READY_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_IDLE_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_DOWN_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_CRAWL_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_RELOAD_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_HOLSTER_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_HOLSTERFULL_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_IIN_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_IIDLE_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_IOUT_M203);
	ADD_ACTIVITY_TO_SR(ACT_VM_ISHOOT_M203);
	ADD_ACTIVITY_TO_SR(ACT_TURN);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_DEPLOY);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_DEPLOY);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_LEANR);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_LEANL);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_DEPLOY);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP);
	ADD_ACTIVITY_TO_SR(ACT_TRANS_STAND_CROUCH);
	ADD_ACTIVITY_TO_SR(ACT_TRANS_STAND_PRONE);
	ADD_ACTIVITY_TO_SR(ACT_TRANS_CROUCH_STAND);
	ADD_ACTIVITY_TO_SR(ACT_TRANS_CROUCH_PRONE);
	ADD_ACTIVITY_TO_SR(ACT_TRANS_PRONE_STAND);
	ADD_ACTIVITY_TO_SR(ACT_TRANS_PRONE_CROUCH);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_PISTOL);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_KNIFE);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_GRENADE);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_DEPLOY_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_DEPLOY_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_DEPLOY_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_M16);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_DEPLOY_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_DEPLOY_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_DEPLOY_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_M249);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_DEPLOY_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_DEPLOY_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_DEPLOY_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_RPK);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_DEPLOY_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_DEPLOY_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_DEPLOY_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_SKS);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_DEPLOY_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_DEPLOY_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_DEPLOY_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_AK47);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_AIM_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_WALK_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_RUN_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANR_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_STAND_LEANL_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_SPRINT_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_AIM_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_WALK_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_CROUCH_RUN_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_PRONE_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_CRAWL_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_INS_JUMP_SHOTGUN);
	ADD_ACTIVITY_TO_SR(ACT_RANGE_ATTACK_RPG);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_RPG_RELAXED);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_ANGRY_RPG);
	ADD_ACTIVITY_TO_SR(ACT_IDLE_RPG);
	ADD_ACTIVITY_TO_SR(ACT_WALK_RPG);
	ADD_ACTIVITY_TO_SR(ACT_WALK_CROUCH_RPG);
	ADD_ACTIVITY_TO_SR(ACT_RUN_RPG);
	ADD_ACTIVITY_TO_SR(ACT_SPRINT_RPG);
	ADD_ACTIVITY_TO_SR(ACT_RUN_CROUCH_RPG);
	ADD_ACTIVITY_TO_SR(ACT_COVER_LOW_RPG);
}

void CreateAIActivityList()
{
	DeleteAIActivityList();
	m_pActivitySR = new CStringRegistry();
	InitDefaultActivitySR();
}

void DeleteAIActivityList()
{
	delete m_pActivitySR;
	m_pActivitySR = NULL;
	m_iNumActivities = 0;
}