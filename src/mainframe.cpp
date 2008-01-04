
#include "mainframe.h"

#include "common.h"

#include <wx/artprov.h>
#include <wx/menu.h>
#include <wx/stdpaths.h>
#include <wx/file.h>

#include "factorybase.h"
#include "model.h"
#include "prefs.h"

#include "cpma/elementsctrl.h"
#include "cpma/propertiesnotebook.h"
#include "cpma/displayctrl.h"
#include "cpma/hudfile.h"

//DECLARE_APP(SHEApp);

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(wxID_ABOUT, MainFrame::OnMenuAbout)
  EVT_MENU(wxID_EXIT, MainFrame::OnMenuExit)
  EVT_MENU(wxID_SAVE, MainFrame::OnMenuSave)
  EVT_MENU(wxID_SAVEAS, MainFrame::OnMenuSaveAs)
  EVT_MENU(wxID_OPEN, MainFrame::OnMenuOpen)
  EVT_MENU(wxID_NEW, MainFrame::OnMenuNew)
  EVT_MENU(ID_MENU_TOOLS_SWITCHGAME, MainFrame::OnMenuToolsSwitchGame)
  EVT_MENU(ID_MENU_TOOLS_PREFERENCES, MainFrame::OnMenuToolsPreferences)
  EVT_MENU(ID_MENU_VIEW_DEFAULTPERSPECTIVE, MainFrame::OnMenuViewDefaultPerspective)
  EVT_CLOSE(MainFrame::OnClose)
  EVT_MENU(ID_MENU_VIEW_CONFIGPREVIEW, MainFrame::OnMenuViewConfigPreview)
  EVT_MENU(ID_MENU_VIEW_TOOLBAR_FILE, MainFrame::OnMenuViewToolbarFile)
  EVT_MENU(ID_MENU_VIEW_GRID, MainFrame::OnMenuViewGrid)
  EVT_MENU(ID_MENU_HELP_UPDATE, MainFrame::OnMenuHelpUpdate)
  
  EVT_UPDATE_UI_RANGE(ID_MENU_VIEW_CONFIGPREVIEW, ID_MENU_VIEW_TOOLBAR_FILE, MainFrame::OnUpdateViewPanes)
END_EVENT_TABLE()




MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxString& title, 
        const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*= wxDefaultSize*/,
        long style /*= wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER*/) :
  wxFrame(parent, id, title, pos, size, style),
    m_model(0)
{

  m_mgr.SetManagedWindow(this);

  // create menu
  wxMenuBar *menu_bar = new wxMenuBar;

  wxMenu *file_menu = new wxMenu;
  file_menu->Append( wxID_NEW, _("&New\tCtrl+N") );
  file_menu->Append( wxID_OPEN, _("&Open...\tCtrl+O") );
  file_menu->AppendSeparator();
  file_menu->Append( wxID_SAVE, _("&Save\tCtrl+S") );
  file_menu->Append( wxID_SAVEAS, _("Save &As...\tCtrl+Shift+S") );
  file_menu->AppendSeparator();
  file_menu->Append( wxID_EXIT, _("E&xit\tCtrl+Q") );
  menu_bar->Append( file_menu, _("&File") );

  wxMenu *tools_menu = new wxMenu;
  tools_menu->Append( ID_MENU_TOOLS_SWITCHGAME, _("&Switch game") );
  tools_menu->AppendSeparator();
  tools_menu->Append( ID_MENU_TOOLS_PREFERENCES, _("&Preferences\tCtrl+P") );
  menu_bar->Append( tools_menu, _("&Tools") );

  wxMenu *elements_menu = new wxMenu;
  menu_bar->Append( elements_menu, _("&Elements") );

  m_view_menu = new wxMenu;
  m_view_menu->Append( ID_MENU_VIEW_DEFAULTPERSPECTIVE, _("&Reset View") );
  wxMenu *view_panes_submenu = new wxMenu;
  view_panes_submenu->AppendCheckItem( ID_MENU_VIEW_CONFIGPREVIEW, _("Config Preview\tCtrl+C"), _("Display the config preview panel") );
  view_panes_submenu->AppendCheckItem( ID_MENU_VIEW_TOOLBAR_FILE, _("Toolbar File"), _("Display the file toolbar") );
  m_view_menu->AppendSubMenu(view_panes_submenu, _("Panes"), _("Select which panes you would like to be visible or not"));
  m_view_menu->AppendSeparator();
  m_view_menu->AppendCheckItem( ID_MENU_VIEW_GRID, _("Display Grid\tCtrl+G"), _("Draws a grid over the hud") );
  
  menu_bar->Append( m_view_menu, _("&View") );

  wxMenu *help_menu = new wxMenu;
  help_menu->Append( ID_MENU_HELP_UPDATE, _("Check for updates...") );
  help_menu->Append( wxID_ABOUT, _("&About\tCtrl+A") );
  
  menu_bar->Append( help_menu, _("Help") );

  SetMenuBar( menu_bar );

  // statusbar plz
  m_statusbar = CreateStatusBar(2);
  m_statusbar->SetStatusText(_("Ready"));
  int statusbar_widths[] = { -1, 100 };
  m_statusbar->SetStatusWidths(2, statusbar_widths);
  int statusbar_styles[] = { wxSB_NORMAL, wxSB_FLAT };
  m_statusbar->SetStatusStyles( 2, statusbar_styles );
  

  // create toolbar
  m_toolbar_file = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
  m_toolbar_file->SetToolBitmapSize(wxSize(16,16));
  m_toolbar_file->AddTool( wxID_NEW, _("New"), wxArtProvider::GetBitmap(wxART_NEW_DIR, wxART_TOOLBAR, wxSize(16,16)), _("New") );
  m_toolbar_file->AddTool( wxID_OPEN, _("Open..."), wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, wxSize(16,16)), _("Open...") );
  m_toolbar_file->AddTool( wxID_SAVE, _("Save"), wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, wxSize(16,16)), _("Save") );
  m_toolbar_file->AddTool( wxID_SAVEAS, _("Save As..."), wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR, wxSize(16,16)), _("Save As...") );
  m_toolbar_file->Realize();
 


  // create panes
  m_elementsctrl = wxGetApp().factory()->create_elementsctrl(this);
  m_mgr.AddPane( m_elementsctrl,
      wxAuiPaneInfo().Name(wxT("elements")).Caption(_("Elements")).MaximizeButton(true).CloseButton(false)
      );
  m_configpreview = new wxTextCtrl(this, ID_TEXTCTRL_CONFIGPREVIEW, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
  m_configpreview->SetFont(wxFont(8, wxMODERN, wxNORMAL, wxNORMAL, 0, wxT("")));
  m_mgr.AddPane( m_configpreview, wxAuiPaneInfo().Name(wxT("configpreview")).Caption(_("Config Preview")).
    CloseButton(true).MaximizeButton(true)
    );
  m_displayctrl = wxGetApp().factory()->create_displayctrl(this);
  m_mgr.AddPane( m_displayctrl, 
      wxAuiPaneInfo().Name(wxT("display")).Caption(_("Display")).MaximizeButton(true).CloseButton(false).
      CenterPane()
      );

  m_propertiesnotebook = wxGetApp().factory()->create_propertiesnotebook(this);
  m_mgr.AddPane( m_propertiesnotebook, 
      wxAuiPaneInfo().Name(wxT("properties")).Caption(_("Properties")).MaximizeButton(true).CloseButton(false).
      Right()
      );

  m_mgr.AddPane(m_toolbar_file, wxAuiPaneInfo().Name(wxT("tb-file")).Caption(_("File")).
	  ToolbarPane().Top().Row(1).Position(1).LeftDockable(false).RightDockable(false)
	  );
  
  m_defaultperspective = m_mgr.SavePerspective();

  GetDockArt()->SetColor(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, wxColour(200, 200, 200));
  GetDockArt()->SetColor(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, wxColour(200, 200, 200));
  GetDockArt()->SetColor(wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR, wxColour(100, 100, 100));
  GetDockArt()->SetColor(wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR, wxColour(100, 100, 100));
  

  // update stuff
  m_mgr.LoadPerspective( Prefs::get().var(wxT("app_perspective")) );
  //m_view_menu->Check( ID_MENU_VIEW_TOOLBAR_FILE, m_toolbar_file->IsShown() ); // done through UpdateUI event
  //m_view_menu->Check( ID_MENU_VIEW_CONFIGPREVIEW, m_configpreview->IsShown() ); // done through UpdateUI event
  m_view_menu->Check( ID_MENU_VIEW_GRID, Prefs::get().var(wxT("view_grid")) );
  

  if( Prefs::get().var(wxT("game")) == wxT("q4max") )
    SetIcon( wxArtProvider::GetIcon(ART_Q4MAX, wxART_FRAME_ICON, wxSize(16,16)) );
  else
    SetIcon( wxArtProvider::GetIcon(ART_CPMA, wxART_FRAME_ICON, wxSize(16,16)) );

  if( Prefs::get().var(wxT("app_maximized")) )
    Maximize();
  else if( Prefs::get().var(wxT("app_height")).intval() != -1 && Prefs::get().var(wxT("app_width")).intval() != -1 )
    SetSize( Prefs::get().var(wxT("app_width")).intval(), Prefs::get().var(wxT("app_height")).intval() );
  
#ifndef WIN32
  // default transparency hints on linux throw assertions all over the place 
  set_floating_hint( wxAUI_MGR_RECTANGLE_HINT );
#endif
  m_mgr.Update();

}


