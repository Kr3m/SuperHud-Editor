// -*- C++ -*- generated by wxGlade 0.6.1 on Sat Dec 08 17:28:33 2007

#include <wx/wx.h>
#include <wx/image.h>

#ifndef POSITIONPROPERTIESCTRL_H
#define POSITIONPROPERTIESCTRL_H

// begin wxGlade: ::dependencies
#include <wx/spinctrl.h>
// end wxGlade

// begin wxGlade: ::extracode

// end wxGlade


class PositionPropertiesCtrl: public wxPanel {
public:
    // begin wxGlade: PositionPropertiesCtrl::ids
    // end wxGlade

    PositionPropertiesCtrl(wxWindow* parent, int id, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=0);

private:
    // begin wxGlade: PositionPropertiesCtrl::methods
    void set_properties();
    void do_layout();
    // end wxGlade

protected:
    // begin wxGlade: PositionPropertiesCtrl::attributes
    wxStaticBox* sizer_8_staticbox;
    wxCheckBox* checkbox_1;
    wxStaticText* label_1;
    wxSpinCtrl* spin_ctrl_1;
    wxStaticText* label_4;
    wxSpinCtrl* spin_ctrl_2;
    wxBitmapButton* bitmap_button_5;
    wxBitmapButton* bitmap_button_6;
    wxBitmapButton* bitmap_button_7;
    wxBitmapButton* bitmap_button_8;
    wxBitmapButton* bitmap_button_9;
    wxBitmapButton* bitmap_button_10;
    wxStaticText* label_5;
    // end wxGlade
}; // wxGlade: end class


#endif // POSITIONPROPERTIESCTRL_H
