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

#include "objinspect2.h"
#include "model/objectbase.h"
#include "utils/debug.h"
#include "utils/typeconv.h"
#include "wx/tokenzr.h"
#include "rad/global.h"
#include "rad/bitmaps.h"
#include <sstream>
#include <memory>

// -----------------------------------------------------------------------
// wxSizeProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP(wxSize,wxSizeProperty,wxSize(0,0))

class wxSizePropertyClass : public wxPGPropertyWithChildren
{
	WX_PG_DECLARE_PROPERTY_CLASS()
public:

	wxSizePropertyClass ( const wxString& label, const wxString& name,
		const wxSize& value );
	virtual ~wxSizePropertyClass ();

	WX_PG_DECLARE_PARENTAL_TYPE_METHODS()
		WX_PG_DECLARE_PARENTAL_METHODS()

protected:
	wxSize                  m_value;
};

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxSizeProperty,wxBaseParentProperty,wxSize,const wxSize&,TextCtrl)

wxSizePropertyClass::wxSizePropertyClass ( const wxString& label, const wxString& name,
										  const wxSize& value) : wxPGPropertyWithChildren(label,name)
{
	wxPG_INIT_REQUIRED_TYPE(wxSize)
		DoSetValue((void*)&value);
	AddChild( wxIntProperty(wxT("Width"),wxPG_LABEL,value.x) );
	AddChild( wxIntProperty(wxT("Height"),wxPG_LABEL,value.y) );
}

wxSizePropertyClass::~wxSizePropertyClass () { }

void wxSizePropertyClass::DoSetValue ( wxPGVariant value )
{
	wxSize* pObj = (wxSize*)wxPGVariantToVoidPtr(value);
	m_value = *pObj;
	RefreshChildren();
}

wxPGVariant wxSizePropertyClass::DoGetValue () const
{
	return wxPGVariant((void*)&m_value);
}

void wxSizePropertyClass::RefreshChildren()
{
	if ( !GetCount() ) return;
	Item(0)->DoSetValue( (long)m_value.x );
	Item(1)->DoSetValue( (long)m_value.y );
}

void wxSizePropertyClass::ChildChanged ( wxPGProperty* p )
{
	switch ( p->GetIndexInParent() )
	{
	case 0: m_value.x = p->DoGetValue().GetLong(); break;
	case 1: m_value.y = p->DoGetValue().GetLong(); break;
	}
}

// -----------------------------------------------------------------------
// wxPointProperty
// -----------------------------------------------------------------------

WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP(wxPoint,wxPointProperty,wxPoint(0,0))

class wxPointPropertyClass : public wxPGPropertyWithChildren
{
	WX_PG_DECLARE_PROPERTY_CLASS()
public:

	wxPointPropertyClass( const wxString& label, const wxString& name,
		const wxPoint& value );
	virtual ~wxPointPropertyClass ();

	WX_PG_DECLARE_PARENTAL_TYPE_METHODS()
		WX_PG_DECLARE_PARENTAL_METHODS()

protected:
	wxPoint                  m_value;
};

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxPointProperty,wxBaseParentProperty,wxPoint,const wxPoint&,TextCtrl)

wxPointPropertyClass::wxPointPropertyClass ( const wxString& label, const wxString& name,
											const wxPoint& value) : wxPGPropertyWithChildren(label,name)
{
	wxPG_INIT_REQUIRED_TYPE(wxPoint)
		DoSetValue((void*)&value);
	AddChild( wxIntProperty(wxT("X"),wxPG_LABEL,value.x) );
	AddChild( wxIntProperty(wxT("Y"),wxPG_LABEL,value.y) );
}

wxPointPropertyClass::~wxPointPropertyClass () { }

void wxPointPropertyClass::DoSetValue ( wxPGVariant value )
{
	wxPoint* pObj = (wxPoint*)wxPGVariantToVoidPtr(value);
	m_value = *pObj;
	RefreshChildren();
}

