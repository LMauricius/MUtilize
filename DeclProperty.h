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
#define decl_property(NAME, ...) _stdOrCompact_decl_property_impl(NAME, __VA_ARGS__)

/*
Enables usage of this_owner inside a class' property's methods.
Use AFTER defining the owner class.
If enable_this_owner is used on a property, it has to be the single
member of its property type.
Example:
    enable_this_owner(Owner::Member);
*/
#define enable_this_owner(PROP_OWNER_T, MEMBER_NAME) constexpr PROP_OWNER_T::property_##MEMBER_NAME##_t PROP_OWNER_T::* PROP_OWNER_T::property_##MEMBER_NAME##_t::_this_member_ptr() {return &PROP_OWNER_T::MEMBER_NAME;}

/*
Pointer to  the owner of the property.
'this' points to the property member itself, so use 'this_owner' instead
*/
#define this_owner ((property_owner_t*)((char*)this - _PROP_OFFSET_OF_MEMBER_PTR(property_owner_t, _this_member_ptr())))


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
#define default_set() inline void operator=(_property_base_t&& val) {_property_value = val;}


/*
EXAMPLES
*/

/*
// simple example
class PropOwner
{
public:
    using property_owner_t = PropOwner;

    decl_property(abSum,
        decl_get(int)
        {
            return this_owner->a;
        }
        void decl_set(int val)
        {
            this_owner->a = val;
        }
    );

    int a;
};
enable_this_owner(PropOwner, abSum);
*/

/*
// defaults example
class PropOwner
{
public:
    using property_owner_t = PropOwner;

    decl_property(abSum,
        enable_property_defaults(int);
        default_get();
        default_set();
    );

    int a;
};
*/

/*
// read-only example (writable by the PropOwner)
class PropOwner
{
public:
    using property_owner_t = PropOwner;

    decl_property(abSum,
        enable_property_defaults(int);
        default_get();
    private:
        default_set();
    );

    int a;
};
*/



/*
UTILITIES
*/

#define _PROP_OFFSET_OF_MEMBER_PTR(OWNER, MEMBER_PTR) ((char*)&((OWNER*)nullptr->*(MEMBER_PTR)) - (char*)nullptr)
#define _PROP_OFFSET_OF_MEMBER(OWNER, MEMBER) ((char*)&((OWNER*)nullptr->*(&OWNER::MEMBER)) - (char*)nullptr)
#define _stdOrCompact_decl_property_impl(NAME, ...) _compact_decl_property_impl(NAME, __VA_ARGS__)
#define _std_decl_property_impl(NAME, ...)\
	struct property_##NAME##_t {\
        friend property_owner_t;\
		\
		__VA_ARGS__\
		\
    private:\
        static constexpr property_##NAME##_t PropOwner::* _this_member_ptr();\
	}  NAME
#define _compact_decl_property_impl(NAME, ...)\
	struct property_##NAME##_t {\
        friend property_owner_t;\
		\
		__VA_ARGS__\
		\
    private:\
        static constexpr property_##NAME##_t PropOwner::* _this_member_ptr();\
        char _placeholder[0]; /* allows the structure to have 0 size, non standard in c++*/\ 
	} NAME

#endif