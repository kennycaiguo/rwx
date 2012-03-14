/*
 * wxDialog.cpp
 *
 *  Created on: 12.02.2012
 *      Author: hanmac
 */

#include "wxWindow.hpp"

VALUE rb_cWXDialog;

#define _self wrap<wxDialog*>(self)

namespace RubyWX {
namespace Dialog {

VALUE _alloc(VALUE self)
{
	return wrap(new wxDialog,self);
}
macro_attr(EscapeId,int)
macro_attr(ReturnCode,int)
macro_attr(AffirmativeId,int)

singlereturn(ShowModal)

}
}

void Init_WXDialog(VALUE rb_mWX)
{
	using namespace RubyWX::Dialog;
	rb_cWXDialog = rb_define_class_under(rb_mWX,"Dialog",rb_cWXTopLevel);
	rb_define_alloc_func(rb_cWXDialog,_alloc);

	rb_define_method(rb_cWXDialog,"show_modal",RUBY_METHOD_FUNC(_ShowModal),0);
}
