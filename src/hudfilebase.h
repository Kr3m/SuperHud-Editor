#ifndef __HUDFILEBASE_H__
#define __HUDFILEBASE_H__

#include "common.h"
#include "elementbase.h"
#include <vector>

/// represents a hudfile (document)
class HudFileBase
{
  public:
    typedef std::vector<ElementBase*>           elements_type;
    typedef elements_type::iterator             it_elements;
    typedef elements_type::const_iterator       cit_elements;
  public:
    HudFileBase();
    virtual ~HudFileBase() { clear(); }

    void set_modified( bool modified = true );
    bool is_modified() const { return m_modified; }
    const wxString& filename() const { return m_filename; }

    /// sets up the document after user clicked File->New
    virtual void on_new();

    /// clears all items
    virtual void clear(); 
    /// adds an element
    virtual void append( ElementBase *el );

    /// Loads a hudfile
    /// @arg wxString filename The full filename
    virtual bool load( const wxString& filename ) = 0;
    // clears all items and then loads default items
    virtual void load_default_elements() = 0;

    virtual bool save( const wxString& filename );

    virtual void write_element( wxTextOutputStream& stream, const ElementBase& el );

  protected:
    ElementBase*        find_element( const wxString& name );

    /// moves an element in the list after a given other element
    /// @arg item The item to move
    /// @arg after Item will be removed and inserted after this (if it's 0 we insert at beginning)
    /// @return True if successfully moved otherwise false;
    bool                move_element_after( ElementBase *item, ElementBase *after );

  protected:
    elements_type       m_els;
    bool                m_modified;
    wxString            m_filename;
};


#endif // __HUDFILEBASE_H__