void MainFrame::set_floating_hint( wxAuiManagerOption hint )
{
  // disable all hints
  unsigned int f = m_mgr.GetFlags();
  f &= ~wxAUI_MGR_TRANSPARENT_HINT;
  f &= ~wxAUI_MGR_VENETIAN_BLINDS_HINT;
  f &= ~wxAUI_MGR_RECTANGLE_HINT;

  f |= hint;
  m_mgr.SetFlags(f);
}



void MainFrame::OnMenuToolsPreferences( wxCommandEvent& )
{
  wxMessageBox(wxT("Not yet available, edit config file directly:\n") + wxStandardPaths::Get().GetUserDataDir() + wxT("/") + APP_CONFIG );
}

void MainFrame::OnMenuToolsSwitchGame( wxCommandEvent& )
{
  Prefs::get().setb(wxT("startup_gameselection"), true);
  Prefs::get().save();
  wxMessageBox(_("The application will now restart"));
  restart_app();
}

void MainFrame::OnMenuExit( wxCommandEvent& )
{
  Close(true);
}
#include <wx/aboutdlg.h>
#include "pakfiledialog.h"
#include "model.h"
void MainFrame::OnMenuAbout( wxCommandEvent& )
{
  PakFileDialog dlg(this, wxID_ANY, wxT(""));
  dlg.ShowModal();
  wxAboutDialogInfo info;
  info.SetName(APP_NAME);
  info.SetVersion(APP_VERSION);
  info.SetDescription(APP_URL);
  info.SetCopyright(wxT("(C) 2007 Roland Schilter <rolansch@ethz.ch>"));
   //wxAboutBox(info);

  /*
  m_model = new Model();
  m_model->load_mde(wxT("model/dfegg.mde"), PM_SEARCH_APPFILE);
  */
}

void MainFrame::OnMenuNew( wxCommandEvent& )
{
  if( !confirm_saving() )
    return;
  wxGetApp().hudfile()->OnNew();
  update_title();
  m_elementsctrl->OnSelectionChanged();
}

void MainFrame::OnMenuOpen( wxCommandEvent& )
{
  if( !confirm_saving() )
    return;
  wxGetApp().hudfile()->OnOpen();
  update_title();
  m_elementsctrl->OnSelectionChanged();
  
}

void MainFrame::OnMenuSave( wxCommandEvent& )
{
  wxGetApp().hudfile()->OnSave();
  update_title();
}

void MainFrame::OnMenuSaveAs( wxCommandEvent& )
{
  int ret = wxID_OK;
  wxFileDialog dlg(
      this,
      _("Save As..."),
      wxT(""),
      wxT(""),
      wxT("Hud Files (*.cfg)|*.cfg|All Files (*.*)|*.*"),
#if wxCHECK_VERSION(2,7,0)
      wxFD_SAVE | wxFD_OVERWRITE_PROMPT
#else // 2.6
      wxSAVE|wxOVERWRITE_PROMPT
#endif
      );
  if( wxID_OK != (ret = dlg.ShowModal()) )
    return; // user clicked cancel

  wxString filename = dlg.GetPath();
  
  wxBeginBusyCursor();
  if( !wxGetApp().hudfile()->save( filename ) )
    wxLogError( _("Failed writing Hud to `%s'"), filename.c_str() );
  update_title();
  wxEndBusyCursor();
}


