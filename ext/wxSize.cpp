/*
 * wxSize.cpp
 *
 *  Created on: 04.02.2012
 *      Author: hanmac
 */

#include "wxSize.hpp"
#include "wxRect.hpp"

VALUE rb_cWXSize;

#define _self unwrap<wxSize*>(self)

ID rwxID_width,rwxID_height;

template <>
VALUE wrap< wxSize >(wxSize *size )
{
	return wrapTypedPtr(size, rb_cWXSize);
}

template <>
bool is_wrapable< wxSize >(const VALUE &vsize)
{
	if (rb_obj_is_kind_of(vsize, rb_cWXSize)){
		return true;
	}else if(rb_respond_to(vsize,rwxID_width) &&
		rb_respond_to(vsize,rwxID_height)){
		return true;
	}else
		return false;
}

template <>
wxSize unwrap< wxSize >(const VALUE &vsize)
{
	if(rb_obj_is_kind_of(vsize, rb_cArray) && RARRAY_LEN(vsize) == 2 ){
			wxSize size;
			size.SetWidth(RB_NUM2INT(RARRAY_AREF(vsize,0)));
			size.SetHeight(RB_NUM2INT(RARRAY_AREF(vsize,1)));
			return size;
	}else if(rb_obj_is_kind_of(vsize, rb_cHash)){
		wxSize size;
		size.SetWidth(RB_NUM2INT(rb_hash_aref(vsize,RB_ID2SYM(rwxID_width))));
		size.SetHeight(RB_NUM2INT(rb_hash_aref(vsize,RB_ID2SYM(rwxID_height))));
		return size;
	}else if(rb_obj_is_kind_of(vsize, rb_cWXRect)){
		return unwrapTypedPtr<wxRect>(vsize, rb_cWXRect)->GetSize();
	}else if(!rb_obj_is_kind_of(vsize, rb_cWXSize) &&
		rb_respond_to(vsize,rwxID_width) &&
		rb_respond_to(vsize,rwxID_height)){
		wxSize size;
		size.SetWidth(RB_NUM2INT(rb_funcall(vsize,rwxID_width,0)));
		size.SetHeight(RB_NUM2INT(rb_funcall(vsize,rwxID_height,0)));
		return size;
	}else{
		return *unwrap<wxSize*>(vsize);
	}
}

bool check_negative_size(VALUE val, wxSize &size)
{
	size = unwrap<wxSize>(val);
	if(size.GetHeight() <= 0 || size.GetWidth() <= 0)
	{
		rb_raise(rb_eArgError,"%" PRIsVALUE " does have invalid size.", RB_OBJ_STRING(rb_inspect(val)));
		return false;
	}
	return true;
}

bool check_negative_size(const int &width, const int &height)
{

	if(height <= 0 || width <= 0)
	{
		rb_raise(rb_eArgError,
			"%" PRIsVALUE "(%d, %d) does have invalid size.",
			RB_CLASSNAME(rb_cWXSize),
			width, height
		);
		return false;
	}
	return true;
}

bool check_negative_size(const wxSize &size)
{
	return check_negative_size(size.GetWidth(), size.GetHeight());
}

