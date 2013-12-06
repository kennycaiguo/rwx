/*
 * wxPropertyGrid.cpp
 *
 *  Created on: 09.05.2012
 *      Author: hanmac
 */

#include "wxPropertyGridPage.hpp"
#include "wxPropertyGridInterface.hpp"
#include "wxEvtHandler.hpp"

VALUE rb_cWXPropertyGridPage;

#if wxUSE_PROPGRID
#define _self unwrap<wxPropertyGrid*>(self)

namespace RubyWX {
namespace PropertyGridPage {

APP_PROTECT(wxPropertyGridPage)

singlereturn(GetRoot)

}
}
#endif
DLL_LOCAL void Init_WXPropertyGridPage(VALUE rb_mWX)
{
#if wxUSE_PROPGRID
	using namespace RubyWX::PropertyGridPage;
	rb_cWXPropertyGridPage = rb_define_class_under(rb_mWX,"PropertyGridPage",rb_cObject);
	rb_define_alloc_func(rb_cWXPropertyGridPage,_alloc);

	rb_undef_method(rb_cWXPropertyGridPage,"initialize_copy");
	rb_undef_method(rb_cWXPropertyGridPage,"_load");
	rb_undef_method(rb_cWXPropertyGridPage,"_dump");

	rb_include_module(rb_cWXPropertyGridPage,rb_mWXPropertyGridInterface);
	rb_include_module(rb_cWXPropertyGridPage,rb_mWXEvtHandler);

	rb_define_method(rb_cWXPropertyGridPage,"root",RUBY_METHOD_FUNC(_GetRoot),0);

	registerInfo<wxPropertyGridPage>(rb_cWXPropertyGridPage);
#endif

}