MainFrame::~MainFrame()
{
  m_mgr.UnInit();
}

wxAuiDockArt* MainFrame::GetDockArt()
{
  return m_mgr.GetArtProvider();
}

void MainFrame::DoUpdate()
{
  m_mgr.Update();
}

void MainFrame::update_title()
{
  wxString fn = wxGetApp().hudfile()->filename();
  wxString mod = (wxGetApp().hudfile()->is_modified() ? wxT("*") : wxEmptyString);

  if( !fn.empty() )
    SetTitle( mod + fn + wxT(" - ") + APP_CAPTION );
  else
    SetTitle( mod + wxT("[no file] - ") + APP_CAPTION );

}

int MainFrame::confirm_savechanges_dlg()
{
  wxMessageDialog dlg(
      this,
      _("Do you want to save the changes?"),
      _("Save changes?"),
      wxYES_NO|wxCANCEL|wxYES_DEFAULT|wxICON_QUESTION );
  return dlg.ShowModal();
}

bool MainFrame::confirm_saving()
{
  HudFileBase *hf = wxGetApp().hudfile();

  // we save it if filename is not empty (kann nie schaden zu speichern, auch wenn
  // es nicht als modifiziert gilt, evtl haben wir wo was vergessen...) oder es
  // modifiziert ist
  //int save = ((!hf->filename().empty() || hf->is_modified())? wxID_YES : wxID_NO);

  // naaah, if we cannot save it there will be endless loop :x
  int save = (hf->is_modified()? wxID_YES : wxID_NO);
  
  // ask if it's modified
  if( wxGetApp().hudfile()->is_modified() )
    save = confirm_savechanges_dlg();

  if( wxID_CANCEL == save )
    return false; // user aborted question
  if( wxID_YES == save )
    if( wxID_CANCEL == wxGetApp().hudfile()->OnSave() )
      return false; // user aborted save dialog

  return true; // user went through... 
}

void MainFrame::OnClose( wxCloseEvent& ev )
{
  wxLogDebug(wxT("MainFrame::OnClose"));

  if( !confirm_saving() )
    return;

  if( m_model )
    wxDELETE( m_model );
  
  // save view
  Prefs::get().set(wxT("app_perspective"), m_mgr.SavePerspective());
  Prefs::get().seti(wxT("app_width"), GetSize().GetWidth());
  Prefs::get().seti(wxT("app_height"), GetSize().GetHeight());
  Prefs::get().setb(wxT("app_maximized"), IsMaximized());
  Prefs::get().set(wxT("startup_loadfile"), wxGetApp().hudfile()->filename());

  // cleanup
  m_displayctrl->cleanup();

  ev.Skip();// this->Destroy()?!
}

void MainFrame::OnMenuViewDefaultPerspective( wxCommandEvent& )
{
  m_mgr.LoadPerspective( m_defaultperspective );
}

void MainFrame::OnMenuViewConfigPreview( wxCommandEvent& )
{
  wxAuiPaneInfo& info = m_mgr.GetPane(wxT("configpreview"));
  info.Show( m_view_menu->IsChecked(ID_MENU_VIEW_CONFIGPREVIEW) );
  DoUpdate();
}

void MainFrame::OnMenuViewToolbarFile( wxCommandEvent& )
{
  wxAuiPaneInfo& info = m_mgr.GetPane(wxT("tb-file"));
  info.Show( m_view_menu->IsChecked(ID_MENU_VIEW_TOOLBAR_FILE) );
  DoUpdate();
}

void MainFrame::OnMenuViewGrid( wxCommandEvent& ev )
{
  Prefs::get().setb(wxT("view_grid"), ev.IsChecked());
  m_displayctrl->Refresh();
}

void MainFrame::OnUpdateViewPanes( wxUpdateUIEvent& ev )
{
  if( ev.GetId() == ID_MENU_VIEW_CONFIGPREVIEW )
    ev.Check( m_configpreview->IsShown() );
  else if( ev.GetId() == ID_MENU_VIEW_TOOLBAR_FILE )
    ev.Check( m_toolbar_file->IsShown() );
}

