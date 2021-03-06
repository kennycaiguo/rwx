/*
 * wxBitmap.cpp
 *
 *  Created on: 16.02.2012
 *      Author: hanmac
 */

#include "wxApp.hpp"
#include "wxBitmap.hpp"
#include "wxSize.hpp"
#include "wxRect.hpp"
#include "wxColor.hpp"
#include "wxPalette.hpp"
#include "wxWindow.hpp"
#include "wxDC.hpp"
#include <map>
#include <wx/artprov.h>

#include <wx/colour.h>

#include <wx/rawbmp.h>

#if wxUSE_MIMETYPE
#include <wx/mimetype.h>
#endif

#define _self unwrap<wxBitmap*>(self)

VALUE rb_cWXBitmap, rb_cWXMask;

typedef std::map<wxWindowID,wxArtID> WindowArt;
WindowArt windowArtHolder;
typedef std::map<ID,wxArtID> RubyArt;
RubyArt rubyArtHolder;

void registerWindowArtID(wxWindowID wid, const wxArtID& artid)
{
	windowArtHolder.insert(std::make_pair(wid,artid));
}

void registerArtID(const char * name,const wxArtID& artid,wxWindowID wid = wxID_NONE)
{
	if(wid != wxID_NONE)
		registerWindowArtID(wid, artid);
	rubyArtHolder.insert(std::make_pair(rb_intern(name),artid));
}


template <>
VALUE wrap< wxBitmap >(const wxBitmap &vbitmap)
{
	if(!vbitmap.IsOk())
		return Qnil;
	return wrap(new wxBitmap(vbitmap));
}

template <>
VALUE wrap< wxBitmap >(wxBitmap *bitmap )
{
	if(!bitmap || !bitmap->IsOk() || bitmap == &wxNullBitmap)
		return Qnil;
	return wrapTypedPtr(bitmap,rb_cWXBitmap);
}

template <>
wxBitmap* unwrap< wxBitmap* >(const VALUE &vbitmap)
{
	if(NIL_P(vbitmap))
		return &wxNullBitmap;
	if(rb_obj_is_kind_of(vbitmap,rb_cWXBitmap))
		return unwrapTypedPtr<wxBitmap>(vbitmap, rb_cWXBitmap);
#if wxUSE_IMAGE
	if(rb_obj_is_kind_of(vbitmap,rb_cWXImage))
		return new wxBitmap(unwrap<wxImage>(vbitmap));
#endif
	if(is_wrapable<wxSize>(vbitmap))
		return new wxBitmap(unwrap<wxSize>(vbitmap));
	else
		return new wxBitmap(unwrap<wxString>(vbitmap),wxBITMAP_TYPE_ANY);
}

template <>
wxBitmap unwrap< wxBitmap >(const VALUE &vbitmap)
{
	return *unwrap<wxBitmap*>(vbitmap);
}

template <>
wxBitmap& unwrap< wxBitmap& >(const VALUE &vbitmap)
{
	return *unwrap< wxBitmap* >(vbitmap);
}


template <>
VALUE wrap< wxIcon >(wxIcon *icon )
{
	if(!icon || !icon->IsOk() || icon == &wxNullIcon)
		return Qnil;
	return wrap< wxBitmap >(wxBitmap(*icon));
}
template <>
VALUE wrap< wxIcon >(const wxIcon &icon )
{
	if(!icon.IsOk() || &icon == &wxNullIcon)
		return Qnil;
	return wrap< wxBitmap >(new wxBitmap(icon));
}

template <>
wxIcon* unwrap< wxIcon* >(const VALUE &vbitmap)
{
	if(NIL_P(vbitmap))
		return &wxNullIcon;
	wxIcon *icon = new wxIcon();
	icon->CopyFromBitmap(unwrap<wxBitmap>(vbitmap));
	return icon;
}

template <>
wxIcon unwrap< wxIcon >(const VALUE &vbitmap)
{
	return *unwrap<wxIcon*>(vbitmap);
}


