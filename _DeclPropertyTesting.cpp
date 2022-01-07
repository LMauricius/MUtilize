#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "DeclProperty.h"

template<class...>
using my_void_t = void;

class PropOwner
{
public:
    using property_owner_t = PropOwner;

    /*struct property_memberSum_t; friend property_memberSum_t;
	struct property_memberSum_t{
        friend property_owner_t;
		property_owner_t *owner;
		property_memberSum_t(property_owner_t *owner) : owner(owner) {}
		
		operator int()
        {
            return owner->a + owner->b;
        }

        //using getter_t = decltype(std::declval<property_memberSum_t>().get());
	}; property_memberSum_t abSum = property_memberSum_t(this);*/
    decl_property(int, abSum,
        decl_get()
        {
            return owner->a + owner->b;
        }
        void decl_set(int val)
        {
            owner->a = val - owner->b;
        }
    );

    int a, b;
};

int main() 
{
    int a, b;

    std::cout << "Enter a: " << std::endl;
    std::cin >> a;
    std::cout << "Enter b: " << std::endl;
    std::cin >> b;

    PropOwner ops;
    ops.a = a;
    ops.b = b;

    std::cout << (int)ops.abSum << std::endl;
    ops.abSum = 10;
    std::cout << (int)ops.abSum << std::endl;

    return 0;
}