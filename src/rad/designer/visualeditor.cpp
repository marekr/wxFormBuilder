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

#include "visualeditor.h"

#include "visualeditor.h"
#include "visualobj.h"
#include "utils/typeconv.h"
#include "utils/debug.h"
#include "menubar.h"
#include "wx/statline.h"
#include "rad/designer/resizablepanel.h"

#ifdef __WX24__
#define wxFULL_REPAINT_ON_RESIZE 0
#endif

#ifdef __WXGTK__
#define VISUAL_EDITOR_BORDER wxRAISED_BORDER
#else
#define VISUAL_EDITOR_BORDER (wxFULL_REPAINT_ON_RESIZE | wxDOUBLE_BORDER)
#endif

BEGIN_EVENT_TABLE(VisualEditor,wxScrolledWindow)
//EVT_SASH_DRAGGED(-1, VisualEditor::OnResizeBackPanel)
//EVT_COMMAND(-1, wxEVT_PANEL_RESIZED, VisualEditor::OnResizeBackPanel)
EVT_PANEL_RESIZED(-1, VisualEditor::OnResizeBackPanel)
EVT_PAINT(VisualEditor::OnPaintPanel)
END_EVENT_TABLE()

VisualEditor::VisualEditor(wxWindow *parent)
: wxScrolledWindow(parent,-1,wxDefaultPosition,wxDefaultSize,wxSUNKEN_BORDER)
{

	// Parece ser que han modificado el comportamiento en wxMSW 2.5.x ya que al
	// poner un color de background, este es aplicado a los hijos también.
	// SetBackgroundColour(wxColour(150,150,150));

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	SetScrollRate(5, 5);

	m_back = new GridPanel(this,-1,wxPoint(10,10),wxSize(350,200),VISUAL_EDITOR_BORDER);
	m_back->SetAutoLayout(true);
	/*m_back->SetSashVisible(wxSASH_BOTTOM,true);
	m_back->SetSashBorder(wxSASH_BOTTOM,true);
	m_back->SetSashVisible(wxSASH_RIGHT,true);
	m_back->SetSashBorder(wxSASH_RIGHT,true);*/
	//  m_back->PushEventHandler(new EditorHandler(GetData()));
}

void VisualEditor::Setup()
{
#ifdef __WXFB_EXPERIMENTAL__
	EditorHandler *handler = new EditorHandler(GetData());
	handler->SetWindow(m_back);
	m_back->PushEventHandler(handler);
#endif //__WXFB_EXPERIMENTAL__

}

void VisualEditor::UpdateVirtualSize()
{
	int w, h, panelW, panelH;
	GetVirtualSize(&w, &h);
	m_back->GetSize(&panelW, &panelH);
	panelW += 20; panelH += 20;
	if (panelW != w || panelH != h) SetVirtualSize(panelW, panelH);
}

void VisualEditor::OnPaintPanel (wxPaintEvent &event)
{
	// es necesario esto para que se pinte el panel de oscuro
	// con wxGTK.
	wxPaintDC dc(this);
	//dc.SetBackground(wxBrush(wxColour(150,150,150),wxSOLID));
	dc.SetBackground(wxBrush(wxColour(192,192,192),wxSOLID));
	dc.Clear();
}

void VisualEditor::OnResizeBackPanel (wxCommandEvent &event) //(wxSashEvent &event)
{
	/*wxRect rect(event.GetDragRect());
	Debug::Print("VisualEditor::OnResizeBackPanel [%d,%d,%d,%d]",rect.x,rect.y,rect.width, rect.height);
	m_back->SetSize(rect.width,rect.height);
	m_back->Layout();*/

	shared_ptr<ObjectBase> form (GetData()->GetSelectedForm());

	if (form)
	{
		shared_ptr<Property> prop(form->GetProperty( wxT("size") ));
		if (prop)
		{
			wxString value(TypeConv::PointToString(wxPoint(m_back->GetSize().x, m_back->GetSize().y)));
			GetData()->ModifyProperty(prop, value);
		}
	}

	//event.Skip();
}