wxPGVariant wxPointPropertyClass::DoGetValue () const
{
	return wxPGVariant((void*)&m_value);
}

void wxPointPropertyClass::RefreshChildren()
{
	if ( !GetCount() ) return;
	Item(0)->DoSetValue( m_value.x );
	Item(1)->DoSetValue( m_value.y );
}

void wxPointPropertyClass::ChildChanged ( wxPGProperty* p )
{
	switch ( p->GetIndexInParent() )
	{
		case 0:
			m_value.x = p->DoGetValue().GetLong();
			break;
		case 1:
			m_value.y = p->DoGetValue().GetLong();
			break;
	}
}

// -----------------------------------------------------------------------
// wxBitmapWithResourcePropertyClass
// -----------------------------------------------------------------------

class wxBitmapWithResourcePropertyClass : public wxPGPropertyWithChildren
{
	WX_PG_DECLARE_PROPERTY_CLASS()
public:

	wxBitmapWithResourcePropertyClass( const wxString& label, const wxString& name, const wxString& value );
	virtual ~wxBitmapWithResourcePropertyClass(){};

	WX_PG_DECLARE_PARENTAL_TYPE_METHODS()
	WX_PG_DECLARE_PARENTAL_METHODS()

protected:
	wxString m_image;
	wxString m_source;
	wxArrayString m_strings;
private:
	enum
	{
		SOURCE_FILE = 0,
		SOURCE_RESOURCE,
		SOURCE_ICON_RESOURCE
	};

	enum
	{
		ITEM_FILE_OR_RESOURCE = 0,
		ITEM_SOURCE
	};
};

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxBitmapWithResourceProperty,wxBaseParentProperty,wxString,const wxString&,TextCtrl)

wxBitmapWithResourcePropertyClass::wxBitmapWithResourcePropertyClass ( const wxString& label, const wxString& name, const wxString& value)
	: wxPGPropertyWithChildren(label,name)
{
	// Parse default value, ( sets m_image and m_source based on 'value' )
	DoSetValue( (void*)&value );

	// Add the appropriate child
	if ( m_source == wxT("Load From File") )
	{
		AddChild( wxImageFileProperty( wxT("file_path"), wxPG_LABEL, m_image ) );
	}
	else
	{
		AddChild( wxStringProperty( wxT("resource_name"), wxPG_LABEL, m_image ) );
	}

	// Add the options
    m_strings.Add(wxT("Load From File"));
    m_strings.Add(wxT("Load From Resource"));
    m_strings.Add(wxT("Load From Icon Resource"));

    AddChild( wxEnumProperty(wxT("source"), wxPG_LABEL, m_strings, m_strings.Index( m_source ) ) );

	RefreshChildren();
}


void wxBitmapWithResourcePropertyClass::DoSetValue ( wxPGVariant value )
{
	wxString* pObj = (wxString*)wxPGVariantToVoidPtr( value );
	wxString newValue = *pObj;

	// Split on the semi-colon
	size_t splitIndex = newValue.find_first_of( wxT(";") );
	if ( splitIndex != newValue.npos )
	{
		m_image = newValue.substr( 0, splitIndex );
		m_source = newValue.substr( splitIndex + 2 );
	}
	else
	{
		m_image = wxEmptyString;
		m_source = wxT("Load From File");
	}

	RefreshChildren();
}

wxPGVariant wxBitmapWithResourcePropertyClass::DoGetValue() const
{
	wxString value;
	value << m_image << wxT("; ") << m_source;
	return wxPGVariant( (void*)&value );
}

void wxBitmapWithResourcePropertyClass::RefreshChildren()
{
	if ( !GetCount() )
	{
		return;
	}

	Item( ITEM_FILE_OR_RESOURCE )->DoSetValue( m_image );
	Item( ITEM_SOURCE )->DoSetValue( m_strings.Index( m_source ) );
}

void wxBitmapWithResourcePropertyClass::ChildChanged( wxPGProperty* p )
{

}

