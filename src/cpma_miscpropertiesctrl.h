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
#ifndef CPMA_MISCPROPERTIESCTRL_H
#define CPMA_MISCPROPERTIESCTRL_H

#include "cpma_propertygrid.h"

class ElementBase;

class MiscPropertiesCtrl : public CPMAPropertyGrid
{
  public:
    MiscPropertiesCtrl( wxWindow *parent );

    void from_element( const ElementBase *el );
    void update_layout();

  private:
    void OnItemChanged( wxPropertyGridEvent& ev );
    void OnItemChanging( wxPropertyGridEvent& ev );
    

    DECLARE_EVENT_TABLE()
};

#endif