// call this in the event handler used to show the wxWebUpdateDlg
static void wxUpdateAndExit(wxFrame *caller, 
					bool savelog = FALSE,
     				bool restart = TRUE,
     				const wxString &xrc = wxEmptyString, 	// --xrc option won't be given using wxEmptyString
         			const wxString &res = wxEmptyString,	// --res option won't be given using wxEmptyString
            		const wxString &xml = wxEmptyString,	// --xml option won't be given using wxEmptyString
         			const wxString &uri = wxEmptyString)	// --uri option won't be given using wxEmptyString
{
	wxString opts;
 
 	if (savelog)
  		opts += wxT(" --savelog");
    if (restart)
    	opts += wxT(" --restart");
    if (!xrc.IsEmpty())
     	opts += wxT(" --xrc=\"") + xrc + wxT("\"");
    if (!res.IsEmpty())
    	opts += wxT(" --res=\"") + res + wxT("\"");
 	if (!xml.IsEmpty())
  		opts += wxT(" --xml=\"") + xml + wxT("\"");
 	if (!uri.IsEmpty())
  		opts += wxT(" --uri=\"") + uri + wxT("\"");

  wxLogDebug(wxT("Invoking webupdater: ") + opts);
#ifdef __WXMSW__
	wxExecute(wxT("webupdater.exe") + opts);
#else	
	wxExecute(wxT("./webupdater") + opts);
#endif
	caller->Close(true); // FIXME this does not go through MainFrame::OnClose?!
}

void MainFrame::restart_app()
{
  wxString cmd = wxStandardPaths::Get().GetExecutablePath();
  wxLogDebug(wxT("restart_app() - restarting [%s]"), cmd.c_str());
  wxExecute( cmd, wxEXEC_ASYNC );
	Close(true);
}


void MainFrame::OnMenuHelpUpdate( wxCommandEvent& )
{
  wxLogError(wxT("FIXME: this does not go through MainFrame::OnClose"));

  wxString dd = wxStandardPaths::Get().GetDataDir() + PATH_SEP;
  wxUpdateAndExit(this, true, true, 
    dd + wxT("data/webupdater/webupdatedlg.xrc"), 
    wxT("wxWebUpdateLogDlg"),
    dd + wxT("data/webupdater/local.xml")
    );

}

#include <wx/sstream.h>
#include <wx/txtstrm.h>
void MainFrame::OnElementSelectionChanged()
{
  wxASSERT(m_elementsctrl);
  elements_type& els = m_elementsctrl->selected_elements();

  // -- update propertiesctrl title
  wxString caption;
  wxAuiPaneInfo& propsinfo = m_mgr.GetPane(wxT("properties"));
  if( els.size() == 1 )
  {
    caption = _("Properties");
    caption += wxT(": ") + els.front()->name();
  }
  else if( els.size() == 0 )
  {
    caption = _("Properties");
    caption += wxT(": ") + wxString(_("(none)"));
  }
  else
  {
    caption = _("Properties");
    caption += wxT(": ") + wxString(_("(multiple)"));
  }
  propsinfo.Caption( caption );
  DoUpdate();

  // -- render view
  m_displayctrl->Refresh();

  // -- update configpreview
  update_configpreview();

  // -- update propertiesctrl
  m_propertiesnotebook->update_from_element(els);
}

void MainFrame::OnPropertiesChanged()
{
  // -- render view
  m_displayctrl->Refresh();

  update_configpreview();
}

void MainFrame::update_configpreview()
{
  wxASSERT(m_elementsctrl);
  elements_type& els = m_elementsctrl->selected_elements();
  // -- update config preview
  // TODO put configpreview in own class
  wxString out;
  wxStringOutputStream sos(&out);
  wxTextOutputStream tos(sos);
  // get all selected
  wxListItem info;
  HudFileBase *hf = wxGetApp().hudfile();
  for( cit_elements cit = els.begin(); cit != els.end(); ++cit )
  {
    hf->write_element(tos, *(*cit));
  }  
  m_configpreview->SetValue(out);
}


