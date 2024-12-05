//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_FRAME_H
#define VGUI_FRAME_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/vgui.h>
#include <vgui/dar.h>
#include <vgui_controls/panel.h>
#include <vgui_controls/editablepanel.h>
#include <vgui_controls/focusnavgroup.h>

namespace vgui
{

class FrameButton;
class FrameSystemButton;

//-----------------------------------------------------------------------------
// Purpose: Windowed frame
//-----------------------------------------------------------------------------
class Frame : public EditablePanel
{
	DECLARE_CLASS_SIMPLE( Frame, EditablePanel );

public:
	Frame(Panel *parent, const char *panelName, bool showTaskbarIcon = true);
	~Frame();

	// Set the text in the title bar.  Set surfaceTitle=true if you want this to be the taskbar text as well.
	virtual void SetTitle(const char *title, bool surfaceTitle);
	virtual void SetTitle(const wchar_t *title, bool surfaceTitle);

	// Bring the frame to the front and requests focus, ensures it's not minimized
	virtual void Activate();

	// activates the dialog; if dialog is not currently visible it starts it minimized and flashing in the taskbar
	virtual void ActivateMinimized();

	// closes the dialog
	MESSAGE_FUNC( Close, "Close" );
	MESSAGE_FUNC( CloseModal, "CloseModal" );

	// sets the dialog to delete self on close
	virtual void SetDeleteSelfOnClose( bool state );

	// Move the dialog to the center of the screen 
	virtual void MoveToCenterOfScreen();

	// Set the movability of the panel
	virtual void SetMoveable(bool state);
	// Check the movability of the panel
	virtual bool IsMoveable();

	// Set the resizability of the panel
	virtual void SetSizeable(bool state);
	// Check the resizability of the panel
	virtual bool IsSizeable();
	// Toggle visibility of the system menu button
	virtual void SetMenuButtonVisible(bool state);
	void SetMenuButtonResponsive(bool state);

	// Toggle visibility of the minimize button
	virtual void SetMinimizeButtonVisible(bool state);
	// Toggle visibility of the maximize button
	virtual void SetMaximizeButtonVisible(bool state);
	// Toggles visibility of the minimize-to-systray icon (defaults to false)
	virtual void SetMinimizeToSysTrayButtonVisible(bool state);

	// Toggle visibility of the close button
	virtual void SetCloseButtonVisible(bool state);

	// returns true if the dialog is currently minimized
	virtual bool IsMinimized();
	// Flash the window system tray button until the frame gets focus
	virtual void FlashWindow();
	// Stops any window flashing
	virtual void FlashWindowStop();
	// command handling
	virtual void OnCommand(const char *command);

	// Get the system menu 
	virtual Menu *GetSysMenu();
	// Set the system menu 
	virtual void SetSysMenu(Menu *menu);

	// set whether the title bar should be rendered
	virtual void SetTitleBarVisible( bool state );

	// Pongles [
	virtual void SetUseFading( bool state );
	// Pongles ]

	// When moving via caption, don't let any part of window go outside parent's bounds
	virtual void SetClipToParent( bool state );
	virtual bool GetClipToParent() const;

	// Set to true to make the caption height small
	virtual void SetSmallCaption( bool state );
	virtual bool IsSmallCaption() const;

	virtual int GetDraggerSize();
	virtual int GetCornerSize();
	virtual int GetBottomRightSize();
	virtual int GetCaptionHeight();

	/* CUSTOM MESSAGE HANDLING
		"SetTitle"
			input:	"text"	- string to set the title to be
	*/

	// Load the control settings 
	virtual void LoadControlSettings(const char *dialogResourceName, const char *pathID = NULL);

	void SetChainKeysToParent( bool state );
	bool CanChainKeysToParent() const;