/**
* Crea la vista preliminar borrando la previa.
*/
void VisualEditor::Create()
{
	bool need_fit = false;
	shared_ptr<ObjectBase> menubar;
	wxWindow *statusbar = NULL;
	wxWindow *toolbar = NULL;

	m_form = GetData()->GetSelectedForm();

	if (IsShown()) Freeze(); // congelamos para evitar el flickering

	// borramos la información previa del editor
	m_back->SetSelectedItem(NULL);
	m_back->SetSelectedSizer(NULL);
	m_back->SetSelectedObject(shared_ptr<ObjectBase>());
	m_back->DestroyChildren();
	m_back->SetSizer(NULL); // *!*

	// limpiamos el registro de objetos del editor
	m_map.clear();

	if (m_form)
	{

		// --- [1] Configuramos el tamaño del form ---------------------------
		shared_ptr<Property> pminsize(m_form->GetProperty( wxT("minimum_size") ) );
		if(pminsize)
		{
			wxSize minsize(TypeConv::StringToSize(_WXSTR(pminsize->GetValue())));
			m_back->SetMinSize( minsize );
		}
		shared_ptr<Property> psize(m_form->GetProperty( wxT("size")));
		if (psize)
		{
			wxSize wsize(TypeConv::StringToSize(_WXSTR(psize->GetValue())));
			wxSize minsize = m_back->GetMinSize();
			int height = wsize.GetHeight();
			height = ( height >= minsize.GetHeight() ) ? height : minsize.GetHeight();

			int width = wsize.GetWidth();
			width = ( width >= minsize.GetWidth() ) ? width : minsize.GetWidth();
			m_back->SetSize( width, height );

			if ( -1 == height || -1 == width )
			{
				// si el tamaño es el predeterminado, haremos un Fit() al final para
				// que se ajuste según los componentes
				need_fit = true;
			}
		}
		else
		{
			//m_back->SetSize(350,200);
			m_back->SetSize(wxDefaultSize);
			need_fit = true;
		}

		// --- [2] Emulamos el color del form -------------------------------
		shared_ptr<Property> background( m_form->GetProperty( wxT("bg") ) );
		if ( background && !background->GetValue().empty() )
		{
			m_back->SetBackgroundColour( TypeConv::StringToColour( _WXSTR(background->GetValue()) ) );
		}
		else
		{
			if (m_form->GetClassName() == wxT("Frame") )
			{
				m_back->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
			}
			else
			{
				m_back->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
			}
		}

		// --- [3] Creamos los componentes del form -------------------------
		for (unsigned int i=0; i < m_form->GetChildCount(); i++)
		{
			shared_ptr<ObjectBase> child = m_form->GetChild(i);

			if (child->GetObjectTypeName() == wxT("menubar") )
				menubar = child; // guardamos el objeto del menú para crearlo después
			else
				// generamos recursivamente todo el arbol de objetos
				Generate(child,m_back,NULL,PVisualObject());


			// si se creó una barra de estado, guardamos el widget para configurar
			// el "frame"
			if (child->GetClassName() == wxT("wxStatusBar") )
			{
				VisualObjectMap::iterator it = m_map.find(child);
				statusbar = shared_dynamic_cast<VisualWindow>(it->second)->GetWindow();
			}

			// si se creó una barra de herramientas, guardamos el widget para configurar
			// el "frame"
			if (child->GetClassName() == wxT("wxToolBar") )
			{
				VisualObjectMap::iterator it = m_map.find(child);
				toolbar = shared_dynamic_cast<VisualWindow>(it->second)->GetWindow();
			}
		}

		if (need_fit)
			m_back->Fit();

		m_back->Layout();

		if (menubar || statusbar || toolbar)
			m_back->SetFrameWidgets(menubar, toolbar, statusbar);

		shared_ptr<Property> enabled( m_form->GetProperty( wxT("enabled") ) );
		if ( enabled )
		{
			m_back->Enable( TypeConv::StringToInt( _WXSTR( enabled->GetValue() ) ) != 0 );
		}

		shared_ptr<Property> hidden( m_form->GetProperty( wxT("hidden") ) );
		if ( hidden )
		{
			m_back->Show( TypeConv::StringToInt( _WXSTR( hidden->GetValue() ) ) == 0 );
		}
	}
	else
	{
		// no hay form que mostrar
		m_back->SetSize(10,10);
		m_back->SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	}

	if (IsShown()) Thaw();
	UpdateVirtualSize();
}