namespace RubyWX {
namespace Bitmap {

macro_attr(Height,int)
macro_attr(Width,int)
macro_attr(Depth,int)

singlereturn(GetMask)
singlereturn(GetSize)

singlereturn(GetScaleFactor)
singlereturn(GetScaledWidth)
singlereturn(GetScaledHeight)
singlereturn(GetScaledSize)

singlereturn(HasAlpha)

#if wxUSE_PALETTE
macro_attr(Palette,wxPalette)
#endif

DLL_LOCAL VALUE _setSize(VALUE self, VALUE val) {
	rb_check_frozen(self);
	wxSize size(unwrap<wxSize>(val));
	_self->SetWidth(size.GetWidth());
	_self->SetHeight(size.GetHeight());
	return val;
}

DLL_LOCAL VALUE _setMask(VALUE self, VALUE val) {
	rb_check_frozen(self);
	if(rb_obj_is_kind_of(val, rb_cWXMask))
		_self->SetMask(unwrap<wxMask*>(val));
	else if(is_wrapable<wxColor>(val))
		_self->SetMask(new wxMask(*_self, unwrap<wxColor>(val)));
#if wxUSE_PALETTE
	else if(RB_FIXNUM_P(val))
		_self->SetMask(new wxMask(*_self, RB_NUM2INT(val)));
#endif
	else
		_self->SetMask(new wxMask(unwrap<wxBitmap>(val)));
	return val;
}

DLL_LOCAL VALUE _alloc(VALUE self) {
	return wrapTypedPtr(new wxBitmap, self);
}

/*
 * call-seq:
 *   draw { | dc | } -> self
 *
 * create a DC for drawing in the bitmap.
 *
 */
DLL_LOCAL VALUE _draw(VALUE self)
{
	app_protected();
	rb_check_frozen(self);

	wxDC *dc;
	wxMemoryDC *mdc = new wxMemoryDC(*_self);
#if wxUSE_GRAPHICS_CONTEXT
	dc = new wxGCDC(*mdc);
#else
	dc = mdc;
#endif
	rb_yield(wrap(dc));
	mdc->SelectObject(wxNullBitmap);

	//TODO add a way to delete the DCs again
	return self;
}

/*
 * call-seq:
 *   Bitmap.new(path)
 *   Bitmap.new(height. width. opts)
 *
 * creates a new bitmap object.
 * ===Arguments
 * * path String path to file
 * * height unsigned Integer
 * * width unsigned Integer
 * * opts: Hash with possible options to set:
 *   * depth Integer
 *   * scale Float
 * === Exceptions
 * [ArgumentError]
 * * if height, width or scale are negative or zero
 */
DLL_LOCAL VALUE _initialize(int argc,VALUE *argv,VALUE self)
{
	VALUE x, y, opts;
	rb_scan_args(argc, argv, "11:",&x,&y, &opts);

	if(NIL_P(x)){
		_self->LoadFile(unwrap<wxString>(x),wxBITMAP_TYPE_ANY);
	}else {

		int cdepth(wxBITMAP_SCREEN_DEPTH);
		if(rb_obj_is_kind_of(opts, rb_cHash)) {
			set_hash_option(opts, "depth", cdepth);
		}

		int width = RB_NUM2INT(x);
		int height = RB_NUM2INT(y);

		double cscale(1.0);

		if(check_negative_size(width, height)) {
			if(rb_obj_is_kind_of(opts, rb_cHash) && set_hash_option(opts, "scale", cscale)) {
				if(cscale <= 0.0) {
					rb_raise(rb_eArgError, "scale cant be negative or zero");
					return self;
				}
				_self->CreateScaled(width, height, cdepth, cscale);
			} else {
				_self->Create(width, height, cdepth);
			}
		}


		// TODO need better way to init the Data
		if(_self->HasAlpha()) {

			wxAlphaPixelData data(*_self);

			wxAlphaPixelData::Iterator p(data);

			for ( int cy = 0; cy < data.GetHeight(); ++cy )
			{
				wxAlphaPixelData::Iterator rowStart = p;

				for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
				{
					p.Red() = p.Green() = p.Blue() = p.Alpha() = 0;
				}

				p = rowStart;
				p.OffsetY(data, 1);
			}
		}else {
			wxNativePixelData data(*_self);

			wxNativePixelData::Iterator p(data);

			for ( int cy = 0; cy < data.GetHeight(); ++cy )
			{
				wxNativePixelData::Iterator rowStart = p;

				for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
				{
					p.Red() = p.Green() = p.Blue() = 0;
				}

				p = rowStart;
				p.OffsetY(data, 1);
			}

		}

	}
	return self;
}

/* Document-method: initialize_copy
 * call-seq:
 *   initialize_copy(orig)
 *
 * Duplicate an object
*/
DLL_LOCAL VALUE _initialize_copy(VALUE self,VALUE other)
{
	wxBitmap cbitmap = unwrap<wxBitmap>(other);

	(*_self) = cbitmap.GetSubBitmap(cbitmap.GetSize());

	return self;
}


#if wxUSE_IMAGE
/*
 * call-seq:
 *   to_image -> WX::Image
 *
 * convert to WX::Image
 *
 */
DLL_LOCAL VALUE _to_image(VALUE self)
{
	return wrap(_self->ConvertToImage());
}
#endif


/*
 * call-seq:
 *   bitmap.scale(size) -> WX::Bitmap
 *   bitmap.scale(x_ratio, y_ratio) -> WX::Bitmap
 *
 * returns a new scaled bitmap
 * ===Arguments
 * * size WX::Size
 * * x_ratio and y_ratio are Float
 * ===Return value
 * WX::Bitmap
 * === Exceptions
 * [ArgumentError]
 * * size is invalid or x_ratio or y_ratio are negative or zero
*/
DLL_LOCAL VALUE _scale(int argc,VALUE *argv,VALUE self)
{
	VALUE x_scale, y_scale;
	rb_scan_args(argc, argv, "11",&x_scale,&y_scale);

	int width, height;
	if(NIL_P(y_scale))
	{
		wxSize size;
		if(!check_negative_size(x_scale,size))
			return Qnil;
		width = size.GetWidth();
		height = size.GetHeight();
	} else {
		width = NUM2DBL(x_scale) * _self->GetWidth();
		height = NUM2DBL(y_scale) * _self->GetHeight();
		if(!check_negative_size(width,height))
			return Qnil;
	}

#if wxUSE_IMAGE
	wxImage img = _self->ConvertToImage();
	img.Rescale(width, height);
	return wrapTypedPtr(new wxBitmap(img, _self->GetDepth()), rb_class_of(self));
#else // !wxUSE_IMAGE
	// Fallback method of scaling the bitmap
	wxBitmap newBmp(width, height, _self->GetDepth());
#if defined(__WXMSW__) || defined(__WXOSX__)
	// wxBitmap::UseAlpha() is used only on wxMSW and wxOSX.
	newBmp.UseAlpha(_self->HasAlpha());
#endif // __WXMSW__ || __WXOSX__
	{
		wxMemoryDC dc(newBmp);
		double scX = (double)width / _self->GetWidth();
		double scY = (double)height / _self->GetHeight();
		dc.SetUserScale(scX, scY);
		dc.DrawBitmap(*_self, 0, 0);
	}
	return wrapTypedPtr(new wxBitmap(newBmp), rb_class_of(self));
#endif // !wxUSE_IMAGE
}

/*
 * call-seq:
 *   bitmap.scale!(size) -> self
 *   bitmap.scale!(x_ratio, y_ratio) -> self
 *
 * scaled this bitmap, return self
 * ===Arguments
 * * size WX::Size
 * * x_ratio and y_ratio are Float
 * ===Return value
 * self
 * === Exceptions
 * [ArgumentError]
 * * size is invalid or x_ratio or y_ratio are negative or zero
*/
DLL_LOCAL VALUE _scale_self(int argc,VALUE *argv,VALUE self)
{
	VALUE x_scale, y_scale;
	rb_scan_args(argc, argv, "11",&x_scale,&y_scale);

	rb_check_frozen(self);

	int width, height;
	if(NIL_P(y_scale))
	{
		wxSize size;
		if(!check_negative_size(x_scale,size))
			return self;
		width = size.GetWidth();
		height = size.GetHeight();
	} else {
		width = NUM2DBL(x_scale) * _self->GetWidth();
		height = NUM2DBL(y_scale) * _self->GetHeight();
		if(!check_negative_size(width,height))
			return self;
	}

#if wxUSE_IMAGE
	wxImage img = _self->ConvertToImage();
	img.Rescale(width, height);
	(*_self) = wxBitmap(img, _self->GetDepth());
#else // !wxUSE_IMAGE
	// Fallback method of scaling the bitmap
	wxBitmap newBmp(width, height, _self->GetDepth());
#if defined(__WXMSW__) || defined(__WXOSX__)
	// wxBitmap::UseAlpha() is used only on wxMSW and wxOSX.
	newBmp.UseAlpha(_self->HasAlpha());
#endif // __WXMSW__ || __WXOSX__
	{
		wxMemoryDC dc(newBmp);
		double scX = (double)width / _self->GetWidth();
		double scY = (double)height / _self->GetHeight();
		dc.SetUserScale(scX, scY);
		dc.DrawBitmap(*_self, 0, 0);
	}
	(*_self) = newBmp;
#endif // !wxUSE_IMAGE
	return self;
}

/*
 * call-seq:
 *   bitmap.sub_image(rect) -> WX::Bitmap or nil
 *
 * return a sub bitmap of the given place
 * ===Arguments
 * * rect is a WX::Rect
 * ===Return value
 * WX::Bitmap or nil
 * === Exceptions
 * [ArgumentError]
 * * rect does have negative size
 * * rect does not fit into the Size of the Image
*/
DLL_LOCAL VALUE _getSubBitmap(VALUE self, VALUE vrect)
{
	if(_self->IsOk())
	{
		wxRect rect;
		wxSize size;

		if(!check_negative_size(vrect, size))
			return Qnil;

		if(!check_contain_rect(_GetSize(self), _self->GetSize(), vrect, rect))
			return Qnil;

		return wrap(_self->GetSubBitmap(rect));
	}
	return Qnil;
}


/*
 * call-seq:
 *   hash -> Fixnum
 *
 * Generates a Fixnum hash value for this object.
 *
 */
DLL_LOCAL VALUE _getHash(VALUE self)
{
	st_index_t h = rb_hash_start(0);

	h = rb_hash_uint(h, _self->GetWidth());
	h = rb_hash_uint(h, _self->GetHeight());
	h = rb_hash_uint(h, _self->GetDepth());

	if(_self->HasAlpha()) {
		wxAlphaPixelData data(*_self);

		wxAlphaPixelData::Iterator p(data);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxAlphaPixelData::Iterator rowStart = p;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
			{
				h = rb_hash_uint32(h, p.Red() | (p.Green() << 8) | (p.Blue() << 16) | (p.Alpha() << 24) );
			}

			p = rowStart;
			p.OffsetY(data, 1);
		}
	} else {
		wxNativePixelData data(*_self);

		wxNativePixelData::Iterator p(data);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxNativePixelData::Iterator rowStart = p;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
			{
				h = rb_hash_uint32(h, p.Red() | (p.Green() << 8) | (p.Blue() << 16) );
			}

			p = rowStart;
			p.OffsetY(data, 1);
		}

	}