// -----------------------------------------------------------------------
// ObjectInspector
// -----------------------------------------------------------------------

DEFINE_EVENT_TYPE( wxEVT_NEW_BITMAP_PROPERTY )

BEGIN_EVENT_TABLE(ObjectInspector, wxPanel)
	EVT_PG_CHANGED(-1, ObjectInspector::OnPropertyGridChange)
	EVT_COMMAND( -1, wxEVT_NEW_BITMAP_PROPERTY, ObjectInspector::OnNewBitmapProperty )
END_EVENT_TABLE()

ObjectInspector::ObjectInspector(wxWindow *parent, int id)
: wxPanel(parent,id)
{
	m_currentSel = shared_ptr<ObjectBase>();
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	m_pg = new wxPropertyGridManager(this, -1, wxDefaultPosition, wxDefaultSize,
		// These and other similar styles are automatically
		// passed to the embedded wxPropertyGrid.
		wxPG_BOLD_MODIFIED|wxPG_SPLITTER_AUTO_CENTER|
		// Include toolbar.
		wxPG_TOOLBAR |
		// Include description box.
		wxPG_DESCRIPTION |
		// Plus defaults.
		wxPGMAN_DEFAULT_STYLE);
	m_pg->SetDescBoxHeight( 50 );
	//m_pg->SetExtraStyle( wxPG_EX_MODE_BUTTONS );
	topSizer->Add(m_pg, 1, wxALL | wxEXPAND, 0);
	SetSizer(topSizer);
}

void ObjectInspector::Create(bool force)
{
	shared_ptr<ObjectBase> sel_obj = GetData()->GetSelectedObject();
	if (sel_obj && (sel_obj != m_currentSel || force))
	{
		Freeze();

		m_currentSel = sel_obj;

		wxSizer* sizer = GetSizer();
		if ( NULL == sizer )
		{
			return;
		}
		int pageNumber = m_pg->GetSelectedPage();
		wxString pageName;
		if ( pageNumber != wxNOT_FOUND )
		{
			pageName = m_pg->GetPageName( pageNumber );
		}

		sizer->Detach( m_pg );
		delete m_pg;
		m_pg = new wxPropertyGridManager(this, -1, wxDefaultPosition, wxDefaultSize,
											// These and other similar styles are automatically
											// passed to the embedded wxPropertyGrid.
											wxPG_BOLD_MODIFIED|wxPG_SPLITTER_AUTO_CENTER|
											// Include toolbar.
											wxPG_TOOLBAR |
											// Include description box.
											wxPG_DESCRIPTION |
											// Plus defaults.
											wxPGMAN_DEFAULT_STYLE);
		m_pg->SetDescBoxHeight( 50 );
		//m_pg->SetExtraStyle( wxPG_EX_MODE_BUTTONS );

		m_propmap.clear();

		shared_ptr<ObjectInfo> obj_desc = sel_obj->GetObjectInfo();
		if (obj_desc)
		{

			map<wxString,shared_ptr< Property > > map, dummy;

			// We create the categories with the properties of the object organized by "classes"
			CreateCategory( _WXSTR(obj_desc->GetClassName()), sel_obj,obj_desc,map);

			for (unsigned int i=0; i<obj_desc->GetBaseClassCount() ; i++)
			{
				shared_ptr<ObjectInfo> info_base = obj_desc->GetBaseClass(i);
				CreateCategory( _WXSTR(info_base->GetClassName()), sel_obj,info_base,map);
			}

			shared_ptr<ObjectBase> parent = sel_obj->GetParent();
			if (parent && parent->GetObjectInfo()->GetObjectType()->IsItem())
			{
				CreateCategory(_WXSTR(parent->GetObjectInfo()->GetClassName()), parent, parent->GetObjectInfo(),dummy);
			}

			// Select previously selected page, or first page
			int pageIndex = m_pg->GetPageByName( pageName );
			if ( wxNOT_FOUND != pageIndex )
			{
				m_pg->SelectPage( pageIndex );
			}
			else
			{
				m_pg->SelectPage( 0 );
			}
		}

		sizer->Add(m_pg, 1, wxALL | wxEXPAND, 0);
		sizer->Layout();

		Thaw();
	}
}

