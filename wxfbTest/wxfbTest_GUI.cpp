///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul 17 2006)
// http://wxformbuilder.sourceforge.net/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "wxfbTest_GUI.h"

#include "..\\bin\\resources\\icons\\bitmap_button.xpm"
#include "..\\bin\\resources\\icons\\button.xpm"
#include "..\\bin\\resources\\icons\\calendar.xpm"

///////////////////////////////////////////////////////////////////////////

MainFrame::MainFrame( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxFrame( parent, id, title, pos, size, style )
{
	m_menubar1 = new wxMenuBar( wxMB_DOCKABLE );
	wxMenu* file;
	file = new wxMenu();
	wxMenuItem* exit = new wxMenuItem( file, ID_EXIT, wxString( wxT("E&xit") ) + wxT('\t') + wxT("Alt+F4"), wxT("Exit the Application"), wxITEM_NORMAL );
	file->Append( exit );
	m_menubar1->Append( file, wxT("&File") );
	wxMenu* help;
	help = new wxMenu();
	wxMenuItem* about = new wxMenuItem( help, ID_ABOUT, wxString( wxT("&About") ) + wxT('\t') + wxT("F1"), wxT("About This Application . . ."), wxITEM_NORMAL );
	help->Append( about );
	m_menubar1->Append( help, wxT("&Help") );
	this->SetMenuBar( m_menubar1 );
	
	m_statusBar1 = this->CreateStatusBar( 1, wxST_SIZEGRIP, ID_DEFAULT );
	m_toolBar1 = this->CreateToolBar( wxTB_HORIZONTAL, ID_DEFAULT ); 
	m_toolBar1->AddTool( ID_DEFAULT, wxT("tool"), wxBitmap( button_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT(""), wxT("") );
	m_toolBar1->AddTool( ID_DEFAULT, wxT("tool"), wxBitmap( bitmap_button_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT(""), wxT("") );
	m_toolBar1->Realize();
	
}

MainPanel::MainPanel( wxWindow* parent, int id, wxPoint pos, wxSize size, int style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button1 = new wxButton( this, ID_DEFAULT, wxT("MyButton"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_button1, 0, wxALL, 5 );
	
	m_bpButton1 = new wxBitmapButton( this, ID_DEFAULT, wxBitmap( calendar_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bSizer2->Add( m_bpButton1, 0, wxALL, 5 );
	
	m_textCtrl1 = new wxTextCtrl( this, ID_DEFAULT, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_textCtrl1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText1 = new wxStaticText( this, ID_DEFAULT, wxT("Unicode Test: ÐœÐ°ÐºÑÐ¸Ð¼ Ð•Ð¼ÐµÐ»ÑŒÑÐ½Ð¾Ð²"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_staticText1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_checkBox1 = new wxCheckBox( this, ID_DEFAULT, wxT("Check Me!"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_checkBox1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_comboBox1 = new wxComboBox( this, ID_DEFAULT, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	bSizer2->Add( m_comboBox1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("label") ), wxHORIZONTAL );
	
	m_listBox1 = new wxListBox( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	m_listBox1->Append( wxT("Test1") );
	m_listBox1->Append( wxT("Test2") );
	sbSizer1->Add( m_listBox1, 0, wxALL, 5 );
	
	{
	wxString __choices[] = { wxT("Test1")wxT("Test1"), wxT("Test2") };
	int __nchoices = sizeof( __choices ) / sizeof( wxString );
	m_radioBox1 = new wxRadioBox( this, ID_DEFAULT, wxT("wxRadioBox"), wxDefaultPosition, wxDefaultSize, __nchoices, __choices, 1, wxRA_SPECIFY_COLS );
	}
	sbSizer1->Add( m_radioBox1, 0, wxALL, 5 );
	
	m_grid1 = new wxGrid( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, 0 );
	m_grid1->CreateGrid( 5, 5 );
	sbSizer1->Add( m_grid1, 0, wxALL, 5 );
	
	bSizer1->Add( sbSizer1, 0, wxALL|wxEXPAND, 5 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 2, 2, 0, 0 );
	
	m_bitmap1 = new wxStaticBitmap( this, ID_DEFAULT, wxBitmap( wxT("..\\bin\\resources\\splash.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 100,20 ), 0 );
	gSizer1->Add( m_bitmap1, 0, wxALL|wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	gSizer1->Add( m_staticline1, 0, wxALL|wxEXPAND, 5 );
	
	m_listCtrl = new wxListCtrl( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxLC_ICON|wxLC_REPORT );
	gSizer1->Add( m_listCtrl, 0, wxALL|wxEXPAND, 5 );
	
	{
	wxString __choices[] = { wxT("Test1")wxT("Test1"), wxT("Test2") };
	int __nchoices = sizeof( __choices ) / sizeof( wxString );
	m_choice1 = new wxChoice( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, __nchoices, __choices, 0 );
	}
	
	gSizer1->Add( m_choice1, 0, wxALL|wxEXPAND, 5 );
	
	m_slider1 = new wxSlider( this, ID_DEFAULT, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	gSizer1->Add( m_slider1, 0, wxALL|wxEXPAND, 5 );
	
	m_gauge1 = new wxGauge( this, ID_DEFAULT, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	gSizer1->Add( m_gauge1, 0, wxALL|wxEXPAND, 5 );
	
	bSizer1->Add( gSizer1, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
}
