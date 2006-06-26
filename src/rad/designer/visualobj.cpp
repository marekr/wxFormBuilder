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

#include "visualobj.h"
#include "visualeditor.h"

#include "utils/typeconv.h"
#include "utils/debug.h"
#include "rad/genericpanel.h"

using namespace TypeConv;

PVisualObject VisualObject::CreateVisualObject
(shared_ptr<ObjectBase> obj, wxWindow *wx_parent)
{
	PVisualObject vobj;

	shared_ptr<ObjectInfo> obj_info = obj->GetObjectInfo();
	string type = obj->GetObjectTypeName();

	// TO-DO: arreglar esto!
	//
	// El tipo de objeto se deberá indicar en el plugin
	// quizá sea conveniente dividir la macro COMPONENT("name",component)
	// estas tres:
	//   WINDOW_COMPONENT
	//   SIZER_COMPONENT
	//   ABSTRACT_COMPONENT
	// y que se pueda consultar el tipo.

	IComponent *comp = obj->GetObjectInfo()->GetComponent();

	if (comp)
	{
		int compType = comp->GetComponentType();

		switch (compType)
		{
		case COMPONENT_TYPE_WINDOW:
			vobj = PVisualObject(new VisualWindow(obj,wx_parent));
			break;

		case COMPONENT_TYPE_SIZER:
			vobj = PVisualObject(new VisualSizer(obj,wx_parent));
			break;

		default: // items y forms
			vobj = PVisualObject(new VisualObject(obj));
			break;
		}
	}
	else
	{
		vobj = PVisualObject(new VisualObject(obj));
		wxLogError( wxT("Component for %s not found!"), _WXSTR(obj->GetClassName()).c_str() );
	}

	return vobj;
}

VisualObject::VisualObject(shared_ptr<ObjectBase> obj) : m_object(obj) {}

VisualObject::~VisualObject() {}

///////////////////////////////////////////////////////////////////////////////
VisualSizer::VisualSizer(shared_ptr<ObjectBase> obj,wxWindow *parent)
: VisualObject(obj)
{
	IComponent *comp = obj->GetObjectInfo()->GetComponent();
	if (comp)
	{

		/*  wxObject* yo = parent;
		IObject *objer =  obj.get();
		wxStaticBox* box = new wxStaticBox( (wxWindow*)yo, -1,
		objer->GetPropertyAsString(wxT("label")));

		wxStaticBoxSizer* sizer = new wxStaticBoxSizer(box,
		objer->GetPropertyAsInteger(wxT("orient")));

		SetSizer( sizer );*/

		SetSizer((wxSizer *)(comp->Create(obj.get(),parent)));
	}
	else
	{
		// para que no pete ponemos un sizer por defecto, aunque deberá mostrar algun
		// mensaje de advertencia
		SetSizer(new wxBoxSizer(wxVERTICAL));
	}

	shared_ptr<Property> pminsize  = obj->GetProperty("minimum_size");
	if (pminsize)
	{
		wxSize minsize = StringToSize(_WXSTR(pminsize->GetValue()));
		m_sizer->SetMinSize( minsize );
		m_sizer->Layout();
	}
}



///////////////////////////////////////////////////////////////////////////////

VisualWindow::VisualWindow(shared_ptr<ObjectBase> obj, wxWindow *parent)
: VisualObject(obj)
{
	IComponent *comp = obj->GetObjectInfo()->GetComponent();

	if (comp)
		SetWindow((wxWindow *)(comp->Create(obj.get(),parent)));

	else
		SetWindow(new GenericWindow(parent));

	SetupWindow();
}

//void VisualWindow::AddToSizer(wxSizer *sizer, shared_ptr<ObjectBase> obj)
//{
//  wxWindow *window_obj = GetWindow();
//  sizer->Add(window_obj,
//    obj->GetProperty("proportion")->GetValueAsInteger(),
//    obj->GetProperty("flag")->GetValueAsInteger(),
//    obj->GetProperty("border")->GetValueAsInteger());
//
//  Debug::Print("Add window to sizer");
//}