	h = rb_hash_end(h);
	return RB_LONG2FIX(h);
}

/*
 * call-seq:
 *   marshal_dump -> Array
 *
 * Provides marshalling support for use by the Marshal library.
 * ===Return value
 * Array
 */
DLL_LOCAL VALUE _marshal_dump(VALUE self)
{

	VALUE result = rb_ary_new();

	rb_ary_push(result, _GetScaledWidth(self));
	rb_ary_push(result, _GetScaledHeight(self));
	rb_ary_push(result, _getDepth(self));
	rb_ary_push(result, _GetScaleFactor(self));

	int cheight = _self->GetHeight();
	int cwidth = _self->GetWidth();

	if(_self->HasAlpha()) {
		wxColourBase::ChannelType color_data[cheight * cwidth * 3];
		wxColourBase::ChannelType alpha_data[cheight * cwidth];

		wxAlphaPixelData data(*_self);

		wxAlphaPixelData::Iterator p(data);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxAlphaPixelData::Iterator rowStart = p;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
			{
				color_data[(cy * data.GetWidth() + cx) * 3] = p.Red();
				color_data[(cy * data.GetWidth() + cx) * 3 + 1] = p.Green();
				color_data[(cy * data.GetWidth() + cx) * 3 + 2] = p.Blue();

				alpha_data[cy * data.GetWidth() + cx] = p.Alpha();

			}

			p = rowStart;
			p.OffsetY(data, 1);
		}
		rb_ary_push(result, rb_str_new((const char*)color_data, cheight * cwidth * 3));
		rb_ary_push(result, rb_str_new((const char*)alpha_data, cheight * cwidth));

	} else {
		wxColourBase::ChannelType color_data[cheight * cwidth * 3];

		wxNativePixelData data(*_self);

		wxNativePixelData::Iterator p(data);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxNativePixelData::Iterator rowStart = p;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
			{
				color_data[(cy * data.GetWidth() + cx) * 3] = p.Red();
				color_data[(cy * data.GetWidth() + cx) * 3 + 1] = p.Green();
				color_data[(cy * data.GetWidth() + cx) * 3 + 2] = p.Blue();
			}

			p = rowStart;
			p.OffsetY(data, 1);
		}
		rb_ary_push(result, rb_str_new((const char*)color_data, cheight * cwidth * 3));
		rb_ary_push(result, Qnil);

	}

	return result;
}