	// Shows the dialog in a modal fashion
	virtual void DoModal();

protected:
	// Respond to mouse presses
	virtual void OnMousePressed(MouseCode code);
	// Respond to Key typing
	virtual void OnKeyCodeTyped(KeyCode code);
	virtual void OnKeyTyped(wchar_t unichar);
	// Respond to Key releases
	virtual void OnKeyCodeReleased(KeyCode code);
	// Respond to Key focus ticks
	virtual void OnKeyFocusTicked();
	virtual void ApplySchemeSettings(IScheme *pScheme);
	// Recalculate the position of all items
	virtual void PerformLayout();
	// Respond when a close message is recieved.  Can be called directly to close a frame.
	virtual void OnClose();
	// Respond to a window finishing its closure. i.e. when a fading window has fully finished its fadeout.
	virtual void OnFinishedClose();
	// Minimize the window on the taskbar.
	MESSAGE_FUNC( OnMinimize, "Minimize" );
	// Called when minimize-to-systray button is pressed (does nothing by default)
	virtual void OnMinimizeToSysTray();
	// the frame close button was pressed
	MESSAGE_FUNC( OnCloseFrameButtonPressed, "CloseFrameButtonPressed" );
	// Add the child to the focus nav group
	virtual void OnChildAdded(VPANEL child);
	// settings
	virtual void ApplySettings(KeyValues *inResourceData);
	// records the settings into the resource data
	virtual void GetSettings(KeyValues *outResourceData);
	virtual const char *GetDescription( void );

	// gets the default position and size on the screen to appear the first time (defaults to centered)
	virtual bool GetDefaultScreenPosition(int &x, int &y, int &wide, int &tall);

	// painting
	virtual void PaintBackground();

	// per-frame thinking, used for transition effects
	virtual void OnThink();

	// screen size
	virtual void OnScreenSizeChanged(int iOldWide, int iOldTall);

	// Get the size of the panel inside the frame edges.
	virtual void GetClientArea(int &x, int &y, int &wide, int &tall);

	// user configuration settings
	// this is used for any control details the user wants saved between sessions
	// eg. dialog positions, last directory opened, list column width
	virtual void ApplyUserConfigSettings(KeyValues *userConfig);

	// returns user config settings for this control
	virtual void GetUserConfigSettings(KeyValues *userConfig);

	// optimization, return true if this control has any user config settings
	virtual bool HasUserConfigSettings();

private:
	MESSAGE_FUNC_CHARPTR( InternalSetTitle, "SetTitle", text );
	MESSAGE_FUNC( InternalFlashWindow, "FlashWindow" );
	MESSAGE_FUNC_PARAMS( OnDialogVariablesChanged, "DialogVariables", dialogVariables );

	void SetupResizeCursors();
	void LayoutProportional( FrameButton *bt);
	void FinishClose();
	void OnFrameFocusChanged(bool bHasFocus);

	Color _titleBarBgColor;
	Color _titleBarDisabledBgColor;
	Color _titleBarFgColor;
	Color _titleBarDisabledFgColor;
	Color m_InFocusBgColor;
	Color m_OutOfFocusBgColor;
	TextImage *_title;
	Panel * _topGrip;
	Panel *_bottomGrip;
	Panel *_leftGrip;
	Panel *_rightGrip;
	Panel *_topLeftGrip;
	Panel *_topRightGrip;
	Panel *_bottomLeftGrip;
	Panel *_bottomRightGrip;
	Panel *_captionGrip;
	FrameButton *_minimizeButton;
	FrameButton	*_maximizeButton;
	FrameButton *_minimizeToSysTrayButton;
	FrameButton	*_closeButton;
	FrameSystemButton *_menuButton;
	Frame *_resizeable;
	bool _sizeable;
	bool _moveable;
	bool m_bHasFocus;
	bool _flashWindow;
	bool _nextFlashState;
	bool _drawTitleBar;
	bool m_bPreviouslyVisible;
	bool m_bFadingOut;
	bool m_bDeleteSelfOnClose;
	float m_flTransitionEffectTime;
	float m_flFocusTransitionEffectTime;
	int m_iClientInsetX, m_iClientInsetY;
	int m_iTitleTextInsetX;
	Menu *_sysMenu;
	bool m_bClipToParent;
	bool m_bSmallCaption;
	int m_nGripperWidth;
	bool	m_bChainKeysToParent;
	bool	m_bPrimed;
	VPANEL	m_hPreviousModal;
	// Pongles [
	bool _useFading;
	// Pongles ]
};

} // namespace vgui

#endif // VGUI_FRAME_H