/**
* ALGORITMO PARA GENERAR LA VISTA PRELIMINAR
*
* @param obj ObjectBase a generar.
* @param parent wxWindow padre, necesario para instanciar un widget.
* @param sizer sizer más próximo, para poder incluir el objeto creado.
* @param is_widget indica si el objeto padre es un widget o no. Si vamos
*                  a crear un sizer y resulta que el padre es un widget
*                  hemos de establecer este como su sizer.
*/
PVisualObject VisualEditor::Generate(shared_ptr<ObjectBase> obj, wxWindow *wxparent,
									 wxSizer *sizer, PVisualObject vparent)//ObjectType parentType)
{
	// en primer lugar creamos la instancia del objeto-wx que nos ocupa
	PVisualObject vobj(VisualObject::CreateVisualObject(obj,wxparent));

	if (!vobj)
		return vobj; // no se debe dar nunca

	IComponent *comp = obj->GetObjectInfo()->GetComponent();
	if ( NULL == comp )
	{
		wxLogFatalError( wxT("Component for %s not found!"), _WXSTR(obj->GetObjectInfo()->GetClassName()).c_str() );
	}

	// registramos el objeto para poder obtener la referencia a VisualObject a
	// partir de un ObjectBase
	m_map.insert(VisualObjectMap::value_type(obj,vobj));

	VisualObjectAdapter obj_view(vobj); // Adaptador IObjectView para obj

	// Si el objeto es un widget, le añadimos el menejador de eventos para
	// poder seleccionarlo desde el designer y que se dibujen los recuadros...
	// FIXME: eliminar dependencias con ObjectType
	//        (quizá con una función en el plugin: bool IsContainer()
	//if (obj_view.Window() &&
	//    (obj->GetObjectTypeName() == "widget" || obj->GetObjectTypeName() == "container")
	//    || obj->GetObjectTypeName() == "statusbar")
	if (obj_view.Window())// && !comp->KeepEvtHandler())
	{
		obj_view.Window()->PushEventHandler(
			new VObjEvtHandler(obj_view.Window(),obj,GetData()));
	}

	// nuevo padre para las ventanas que se encuentren por debajo
	wxWindow *new_wxparent = (obj_view.Window() ? obj_view.Window() : wxparent);

	// Generamos recursivamente todos los hijos conservando la refenrencia
	// del primero, ya que será pasado como parámetros en la función del
	// plugin OnCreated.

	PVisualObject first_child;

	if (obj->GetChildCount()>0)
		first_child = Generate(obj->GetChild(0),new_wxparent,NULL,vobj);

	for (unsigned int i=1; i<obj->GetChildCount() ; i++)
	{
		PVisualObject child = Generate(obj->GetChild(i),new_wxparent,NULL,vobj);
		VisualObjectAdapter adapter(child);
		if (adapter.Window() && new_wxparent->IsKindOf(CLASSINFO(wxToolBar)))
			((wxToolBar*)new_wxparent)->AddControl((wxControl*) adapter.Window());
	}

	// Procesamos el evento OnCreated
	VisualObjectAdapter parent_view(vparent);
	VisualObjectAdapter first_child_view(first_child);

	comp->OnCreated(&obj_view,new_wxparent,&parent_view, &first_child_view);

	// Por último, debemos asignar el sizer al widget, en los siguientes casos:
	// 1. El objeto creado sea un sizer y el objeto padre sea una ventana.
	// 2. No objeto padre (wxparent == m_back).

	if ((obj_view.Sizer() && parent_view.Window()) || (!vparent && obj_view.Sizer()))
	{
		wxparent->SetSizer(obj_view.Sizer());

		if (vparent)
			obj_view.Sizer()->SetSizeHints(wxparent);

		wxparent->SetAutoLayout(true);
		wxparent->Layout();
	}

	return vobj;
}

