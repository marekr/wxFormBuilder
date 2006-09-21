///////////////////////////////////////////////////////////////////////////////
//
// wxFormBuilder - A Visual Dialog Editor for wxWidgets.
// Copyright (C) 2005 José Antonio Hurtado
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Written by
//   José Antonio Hurtado - joseantonio.hurtado@gmail.com
//   Juan Antonio Ortega  - jortegalalmolda@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

#include "component.h"
#include "plugin.h"
#include "xrcconv.h"

#include <wx/plotctrl/plotctrl.h>
#include <wx/wxFlatNotebook/wxFlatNotebook.h>
#include <wx/awx/led.h>

// wxPropertyGrid
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

// wxScintilla
#include <wx/wxscintilla.h>

#include <math.h>

#include <wx/object.h>
#include <wx/image.h>
///////////////////////////////////////////////////////////////////////////////
class PlotCtrlComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		// Set plot style
		wxPlotCtrl* plot = new wxPlotCtrl((wxWindow*)parent,-1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")) );

		plot->SetScrollOnThumbRelease( obj->GetPropertyAsInteger(_("scroll_on_thumb_release")) != 0 );
		plot->SetCrossHairCursor( obj->GetPropertyAsInteger( _("crosshair_cursor") ) != 0 );
		plot->SetDrawSymbols( obj->GetPropertyAsInteger( _("draw_symbols") ) != 0 );
		plot->SetDrawLines( obj->GetPropertyAsInteger( _("draw_lines") ) != 0 );
		plot->SetDrawSpline( obj->GetPropertyAsInteger( _("draw_spline") ) != 0 );
		plot->SetDrawGrid( obj->GetPropertyAsInteger( _("draw_grid") ) != 0 );

		plot->SetShowXAxis( obj->GetPropertyAsInteger( _("show_x_axis") ) != 0 );
		plot->SetShowYAxis( obj->GetPropertyAsInteger( _("show_y_axis") ) != 0 );
		plot->SetShowXAxisLabel( obj->GetPropertyAsInteger( _("show_x_axis_label") ) != 0 );
		plot->SetShowYAxisLabel( obj->GetPropertyAsInteger( _("show_y_axis_label") ) != 0 );
		plot->SetShowPlotTitle( obj->GetPropertyAsInteger( _("show_plot_title") ) != 0 );
		plot->SetShowKey( obj->GetPropertyAsInteger( _("show_key") ) != 0 );

		plot->SetAreaMouseFunction( (wxPlotCtrlMouse_Type)obj->GetPropertyAsInteger( _("area_mouse_function") ) );
		plot->SetAreaMouseMarker( (wxPlotCtrlMarker_Type)obj->GetPropertyAsInteger( _("area_mouse_marker") ) );

		if ( !obj->IsNull( _("grid_colour") ) )
		{
			plot->SetGridColour( obj->GetPropertyAsColour( _("grid_colour") ) );
		}
		if ( !obj->IsNull( _("border_colour") ) )
		{
			plot->SetBorderColour( obj->GetPropertyAsColour( _("border_colour") ) );
		}

		if ( !obj->IsNull( _("axis_font") ) )
		{
			plot->SetAxisFont( obj->GetPropertyAsFont( _("axis_font") ) );
		}
		if ( !obj->IsNull( _("axis_colour") ) )
		{
			plot->SetAxisColour( obj->GetPropertyAsColour( _("axis_colour") ) );
		}

		if ( !obj->IsNull( _("axis_label_font") ) )
		{
			plot->SetAxisLabelFont( obj->GetPropertyAsFont( _("axis_label_font") ) );
		}
		if ( !obj->IsNull( _("axis_label_colour") ) )
		{
			plot->SetAxisLabelColour( obj->GetPropertyAsColour( _("axis_label_colour") ) );
		}

		if ( !obj->IsNull( _("plot_title_font") ) )
		{
			plot->SetPlotTitleFont( obj->GetPropertyAsFont( _("plot_title_font") ) );
		}
		if ( !obj->IsNull( _("plot_title_colour") ) )
		{
			plot->SetPlotTitleColour( obj->GetPropertyAsColour( _("plot_title_colour") ) );
		}

		if ( !obj->IsNull( _("key_font") ) )
		{
			plot->SetKeyFont( obj->GetPropertyAsFont( _("key_font") ) );
		}
		if ( !obj->IsNull( _("key_colour") ) )
		{
			plot->SetKeyColour( obj->GetPropertyAsColour( _("key_colour") ) );
		}

		plot->SetXAxisLabel( obj->GetPropertyAsString( _("x_axis_label") ) );
		plot->SetYAxisLabel( obj->GetPropertyAsString( _("y_axis_label") ) );
		plot->SetPlotTitle( obj->GetPropertyAsString( _("plot_title") ) );
		plot->SetKeyPosition( obj->GetPropertyAsPoint( _("key_position") ) );

		wxPlotFunction plotFunc;
		plotFunc.Create( obj->GetPropertyAsString(_("sample_function")), wxT("x") );
		if ( plotFunc.Ok() )
		{
			plot->AddCurve( plotFunc, true, true );
		}

		return plot;

	}
	/*TiXmlElement* ExportToXrc(IObject *obj)
	{
		ObjectToXrcFilter xrc(obj, _("wxPlotWindow"), obj->GetPropertyAsString(_("name")));
		xrc.AddWindowProperties();
		xrc.AddProperty(_("style"),_("style"), XRC_TYPE_BITLIST);
		return xrc.GetXrcObject();
	}

	TiXmlElement* ImportFromXrc(TiXmlElement *xrcObj)
	{
		XrcToXfbFilter filter(xrcObj, _("wxPlotWindow"));
		filter.AddWindowProperties();
		filter.AddProperty(_("style"),_("style"), XRC_TYPE_BITLIST);
		return filter.GetXfbObject();
	}*/
};
class PropertyGridComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		wxPropertyGrid* pg = new wxPropertyGrid((wxWindow *)parent,-1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")),
			obj->GetPropertyAsInteger(_("style")) | obj->GetPropertyAsInteger(_("window_style")));

		if ( !obj->GetPropertyAsString(_("extra_style")).empty() )
		{
			pg->SetExtraStyle( obj->GetPropertyAsInteger( _("extra_style") ) );
		}

		pg->AppendCategory(wxT("Sample Category"));

		// Add string property
		pg->Append( wxStringProperty(wxT("Label"),wxT("Name"),wxT("Initial Value")) );

		// Add int property
		pg->Append ( wxIntProperty ( wxT("IntProperty"), wxPG_LABEL, 12345678 ) );

		// Add float property (value type is actually double)
		pg->Append ( wxFloatProperty ( wxT("FloatProperty"), wxPG_LABEL, 12345.678 ) );

		// Add a bool property
		pg->Append ( wxBoolProperty ( wxT("BoolProperty"), wxPG_LABEL, false ) );
		pg->Append ( wxBoolProperty ( wxT("BoolPropertyAsCheckbox"), wxPG_LABEL, true ) );
		pg->SetPropertyAttribute( wxT("BoolPropertyAsCheckbox"), wxPG_BOOL_USE_CHECKBOX, (long)1);

		// A string property that can be edited in a separate editor dialog.
		pg->Append ( wxLongStringProperty (wxT("LongStringProperty"),
			wxPG_LABEL,
			wxT("This is much longer string than the ")
			wxT("first one. Edit it by clicking the button.")));

		// String editor with dir selector button.
		pg->Append ( wxDirProperty( wxT("DirProperty"), wxPG_LABEL, ::wxGetUserHome()) );

		// A file selector property.
		wxPGId fid = pg->Append ( wxFileProperty( wxT("FileProperty"), wxPG_LABEL, wxEmptyString ) );

		pg->AppendCategory( wxT("Sample Parent Property") );
		wxPGId pid = pg->Append( wxParentProperty(wxT("Car"),wxPG_LABEL) );
		pg->AppendIn( pid, wxStringProperty(wxT("Model"), wxPG_LABEL,wxT("Lamborghini Diablo SV")) );
		pg->AppendIn( pid, wxIntProperty(wxT("Engine Size (cc)"), wxPG_LABEL, 5707) );

		wxPGId speedId = pg->AppendIn( pid, wxParentProperty(wxT("Speeds"),wxPG_LABEL) );
		pg->AppendIn( speedId, wxIntProperty(wxT("Max. Speed (mph)"),wxPG_LABEL,300) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("0-100 mph (sec)"),wxPG_LABEL,3.9) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("1/4 mile (sec)"),wxPG_LABEL,8.6) );
		pg->AppendIn( pid, wxIntProperty(wxT("Price ($)"), wxPG_LABEL, 300000) );

		if ( obj->GetPropertyAsInteger( wxT("include_advanced") ) )
		{
			pg->AppendCategory( wxT("Advanced Properties") );
			// wxArrayStringProperty embeds a wxArrayString.
			pg->Append ( wxArrayStringProperty( wxT("Example of ArrayStringProperty"), wxT("ArrayStringProp") ) );

			// Image file property. Wildcard is auto-generated from available
			// image handlers, so it is not set this time.
			pg->Append ( wxImageFileProperty(wxT("Example of ImageFileProperty"), wxT("ImageFileProp")));

			// Font property has sub-properties. Note that we give window's font as initial value.
			pg->Append ( wxFontProperty(wxT("Font"), wxPG_LABEL, wxFont()) );

			// Colour property with arbitrary colour.
			pg->Append ( wxColourProperty(wxT("My Colour 1"), wxPG_LABEL, wxColour(242,109,0) ) );

			// System colour property.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 1"), wxPG_LABEL, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)) );

			// System colour property with custom colour.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 2"), wxPG_LABEL, wxColour(0,200,160) ) );

			// Cursor property
			pg->Append ( wxCursorProperty (wxT("My Cursor"), wxPG_LABEL, wxCURSOR_ARROW));
		}

		return pg;
	}
};

class PropertyGridManagerComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		wxPropertyGridManager* pg = new wxPropertyGridManager((wxWindow *)parent, -1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")),
			obj->GetPropertyAsInteger(_("style")) | obj->GetPropertyAsInteger(_("window_style")));

		if ( !obj->GetPropertyAsString(_("extra_style")).empty() )
		{
			pg->SetExtraStyle( obj->GetPropertyAsInteger( _("extra_style") ) );
		}

		// Adding a page sets target page to the one added, so
		// we don't have to call SetTargetPage if we are filling
		// it right after adding.
		pg->AddPage(wxT("First Page"));

		pg->AppendCategory( wxT("Sample Category") );

		// Add string property
		wxPGId id = pg->Append( wxStringProperty(wxT("Label"),wxT("Name"),wxT("Initial Value")) );
		pg->SetPropertyHelpString( id, wxT("A string property") );

		// Add int property
		pg->Append ( wxIntProperty ( wxT("IntProperty"), wxPG_LABEL, 12345678 ) );

		// Add float property (value type is actually double)
		pg->Append ( wxFloatProperty ( wxT("FloatProperty"), wxPG_LABEL, 12345.678 ) );

		// Add a bool property
		pg->Append ( wxBoolProperty ( wxT("BoolProperty"), wxPG_LABEL, false ) );
		pg->Append ( wxBoolProperty ( wxT("BoolPropertyAsCheckbox"), wxPG_LABEL, true ) );
		pg->SetPropertyAttribute( wxT("BoolPropertyAsCheckbox"), wxPG_BOOL_USE_CHECKBOX, (long)1);

		// Add an enum property
        wxArrayString strings;
        strings.Add(wxT("Herbivore"));
        strings.Add(wxT("Carnivore"));
        strings.Add(wxT("Omnivore"));

        pg->Append( wxEnumProperty(wxT("EnumProperty"), wxPG_LABEL, strings) );

		pg->AppendCategory( wxT("Low Priority Properties") );

		// A string property that can be edited in a separate editor dialog.
		pg->TogglePropertyPriority( pg->Append ( wxLongStringProperty (wxT("LongStringProperty"),
			wxPG_LABEL,
			wxT("This is much longer string than the ")
			wxT("first one. Edit it by clicking the button."))));

		// String editor with dir selector button.
		pg->TogglePropertyPriority( pg->Append ( wxDirProperty( wxT("DirProperty"), wxPG_LABEL, ::wxGetUserHome()) ));

		// A file selector property.
		pg->TogglePropertyPriority( pg->Append ( wxFileProperty( wxT("FileProperty"), wxPG_LABEL, wxEmptyString ) ));

		pg->AddPage(wxT("Second Page"));

		pg->AppendCategory( wxT("Sample Parent Property") );
		wxPGId pid = pg->Append( wxParentProperty(wxT("Car"),wxPG_LABEL) );
		pg->AppendIn( pid, wxStringProperty(wxT("Model"), wxPG_LABEL,wxT("Lamborghini Diablo SV")) );
		pg->AppendIn( pid, wxIntProperty(wxT("Engine Size (cc)"), wxPG_LABEL, 5707) );

		wxPGId speedId = pg->AppendIn( pid, wxParentProperty(wxT("Speeds"),wxPG_LABEL) );
		pg->AppendIn( speedId, wxIntProperty(wxT("Max. Speed (mph)"),wxPG_LABEL,300) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("0-100 mph (sec)"),wxPG_LABEL,3.9) );
		pg->AppendIn( speedId, wxFloatProperty(wxT("1/4 mile (sec)"),wxPG_LABEL,8.6) );
		pg->AppendIn( pid, wxIntProperty(wxT("Price ($)"), wxPG_LABEL, 300000) );

		if ( obj->GetPropertyAsInteger( wxT("include_advanced") ) )
		{
			pg->AppendCategory( wxT("Advanced Properties") );
			// wxArrayStringProperty embeds a wxArrayString.
			pg->Append ( wxArrayStringProperty(wxT("Example of ArrayStringProperty"),
				wxT("ArrayStringProp")));

			// Image file property. Wildcard is auto-generated from available
			// image handlers, so it is not set this time.
			pg->Append ( wxImageFileProperty(wxT("Example of ImageFileProperty"), wxT("ImageFileProp")));

			// Font property has sub-properties. Note that we give window's font as initial value.
			pg->Append ( wxFontProperty(wxT("Font"), wxPG_LABEL, wxFont()) );

			// Colour property with arbitrary colour.
			pg->Append ( wxColourProperty(wxT("My Colour 1"), wxPG_LABEL, wxColour(242,109,0) ) );

			// System colour property.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 1"), wxPG_LABEL, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)) );

			// System colour property with custom colour.
			pg->Append ( wxSystemColourProperty (wxT("My SysColour 2"), wxPG_LABEL, wxColour(0,200,160) ) );

			// Cursor property
			pg->Append ( wxCursorProperty (wxT("My Cursor"), wxPG_LABEL, wxCURSOR_ARROW));
		}

		// For total safety, finally reset the target page.
		pg->SetTargetPage(0);

		return pg;
	}
};

class FlatNotebookComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		wxFlatNotebook* book = new wxFlatNotebook((wxWindow *)parent,-1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")),
			obj->GetPropertyAsInteger(_("style")) | obj->GetPropertyAsInteger(_("window_style")));

		if ( obj->GetPropertyAsInteger( _("has_images") ) != 0 )
		{
			wxFlatNotebookImageList* images = new wxFlatNotebookImageList();
			book->SetImageList( images );
		}

		return book;
	}

	/*TiXmlElement* ExportToXrc(IObject *obj)
	{
	ObjectToXrcFilter xrc(obj, _("wxNotebook"), obj->GetPropertyAsString(_("name")));
	xrc.AddWindowProperties();
	//xrc.AddProperty(_("style"),_("style"),XRC_TYPE_BITLIST);
	return xrc.GetXrcObject();
	}

	TiXmlElement* ImportFromXrc(TiXmlElement *xrcObj)
	{
	XrcToXfbFilter filter(xrcObj, _("wxNotebook"));
	filter.AddWindowProperties();
	//filter.AddProperty(_("style"),_("style"),XRC_TYPE_BITLIST);
	return filter.GetXfbObject();
	}*/

	bool KeepEvtHandler() { return true; }
};

class FlatNotebookPageComponent : public ComponentBase
{
public:
	void OnCreated(IObjectView *objview, wxWindow *wxparent, IObjectView *parent,
		IObjectView *first_child)
	{
		IObject *obj = objview->Object();
		wxFlatNotebook *nb = (wxFlatNotebook *)parent->Window();
		wxWindow *page = first_child->Window();

		wxEvtHandler* visObjEvtHandler = nb->PopEventHandler();

		int selection = nb->GetSelection();

		// Apply image to page
		IObject* parentObj = parent->Object();
		if ( parentObj->GetPropertyAsInteger( _("has_images") ) != 0 )
		{
			if ( !obj->GetPropertyAsString( _("bitmap") ).empty() )
			{
				wxFlatNotebookImageList* imageList = nb->GetImageList();
				if ( parentObj->GetPropertyAsInteger( _("auto_scale_images") ) != 0 )
				{
					wxImage image = obj->GetPropertyAsBitmap( _("bitmap") ).ConvertToImage();
					imageList->Add( image.Scale( 16, 16 ) );
				}
				else
				{
					imageList->Add( obj->GetPropertyAsBitmap( _("bitmap") ) );
				}
				nb->AddPage( page, obj->GetPropertyAsString( _("label") ), false, imageList->GetCount() - 1 );
			}
			else
			{
				nb->AddPage(page,obj->GetPropertyAsString(_("label")));
			}
		}
		else
		{
			nb->AddPage(page,obj->GetPropertyAsString(_("label")));
		}

		if (obj->GetPropertyAsString(_("select"))==wxT("0") && selection >= 0)
			nb->SetSelection(selection);
		else
			nb->SetSelection(nb->GetPageCount()-1);

		nb->PushEventHandler( visObjEvtHandler );
	}