int ObjectInspector::StringToBits(const wxString& strVal, wxPGChoices& constants)
{
	wxStringTokenizer strTok(strVal, wxT(" |"));
	int val = 0;
	while (strTok.HasMoreTokens())
	{
		wxString token = strTok.GetNextToken();
		unsigned int i = 0;
		bool done = false;
		while (i < constants.GetCount() && !done)
		{
			if (constants.GetLabel(i) == token)
			{
				val |= constants.GetValue(i);
				done = true;
			}
			i++;
		}
	}
	return val;
}

wxPGProperty* ObjectInspector::GetProperty(shared_ptr<Property> prop)
{
	wxPGProperty *result;
	PropertyType type = prop->GetType();
	wxString name = _WXSTR(prop->GetName());
	wxVariant vTrue = wxVariant( true, wxT("true") );

	if (type == PT_TEXT || type == PT_MACRO)
	{
		result = wxStringProperty(name, wxPG_LABEL, prop->GetValueAsString());
		result->SetAttribute(wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, vTrue);
	}
  	else if (type == PT_WXSTRING || type == PT_WXSTRING_I18N)
	{
		result = wxStringProperty(name, wxPG_LABEL, prop->GetValueAsText());
		result->SetAttribute(wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, vTrue);
	}
	else if (type == PT_BOOL)
	{
		result = wxBoolProperty(name, wxPG_LABEL, _WXSTR(prop->GetValue()) == wxT("1"));
	}
	else if (type == PT_BITLIST)
	{
		shared_ptr<PropertyInfo> prop_desc = prop->GetPropertyInfo();
		shared_ptr<OptionList> opt_list = prop_desc->GetOptionList();

		assert(opt_list && opt_list->GetOptionCount() > 0);

		wxPGChoices constants;
		const map< wxString, wxString > options = opt_list->GetOptions();
		map< wxString, wxString >::const_iterator it;
		unsigned int i = 0;
		for( it = options.begin(); it != options.end(); ++it )
		{
			constants.Add( _WXSTR(it->first), 1 << i++ );
		}

		int val = StringToBits(prop->GetValueAsString(), constants);
		result = wxFlagsProperty(name, wxPG_LABEL, constants, val);

		// Workaround to set the help strings for individual members of a wxFlagsProperty
		wxFlagsPropertyClass* flagsProp = dynamic_cast<wxFlagsPropertyClass*>(result);
		if ( NULL != flagsProp )
		{
			for ( size_t i = 0; i < flagsProp->GetCount(); i++ )
			{
				wxPGProperty* prop = flagsProp->Item( i );
				map< wxString, wxString >::const_iterator option = options.find( prop->GetLabel().c_str() );
				if ( option != options.end() )
				{
					m_pg->SetPropertyHelpString( prop, _WXSTR( option->second )  );
				}
			}
		}
	}
	else if (type == PT_INTLIST)
	{
		result = wxStringProperty(name, wxPG_LABEL, _WXSTR( IntList( prop->GetValueAsString().c_str() ).ToString()));
	}
	else if (type == PT_OPTION)
	{
		shared_ptr<PropertyInfo> prop_desc = prop->GetPropertyInfo();
		shared_ptr<OptionList> opt_list = prop_desc->GetOptionList();

		assert(opt_list && opt_list->GetOptionCount() > 0);

		wxPGChoices constants;
		const map< wxString, wxString > options = opt_list->GetOptions();
		map< wxString, wxString >::const_iterator it;
		unsigned int i = 0;
		for( it = options.begin(); it != options.end(); ++it )
		{
			constants.Add( _WXSTR(it->first), i++ );
		}

		result = wxEnumProperty(name, wxPG_LABEL, constants);
		result->SetValueFromString(prop->GetValueAsString(), 0);
	}
	else if (type == PT_WXPOINT)
	{
		result = wxPointProperty(name, wxPG_LABEL, prop->GetValueAsPoint());
	}
	else if (type == PT_WXSIZE)
	{
		result = wxSizeProperty(name, wxPG_LABEL, prop->GetValueAsSize());
	}
	else if (type == PT_WXFONT)
	{
		result = wxFontProperty(name, wxPG_LABEL, prop->GetValueAsFont());
	}
	else if (type == PT_WXCOLOUR)
	{
		wxString value = prop->GetValueAsString();
		if ( value.empty() )
		{
			wxColourPropertyValue def;
			def.m_type = wxSYS_COLOUR_3DFACE;
			result = wxSystemColourProperty( name, wxPG_LABEL, def );
		}
		else
		{
			if ( value.find_first_of( wxT("wx") ) == 0 )
			{
				// System Colour
				wxColourPropertyValue def;
				def.m_type = TypeConv::StringToSystemColour( value );
				result = wxSystemColourProperty( name, wxPG_LABEL, def );
			}
			else
			{
				result = wxSystemColourProperty( name, wxPG_LABEL, prop->GetValueAsColour() );
			}
		}
	}
	else if (type == PT_PATH)
	{
		result = wxDirProperty(name, wxPG_LABEL, prop->GetValueAsString());
	}
	else if (type == PT_BITMAP)
	{
		result = wxBitmapWithResourceProperty( name, wxPG_LABEL, prop->GetValueAsString() );
	}
	else if (type == PT_STRINGLIST)
	{
		result = wxArrayStringProperty(name, wxPG_LABEL,prop->GetValueAsArrayString());
	}
	else if (type == PT_FLOAT)
	{
		result = wxFloatProperty(name, wxPG_LABEL,prop->GetValueAsFloat());
	}
	else // propiedad desconocida
	{
		result = wxStringProperty(name, wxPG_LABEL, prop->GetValueAsString());
		result->SetAttribute(wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING, vTrue);
		wxLogError(wxT("Property type Unknown"));
	}

	return result;
}