/*
 * call-seq:
 *   marshal_load(array) -> nil
 *
 * Provides marshalling support for use by the Marshal library.
 *
 *
 */
DLL_LOCAL VALUE _marshal_load(VALUE self,VALUE data)
{
	data = rb_Array(data);

	_self->CreateScaled(
		RB_NUM2UINT(RARRAY_AREF(data,0)),
		RB_NUM2UINT(RARRAY_AREF(data,1)),
		RB_NUM2INT(RARRAY_AREF(data,2)),
		NUM2DBL(RARRAY_AREF(data,3))
	);


	if(_self->HasAlpha()) {
		VALUE val = RARRAY_AREF(data,4);
		wxColourBase::ChannelType *color_data = (wxColourBase::ChannelType*)StringValuePtr(val);

		val = RARRAY_AREF(data,5);
		wxColourBase::ChannelType *alpha_data = (wxColourBase::ChannelType*)StringValuePtr(val);

		wxAlphaPixelData data(*_self);

		wxAlphaPixelData::Iterator p(data);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxAlphaPixelData::Iterator rowStart = p;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
			{
				p.Red() = color_data[(cy * data.GetWidth() + cx) * 3];
				p.Green() = color_data[(cy * data.GetWidth() + cx) * 3 + 1];
				p.Blue() = color_data[(cy * data.GetWidth() + cx) * 3 + 2];

				p.Alpha() = alpha_data[cy * data.GetWidth() + cx];

			}

			p = rowStart;
			p.OffsetY(data, 1);
		}

	} else {
		VALUE val = RARRAY_AREF(data,4);
		wxColourBase::ChannelType *color_data = (wxColourBase::ChannelType*)StringValuePtr(val);

		wxNativePixelData data(*_self);

		wxNativePixelData::Iterator p(data);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxNativePixelData::Iterator rowStart = p;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p )
			{
				p.Red() = color_data[(cy * data.GetWidth() + cx) * 3];
				p.Green() = color_data[(cy * data.GetWidth() + cx) * 3 + 1];
				p.Blue() = color_data[(cy * data.GetWidth() + cx) * 3 + 2];
			}

			p = rowStart;
			p.OffsetY(data, 1);
		}
	}
	return self;
}

