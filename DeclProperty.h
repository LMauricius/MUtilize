/*
Made by Mauricius

Part of my MUtilize repo: https://github.com/LegendaryMauricius/MUtilize
*/

#pragma once
#ifndef _DECL_PROPERTY_H
#define _DECL_PROPERTY_H

#define decl_property(BASETYPE, NAME, DECL)\
    struct property_##NAME##_t; friend property_##NAME##_t;\
	struct property_##NAME##_t{\
        friend property_owner_t;\
        using property_wrapper_t = property_##NAME##_t;\
        using property_base_t = BASETYPE;\
		property_owner_t *owner;\
		property_##NAME##_t(property_owner_t *owner) : owner(owner) {}\
		\
		DECL\
		\
		template<class _mySFINAE_t = property_##NAME##_t>\
		inline auto operator->() {return ;}\
		\
	}; property_##NAME##_t NAME = property_##NAME##_t(this)\

#define decl_get() operator property_base_t ()
#define decl_set(TYPE_VAL) operator=(TYPE_VAL)

#define property_defaults property_base_t property_value
#define default_get inline operator property_base_t& () {return property_value;}
#define default_set inline property_wrapper_t& operator=(property_base_t&& _val) {property_value = _val; return *owner;}

#endif