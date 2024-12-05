//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef TRACKIR_INTERFACE_H
#define TRACKIR_INTERFACE_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
extern ConVar trackir;
extern ConVar trackir_mode;
extern ConVar trackir_igheadi;

//=========================================================
//=========================================================
enum TrackIRModes_t
{
	TMODE_PRESS = 0,		// default is off
	TMODE_RELEASE,			// default is on
	TMODE_COUNT
};

//=========================================================
//=========================================================
struct TrackIRData_t
{
	TrackIRData_t( );

	QAngle m_angHeadAngles;
	float m_flLean;
	float m_flLookOver;
	float m_flZoom;
};

//=========================================================
//=========================================================
class CTrackIRInterface : public CAutoGameSystemPerFrame
{
public:
	~CTrackIRInterface( );

	static CTrackIRInterface &TrackIRInterface( void );

	bool IsInit( void ) const { return m_bInit; }

	bool Init( void );
	void Shutdown( void );

	const TrackIRData_t &GetData( void ) const { return m_Data; }

private:
	CTrackIRInterface( );

	bool GetDLLPath( char *pszBuffer );

	void StartDataTransmission( void );
	void StopDataTransmission( void );

	void LevelInitPreEntity( void );
	void LevelShutdownPreEntity( void );

	void Update( float flFrametime );

private:
	static CTrackIRInterface m_TrackIRInterface;

	bool m_bInit;
	bool m_bCanTransmit;

	unsigned long m_iNPFrameSignature;

	TrackIRData_t m_Data;

	bool m_bInterpActive;
	float m_flStartInterpTime;
	TrackIRData_t m_InterpData;
};

//=========================================================
//=========================================================
extern CTrackIRInterface &TrackIRInterface( void );

#endif // TRACKIR_INTERFACE_H