BEGIN_EVENT_TABLE(GridPanel, ResizablePanel) //wxSashWindow)
EVT_PAINT(GridPanel::OnPaint)
//  EVT_MOTION(GridPanel::OnMouseMove)
END_EVENT_TABLE()

IMPLEMENT_CLASS(GridPanel, ResizablePanel) //wxSashWindow)

GridPanel::GridPanel(wxWindow *parent, int id, const wxPoint& pos,

					 const wxSize &size, long style, const wxString &name)
					 : ResizablePanel(parent, pos, size, style) //wxSashWindow(parent,id,pos,size,style,name)
{
	SetGrid(10,10);
	m_selSizer = NULL;
	m_selItem = NULL;
	m_actPanel = NULL;
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
}

void GridPanel::SetGrid(int x, int y)
{
	m_x = x;
	m_y = y;
}

void GridPanel::DrawRectangle(wxDC& dc, const wxPoint& point, const wxSize& size, shared_ptr<ObjectBase> object)
{
	int border = object->GetParent()->GetPropertyAsInteger(wxT("border"));
	if (border == 0) border = 1;
	int flag = object->GetParent()->GetPropertyAsInteger(wxT("flag"));
	int topBorder = (flag & wxTOP) == 0 ? 1 : border;
	int bottomBorder = (flag & wxBOTTOM) == 0 ? 1 : border;
	int rightBorder = (flag & wxRIGHT) == 0 ? 1 : border;
	int leftBorder = (flag & wxLEFT) == 0 ? 1 : border;
	dc.DrawRectangle(point.x - leftBorder + 1, point.y - topBorder + 1,
		size.x + leftBorder + rightBorder - 2,
		size.y + topBorder + bottomBorder - 2);
	dc.DrawRectangle(point.x - leftBorder, point.y - topBorder,
		size.x + leftBorder + rightBorder,
		size.y + topBorder + bottomBorder);
}

void GridPanel::HighlightSelection(wxDC& dc)
{
	wxSize size;
	shared_ptr<ObjectBase> object = m_selObj.lock();

	if (m_selSizer)
	{
		wxPoint point = m_selSizer->GetPosition();
		size = m_selSizer->GetSize();
		wxPen bluePen(*wxBLUE, 1, wxSOLID);
		dc.SetPen(bluePen);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		shared_ptr<ObjectBase> sizerParent = object->FindNearAncestor( wxT("sizer") );
		if (sizerParent && sizerParent->GetParent())
			DrawRectangle(dc, point, size, sizerParent);
	}
	if (m_selItem)
	{
		wxPoint point;
		bool shown;

		// Tenemos un problema (de momento) con los wxClassInfo's de los
		// componentes que no forman parte de wxWidgets, debido a que el componente
		// se compila por separado en una librería compartida/dll
		// y parece ser que la información de tipo de wxWidgets se configura
		// estáticamente.
		// Por tanto, no vamos a usar la información de tipos de wxWidgets.

		if (m_selItem->IsKindOf(CLASSINFO(wxWindow)))
		{
			point = ((wxWindow*)m_selItem)->GetPosition();
			size = ((wxWindow*)m_selItem)->GetSize();
			shown = ((wxWindow*)m_selItem)->IsShown();
		}
		else if (m_selItem->IsKindOf(CLASSINFO(wxSizer)))
		{
			point = ((wxSizer*)m_selItem)->GetPosition();
			size = ((wxSizer*)m_selItem)->GetSize();
			shown = true;
		}
		else
		{
			Debug::Print( wxT("Unknown class: %s"), m_selItem->GetClassInfo()->GetClassName());
			return;
		}

		if (shown)
		{
			wxPen redPen(*wxRED, 1, wxSOLID);
			dc.SetPen(redPen);
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			DrawRectangle(dc, point, size, object);
		}
	}
}