/*
 * call-seq:
 *   to_bitmap -> WX::Bitmap
 *
 * return self
 *
 */
DLL_LOCAL VALUE _to_bitmap(VALUE self)
{
	return self;
}

DLL_LOCAL VALUE _save_file(int argc,VALUE *argv,VALUE self)
{
	VALUE name;
	rb_scan_args(argc, argv, "10",&name);
	return wrap(_self->SaveFile(unwrap<wxString>(name),wxBITMAP_TYPE_PNG));
}


struct equal_obj {
	wxBitmap* self;
	VALUE other;
};

VALUE _equal_block(equal_obj *obj)
{
	wxBitmap cbitmap = unwrap<wxBitmap>(obj->other);
	if(obj->self->IsSameAs(cbitmap))
		return Qtrue;

	if(obj->self->GetWidth() != cbitmap.GetWidth())
		return Qfalse;
	if(obj->self->GetHeight() != cbitmap.GetHeight())
		return Qfalse;
	if(obj->self->GetDepth() != cbitmap.GetDepth())
		return Qfalse;

	if(obj->self->HasAlpha()) {
		wxAlphaPixelData data(*obj->self);
		wxAlphaPixelData cdata(cbitmap);

		wxAlphaPixelData::Iterator p(data);
		wxAlphaPixelData::Iterator cp(cdata);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxAlphaPixelData::Iterator rowStart = p;
			wxAlphaPixelData::Iterator crowStart = cp;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p, ++cp )
			{
				if(p.Red() != cp.Red()) {
					return Qfalse;
				}
				if(p.Green() != cp.Green()) {
					return Qfalse;
				}
				if(p.Blue() != cp.Blue()) {
					return Qfalse;
				}
				if(p.Alpha() != cp.Alpha()) {
					return Qfalse;
				}
			}

			p = rowStart;
			cp = crowStart;
			p.OffsetY(data, 1);
			cp.OffsetY(cdata, 1);
		}

	} else {
		wxNativePixelData data(*obj->self);
		wxNativePixelData cdata(cbitmap);

		wxNativePixelData::Iterator p(data);
		wxNativePixelData::Iterator cp(cdata);

		for ( int cy = 0; cy < data.GetHeight(); ++cy )
		{
			wxNativePixelData::Iterator rowStart = p;
			wxNativePixelData::Iterator crowStart = cp;

			for ( int cx = 0; cx < data.GetWidth(); ++cx, ++p, ++cp )
			{
				if(p.Red() != cp.Red()) {
					return Qfalse;
				}
				if(p.Green() != cp.Green()) {
					return Qfalse;
				}
				if(p.Blue() != cp.Blue()) {
					return Qfalse;
				}
			}

			p = rowStart;
			cp = crowStart;
			p.OffsetY(data, 1);
			cp.OffsetY(cdata, 1);
		}

	}
	return Qtrue;

}

/*
 * call-seq:
 *   == brush -> bool
 *
 * compares two brush.
 *
 *
 */
DLL_LOCAL VALUE _equal(VALUE self, VALUE other)
{
	equal_obj obj;
	obj.self = _self;
	obj.other = other;

	return rb_rescue(
		RUBY_METHOD_FUNC(_equal_block),(VALUE)&obj,
		RUBY_METHOD_FUNC(_equal_rescue),Qnil
	);
}

}
}

#if wxUSE_MIMETYPE
DLL_LOCAL VALUE _getIconfromFileType(wxFileType *type)
{
	wxIconLocation iconloc;
	if(type->GetIcon(&iconloc)) {
		return wrap(new wxIcon(iconloc));
	}
	return Qnil;
}

/*
 * call-seq:
 *   from_extension(ext) -> WX::Bitmap
 *
 * returns the icon from the given file extension
 * ===Arguments
 * * ext String file extension
 * ===Return value
 * WX::Bitmap
 *
 */
DLL_LOCAL VALUE _getIconfromFromExtension(VALUE self, VALUE ext)
{
	app_protected();
	wxMimeTypesManager *man = wxTheMimeTypesManager;

	if(!man) {
		man = new wxMimeTypesManager;
	}

	wxFileType *type = man->GetFileTypeFromExtension(unwrap<wxString>(ext));
	if(type) {
		VALUE result = _getIconfromFileType(type);
		delete type;
		return result;
	}

	return Qnil;
}