	/*TiXmlElement* ExportToXrc(IObject *obj)
	{
	ObjectToXrcFilter xrc(obj, _("bookpage"));
	xrc.AddProperty(_("label"),_("label"),XRC_TYPE_TEXT);
	xrc.AddProperty(_("selected"),_("selected"),XRC_TYPE_BOOL);
	return xrc.GetXrcObject();
	}

	TiXmlElement* ImportFromXrc(TiXmlElement *xrcObj)
	{
	XrcToXfbFilter filter(xrcObj, _("bookpage"));
	filter.AddWindowProperties();
	filter.AddProperty(_("selected"),_("selected"),XRC_TYPE_BOOL);
	filter.AddProperty(_("label"),_("label"),XRC_TYPE_TEXT);
	return filter.GetXfbObject();
	}*/
};

class awxLedComponent : public ComponentBase
{
public:
	wxObject* Create(IObject *obj, wxObject *parent)
	{
		awxLed* led = new awxLed((wxWindow *)parent, -1,
			obj->GetPropertyAsPoint(_("pos")),
			obj->GetPropertyAsSize(_("size")),
			(awxLedColour)obj->GetPropertyAsInteger(_("color")),
			obj->GetPropertyAsInteger(_("window_style")));
		led->SetState( (awxLedState)obj->GetPropertyAsInteger(_("state")));
		return led;
	}
};

