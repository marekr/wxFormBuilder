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

#ifndef __VISUAL_OBJS__
#define __VISUAL_OBJS__

#include "wx/wx.h"
#include "model/objectbase.h"
#include <wx/notebook.h>
#include <wx/wxFlatNotebook/wxFlatNotebook.h>
#include <wx/listbook.h>
#include <wx/choicebk.h>
#include "rad/appobserver.h"

class VisualObject;
class VisualWindow;
class VisualSizer;

typedef shared_ptr<VisualObject> PVisualObject;
typedef shared_ptr<VisualWindow> PVisualWindow;
typedef shared_ptr<VisualSizer> PVisualSizer;

/**
 * Objeto Visual.
 *
 * La vista preliminar (Designer), tiene un conjunto de objetos de esta clase.
 * Esta clase además de contener una referencia al objeto-wx que representa,
 * es la responsable de crear la instancia de dicho objeto-wx.
 */
class VisualObject
{

 private:
  shared_ptr<ObjectBase> m_object;

  VisualObject() {};

 public:
  /**
   * Método Fábrica.
   * Crea un objeto a partir del ObjectBase asociado.
   * Para crear los widgets es necesaria una referencia al padre (wxWindow*)
   */
  static PVisualObject CreateVisualObject(shared_ptr<ObjectBase> obj, wxWindow *wx_parent);

  VisualObject(shared_ptr<ObjectBase> obj);
  virtual ~VisualObject();

  shared_ptr<ObjectBase> GetObject() { return m_object; }

  /**
   * Añade el objeto visual (wxSizer* /wxWindow* /spacer) asociado,  al sizer que
   * se pasa como parámetro. Será redefinida según el objeto que sea.
   * obj -> Objeto de tipo T_SIZERITEM con las propiedades de layout
   */
  //virtual void AddToSizer(wxSizer *sizer, shared_ptr<ObjectBase> sizeritem){};

};


/**
 * Objeto que representa un sizer.
 */
class VisualSizer : public VisualObject
{
 private:
  wxSizer *m_sizer;

 public:
  VisualSizer(shared_ptr<ObjectBase> obj, wxWindow *parent);
  wxSizer *GetSizer() { return m_sizer; }
  void SetSizer(wxSizer *sizer) { m_sizer = sizer; }
//  void AddToSizer(wxSizer *sizer, shared_ptr<ObjectBase> sizeritem);
};

/**
 * Objeto que representa una ventana.
 */
class VisualWindow : public VisualObject
{
 private:
  wxWindow *m_window;

 protected:
 public:
  VisualWindow(shared_ptr<ObjectBase> obj, wxWindow *parent);
  wxWindow *GetWindow() { return m_window; }
  void SetWindow(wxWindow *window) { m_window = window; }
//  void AddToSizer(wxSizer *sizer, shared_ptr<ObjectBase> sizeritem);

  /**
   * Configura la ventana con las propiedades comunes definias en la interfaz
   * wxWindow.
   */
  void SetupWindow();

};

/**
 * Procesa los eventos asociados a un objeto.
 * En principio vamos a querer seleccionar el objeto
 * al poner el foco
 */
class VObjEvtHandler : public wxEvtHandler
{
 private:
   weak_ptr<ObjectBase> m_object;
   wxWindow *m_window;
   DataObservable *m_data;

   VObjEvtHandler() {};

 protected:
  DECLARE_EVENT_TABLE()

 public:
   VObjEvtHandler(wxWindow *win, shared_ptr<ObjectBase> obj, DataObservable *data);
   void OnLeftClick(wxMouseEvent &event);
   void OnPaint(wxPaintEvent &event);
   void OnSetCursor(wxSetCursorEvent &event);

   // A patch to solve the notebook problem about page selection
   void OnNotebookPageChanged(wxNotebookEvent &event);
   void OnListbookPageChanged(wxListbookEvent &event);
   void OnFlatNotebookPageChanged(wxFlatNotebookEvent &event);
   void OnChoicebookPageChanged(wxChoicebookEvent &event);
   void OnBookPageChanged( shared_ptr<ObjectBase> obj, int selPage );
};


///////////////////////////////////////////////////////////////////////////////
/**
 * Implementación de la interfaz IObjectView como envoltorio de VisualObject.
 *
 * Se ha diseñado la interfaz IObjectView para acceder a estos objetos
 * desde los plugins y por tanto usa punteros "raw" en lugar de
 * punteros inteligentes (smart pointers) para así no depender de la librería
 * "boost".
 *
 * @note se requiere activar RTTI.
 *
 * @todo refactorizar la clase VisualObject.
 *       Tras las modificaciones realizadas en la generación del designer,
 *       no tiene mucho sentido la jerarquía de clases
 *       (VisualWindow,VisualSizer...) ya que se ha suprimido el método
 *       AddToSizer.
 *
 */
class VisualObjectAdapter : public IObjectView
{
 private:
  PVisualObject m_vObj;

 public:
  VisualObjectAdapter(PVisualObject vobj) : m_vObj(vobj) {};
  virtual ~VisualObjectAdapter() {};

  wxWindow* Window()
  {
    PVisualWindow winobj(shared_dynamic_cast<VisualWindow>(m_vObj));
    if (winobj)
      return winobj->GetWindow();

    return NULL;
  }

  wxSizer*  Sizer()
  {
    PVisualSizer sizerobj(shared_dynamic_cast<VisualSizer>(m_vObj));
    if (sizerobj)
      return sizerobj->GetSizer();

    return NULL;
  }

  IObject*  Object()
  {
    return m_vObj->GetObject().get();
  }
};


#endif //__VISUAL_OBJS__


