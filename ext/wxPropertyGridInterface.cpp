/*
 * wxPropertyGridInterface.cpp
 *
 *  Created on: 09.05.2012
 *      Author: hanmac
 */

#include "wxPropertyGridInterface.hpp"
#include "wxPropertyGrid.hpp"
#include "wxPropertyGridPage.hpp"
#include "wxPropertyGridManager.hpp"

VALUE rb_mWXPropertyGridInterface;

#if wxUSE_PROPGRID
// i need this because if i try to access the interface directly it fails
template <>
wxPropertyGridInterface* unwrap< wxPropertyGridInterface* >(const VALUE &obj)
{
 if(rb_obj_is_kind_of(obj,rb_cWXPropertyGrid))
	 return unwrap<wxPropertyGrid*>(obj);
 if(rb_obj_is_kind_of(obj,rb_cWXPropertyGridPage))
	 return unwrap<wxPropertyGridPage*>(obj);
 if(rb_obj_is_kind_of(obj,rb_cWXPropertyGridManager))
	 return unwrap<wxPropertyGridManager*>(obj);

 return NULL;
}

#define _self unwrap<wxPropertyGridInterface*>(self)

namespace RubyWX {
namespace PropertyGridInterface {

singlefunc(Clear)
singlereturn(CollapseAll)
singlereturn(ExpandAll)
singlereturn(GetSelection)

DLL_LOCAL VALUE _append(int argc,VALUE *argv,VALUE self)
{
	VALUE val,hash;
	rb_scan_args(argc, argv, "11",&val,&hash);
	
	VALUE result = wrap(_self->Append(unwrap<wxPGProperty*>(val)));
	if(rb_block_given_p())
		rb_yield(result);
	return result;
}

}
}
#endif

DLL_LOCAL void Init_WXPropertyGridInterface(VALUE rb_mWX)
{
#if wxUSE_PROPGRID
	using namespace RubyWX::PropertyGridInterface;
	rb_mWXPropertyGridInterface = rb_define_module_under(rb_mWX,"PropertyGridInterface");

	rb_define_method(rb_mWXPropertyGridInterface,"append",RUBY_METHOD_FUNC(_append),-1);

	rb_define_method(rb_mWXPropertyGridInterface,"clear",RUBY_METHOD_FUNC(_Clear),0);

	rb_define_method(rb_mWXPropertyGridInterface,"collapse_all",RUBY_METHOD_FUNC(_CollapseAll),0);
	rb_define_method(rb_mWXPropertyGridInterface,"expand_all",RUBY_METHOD_FUNC(_ExpandAll),0);

	rb_define_method(rb_mWXPropertyGridInterface,"selection",RUBY_METHOD_FUNC(_GetSelection),0);

	wxPropertyGrid::InitAllTypeHandlers();
#endif

}