class ScintillaComponent : public ComponentBase
{
public:
	wxObject* Create( IObject* obj, wxObject* parent )
	{
		wxScintilla* m_code = new wxScintilla( 	(wxWindow *)parent, -1,
												obj->GetPropertyAsPoint(_("pos")),
												obj->GetPropertyAsSize(_("size")),
												obj->GetPropertyAsInteger(_("window_style")),
												obj->GetPropertyAsString(_("name"))
											);

		// Line Numbers
		if ( 0 != obj->GetPropertyAsInteger(_("line_numbers") ) )
		{
			m_code->SetMarginType( 0, wxSCI_MARGIN_NUMBER );
			m_code->SetMarginWidth( 0, m_code->TextWidth (wxSCI_STYLE_LINENUMBER, wxT("_99999"))  );
		}
		else
		{
			m_code->SetMarginWidth( 0, 0 );
		}

		// markers
		m_code->MarkerDefine (wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS);
		m_code->MarkerSetBackground (wxSCI_MARKNUM_FOLDER, wxColour (wxT("BLACK")));
		m_code->MarkerSetForeground (wxSCI_MARKNUM_FOLDER, wxColour (wxT("WHITE")));
		m_code->MarkerDefine (wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS);
		m_code->MarkerSetBackground (wxSCI_MARKNUM_FOLDEROPEN, wxColour (wxT("BLACK")));
		m_code->MarkerSetForeground (wxSCI_MARKNUM_FOLDEROPEN, wxColour (wxT("WHITE")));
		m_code->MarkerDefine (wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_EMPTY);
		m_code->MarkerDefine (wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_BOXPLUS);
		m_code->MarkerSetBackground (wxSCI_MARKNUM_FOLDEREND, wxColour (wxT("BLACK")));
		m_code->MarkerSetForeground (wxSCI_MARKNUM_FOLDEREND, wxColour (wxT("WHITE")));
		m_code->MarkerDefine (wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_BOXMINUS);
		m_code->MarkerSetBackground (wxSCI_MARKNUM_FOLDEROPENMID, wxColour (wxT("BLACK")));
		m_code->MarkerSetForeground (wxSCI_MARKNUM_FOLDEROPENMID, wxColour (wxT("WHITE")));
		m_code->MarkerDefine (wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_EMPTY);
		m_code->MarkerDefine (wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_EMPTY);

		// folding
		if ( 0 != obj->GetPropertyAsInteger(_("folding") ) )
		{
			m_code->SetMarginType (1, wxSCI_MARGIN_SYMBOL);
			m_code->SetMarginMask (1, wxSCI_MASK_FOLDERS);
			m_code->SetMarginWidth (1, 16);
			m_code->SetMarginSensitive (1, true);

			m_code->SetProperty( wxT("fold"), wxT("1") );
			m_code->SetFoldFlags( wxSCI_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSCI_FOLDFLAG_LINEAFTER_CONTRACTED );
		}
		else
		{
			m_code->SetMarginWidth( 1, 0 );
		}
		m_code->SetIndentationGuides( ( 0 != obj->GetPropertyAsInteger( _("indentation_guides") ) ) );

		m_code->SetMarginWidth( 2, 0 );

		m_code->SetLexer(wxSCI_LEX_CPP);
		m_code->SetKeyWords(0, wxT("asm auto bool break case catch char class const const_cast \
							   continue default delete do double dynamic_cast else enum explicit \
							   export extern false float for friend goto if inline int long \
							   mutable namespace new operator private protected public register \
							   reinterpret_cast return short signed sizeof static static_cast \
							   struct switch template this throw true try typedef typeid \
							   typename union unsigned using virtual void volatile wchar_t \
							   while"));

		wxFont font(10, wxMODERN, wxNORMAL, wxNORMAL);
		if ( !obj->GetPropertyAsString(_("font")).empty() )
		{
			font = obj->GetPropertyAsFont(_("font"));
		}

		m_code->StyleSetFont(wxSCI_STYLE_DEFAULT, font );

		m_code->StyleClearAll();
		m_code->StyleSetBold(wxSCI_C_WORD, true);
		m_code->StyleSetForeground(wxSCI_C_WORD, *wxBLUE);
		m_code->StyleSetForeground(wxSCI_C_STRING, *wxRED);
		m_code->StyleSetForeground(wxSCI_C_STRINGEOL, *wxRED);
		m_code->StyleSetForeground(wxSCI_C_PREPROCESSOR, wxColour(49, 106, 197));
		m_code->StyleSetForeground(wxSCI_C_COMMENT, wxColour(0, 128, 0));
		m_code->StyleSetForeground(wxSCI_C_COMMENTLINE, wxColour(0, 128, 0));
		m_code->StyleSetForeground(wxSCI_C_COMMENTDOC, wxColour(0, 128, 0));
		m_code->StyleSetForeground(wxSCI_C_COMMENTLINEDOC, wxColour(0, 128, 0));
		m_code->StyleSetForeground(wxSCI_C_NUMBER, *wxBLUE );
		m_code->SetUseTabs( ( 0 != obj->GetPropertyAsInteger( _("use_tabs") ) ) );
		m_code->SetTabWidth( obj->GetPropertyAsInteger( _("tab_width") ) );
		m_code->SetTabIndents( ( 0 != obj->GetPropertyAsInteger( _("tab_indents") ) ) );
		m_code->SetBackSpaceUnIndents( ( 0 != obj->GetPropertyAsInteger( _("backspace_unindents") ) ) );
		m_code->SetIndent( obj->GetPropertyAsInteger( _("tab_width") ) );
		m_code->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
		m_code->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
		m_code->SetViewEOL( ( 0 != obj->GetPropertyAsInteger( _("view_eol") ) ) );
		m_code->SetViewWhiteSpace( ( 0 != obj->GetPropertyAsInteger( _("view_whitespace") ) ) );

		m_code->SetCaretWidth(2);

		m_code->SetText( 	wxT( "/** Sample Class to Display wxScintilla */\n" )
							wxT( "class ScintillaSampleCode\n" )
							wxT( "{\n" )
							wxT( "private:\n" )
							wxT( "\tint m_privateMember;\n\n" )
							wxT( "public:\n\n" )
							wxT( "\t// Sample Member Function\n" )
							wxT( "\tint SampleFunction( int sample = 0 )\n" )
							wxT( "\t{\n" )
							wxT( "\t\treturn sample;\n" )
							wxT( "\t}\n" )
							wxT( "};\n" )
						);

		return m_code;
	}
};
///////////////////////////////////////////////////////////////////////////////