wxMenu* GridPanel::GetMenuFromObject(shared_ptr<ObjectBase> menu)
{
	int lastMenuId = wxID_HIGHEST + 1;
	wxMenu *menuWidget = new wxMenu();
	for (unsigned int j = 0; j < menu->GetChildCount(); j++)
	{
		shared_ptr<ObjectBase> menuItem = menu->GetChild(j);
		if (menuItem->GetObjectTypeName() == wxT("submenu") )
			menuWidget->Append(lastMenuId++, menuItem->GetPropertyAsString(wxT("label")), GetMenuFromObject(menuItem));
		else if (menuItem->GetClassName() == wxT("separator") )
			menuWidget->AppendSeparator();
		else
		{
			wxString label = menuItem->GetPropertyAsString(wxT("label"));
			wxString shortcut = menuItem->GetPropertyAsString(wxT("shortcut"));
			if (!shortcut.IsEmpty())
				label = label + wxChar('\t') + shortcut;

			wxMenuItem *item = new wxMenuItem(menuWidget, lastMenuId++,
				label, menuItem->GetPropertyAsString(wxT("help")),
				(menuItem->GetPropertyAsInteger(wxT("kind")) != 0));

			if (!menuItem->GetProperty(wxT("bitmap"))->IsDefaultValue())
				item->SetBitmap(menuItem->GetPropertyAsBitmap(wxT("bitmap")));

			menuWidget->Append(item);

			if (item->GetKind() == wxITEM_CHECK && menuItem->GetPropertyAsInteger(wxT("checked")))
				item->Check(true);

			item->Enable((menuItem->GetPropertyAsInteger(wxT("enabled")) != 0 ));
		}
	}
	return menuWidget;
}

void GridPanel::SetFrameWidgets(shared_ptr<ObjectBase> menubar, wxWindow *toolbar, wxWindow *statusbar)
{
	Menubar *mbWidget = NULL;

	if (menubar)
	{
		mbWidget = new Menubar(this, -1);
		for (unsigned int i = 0; i < menubar->GetChildCount(); i++)
		{
			shared_ptr<ObjectBase> menu = menubar->GetChild(i);
			wxMenu *menuWidget = GetMenuFromObject(menu);
			mbWidget->AppendMenu(menu->GetPropertyAsString(wxT("label")), menuWidget);
		}
	}

	wxSizer *mainSizer = GetSizer();

	SetSizer(NULL, false);

	wxSizer *dummySizer = new wxBoxSizer(wxVERTICAL);
	if (mbWidget)
	{
		dummySizer->Add(mbWidget, 0, wxEXPAND | wxTOP | wxBOTTOM, 0);
		dummySizer->Add(new wxStaticLine(this, -1), 0, wxEXPAND | wxALL, 0);
	}

	if (toolbar)
		dummySizer->Add(toolbar, 0, wxEXPAND | wxALL, 0);

	if (mainSizer)
		dummySizer->Add(mainSizer, 1, wxEXPAND | wxALL, 0);
	else
		dummySizer->AddStretchSpacer(1);

	if (statusbar)
		dummySizer->Add(statusbar, 0, wxEXPAND | wxALL, 0);


	SetSizer(dummySizer, false);
	Layout();
}

void GridPanel::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	wxSize size = GetSize();
	dc.SetPen(*wxBLACK_PEN);
	for (int i=0;i<size.GetWidth();i += m_x)
		for (int j=0;j<size.GetHeight();j += m_y)
			dc.DrawPoint(i-1,j-1);

	if (m_actPanel != this) return;
	HighlightSelection(dc);
}
/*
void GridPanel::OnMouseMove(wxMouseEvent &event)
{
wxLogMessage(wxT("Moving.."));
event.Skip();
}*/

//////////////////////////////////////////////////////////////////////////////

void VisualEditor::ProjectLoaded()
{
	Create();
}

void VisualEditor::ProjectRefresh()
{
	Create();
}

void VisualEditor::ProjectSaved()
{
}