void ObjectInspector::CreateCategory(const wxString& name, shared_ptr<ObjectBase> obj, shared_ptr<ObjectInfo> obj_info, map< wxString, shared_ptr< Property > >& properties )
{
	int pageIndex = m_pg->GetPageByName( name.c_str() );
	if ( wxNOT_FOUND == pageIndex )
	{
		m_pg->AddPage( name, obj_info->GetSmallIconFile() );
	}
	m_pg->SelectPage( name.c_str() );

	m_pg->AppendCategory(name);
	unsigned int prop_count = obj_info->GetPropertyCount();

	if (prop_count > 0)
	{
		//wxFlexGridSizer* sizer = new wxFlexGridSizer(prop_count, 2, 0, 0);
		for (unsigned int i=0; i<prop_count ; i++)
		{
			shared_ptr<PropertyInfo> prop_desc = obj_info->GetPropertyInfo(i);
			shared_ptr<Property>     prop      = obj->GetProperty(prop_desc->GetName());
			//wxWindow *prop_editor = NULL;

			assert(prop_desc && prop);

			wxString prop_name( _WXSTR(prop_desc->GetName()) );
			Debug::Print( wxT("[ObjectInspector::CreatePropertyPanel] Creating Property Editor") );

			// we do not want to duplicate inherited properties
			if (properties.find(prop_desc->GetName()) == properties.end())
			{
				wxPGId id = m_pg->Append(GetProperty(prop));
				m_pg->SetPropertyHelpString( id, _WXSTR( prop_desc->GetDescription() ) );

				properties.insert( map< wxString, shared_ptr< Property > >::value_type( prop_desc->GetName(), prop ) );
				m_propmap.insert(ObjInspectorMap::value_type(id.GetPropertyPtr(), prop));
			}
		}

	}

}