/*
 * call-seq:
 *   from_mimetype(mimetype) -> WX::Bitmap
 *
 * returns the icon from the given mimetype
 * ===Arguments
 * * mimetype String MimeType
 * ===Return value
 * WX::Bitmap
 *
 */
DLL_LOCAL VALUE _getIconfromFromMimeType(VALUE self, VALUE ext)
{
	app_protected();
	wxMimeTypesManager *man = wxTheMimeTypesManager;

	if(!man) {
		man = new wxMimeTypesManager;
	}

	wxFileType *type = man->GetFileTypeFromMimeType(unwrap<wxString>(ext));
	if(type) {
		VALUE result = _getIconfromFileType(type);
		delete type;
		return result;
	}

	return Qnil;
}

#endif

DLL_LOCAL wxArtID get_art_from_id(wxWindowID id)
{
	wxString result;
	WindowArt::iterator it = windowArtHolder.find(id);

	if(it == windowArtHolder.end())
	{
	#ifdef __WXGTK20__
		if(wxIsStockID(id)) {
			const char *bitmap = wxGetStockGtkID(id);
			if(bitmap) {
				result = wxString(bitmap);
			}
		}
	#endif
	}	else {
		result = it->second;
	}
	return result;
}

DLL_LOCAL wxArtID get_art_from_sym(ID id)
{
	wxString artid;
	RubyArt::iterator it = rubyArtHolder.find(id);
	if(it != rubyArtHolder.end())
		artid = it->second;

	return artid;
}

DLL_LOCAL wxArtClient get_client_from_sym(ID id)
{
	if(id == rb_intern("toolbar"))
		return wxART_TOOLBAR;
	else if(id == rb_intern("menu"))
		return wxART_MENU;
	else if(id == rb_intern("frame_icon"))
		return wxART_FRAME_ICON;
	else if(id == rb_intern("cmn_dialog"))
		return wxART_CMN_DIALOG;
	else if(id == rb_intern("help_browser"))
		return wxART_HELP_BROWSER;
	else if(id == rb_intern("message_box"))
		return wxART_MESSAGE_BOX;
	else if(id == rb_intern("button"))
		return wxART_BUTTON;
	else if(id == rb_intern("list"))
		return wxART_LIST;
	else
		return wxART_OTHER;
}

/*
 * call-seq:
 *   art_from_id(id) -> String
 *
 * returns the Art String for a given ID
 * ===Arguments
 * * path String path to file
 *
 */
DLL_LOCAL VALUE _getArtFromID(VALUE self, VALUE id)
{
	wxArtID result = get_art_from_id(unwrapID(id));
	return result != wxEmptyString ? wrap(result) : Qnil;
}

/*
 * call-seq:
 *   from_id(id, [client], [size]) -> WX::Bitmap
 *
 * returns the bitmap for a given ID
 * ===Arguments
 * * id Symbol of bitmap
 * * client Symbol
 * * size WX::Size
 *
 */
DLL_LOCAL VALUE _getBitmapFromID(int argc,VALUE *argv,VALUE self)
{
	VALUE id, client, size;
	rb_scan_args(argc, argv, "12",&id, &client, &size);

	wxArtClient artclient = wxART_OTHER;

	wxArtID artid = get_art_from_id(unwrapID(id));

	if(RB_SYMBOL_P(client))
		 artclient = get_client_from_sym(RB_SYM2ID(client));

	wxSize csize = NIL_P(size) ? wxDefaultSize : unwrap<wxSize>(size);
	if(artid == wxEmptyString)
		return Qnil;
	return wrap(wxArtProvider::GetBitmap(artid,artclient,csize));
}

/*
 * call-seq:
 *   from_provider(name, [client], [size]) -> WX::Bitmap
 *
 * returns the bitmap for a given name
 * ===Arguments
 * * name Symbol
 * * client Symbol
 * * size WX::Size
 *
 */
DLL_LOCAL VALUE _getBitmapProvider(int argc,VALUE *argv,VALUE self)
{
	VALUE id, client, size;
	rb_scan_args(argc, argv, "12",&id, &client, &size);

	wxArtClient artclient = wxART_OTHER;

	wxArtID artid =	RB_SYMBOL_P(client) ? get_art_from_sym(RB_SYM2ID(client)) : unwrap<wxString>(id);

	if(RB_SYMBOL_P(client))
		 artclient = get_client_from_sym(RB_SYM2ID(client));

	wxSize csize = NIL_P(size) ? wxDefaultSize : unwrap<wxSize>(size);
	if(artid == wxEmptyString)
		return Qnil;
	return wrap(wxArtProvider::GetBitmap(artid,artclient,csize));
}