void VisualWindow::SetupWindow()
{
	shared_ptr<ObjectBase> obj = GetObject();

	// All of the properties of the wxWindow object are applied in this function
	shared_ptr<Property> ppos   = obj->GetProperty("pos");
	shared_ptr<Property> psize  = obj->GetProperty("size");
	shared_ptr<Property> pminsize  = obj->GetProperty("minimum_size");
	shared_ptr<Property> pfont  = obj->GetProperty("font");
	shared_ptr<Property> pfg_colour  = obj->GetProperty("fg");
	shared_ptr<Property> pbg_colour  = obj->GetProperty("bg");
	shared_ptr<Property> penabled = obj->GetProperty("enabled");
	shared_ptr<Property> phidden  = obj->GetProperty("hidden");
	shared_ptr<Property> ptooltip = obj->GetProperty("tooltip");
	shared_ptr<Property> pextra_style = obj->GetProperty("window_extra_style");

	wxPoint pos;
	wxSize size;
	wxSize minsize;
	wxFont font;
	wxColour fg_colour;
	wxColour bg_colour;
	long extra_style;

	if (ppos)
		pos = StringToPoint(_WXSTR(ppos->GetValue()));

	if (psize)
		size = StringToSize(_WXSTR(psize->GetValue()));

	if (pminsize)
		minsize = StringToSize(_WXSTR(pminsize->GetValue()));

	if (pfont)
		font = StringToFont(_WXSTR(pfont->GetValue()));

	if (pfg_colour)
		fg_colour = StringToColour(_WXSTR(pfg_colour->GetValue()));

	if (pbg_colour)
		bg_colour = StringToColour(_WXSTR(pbg_colour->GetValue()));

	if (pextra_style)
		extra_style = StringToInt( _WXSTR(pextra_style->GetValue()));

	if (GetWindow())
	{
		GetWindow()->SetSize(pos.x,pos.y,size.GetWidth(), size.GetHeight());

		if (pminsize && pminsize->GetValue() != "")
			GetWindow()->SetMinSize(minsize);

		if (pfont && pfont->GetValue() != "")
			GetWindow()->SetFont(font);

		if (pfg_colour && pfg_colour->GetValue() != "")
			GetWindow()->SetForegroundColour(fg_colour);

		if (pbg_colour && pbg_colour->GetValue() != "")
			GetWindow()->SetBackgroundColour(bg_colour);

		if (penabled)
			GetWindow()->Enable((penabled->GetValueAsInteger() !=0 ));

		if (phidden)
			GetWindow()->Show(!phidden->GetValueAsInteger());

		if (ptooltip)
			GetWindow()->SetToolTip(ptooltip->GetValueAsString());

		if (pextra_style)
			GetWindow()->SetExtraStyle( extra_style );
	}
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( VObjEvtHandler,wxEvtHandler )
	EVT_LEFT_DOWN( VObjEvtHandler::OnLeftClick )
	EVT_PAINT( VObjEvtHandler::OnPaint )
	EVT_SET_CURSOR( VObjEvtHandler::OnSetCursor )
	EVT_NOTEBOOK_PAGE_CHANGED( -1, VObjEvtHandler::OnNotebookPageChanged )
	EVT_FLATNOTEBOOK_PAGE_CHANGED( -1, VObjEvtHandler::OnFlatNotebookPageChanged )
	EVT_LISTBOOK_PAGE_CHANGED( -1, VObjEvtHandler::OnListbookPageChanged )
	EVT_CHOICEBOOK_PAGE_CHANGED( -1, VObjEvtHandler::OnChoicebookPageChanged )
END_EVENT_TABLE()

VObjEvtHandler::VObjEvtHandler(wxWindow *win, shared_ptr<ObjectBase> obj, DataObservable *data)
{
	m_window = win;
	m_object = obj;
	m_data = data;
};

void VObjEvtHandler::OnLeftClick(wxMouseEvent &event)
{
	shared_ptr<ObjectBase> obj = m_object.lock();

	if (obj)
	{
		if (m_data->GetSelectedObject() != obj)
			m_data->SelectObject(obj);
		else
			event.Skip();
	}


	//event.Skip();
	m_window->ClientToScreen(&event.m_x, &event.m_y);
	m_window->GetParent()->ScreenToClient(&event.m_x, &event.m_y);
	::wxPostEvent(m_window->GetParent(), event);
}


void VObjEvtHandler::OnPaint(wxPaintEvent &event)
{
	shared_ptr<ObjectBase> wo = shared_dynamic_cast<ObjectBase>(m_object.lock());
	if (wo->IsContainer())
	{
		wxWindow *aux = m_window;
		while (!aux->IsKindOf(CLASSINFO(GridPanel))) aux = aux->GetParent();
		GridPanel *gp = (GridPanel*) aux;
		if (gp->GetActivePanel() == m_window)
		{
			wxPaintDC dc(m_window);
			gp->HighlightSelection(dc);
		}
	}
	event.Skip();
}

void VObjEvtHandler::OnSetCursor(wxSetCursorEvent &event)
{
	wxCoord x = event.GetX(), y = event.GetY();
	m_window->ClientToScreen(&x, &y);
	m_window->GetParent()->ScreenToClient(&x, &y);
	wxSetCursorEvent sce(x, y);
	::wxPostEvent(m_window->GetParent(), sce);
}

//////////////////////////////////////
// Object Tree for notebook objects:
//
//  notebook
//      |
//    notebookpage (item)
//          |
//       container
//////////////////////////////////////

void VObjEvtHandler::OnNotebookPageChanged(wxNotebookEvent &event)
{
	shared_ptr<ObjectBase> obj = m_data->GetSelectedObject();
	if (obj->GetObjectTypeName() == "notebook")
	{
		OnBookPageChanged( obj, event.GetSelection() );
	}
	event.Skip();
}
void VObjEvtHandler::OnFlatNotebookPageChanged(wxFlatNotebookEvent &event)
{
	shared_ptr<ObjectBase> obj = m_data->GetSelectedObject();
	if (obj->GetObjectTypeName() == "flatnotebook")
	{
		OnBookPageChanged( obj, event.GetSelection() );
	}
	event.Skip();
}
void VObjEvtHandler::OnListbookPageChanged(wxListbookEvent &event)
{
	shared_ptr<ObjectBase> obj = m_data->GetSelectedObject();
	if (obj->GetObjectTypeName() == "listbook")
	{
		OnBookPageChanged( obj, event.GetSelection() );
	}
	event.Skip();
}

void VObjEvtHandler::OnChoicebookPageChanged( wxChoicebookEvent& event )
{
	shared_ptr<ObjectBase> obj = m_data->GetSelectedObject();
	if (obj->GetObjectTypeName() == "chiocebook")
	{
		OnBookPageChanged( obj, event.GetSelection() );
	}
	event.Skip();
}

void VObjEvtHandler::OnBookPageChanged( shared_ptr<ObjectBase> obj, int selPage )
{
	if (selPage >= 0)
	{
		for (unsigned int i=0; i<obj->GetChildCount(); i++)
		{
			shared_ptr<ObjectBase> child = obj->GetChild(i);
			shared_ptr<Property> propSelect = child->GetProperty("select");
			if (propSelect)
			{
				// we can't use DataObservable::ModifyProperty because
				// it will regenerate de gui, so these modifications won't be
				// undoable
				if ((int)i == selPage && !propSelect->GetValueAsInteger())
					propSelect->SetValue(string("1"));
				else if ((int)i != selPage && propSelect->GetValueAsInteger())
					propSelect->SetValue(string("0"));
			}
		}
	}
}