BEGIN_LIBRARY()

// wxPlotWindow
WINDOW_COMPONENT("wxPlotCtrl",PlotCtrlComponent)
MACRO(wxPLOTCTRL_MOUSE_NOTHING)
MACRO(wxPLOTCTRL_MOUSE_ZOOM)
MACRO(wxPLOTCTRL_MOUSE_SELECT)
MACRO(wxPLOTCTRL_MOUSE_DESELECT)
MACRO(wxPLOTCTRL_MOUSE_PAN)
MACRO(wxPLOTCTRL_MARKER_NONE)
MACRO(wxPLOTCTRL_MARKER_RECT)
MACRO(wxPLOTCTRL_MARKER_VERT)
MACRO(wxPLOTCTRL_MARKER_HORIZ)

// wxPropertyGrid
WINDOW_COMPONENT("wxPropertyGrid", PropertyGridComponent)
MACRO(wxPG_AUTO_SORT)
MACRO(wxPG_HIDE_CATEGORIES)
MACRO(wxPG_ALPHABETIC_MODE)
MACRO(wxPG_BOLD_MODIFIED)
MACRO(wxPG_SPLITTER_AUTO_CENTER)
MACRO(wxPG_TOOLTIPS)
MACRO(wxPG_HIDE_MARGIN)
MACRO(wxPG_STATIC_SPLITTER)
MACRO(wxPG_STATIC_LAYOUT)
MACRO(wxPG_LIMITED_EDITING)
MACRO(wxPG_EX_INIT_NOCAT)
MACRO(wxPG_DEFAULT_STYLE)
MACRO(wxTAB_TRAVERSAL)

