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
        using base_t = BASETYPE;\
		property_owner_t *owner;\
		property_##NAME##_t(property_owner_t *owner) : owner(owner) {}\
		\
		DECL\
		\
		template<class _mySFINAE_t = property_##NAME##_t>\
		inline auto operator->() {return ;}\
		\
	}; property_##NAME##_t NAME = property_##NAME##_t(this)\

#define decl_get() operator base_t ()
#define decl_set(TYPE_VAL) operator=(TYPE_VAL)

#endif