void ObjectInspector::OnPropertyGridChange( wxPropertyGridEvent& event )
{
	ObjInspectorMap::iterator it = m_propmap.find( event.GetPropertyPtr() );

	if ( it != m_propmap.end() )
	{
		shared_ptr<Property> prop = it->second;
		switch ( prop->GetType() )
		{
			case PT_TEXT:
			case PT_MACRO:
			case PT_OPTION:
			case PT_FLOAT:
			{
				GetData()->ModifyProperty( prop, event.GetPropertyValueAsString() );
				break;
			}
			case PT_WXSTRING:
			{
				// las cadenas de texto del inspector son formateadas
				wxString value = _WXSTR(TypeConv::TextToString( event.GetPropertyValueAsString().c_str() ));
				GetData()->ModifyProperty( prop, value );
				break;
			}
			case PT_WXSTRING_I18N:
			{
				// las cadenas de texto del inspector son formateadas
				wxString value = _WXSTR(TypeConv::TextToString( event.GetPropertyValueAsString().c_str() ));
				GetData()->ModifyProperty( prop, value );
				break;
			}
			case PT_BOOL:
			{
				GetData()->ModifyProperty( prop, event.GetPropertyValueAsBool() ? wxT("1") : wxT("0") );
				break;
			}
			case PT_BITLIST:
			{
				wxString aux = event.GetPropertyValueAsString();
				aux.Replace( wxT(" "), wxT("") );
				aux.Replace( wxT(","), wxT("|") );
				GetData()->ModifyProperty( prop, aux );
				break;
			}
			case PT_WXPOINT: case PT_WXSIZE:
			{
				wxString aux = event.GetPropertyValueAsString();
				aux.Replace( wxT(" "), wxT("") );
				aux.Replace( wxT(";"), wxT(",") );
				GetData()->ModifyProperty( prop, aux );
				break;
			}
			case PT_WXFONT:
			{
				wxFont* font = wxPGVariantToWxObjectPtr( event.GetPropertyPtr()->DoGetValue(), wxFont );
				GetData()->ModifyProperty( prop, TypeConv::FontToString( *font ) );
				break;
			}
			case PT_WXCOLOUR:
			{
				wxColourPropertyValue* colour = wxDynamicCast( event.GetPropertyValueAsWxObjectPtr(), wxColourPropertyValue );
				if ( colour->m_type == wxPG_COLOUR_CUSTOM )
				{
					GetData()->ModifyProperty( prop, TypeConv::ColourToString( colour->m_colour ) );
				}
				else
				{
					GetData()->ModifyProperty( prop, TypeConv::SystemColourToString( colour->m_type ) );
				}
				break;
			}
			case PT_STRINGLIST:
			{
				const wxArrayString &arraystr = event.GetPropertyValueAsArrayString();
				GetData()->ModifyProperty(prop, TypeConv::ArrayStringToString(arraystr));
				break;
			}
			case PT_BITMAP:
			{
				// TODO: Usar ruta relativa al directorio de salida en el caso
				//       de que la imagen se encuentre en un subdirectorio de este.

				// Get property value
				wxString path = event.GetPropertyValueAsString();
				size_t semicolon_index = path.find_first_of( wxT(";") );
				if ( semicolon_index != path.npos )
				{
					path = TypeConv::MakeRelativePath( path.substr( 0, semicolon_index ), GlobalData()->GetProjectPath() ) + path.substr( semicolon_index  );
				}

				// Create event to spawn update of the bitmap property
				wxCommandEvent bitmapEvent( wxEVT_NEW_BITMAP_PROPERTY, -1 );
				bitmapEvent.SetEventObject( this );

				// Fill event data with property grid pointer, Property pointer, and property value
				NewBitmapEventDataHolder* dataHolder = new NewBitmapEventDataHolder();
				dataHolder->m_grid = event.GetPropertyPtr()->GetGrid();
				dataHolder->m_prop = prop;
				dataHolder->m_string = path;
				bitmapEvent.SetClientData( dataHolder );

				// Post Event
				GetEventHandler()->AddPendingEvent( bitmapEvent );

				// Respond to property modification
				GetData()->ModifyProperty( prop, path );
				break;
			}


			default:
				GetData()->ModifyProperty( prop, event.GetPropertyValueAsString() );
		}
	}
}

