// Copyright (C) 2008 Roland Schilter <rolansch@ethz.ch>
// This file is part of SuperHud Editor.
//
// SuperHud Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// SuperHud Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with SuperHud Editor.  If not, see <http://www.gnu.org/licenses/>.
#include "hudfilebase.h"
#include "mainframe.h"
#include "elementsctrlbase.h"
#include "displayctrlbase.h"
#include "prefs.h"
#include "optionalmessagedialog.h"
#include "convertdialog.h"
#include "factorybase.h"
#include "pakmanager.h"

#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/datetime.h>
#include <wx/progdlg.h>
#include <wx/mstream.h>
#include <wx/tokenzr.h>

#include <algorithm>
using namespace std;

DECLARE_APP(SHEApp);

HudFileBase::HudFileBase() :
  m_modified(false),
  m_load_prevel(0)
{
}




void HudFileBase::set_modified( bool modified /*= true*/ )
{ 
  if( m_modified != modified )
  {
    m_modified = modified;
    wxGetApp().mainframe()->update_title();
    if( modified ) wxGetApp().mainframe()->displayctrl()->Refresh();
  }
}

int HudFileBase::OnOpen( const wxString& filename /*=wxT("")*/, bool force_convert /*=false*/ )
{
  m_opt_aspectratio = wxT("4:3");

  wxString f = filename;
  int ret = wxID_OK;
  if( f.empty() )
  {
    wxFileDialog dlg(
        wxGetApp().mainframe(),
        _("Open..."),
        Prefs::get().filedialog_path(),
        wxT(""),
        wxT("HUD Files (*.cfg)|*.cfg|All Files (*.*)|*.*"),
  #if wxCHECK_VERSION(2,7,0)
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
  #else
        wxOPEN | wxFILE_MUST_EXIST
  #endif
        );
    if( wxID_OK != (ret = dlg.ShowModal()) )
      return ret;
    f = dlg.GetPath();
    wxGetApp().factory()->set_filedialog_path( dlg.GetDirectory() );
  }

  wxASSERT( !f.empty() );

  wxGetApp().mainframe()->statusbar()->PushStatusText(wxString::Format(_("Loading HUD: %s"), f.c_str()));
  wxBusyCursor wait0r;
  if( !load( f ) )
  {
    wxLogError( _("Failed reading HUD from file `%s'"), f.c_str() ); // default elements have been loaded
    // try again with default
    if( f != default_hudfilename() )
    {
      OnNew();
      return wxID_OK;
    }
  }


  // load icons for elements
  {
    wxProgressDialog dlg(_("Processing elements"), wxEmptyString, m_els.size(), wxGetApp().mainframe(), 
        wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
    int i=0;
    for( it_elements it = m_els.begin(); it != m_els.end(); ++it, ++i )
    {
      dlg.Update(i, (*it)->name());
      // only preload if it's gonna be drawn afterwards.. 
      if( (*it)->is_rendered() )
        (*it)->prerender();
    }
  }

  // conversion needed?
  double hudar;
  if( !she::ratio_string2double(m_opt_aspectratio, &hudar) )
  {
    wxLogWarning( _("Found an invalid aspect ratio stored inside the HUD file, will assume it's 4:3") );
    m_opt_aspectratio = wxT("4:3");
    hudar = 4/3.0;
  }

  wxLogDebug(wxT("HudFileBase::OnOpen - hudfile aspect ratio: ") + m_opt_aspectratio);
  wxLogDebug(wxT("HudFileBase::OnOpen - view aspect ratio: ") + Prefs::get().var(wxT("view_aspectratio")).sval());

  if( m_opt_aspectratio != Prefs::get().var(wxT("view_aspectratio")).sval() )
  { // aspectratios are different.
    if( Prefs::get().var(wxT("load_autoconvert")).bval() )
    { // so we actually need to convert
      wxLogDebug(wxT("HudFileBase::OnOpen - auto converting from %.2f to %.2f"), hudar, Prefs::get().var(wxT("view_aspectratio")).dval());
      wxLogMessage(_("The HUD was converted from %s to %s\n\n(Due to your Preferences `auto convert on load' and the HUD being stored differently than current aspect ratio)"), m_opt_aspectratio.c_str(), Prefs::get().var(wxT("view_aspectratio")).sval().c_str());
      convert_all( hudar, Prefs::get().var(wxT("view_aspectratio")).dval(), true, true, true );
    }
    else
    { // ask the user what he would like to do.
      // display in correct ratio or convert to his current ratio

      OptionalMessageDialog dlg(wxT("dlg_convertonload"), wxID_ANY, wxID_CANCEL); 
      dlg.add_button(_("Convert HUD..."), wxID_YES);
      dlg.add_button(_("Modify preferences"), wxID_NO);
      dlg.add_button(_("Continue"), wxID_CANCEL);
      wxString msg =  
        wxString::Format(
            _("The HUD I just loaded was very likely created for a different (%s)\naspect ratio than you are currently displaying (%s).\n\nYou can either convert the hud elements to your ratio,\ndisplay it in its original ratio (changing your preferences)\nor just show it as it is (skewed)."),
            m_opt_aspectratio.c_str(),
            Prefs::get().var(wxT("view_aspectratio")).sval().c_str()
            );

      dlg.Create(0, msg, f);
      int ret = dlg.ShowModal();
      if( ret == wxID_YES )
      { // convert
        ConvertDialog dlg(wxGetApp().mainframe());
        dlg.set(true, true, false);
        if( m_opt_aspectratio == wxT("4:3") ) // file is 4:3 (hence view 16:10 probably)
          dlg.set_conversion( CONVERT_4_3, CONVERT_CURRENT );
        else // file is 16:10 (hence view 4:3 probably)
          dlg.set_conversion( CONVERT_16_10, CONVERT_CURRENT );
        if( wxID_OK == dlg.ShowModal() )
        {
          wxGetApp().hudfile()->convert_all( dlg.convert_from(), dlg.convert_to(), dlg.size(), dlg.stretchposition(), dlg.fontsize() );
          Prefs::get().set(wxT("view_aspectratio"), dlg.convert_to_str());
        }
      }
      else if( wxID_NO == ret )
      {
        Prefs::get().set(wxT("view_aspectratio"), m_opt_aspectratio);
        wxGetApp().mainframe()->displayctrl()->reset_projection_mode();
      }
//      update_displayctrl(); // redraw
      // else wxID_CANCEL
    }
  }
  wxGetApp().mainframe()->statusbar()->PopStatusText();
  return ret;
}

int HudFileBase::OnSave( bool saveas /*=false*/)
{
  int ret = wxID_OK;
  if( saveas || m_filename.empty() )
  {
    wxFileDialog dlg(
        wxGetApp().mainframe(),
        (saveas ? _("Save As...") : _("Save...")),
        Prefs::get().filedialog_path(),
        wxT(""),
        wxT("Hud Files (*.cfg)|*.cfg|All Files (*.*)|*.*"),
  #if wxCHECK_VERSION(2,7,0)
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
  #else // 2.6
        wxSAVE|wxOVERWRITE_PROMPT
  #endif
        );
    if( wxID_OK == (ret = dlg.ShowModal()) )
    {
      wxGetApp().factory()->set_filedialog_path( dlg.GetDirectory() );
      m_filename = dlg.GetPath();
    }
    else
      return wxID_CANCEL;
  }
  if( m_filename.empty() )
    return wxID_CANCEL; // this shouldn't reach here.. but make sure anyway

  if( !saveas && Prefs::get().var(wxT("save_backup")) && wxFile::Exists(m_filename)  )
  {
    wxString target = m_filename + wxT(".bak");
    wxGetApp().mainframe()->statusbar()->PushStatusText(wxString::Format(_("Copying backup HUD: %s"), target.c_str()));
    if( !wxCopyFile( m_filename, target, true ) )
      wxLogError( _("Failed copying backup HUD: %s"), target.c_str() );
    wxGetApp().mainframe()->statusbar()->PopStatusText();
  }
  wxGetApp().mainframe()->statusbar()->PushStatusText(wxString::Format(_("Saving HUD: %s"), m_filename.c_str()));
  wxBusyCursor wait;
  if( !save( m_filename ) )
  {
    wxLogError( _("Failed writing hud to `%s'"), m_filename.c_str() );
    wxGetApp().mainframe()->statusbar()->PopStatusText();
    return wxID_CANCEL;
  }
  wxGetApp().mainframe()->statusbar()->PopStatusText();
  return wxID_OK;

}

void HudFileBase::cleanup()
{
  for( cit_elements cit = m_els.begin(); cit != m_els.end(); ++cit )
    delete (*cit);
  m_els.clear();
}

void HudFileBase::clear()
{
  set_modified();
  cleanup();  
}


void HudFileBase::OnNew()
{
  clear();
  load_default_elements();
  OnOpen(default_hudfilename(), false); // load default hud and force conversion
  m_filename = wxEmptyString;
  wxGetApp().elementsctrl()->list_refresh(m_els);
}

void HudFileBase::append( ElementBase *el )
{
  set_modified();
  m_els.push_back(el);
}

void HudFileBase::insert( size_t pos, ElementBase *el )
{
  set_modified();
  m_els.insert( m_els.begin() + pos, el );
}
void HudFileBase::insert( const ElementBase * const after, ElementBase *el )
{
  set_modified();
  for( it_elements it = m_els.begin(); it != m_els.end(); ++it )
  {
    if( *it == after )
    {
      m_els.insert( ++it, el );
      return;
    }
  }
  // just append if we can't find it
  wxLogDebug(wxT("HudFileBase::insert - Couldn't find element to insert after"));
  append(el);
}

bool HudFileBase::remove( ElementBase *el )
{
  set_modified();
  for( it_elements it = m_els.begin(); it != m_els.end(); ++it )
  {
    if( *it == el )
    {
      set_modified();
      m_els.erase(it);
      return true;
    }
  }
  return false;
}

ElementBase* HudFileBase::find_element( const wxString& name )
{
  for( cit_elements cit = m_els.begin(); cit != m_els.end(); ++cit )
    if( (*cit)->name().CmpNoCase(name) == 0 )
      return (*cit);
  return 0;
}

bool HudFileBase::move_element_after( ElementBase *el, ElementBase *after )
{
  if( el == after )
    return true;
  it_elements el_it = find( m_els.begin(), m_els.end(), el );
  if( el_it == m_els.end() )
    return false; // element to move not found
 
  if( 0 == after )
  { // add at beginning
    m_els.erase( el_it );
    m_els.insert( m_els.begin(), el );
  }
  else
  {
    if( find( m_els.begin(), m_els.end(), after ) == m_els.end() )
      return false; // element to move it after not found
    m_els.erase( el_it );
    it_elements after_it = find( m_els.begin(), m_els.end(), after );
    m_els.insert( ++after_it, el );
  }
  set_modified();
  return true;
}

void HudFileBase::write_header( wxTextOutputStream& stream )
{
  wxDateTime dt(wxDateTime::Now());
  stream << wxT("# written by ") << APP_NAME << wxT(" v") << APP_VERSION << wxT(" on ") << dt.FormatISODate() << wxT(" ") << dt.FormatISOTime() << wxT("\n");
  stream << wxT("# ") << APP_URL << wxT("\n");
  stream << wxT("#\n");
  stream << wxT("# -- DO NOT EDIT THE NEXT FEW LINES --\n");
  stream << wxT("# version = ") << APP_VERSION << wxT("\n");
  stream << wxT("# view_aspectratio = ") << Prefs::get().var(wxT("view_aspectratio")).sval() << wxT("\n");
  stream << wxT("# -----------------------------------\n");
}

bool HudFileBase::save( const wxString& filename )
{
  wxFFileOutputStream file( filename.c_str() );
  if( !file.Ok() )
    return false;

  wxTextOutputStream stream( file );
  write_header(stream);

  for( cit_elements it = m_els.begin(); it != m_els.end(); ++it )
    write_element( stream, *(*it) );

  m_filename = filename;
  m_modified = false;

  return true;
}

void HudFileBase::write_element( wxTextOutputStream& stream, const ElementBase& el )
{
  if( !el.is_enabled() )
  {
    if( Prefs::get().var(wxT("save_writedisabled")) )
      stream << wxT("# ") << el.name() << wxT(" { }\n");
    return;
  }
  if( el.flags() & E_SHORT )
  {
    stream << el.name() << wxT(" { ");
    el.write_properties(stream);
    stream << wxT("}\n");
  }
  else
  {
    stream << wxT("\n") << el.name() << wxT("\n{");
    el.write_properties(stream);
    stream << wxT("\n}\n\n");
  }
}

const ElementBase* HudFileBase::get_parent( const ElementBase * const from, int specifies /*= E_HAS_NONE*/ ) const
{
  if( from->flags() & E_NOINHERIT )
    return 0; // the item (of which we would like to retrieve the parent) doesn't inherit at all.

  const ElementBase *d = 0;
  for( cit_elements cit = m_els.begin(); cit != m_els.end(); ++cit )
  {
    if( *cit == from )
      break;
    // is he a parent and does specify what we are looking for?
    if( ((*cit)->flags() & E_PARENT) && ((*cit)->has() & specifies) )
      d = *cit;
    // since we start from top and we want the most near parent, we continue here :o
  }
  return d;
}

bool HudFileBase::load( const wxString& filename )
{
  wxLogDebug(wxT("Loading HUD: ") + filename);

  wxString content;
  char *buf;
  size_t size;

  if( !PakManager::get().load( &buf, filename, PM_SEARCH_EVERYWHERE, &size ) )
  {
    wxLogError(_("Couldn't find/load HUD: %s"), filename.c_str());
    return false;
  }


  wxMemoryInputStream mis( buf, size );

  m_load_prevel = 0;

  load_default_elements();
  wxTextInputStream tis( mis );
  wxString line;
  size_t pos;
  // default opts

  // read file line by line and remove `#' comments
  while(!mis.Eof())
  {
    line = tis.ReadLine();
    she::wxTrim(line);
    if( 0 == line.length() || line[0] == '#' )
    { // check for options
      if( (pos = line.find(wxT("="))) != wxString::npos )
      {
        wxString optname = line.Mid(1, pos-1);
        wxString optval = line.Mid(pos+1);
        she::wxTrim(optname);
        she::wxTrim(optval);
        wxLogDebug(wxT("HudFileBase::load - have found option: ") + optname + wxT(" = ") + optval);
        if( optname == wxT("version") )
          m_opt_version = optval;
        else if( optname == wxT("view_aspectratio") )
          m_opt_aspectratio = optval;
        else
          wxLogDebug(wxT("HudFileBase::load - WARNING: invalid option ") + optname);
      }
      continue;
    }
    if( (pos = line.find( wxT("#") )) != wxString::npos )
      line = line.substr( 0, pos );
    content += line;
    content += wxT("\n");
  }

  she::wxTrim( content ); 
  try
  {
    wxStringTokenizer tok( content, wxT("}") );
    while(tok.HasMoreTokens())
      parse_item( tok.GetNextToken() );
  }
  catch( hudfile_parse_error& err )
  {
    wxString str = wxString::Format(_("ERROR while parsing `%s'"), filename.c_str());
    str += wxString(err.what(), wxConvUTF8);
    wxLogError( str );
    load_default_elements();
    return false;
  }
  // removeo all non-unique elements that aren't enabled
  for( it_elements it = m_els.begin(); it != m_els.end(); ++it )
  {
    if( ((*it)->flags() & E_NOTUNIQ) && !(*it)->is_enabled() )
      m_els.erase( it );
  }

  // remove E_DRAWNEVER and E_PARENT from end
  ElementBase *el;
  for( int i = m_els.size()-1; i >= 0; --i )
  {
    el = m_els[i];
    if( el->flags() & E_DRAWNEVER && el->flags() & E_PARENT )
    {
      m_els.erase(m_els.begin() + i);
      delete el;
    }
    else
      break;
  }
  
  m_filename = filename;
  wxGetApp().mainframe()->update_elementsctrl();
  m_modified = false;
  return true;
}


void HudFileBase::convert_all( double from, double to, bool size, bool stretchposition, bool fontsize)
{
  // as all huds are stored with 640x480 we 
  for( it_elements it = m_els.begin(); it != m_els.end(); ++it )
    (*it)->convert( from, to, size, stretchposition, fontsize);
}

#ifndef NDEBUG
void HudFileBase::debug() const
{
  for( cit_elements it = m_els.begin(); it != m_els.end(); ++it )
  {
    wxLogDebug(wxT("0x%x %s"), (*it), (*it)->name().c_str());
  }

}
#endif // NDEBUG