namespace RubyWX {
namespace Size {

macro_attr(Width,int)
macro_attr(Height,int)

DLL_LOCAL VALUE _alloc(VALUE self)
{
	return wrapTypedPtr(new wxSize,self);
}

/*
 * call-seq:
 *   Size.new(width, height)
 *
 * Creates a new Size object.
*/
DLL_LOCAL VALUE _initialize(VALUE self,VALUE width,VALUE height)
{
	_setWidth(self,width);
	_setHeight(self,height);
	return self;
}


/* Document-method: initialize_copy
 * call-seq:
 *   initialize_copy(orig)
 *
 * Duplicate an object
*/
DLL_LOCAL VALUE _initialize_copy(VALUE self, VALUE other)
{
	VALUE result = rb_call_super(1,&other);
	_setWidth(self,_getWidth(other));
	_setHeight(self,_getHeight(other));
	return result;
}

/*
 * call-seq:
 *   inspect -> String
 *
 * Human-readable description.
 * ===Return value
 * String
*/
DLL_LOCAL VALUE _inspect(VALUE self)
{
	return rb_sprintf( "%s(%d, %d)",
		rb_obj_classname( self ),
		RB_NUM2INT(_getWidth(self)),
		RB_NUM2INT(_getHeight(self)));
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
    VALUE ptr[2];
    ptr[0] = _getWidth(self);
    ptr[1] = _getHeight(self);
    return rb_ary_new4(2, ptr);
}

/*
 * call-seq:
 *   marshal_load(array) -> nil
 *
 * Provides marshalling support for use by the Marshal library.
 *
 *
 */
DLL_LOCAL VALUE _marshal_load(VALUE self, VALUE data)
{
    data = rb_Array(data);
    _setWidth(self, RARRAY_AREF(data,0));
    _setHeight(self, RARRAY_AREF(data,1));
    return Qnil;
}

/*
 * call-seq:
 *   hash -> Fixnum
 *
 * Generates a Fixnum hash value for this object.
 *
 *
 */
DLL_LOCAL VALUE _getHash(VALUE self)
{
	st_index_t h = rb_hash_start(0);

	h = rb_hash_uint(h, _self->GetWidth());
	h = rb_hash_uint(h, _self->GetHeight());

	h = rb_hash_end(h);
	return RB_LONG2FIX(h);
}

struct equal_obj {
	wxSize* self;
	VALUE other;
};

VALUE _equal_block(equal_obj *obj)
{
	return wrap(*obj->self == unwrap<wxSize>(obj->other));
}

/*
 * call-seq:
 *   == size -> bool
 *
 * compares two sizes.
 * ===Arguments
 * * size is a WX::Size
 * ===Return value
 * bool
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


/*
 * call-seq:
 *   scale(x, y) -> WX::Size
 *   scale(i) -> WX::Size
 *   * i -> WX::Size
 *
 * scale this size and return new size.
 * ===Arguments
 * * x, y and i are Float
 * ===Return value
 * WX::Size
 */
DLL_LOCAL VALUE _scale(int argc,VALUE *argv,VALUE self)
{
	VALUE x, y;
	rb_scan_args(argc, argv, "11", &x, &y);

	wxSize* result = new wxSize(*_self);

	result->Scale(NUM2DBL(x), NUM2DBL(NIL_P(y) ? x : y));
	return wrapTypedPtr(result, rb_class_of(self));
}

/*
 * call-seq:
 *   scale!(x, y) -> self
 *   scale!(i) -> self
 *
 * scale this size and return new size.
 * ===Arguments
 * * x, y and i are Float
 * ===Return value
 * self
 */
DLL_LOCAL VALUE _scale_self(int argc,VALUE *argv,VALUE self)
{
	VALUE x, y;
	rb_scan_args(argc, argv, "11", &x, &y);

	rb_check_frozen(self);
	_self->Scale(NUM2DBL(x), NUM2DBL(NIL_P(y) ? x : y));
	return self;
}
/*
 * call-seq:
 *   / n -> WX::Size
 *
 * devide this size and return new size.
 * ===Arguments
 * * n are Float
 * ===Return value
 * WX::Size
 * === Exceptions
 * [ZeroDivisionError]
 * * if n is zero
 */
DLL_LOCAL VALUE _devide(VALUE self, VALUE n)
{

	float d = NUM2DBL(n);
	if(d == 0)
		rb_raise(rb_eZeroDivError, "divided by 0");

	d = 1.0 / d;

	wxSize* result = new wxSize(*_self);
	result->Scale(d, d);
	return wrapTypedPtr(result, rb_class_of(self));
}
/*
 * call-seq:
 *   inc_by(x, y) -> WX::Size
 *   inc_by(i) -> WX::Size
 *   inc_by(size) -> WX::Size
 *   + i -> WX::Size
 *   + size -> WX::Size
 *
 * increase this size and return new size.
 * ===Arguments
 * * x, y and i are Integer
 * * size is a WX::Size
 * ===Return value
 * WX::Size
 */
DLL_LOCAL VALUE _incBy(int argc,VALUE *argv,VALUE self)
{
	VALUE x, y;
	rb_scan_args(argc, argv, "11", &x, &y);

	wxSize* result = new wxSize(*_self);
	if(NIL_P(y)) {
		if(is_wrapable<wxSize>(x)) {
			result->IncBy(unwrap<wxSize>(x));
		} else {
			result->IncBy(RB_NUM2INT(x));
		}
	} else {
		result->IncBy(RB_NUM2INT(x), RB_NUM2INT(y));
	}
	return wrapTypedPtr(result, rb_class_of(self));
}

/*
 * call-seq:
 *   inc_by!(x, y) -> self
 *   inc_by!(i) -> self
 *   inc_by!(size) -> self
 *
 * increase this size and return new size.
 * ===Arguments
 * * x, y and i are Integer
 * * size is a WX::Size
 * ===Return value
 * self
 */
DLL_LOCAL VALUE _incBy_self(int argc,VALUE *argv,VALUE self)
{
	VALUE x, y;
	rb_scan_args(argc, argv, "11", &x, &y);

	rb_check_frozen(self);

	if(NIL_P(y)) {
		if(is_wrapable<wxSize>(x)) {
			_self->IncBy(unwrap<wxSize>(x));
		} else {
			_self->IncBy(RB_NUM2INT(x));
		}
	} else {
		_self->IncBy(RB_NUM2INT(x), RB_NUM2INT(y));
	}
	return self;
}

/*
 * call-seq:
 *   dec_by(x, y) -> WX::Size
 *   dec_by(i) -> WX::Size
 *   dec_by(size) -> WX::Size
 *   - i -> WX::Size
 *   - size -> WX::Size
 *
 * decrease this size and return new size.
 * ===Arguments
 * * x, y and i are Integer
 * * size is a WX::Size
 * ===Return value
 * WX::Size
 */
DLL_LOCAL VALUE _decBy(int argc,VALUE *argv,VALUE self)
{
	VALUE x, y;
	rb_scan_args(argc, argv, "11", &x, &y);

	wxSize* result = new wxSize(*_self);
	if(NIL_P(y)) {
		if(is_wrapable<wxSize>(x)) {
			result->DecBy(unwrap<wxSize>(x));
		} else {
			result->DecBy(RB_NUM2INT(x));
		}
	} else {
		result->DecBy(RB_NUM2INT(x), RB_NUM2INT(y));
	}
	return wrapTypedPtr(result, rb_class_of(self));
}

/*
 * call-seq:
 *   dec_by!(x, y) -> self
 *   dec_by!(i) -> self
 *   dec_by!(size) -> self
 *
 * decrease this size and return new size.
 * ===Arguments
 * * x, y and i are Integer
 * * size is a WX::Size
 * ===Return value
 * self
 */
DLL_LOCAL VALUE _decBy_self(int argc,VALUE *argv,VALUE self)
{
	VALUE x, y;
	rb_scan_args(argc, argv, "11", &x, &y);

	rb_check_frozen(self);

	if(NIL_P(y)) {
		if(is_wrapable<wxSize>(x)) {
			_self->DecBy(unwrap<wxSize>(x));
		} else {
			_self->DecBy(RB_NUM2INT(x));
		}
	} else {
		_self->DecBy(RB_NUM2INT(x), RB_NUM2INT(y));
	}
	return self;
}


}
}


