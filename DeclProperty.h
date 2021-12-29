/*
Made by Mauricius

Part of my MUtilize repo: https://github.com/LegendaryMauricius/MUtilize
*/

#pragma once
#ifndef _DECL_PROPERTY_H
#define _DECL_PROPERTY_H

#define DECL_PROPERTY(TYPE, NAME, DECL)\
	struct property_##NAME##_t<class _PropertyOwner_t>{\
		_PropertyOwner_t *owner = (this - ());\
		property_##NAME##_t(_PropertyOwner_t *owner) : owner(owner) {}\
		\
		DECL\
	} NAME<decltype(*this)>(this)\


#endif