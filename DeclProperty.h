/*
Made by Mauricius

Part of my MUtilize repo: https://github.com/LegendaryMauricius/MUtilize
*/

#pragma once
#ifndef _DECL_PROPERTY_H
#define _DECL_PROPERTY_H

#include <type_traits>

/*
Declares a property member.
To use it inside a class, you must typedef property_owner_t
to the type of the class.
*/
#define decl_property(NAME, DECL) _stdOrCompact_decl_property_impl(NAME, DECL)

/*
Enables usage of this_owner inside a class' property's methods.
Use AFTER defining the owner class.
Example:
    enable_this_owner(Owner::Member);
*/
#define enable_this_owner(PROP_OWNER_T, MEMBER_NAME) const size_t PROP_OWNER_T::property_##MEMBER_NAME##_t::_this_offset_from_owner = _OFFSET_OF_MEMBER(PROP_OWNER_T, MEMBER_NAME)

/*
Pointer to  the owner of the property.
'this' points to the property member itself, so use 'this_owner' instead
*/
#define this_owner ((property_owner_t*)((char*)this - _this_offset_from_owner))


/*
Declaration of a getter. Used inside the property declaration body.
It can be used to either only declare a getter, or define it later.
*/
#define decl_get(TYPE) operator TYPE ()
/*
Declaration of a setter. Used inside the property declaration body.
It can be used to either only declare a setter, or define it later.
*/
#define decl_set(TYPE_VAL) operator=(TYPE_VAL)

/*
Enables usage of 'default_get' and 'default_set'.
Requires specifying a base property type.
*/
#define enable_property_defaults(BASETYPE) using _property_base_t = BASETYPE; _property_base_t _property_value
/*
Declares a default getter
*/
#define default_get() inline operator _property_base_t& () {return _property_value;}
/*
Declares a default setter
*/
#define default_set() inline _property_wrapper_t& operator=(_property_base_t&& val) {_property_value = val; return *this;}



/*
UTILITIES
*/

#define _OFFSET_OF_MEMBER(OWNER, MEMBER) ((char*)&((OWNER*)nullptr->*(&OWNER::MEMBER)) - (char*)nullptr)
#define _stdOrCompact_decl_property_impl(NAME, DECL) _compact_decl_property_impl(NAME, DECL)
#define _std_decl_property_impl(NAME, DECL)\
    struct property_##NAME##_t; friend property_##NAME##_t;\
	struct property_##NAME##_t{\
        friend property_owner_t;\
        using property_wrapper_t = property_##NAME##_t;\
		\
		DECL\
		\
    private:\
        static const size_t _this_offset_from_owner;\
		\
	}; property_##NAME##_t NAME
#define _compact_decl_property_impl(NAME, DECL)\
    struct property_##NAME##_t; friend property_##NAME##_t;\
	struct property_##NAME##_t{\
        friend property_owner_t;\
        using property_wrapper_t = property_##NAME##_t;\
		\
		DECL\
		\
    private:\
        static const size_t _this_offset_from_owner;\
        char _placeholder[0]; /* allows the structure to have 0 size, non standard in c++*/\ 
		\
	}; property_##NAME##_t NAME

#endif