/*
 * Document-class: WX::Size
 *
 * This class represents an Size.
*/

/* Document-attr: width
 * returns the width value of Size. */
/* Document-attr: height
 * returns the height value of Size. */


DLL_LOCAL void Init_WXSize(VALUE rb_mWX)
{

	using namespace RubyWX::Size;
	rb_cWXSize = rb_define_class_under(rb_mWX,"Size",rb_cObject);

	rb_define_alloc_func(rb_cWXSize,_alloc);

#if 0
	rb_define_attr(rb_cWXSize,"width",1,1);
	rb_define_attr(rb_cWXSize,"height",1,1);
#endif

	rb_define_method(rb_cWXSize,"initialize",RUBY_METHOD_FUNC(_initialize),2);
	rb_define_private_method(rb_cWXSize,"initialize_copy",RUBY_METHOD_FUNC(_initialize_copy),1);

	rb_define_attr_method(rb_cWXSize,"width",_getWidth,_setWidth);
	rb_define_attr_method(rb_cWXSize,"height",_getHeight,_setHeight);

	rb_define_method(rb_cWXSize,"inspect",RUBY_METHOD_FUNC(_inspect),0);

	rb_define_method(rb_cWXSize,"==",RUBY_METHOD_FUNC(_equal),1);
	rb_define_alias(rb_cWXSize,"eql?","==");

	rb_define_method(rb_cWXSize,"hash",RUBY_METHOD_FUNC(_getHash),0);

	rb_define_method(rb_cWXSize,"marshal_dump",RUBY_METHOD_FUNC(_marshal_dump),0);
	rb_define_method(rb_cWXSize,"marshal_load",RUBY_METHOD_FUNC(_marshal_load),1);

	rb_define_method(rb_cWXSize,"inc_by",RUBY_METHOD_FUNC(_incBy),-1);
	rb_define_method(rb_cWXSize,"inc_by!",RUBY_METHOD_FUNC(_incBy_self),-1);
	rb_define_alias(rb_cWXSize,"+","inc_by");

	rb_define_method(rb_cWXSize,"dec_by",RUBY_METHOD_FUNC(_decBy),-1);
	rb_define_method(rb_cWXSize,"dec_by!",RUBY_METHOD_FUNC(_decBy_self),-1);
	rb_define_alias(rb_cWXSize,"-","dec_by");

	rb_define_method(rb_cWXSize,"scale",RUBY_METHOD_FUNC(_scale),-1);
	rb_define_method(rb_cWXSize,"scale!",RUBY_METHOD_FUNC(_scale_self),-1);
	rb_define_alias(rb_cWXSize,"*","scale");

	rb_define_method(rb_cWXSize,"/",RUBY_METHOD_FUNC(_devide),1);

	registerType<wxSize>(rb_cWXSize, true);

	rwxID_width = rb_intern("width");
	rwxID_height = rb_intern("height");
}
