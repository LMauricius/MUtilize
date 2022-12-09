#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "DeclProperty.h"

class PropOwner
{
public:
    using property_owner_t = PropOwner;

    decl_property(int, myIntProperty,
        default_get;
        default_set;
    );
};

int main() 
{   
    PropOwner intOwner;

    intOwner.myIntProperty = 10;

    std::cout << intOwner.myIntProperty << std::endl;

    return 0;
}