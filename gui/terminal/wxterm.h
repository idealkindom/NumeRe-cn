/*****************************************************************************
    NumeRe: Framework fuer Numerische Rechnungen
    Copyright (C) 2018  Erik Haenel et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/


#ifndef INCLUDE_WXTERM
#define INCLUDE_WXTERM

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include "../NumeReWindow.h"
#include "../../common/Options.h"
#include "gterm.hpp"
#include "../../kernel/kernel.hpp"
#include "../../kernel/debugmessenger.hpp"

#define wxEVT_COMMAND_TERM_RESIZE        wxEVT_USER_FIRST + 1000
#define wxEVT_COMMAND_TERM_NEXT          wxEVT_USER_FIRST + 1001

#define EVT_TERM_RESIZE(id, fn) { wxEVT_COMMAND_TERM_RESIZE, id, -1, (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &fn, (wxObject *)NULL },

using namespace std;

// The terminal class for the GUI
// It's a specialisation of the GenericTerminal
class wxTerm : public wxWindow, public GenericTerminal, public wxThreadHelper
{
    // Easier to use the NumeReKernel as a friend
    // to create the communication
    friend class NumeReKernel;

	private:
		int
		m_charWidth,
		m_charHeight,
		m_init,
		m_width,
		m_height,
		m_selx1,
		m_sely1,
		m_selx2,
		m_sely2,
		m_curX,
		m_curY,
		m_curFG,
		m_curBG,
		m_curFlags,
		m_curState,
		m_curBlinkRate;

		int m_scrollBarWidth;
		int m_charsInLine;
		int m_linesDisplayed;

		unsigned char
		m_curChar;

		bool
		m_selecting,
		m_marking;

		bool m_inUpdateSize;
		bool m_isActive;
		bool m_updateProcedureLibrary;

		wxColour
		m_color_defs[16], *m_colors;

		wxPen
		m_colorPen_defs[16], *m_colorPens;

		wxFont
		m_normalFont,
		m_underlinedFont,
		m_boldFont,
		m_boldUnderlinedFont;

		wxDC
		* m_curDC;

		wxMemoryDC
		m_memDC;

		wxBitmap
		* m_bitmap;

		wxTimer
		m_timer;

		NumeReWindow* m_wxParent;

		Options* m_options;

	public:
		enum BOLDSTYLE
		{
			DEFAULT = -1,
			COLOR = 0,
			OVERSTRIKE = 1,
			FONT = 2
		};

		void SetParent(NumeReWindow* frame)
		{
			if (frame && !m_wxParent)
				m_wxParent = frame;
		}
		int getTextHeight()
		{
			return m_charHeight;
		}
		Debugmessenger _guimessenger;

	private:
		void pipe_command(const string& sCommand);
		BOLDSTYLE
		m_boldStyle;

        bool filterKeyCodes(int keyCode);
		void scrollToInput();
		void MarkSelection(bool bRectangular = false);
		void DoDrawCursor(int fg_color, int bg_color, int flags,
						  int x, int y, unsigned char c);

        // Private event handler functions
		void OnChar(wxKeyEvent& event);
		void OnKeyDown(wxKeyEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnLeftDown(wxMouseEvent& event);
		void OnLoseMouseCapture(wxMouseCaptureLostEvent& event);
		void OnLeftUp(wxMouseEvent& event);
		void OnMouseMove(wxMouseEvent& event);
		void OnEnter(wxMouseEvent& event);
		void OnTimer(wxTimerEvent& event);
		void OnActivate(wxActivateEvent& event);
		void OnSize(wxSizeEvent& event);
		void OnGainFocus(wxFocusEvent& event);
		void OnLoseFocus(wxFocusEvent& event);

	protected:
		virtual wxThread::ExitCode Entry();
		NumeReKernel _kernel;
		wxCriticalSection m_kernelCS;
		NumeReKernel::KernelStatus m_KernelStatus;
		bool m_bCommandAvailable;
		bool m_bTableEditAvailable;
		bool m_bTableEditCanceled;
		bool m_bContinueDebug;
		string m_sCommandLine;
		string m_sAnswer;

	public:
	    // Constructor and destructor
		wxTerm(wxWindow* parent, wxWindowID id,
			   Options* _option,
			   const wxString& sPath,
			   const wxPoint& pos = wxDefaultPosition,
			   int width = 80, int height = 24,
			   const wxString& name = "wxTerm");
		virtual ~wxTerm();

		// Kernel communication functions
		void pass_command(const string& command);
		Settings getKernelSettings();
		void setKernelSettings(const Settings&);
		void EndKernelTask();
		void CancelCalculation();
		void StartKernelTask();
		void OnThreadUpdate(wxThreadEvent& event);
		void OnClose(wxCloseEvent& event);
		vector<string> getPathSettings();
		void passEditedTable(NumeRe::Container<string>& _container);
		void cancelTableEdit()
		{
			wxCriticalSectionLocker lock(m_kernelCS);
			m_bTableEditCanceled = true;
		}
		void continueDebug()
		{
			wxCriticalSectionLocker lock(m_kernelCS);
			m_bContinueDebug = true;
		}
		string getDocumentation(const string& sCommand);
		map<string, string> getPluginLanguageStrings();
		map<string, string> getFunctionLanguageStrings();
		void UpdateLibrary()
		{
			m_updateProcedureLibrary = true;
		}

		// Styling functions
        bool SetFont(const wxFont& font);
		void GetDefColors(wxColor colors[16], wxTerm::BOLDSTYLE boldStyle = wxTerm::DEFAULT);
		int GetCursorBlinkRate()
		{
			return m_curBlinkRate;
		}
		void SetCursorBlinkRate(int rate);

		// Text printing functions
		virtual void DrawText(int fg_color, int bg_color, int flags,
							  int x, int y, const string& sText);
		virtual void DrawCursor(int fg_color, int bg_color, int flags,
								int x, int y, unsigned char c);

		virtual void ClearChars(int bg_color, int x, int y, int w, int h);
		virtual void ProcessInput(int len, const string& sData);
		virtual void ProcessOutput(int len, const string& sData);

		// Terminal control functions
		void ScrollTerminal(int numLines, bool scrollUp = true);
		void ClearSelection();
		bool HasSelection();
		wxString GetSelection();
		bool IsWorking()
		{
			return GetThread() && GetThread()->IsRunning();
		}
		void UpdateSize();
		void UpdateColors();
		virtual void ModeChange(int state);
		virtual void Bell();
		virtual void ResizeTerminal(int w, int h);
		virtual void UpdateRemoteSize(int width, int height);
		int GetTermWidth()
		{
			return m_charsInLine;
		}
		int GetTermHeight()
		{
			return m_linesDisplayed;
		}

		DECLARE_EVENT_TABLE()
};

#endif /* INCLUDE_WXTERM */