// wxPropertyGridManager
WINDOW_COMPONENT("wxPropertyGridManager", PropertyGridManagerComponent)
MACRO(wxPG_EX_NO_FLAT_TOOLBAR)
MACRO(wxPG_EX_MODE_BUTTONS)
MACRO(wxPG_COMPACTOR)
MACRO(wxPGMAN_DEFAULT_STYLE)
MACRO(wxPG_DESCRIPTION)
MACRO(wxPG_TOOLBAR)

// wxFlatNotebook
WINDOW_COMPONENT("wxFlatNotebook",FlatNotebookComponent)
ABSTRACT_COMPONENT("flatnotebookpage",FlatNotebookPageComponent)
MACRO(wxFNB_VC71)
MACRO(wxFNB_FANCY_TABS)
MACRO(wxFNB_TABS_BORDER_SIMPLE)
MACRO(wxFNB_NO_X_BUTTON)
MACRO(wxFNB_NO_NAV_BUTTONS)
MACRO(wxFNB_MOUSE_MIDDLE_CLOSES_TABS)
MACRO(wxFNB_BOTTOM)
MACRO(wxFNB_NODRAG)
MACRO(wxFNB_ALLOW_FOREIGN_DND)
MACRO(wxFNB_X_ON_TAB)
MACRO(wxFNB_DCLICK_CLOSES_TABS)

// awxLed
WINDOW_COMPONENT("awxLed", awxLedComponent )
MACRO(awxLED_OFF)
MACRO(awxLED_ON)
MACRO(awxLED_BLINK)
MACRO(awxLED_LUCID)
MACRO(awxLED_RED)
MACRO(awxLED_GREEN)
MACRO(awxLED_YELLOW)

// wxScintilla
WINDOW_COMPONENT("wxScintilla", ScintillaComponent )

END_LIBRARY()