void VisualEditor::ObjectSelected(shared_ptr<ObjectBase> obj)
{
	// sólo es necesario regenerar la vista si el objeto
	// seleccionado pertenece a otro form
	if (GetData()->GetSelectedForm() != m_form)
		Create();

	// Asignamos el widget y el sizer para mostrar el recuadro

	PVisualObject visualObj;
	shared_ptr<ObjectBase> objAuxCt, objAuxNb, objAux;
	VisualObjectMap::iterator it = m_map.find(obj);

	if (it != m_map.end())
	{
		wxWindow *selPanel = NULL;
		visualObj = it->second;

		// 1. Buscar el panel activo, sobre este es donde se dibujarán los recuadros
		//    en el evento OnPaint.
		// buscamos hacia arriba el objeto más cercano cuyo componente sea
		// de tipo WINDOW.

		//objAuxCt = obj->FindNearAncestor("container");
		//objAuxNb = obj->FindNearAncestor("notebook");
		//if (!objAuxCt)
		//  objAux = objAuxNb;
		//else if (!objAuxNb)
		//  objAux = objAuxCt;
		//else
		//  objAux = objAuxNb->Deep() > objAuxCt->Deep() ? objAuxNb : objAuxCt;

		objAux = obj->GetParent();
		while (objAux)
		{
			IComponent *compAux = objAux->GetObjectInfo()->GetComponent();
			if (!compAux)
			{
				objAux.reset();
				break;
			}

			if (compAux->GetComponentType() == COMPONENT_TYPE_WINDOW)
				break;

			objAux = objAux->GetParent();
		}

		if (objAux)  // Un padre de tipo T_WIDGET es siempre un contenedor
		{
			it = m_map.find(objAux);
			selPanel = shared_dynamic_cast<VisualWindow>(it->second)->GetWindow();
		}
		else
			selPanel = m_back;

		// 2. Buscar el item
		wxObject *item = NULL;
		wxSizer *sizer = NULL;
		wxString typeName = obj->GetObjectTypeName();

		int componentType = COMPONENT_TYPE_ABSTRACT;
		IComponent *comp = obj->GetObjectInfo()->GetComponent();
		if (comp)
			componentType = comp->GetComponentType();

		//if ( typeName == "widget" || typeName == "container" ||
		//     typeName == "notebook" || typeName == "statusbar")
		if (componentType == COMPONENT_TYPE_WINDOW)
			item = shared_dynamic_cast<VisualWindow>(visualObj)->GetWindow();

		//else if (typeName == "sizer")
		else if (componentType == COMPONENT_TYPE_SIZER)
			item = shared_dynamic_cast<VisualSizer>(visualObj)->GetSizer();

		// 3. Buscar el sizer.
		// lo que se hace a continuación es buscar el objeto más próximo que sea
		// un componente WINDOW o un componente SIZER y en el caso de ser un
		// sizer lo guardamos.

		//objAux = obj->FindNearAncestor("sizer");
		//objAuxCt = obj->FindNearAncestor("container");
		//if (objAux && (!objAuxCt || objAux->Deep() > objAuxCt->Deep()))
		//{
		//  it = m_map.find(objAux);
		//  sizer = shared_dynamic_cast<VisualSizer>(it->second)->GetSizer();
		//}

		objAux = obj->GetParent();
		while (objAux)
		{
			IComponent *compAux = objAux->GetObjectInfo()->GetComponent();
			if (!compAux)
				break;

			if (compAux->GetComponentType() == COMPONENT_TYPE_SIZER)
			{
				it = m_map.find(objAux);
				sizer = shared_dynamic_cast<VisualSizer>(it->second)->GetSizer();
				break;
			}
			else if (compAux->GetComponentType() == COMPONENT_TYPE_WINDOW)
				break;

			objAux = objAux->GetParent();
		}


		m_back->SetSelectedSizer(sizer);
		m_back->SetSelectedItem(item);
		m_back->SetSelectedObject(obj);
		m_back->SetSelectedPanel(selPanel);
		m_back->Refresh();
	}
	else
	{
		m_back->SetSelectedSizer(NULL);
		m_back->SetSelectedItem(NULL);
		m_back->SetSelectedObject(shared_ptr<ObjectBase>());
		m_back->SetSelectedPanel(NULL);
		m_back->Refresh();
	}
}

void VisualEditor::ObjectCreated(shared_ptr<ObjectBase> obj)
{
	Create();
}

void VisualEditor::ObjectRemoved(shared_ptr<ObjectBase> obj)
{
	Create();
}

void VisualEditor::PropertyModified(shared_ptr<Property> prop)
{
	shared_ptr<ObjectBase> aux = m_back->GetSelectedObject();
	Create();
	ObjectSelected(aux);
	UpdateVirtualSize();
}




