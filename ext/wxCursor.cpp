/*
 * wxCursor.cpp
 *
 *  Created on: 10.03.2012
 *      Author: hanmac
 */

#include "wxCursor.hpp"
#include "wxApp.hpp"
#include "wxBitmap.hpp"
#include "wxColor.hpp"

#if wxUSE_BUSYINFO
#include <wx/busyinfo.h>
#endif

VALUE rb_cWXCursor;

template <>
VALUE wrap< wxCursor >(wxCursor *cursor )
{
	return wrapTypedPtr(cursor, rb_cWXCursor);
}

template <>
wxCursor* unwrap< wxCursor* >(const VALUE &vbitmap)
{
	if(NIL_P(vbitmap))
		return &wxNullCursor;
	if(RB_FIXNUM_P(vbitmap) || RB_SYMBOL_P(vbitmap))
	{
		return new wxCursor(unwrapenum<wxStockCursor>(vbitmap));
	}else if(rb_obj_is_kind_of(vbitmap,rb_cWXCursor))
		return unwrapTypedPtr<wxCursor>(vbitmap, rb_cWXCursor);
	else
#if wxUSE_IMAGE
	return new wxCursor(unwrap<wxImage>(vbitmap));
#else
	return &wxNullCursor;
#endif
}

template <>
wxCursor unwrap< wxCursor >(const VALUE &vbitmap)
{
	if(NIL_P(vbitmap))
		return wxNullCursor;
	return *unwrap< wxCursor* >(vbitmap);
}


DLL_LOCAL VALUE _busy(int argc,VALUE *argv,VALUE self)
{
	VALUE cursor;
	rb_scan_args(argc, argv, "01",&cursor);

	app_protected();

	wxBeginBusyCursor(NIL_P(cursor) ? wxHOURGLASS_CURSOR : unwrap<wxCursor*>(cursor));
	rb_yield(Qnil);
	wxEndBusyCursor();
	return self;
}

DLL_LOCAL VALUE _isBusy(VALUE self)
{
	app_protected();
	return wrap(wxIsBusy());
}

#if wxUSE_BUSYINFO

DLL_LOCAL VALUE _busyinfo(int argc,VALUE *argv,VALUE self)
{
	VALUE message, parent, hash;
	rb_scan_args(argc, argv, "11:",&message, &parent, &hash);

	app_protected();
#ifdef HAVE_WXBUSYINFOFLAGS
	wxBusyInfoFlags flags;
	flags.Label(unwrap<wxString>(message));
	flags.Parent(unwrap<wxWindow*>(parent));

	if(rb_obj_is_kind_of(hash,rb_cHash))
	{

		set_obj_option(hash,"icon",&wxBusyInfoFlags::Icon, flags);
		set_obj_option(hash,"title",&wxBusyInfoFlags::Title, flags);
		set_obj_option(hash,"text",&wxBusyInfoFlags::Text, flags);
		set_obj_option(hash,"label",&wxBusyInfoFlags::Label, flags);
		set_obj_option(hash,"parent",&wxBusyInfoFlags::Parent, flags);
		set_obj_option(hash,"foreground",&wxBusyInfoFlags::Foreground, flags);
		set_obj_option(hash,"background",&wxBusyInfoFlags::Background, flags);
		//set_obj_option(hash,"transparency",&wxBusyInfoFlags::Transparency, flags, unwrap<unsigned int>);

	}

	wxBusyInfo info(flags);
#else
	wxBusyInfo info(unwrap<wxString>(message),unwrap<wxWindow*>(parent));
#endif
	rb_yield(Qnil);
	return self;
}
#endif


DLL_LOCAL void Init_WXCursor(VALUE rb_mWX)
{
	rb_cWXCursor = rb_define_class_under(rb_mWX,"Cursor",rb_cObject);
	rb_undef_alloc_func(rb_cWXCursor);

	rb_define_module_function(rb_mWX,"busy",RUBY_METHOD_FUNC(_busy),-1);
	rb_define_module_function(rb_mWX,"busy?",RUBY_METHOD_FUNC(_isBusy),0);
#if wxUSE_BUSYINFO
	rb_define_module_function(rb_mWX,"busy_info",RUBY_METHOD_FUNC(_busyinfo),-1);
#endif
	registerType<wxCursor>(rb_cWXCursor);

	registerEnum<wxStockCursor>("WX::StockCursor")
		->add(wxCURSOR_NONE,"none")
		->add(wxCURSOR_ARROW,"arrow")
		->add(wxCURSOR_RIGHT_ARROW,"right_arrow")
		->add(wxCURSOR_BULLSEYE,"bullseye")
		->add(wxCURSOR_CHAR,"char")
		->add(wxCURSOR_CROSS,"cross")
		->add(wxCURSOR_HAND,"hand")
		->add(wxCURSOR_IBEAM,"ibeam")
		->add(wxCURSOR_LEFT_BUTTON,"left_button")
		->add(wxCURSOR_MAGNIFIER,"magnifier")
		->add(wxCURSOR_MIDDLE_BUTTON,"middle_button")
		->add(wxCURSOR_NO_ENTRY,"no_entry")
		->add(wxCURSOR_PAINT_BRUSH,"paint_brush")
		->add(wxCURSOR_PENCIL,"pencil")
		->add(wxCURSOR_POINT_LEFT,"point_left")
		->add(wxCURSOR_POINT_RIGHT,"point_right")
		->add(wxCURSOR_QUESTION_ARROW,"question_arrow")
		->add(wxCURSOR_RIGHT_BUTTON,"right_button")
		->add(wxCURSOR_SIZENESW,"sizenesw")
		->add(wxCURSOR_SIZENS,"sizens")
		->add(wxCURSOR_SIZENWSE,"sizenwse")
		->add(wxCURSOR_SIZEWE,"sizenewe")
		->add(wxCURSOR_SIZING,"sizing")
		->add(wxCURSOR_SPRAYCAN,"spraycan")
		->add(wxCURSOR_WAIT,"wait")
		->add(wxCURSOR_WATCH,"watch")
		->add(wxCURSOR_BLANK,"blank")
		->add(wxCURSOR_ARROWWAIT,"arrow_wait");

}
