/*
 * wxFilePicker.hpp
 *
 *  Created on: 12.03.2012
 *      Author: hanmac
 */

#ifndef WXFILEPICKER_HPP_
#define WXFILEPICKER_HPP_

#include "wxEvtHandler.hpp"

extern VALUE rb_cWXFilePicker;

void Init_WXFilePicker(VALUE rb_mWX);
#if wxUSE_FILEPICKERCTRL
#include <wx/filepicker.h>

namespace RubyWX {
namespace FilePicker {

void set_style_flags(VALUE hash,int& style);

}
}
#endif

#endif /* WXFILEPICKER_HPP_ */