wxBitmap wrapBitmap(const VALUE &vbitmap,wxWindowID id,wrapBitmapType type,const wxArtClient &client)
{
	wxArtID artid;
	bool useid = false;
	if(NIL_P(vbitmap))
	{
		if(type == WRAP_BITMAP_NULL)
			return wxNullBitmap;
		else
		{
			artid = get_art_from_id(id);
			useid = true;
		}
	}else if(RB_SYMBOL_P(vbitmap))
	{
		artid = get_art_from_sym(RB_SYM2ID(vbitmap));
		useid = true;
	}

	if(useid) {
		if(artid != wxEmptyString) {
			return wxArtProvider::GetBitmap(artid, client);
		} else if(type == WRAP_BITMAP_RAISE)
			rb_raise(rb_eArgError,"need an valid bitmap");
		return wxNullBitmap;
	}

	wxBitmap temp = wxArtProvider::GetBitmap(unwrap<wxString>(vbitmap),client);
	if(temp.IsOk())
		return temp;
	return *unwrap<wxBitmap*>(vbitmap);
}


/* Document-attr: width
* returns the width of the Bitmap. Integer
*/
/* Document-attr: height
* returns the height of the Bitmap. Integer
*/
/* Document-attr: depth
* returns the depth of the Bitmap. Integer
*/
/* Document-attr: size
* returns the size of the Bitmap. WX::Size
*/
/* Document-attr: mask
* returns the mask color of the Bitmap. WX::Mask
*/
/* Document-attr: palette
* returns the color palette of the Bitmap. WX::Palette
*/

/* Document-attr: scale_factor
* returns the scale factor of the Bitmap. Float
*/
/* Document-attr: scaled_width
* returns the scaled width of the Bitmap. Integer
*/
/* Document-attr: scaled_height
* returns the scaled height of the Bitmap. Integer
*/
/* Document-attr: sscaled_ize
* returns the scaled size of the Bitmap. WX::Size
*/