void ObjectInspector::OnNewBitmapProperty( wxCommandEvent& event )
{
	// Update property grid - change bitmap property
	auto_ptr< NewBitmapEventDataHolder > data ( (NewBitmapEventDataHolder*)event.GetClientData() );
	data->m_grid->Freeze();
	wxPGId newId = data->m_grid->ReplaceProperty( wxT("bitmap"), wxBitmapWithResourceProperty( wxT("bitmap"), wxPG_LABEL, data->m_string ) );
	m_propmap[ newId.GetPropertyPtr() ] = data->m_prop;
	data->m_grid->Expand( newId );
	data->m_grid->Thaw();
}
///////////////////////////////////////////////////////////////////////////////

void ObjectInspector::ProjectLoaded()
{
}

void ObjectInspector::ProjectSaved()
{
}

void ObjectInspector::ObjectSelected(shared_ptr<ObjectBase> obj)
{
	Create();
}

void ObjectInspector::ObjectCreated(shared_ptr<ObjectBase> obj)
{
}

void ObjectInspector::ObjectRemoved(shared_ptr<ObjectBase> obj)
{
}

void ObjectInspector::ProjectRefresh()
{
	Create(true);
}

void ObjectInspector::PropertyModified(shared_ptr<Property> prop)
{
	wxPGId pgid = m_pg->GetPropertyByLabel(_WXSTR(prop->GetName()));
	if (!pgid.IsOk()) return; // Puede que no se esté mostrando ahora esa página
	wxPGProperty *pgProp = pgid.GetPropertyPtr();

	switch (prop->GetType())
	{
	case PT_TEXT: case PT_MACRO: case PT_OPTION: case PT_FLOAT:
		pgProp->SetValueFromString(prop->GetValueAsString(), 0);
		break;
	case PT_WXSTRING:
		pgProp->SetValueFromString(prop->GetValueAsText(), 0);
		break;
	case PT_WXSTRING_I18N:
		pgProp->SetValueFromString(prop->GetValueAsText(), 0);
		break;
	case PT_BOOL:
		pgProp->SetValueFromInt(prop->GetValueAsString() == wxT("0") ? 0 : 1, 0);
		break;
	case PT_BITLIST:
		{
			wxString aux = prop->GetValueAsString();
			aux.Replace(wxT("|"), wxT(", "));
			if (aux == wxT("0")) aux = wxT("");
			pgProp->SetValueFromString(aux, 0);
		}
		break;
	case PT_WXPOINT:
		m_pg->SetPropertyValue(pgid, prop->GetValueAsPoint());
		break;
	case PT_WXSIZE:
		m_pg->SetPropertyValue(pgid, prop->GetValueAsSize());
		break;
	case PT_WXFONT:
		{
			wxFont val = prop->GetValueAsFont();
			pgProp->DoSetValue((void*)&val);
		}
		break;
	case PT_WXCOLOUR:
//		if (prop->GetValueAsString() == wxT(""))
//			pgProp->SetValueFromString(wxT("Default"), 0);
//		else{
//			wxColour val = prop->GetValueAsColour();
//			pgProp->DoSetValue((void*)&val);
//		}
		break;
	case PT_STRINGLIST:
		{
			wxArrayString val = prop->GetValueAsArrayString();
			pgProp->DoSetValue((void*)&val);
		}
		break;
	default:
		pgProp->SetValueFromString(prop->GetValueAsString(), 0);
	}
	m_pg->Refresh();
}
