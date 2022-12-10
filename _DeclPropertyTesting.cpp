#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "DeclProperty.h"

class PropOwner
{
public:
    using property_owner_t = PropOwner;

    decl_property(abSum,
        decl_get(int)
        {
            return this_owner->a + this_owner->b;
        }
        void decl_set(int val)
        {
            this_owner->a = val - this_owner->b;
        }
    );

    int a, b;
};
enable_this_owner(PropOwner, abSum);

int main() 
{
    PropOwner ops;

    std::cout << "PropOwner size: " << sizeof(ops) << std::endl;
    std::cout << "Prop size: " << sizeof(ops.abSum) << std::endl;

    std::cout << "Enter a: " << std::endl;
    std::cin >> ops.a;
    std::cout << "Enter b: " << std::endl;
    std::cin >> ops.b;

    std::cout << "Sum is: " << (int)ops.abSum << std::endl;

    std::cout << "Enter new sum: " << std::endl;
    int s;
    std::cin >> s;
    ops.abSum = s;
    std::cout << "Sum is: " << (int)ops.abSum << std::endl;

    return 0;
}