DLL_LOCAL void Init_WXBitmap(VALUE rb_mWX)
{
	wxBitmap::InitStandardHandlers();

	using namespace RubyWX::Bitmap;
	rb_cWXBitmap = rb_define_class_under(rb_mWX,"Bitmap",rb_cObject);
	rb_define_alloc_func(rb_cWXBitmap,_alloc);

#if 0
	rb_define_attr(rb_cWXBitmap,"width",1,1);
	rb_define_attr(rb_cWXBitmap,"height",1,1);
	rb_define_attr(rb_cWXBitmap,"depth",1,1);
	rb_define_attr(rb_cWXBitmap,"mask",1,1);
	rb_define_attr(rb_cWXBitmap,"palette",1,1);
#endif

	rb_define_method(rb_cWXBitmap,"initialize",RUBY_METHOD_FUNC(_initialize),-1);
	rb_define_private_method(rb_cWXBitmap,"initialize_copy",RUBY_METHOD_FUNC(_initialize_copy),1);

	rb_define_attr_method(rb_cWXBitmap,"height",_getHeight,_setHeight);
	rb_define_attr_method(rb_cWXBitmap,"width",_getWidth,_setWidth);
	rb_define_attr_method(rb_cWXBitmap,"depth",_getDepth,_setDepth);
	rb_define_attr_method(rb_cWXBitmap,"size",_GetSize,_setSize);

	rb_define_attr_method(rb_cWXBitmap,"mask",_GetMask,_setMask);


	rb_define_method(rb_cWXBitmap,"scale_factor",RUBY_METHOD_FUNC(_GetScaleFactor),0);
	rb_define_method(rb_cWXBitmap,"scaled_height",RUBY_METHOD_FUNC(_GetScaledHeight),0);
	rb_define_method(rb_cWXBitmap,"scaled_width",RUBY_METHOD_FUNC(_GetScaledWidth),0);
	rb_define_method(rb_cWXBitmap,"scaled_size",RUBY_METHOD_FUNC(_GetScaledSize),0);

#if wxUSE_IMAGE
	rb_define_method(rb_cWXBitmap,"to_image",RUBY_METHOD_FUNC(_to_image),0);
#endif

	rb_define_method(rb_cWXBitmap,"alpha?",RUBY_METHOD_FUNC(_HasAlpha),0);

	rb_define_method(rb_cWXBitmap,"marshal_dump",RUBY_METHOD_FUNC(_marshal_dump),0);
	rb_define_method(rb_cWXBitmap,"marshal_load",RUBY_METHOD_FUNC(_marshal_load),1);

	rb_define_method(rb_cWXBitmap,"hash",RUBY_METHOD_FUNC(_getHash),0);

	rb_define_method(rb_cWXBitmap,"==",RUBY_METHOD_FUNC(_equal),1);
	rb_define_alias(rb_cWXBitmap,"eql?","==");

#if wxUSE_PALETTE
	rb_define_attr_method(rb_cWXBitmap,"palette",_getPalette,_setPalette);
#else
	rb_define_attr_method_missing(rb_cWXBitmap,"palette");
#endif

	rb_define_method(rb_cWXBitmap,"scale",RUBY_METHOD_FUNC(_scale),-1);
	rb_define_method(rb_cWXBitmap,"scale!",RUBY_METHOD_FUNC(_scale_self),-1);

	rb_define_method(rb_cWXBitmap,"draw",RUBY_METHOD_FUNC(_draw),0);

	rb_define_method(rb_cWXBitmap,"to_bitmap",RUBY_METHOD_FUNC(_to_bitmap),0);

	rb_define_method(rb_cWXBitmap,"sub_bitmap",RUBY_METHOD_FUNC(_getSubBitmap),1);

	rb_define_method(rb_cWXBitmap,"save_file",RUBY_METHOD_FUNC(_save_file),-1);

	rb_define_singleton_method(rb_cWXBitmap,"art_from_id", RUBY_METHOD_FUNC(_getArtFromID), 1);
	rb_define_singleton_method(rb_cWXBitmap,"from_id",RUBY_METHOD_FUNC(_getBitmapFromID),-1);
	rb_define_singleton_method(rb_cWXBitmap,"from_provider",RUBY_METHOD_FUNC(_getBitmapProvider),-1);

#if wxUSE_MIMETYPE
	rb_define_singleton_method(rb_cWXBitmap,"from_extension",RUBY_METHOD_FUNC(_getIconfromFromExtension),1);
	rb_define_singleton_method(rb_cWXBitmap,"from_mimetype",RUBY_METHOD_FUNC(_getIconfromFromMimeType),1);
#endif

	registerInfo<wxBitmap>(rb_cWXBitmap);

	rb_cWXMask = rb_define_class_under(rb_mWX,"Mask",rb_cObject);
	rb_undef_alloc_func(rb_cWXMask);
	rb_undef_method(rb_cWXMask,"initialize_copy");
	rb_undef_method(rb_cWXMask,"_load");
	rb_undef_method(rb_cWXMask,"_dump");

	registerInfo<wxMask>(rb_cWXMask);

	registerArtID("open",wxART_FILE_OPEN,wxID_OPEN);
	registerArtID("save",wxART_FILE_SAVE,wxID_SAVE);
	registerArtID("save_as",wxART_FILE_SAVE_AS,wxID_SAVEAS);

	registerArtID("folder",wxART_FOLDER);
	registerArtID("folder_open",wxART_FOLDER_OPEN);


	registerArtID("copy",wxART_COPY,wxID_COPY);
#ifdef wxART_EDIT
	registerArtID("edit",wxART_EDIT,wxID_EDIT);
#endif
	registerArtID("cut",wxART_CUT,wxID_CUT);
	registerArtID("paste",wxART_PASTE,wxID_PASTE);
	registerArtID("delete",wxART_DELETE,wxID_DELETE);
	registerArtID("new",wxART_NEW,wxID_NEW);

	registerArtID("undo",wxART_UNDO,wxID_UNDO);
	registerArtID("redo",wxART_REDO,wxID_REDO);

	registerArtID("plus",wxART_PLUS);
	registerArtID("minus",wxART_MINUS);

	registerArtID("print",wxART_PRINT,wxID_PRINT);

	registerArtID("help",wxART_HELP,wxID_HELP);
	registerArtID("tip",wxART_TIP);


	registerArtID("close",wxART_CLOSE, wxID_CLOSE);
	registerArtID("quit",wxART_QUIT, wxID_EXIT);

	registerArtID("find",wxART_FIND,wxID_FIND);
	registerArtID("replace",wxART_FIND_AND_REPLACE,wxID_REPLACE);

	registerArtID("floppy",wxART_FLOPPY,wxID_FLOPPY);
	registerArtID("cdrom",wxART_CDROM,wxID_CDROM);

	registerArtID("error",wxART_ERROR);
	registerArtID("question",wxART_QUESTION);
	registerArtID("warning",wxART_WARNING);
	registerArtID("information",wxART_INFORMATION);
	registerArtID("missing_image",wxART_MISSING_IMAGE);

	registerArtID("help_side_panel",wxART_HELP_SIDE_PANEL);
	registerArtID("help_settings",wxART_HELP_SETTINGS);
	registerArtID("help_book",wxART_HELP_BOOK);
	registerArtID("help_folder",wxART_HELP_FOLDER);
	registerArtID("help_page",wxART_HELP_PAGE);

	registerArtID("go_back",wxART_GO_BACK, wxID_BACKWARD);
	registerArtID("go_forward",wxART_GO_FORWARD, wxID_FORWARD);
	registerArtID("go_up",wxART_GO_UP, wxID_UP);
	registerArtID("go_down",wxART_GO_DOWN, wxID_DOWN);
	registerArtID("go_to_parent",wxART_GO_TO_PARENT);
	registerArtID("go_home",wxART_GO_HOME, wxID_HOME);
	registerArtID("goto_first",wxART_GOTO_FIRST, wxID_FIRST);
	registerArtID("goto_last",wxART_GOTO_LAST, wxID_LAST);

#ifdef wxART_FULL_SCREEN
	registerArtID("full_screen",wxART_FULL_SCREEN);
#endif
}
