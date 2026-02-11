**Objective:** learn about exception handling and namespaces.

In this exercise we are going to extend the `RestrictedPtr` from last round.
Here, you need to use your own implementation from the last round.

In the *restricted_ptr.hpp* you should implement your `RestrictedPtr` with 
exceptions. This includes expanding the original implementation so 
that:

* it throws an **RestrictedCopyException** if you try to copy the `RestrictedPtr`
  over 3 times.
* it throws an **RestrictedNullException** if you try to call `GetData` method 
  with a RestrictedPtr that points to a `nullptr`.

These 2 error classes inherit from the base exception class
**RestrictedPtrException**. `RestrictedPtrException` inherits from 
`std::exception` (See image). For more details, see the header files.

You need to implement these 3 classes to the *restricted_ptr_ex.hpp* header file.

On top of this, the whole implementation in the *restricted_ptr_ex.hpp* and 
*restricted_ptr.hpp* files should be wrapped in a `WeirdMemoryAllocator` 
namespace. After this, the written namespace calls to those functions in 
*main.cpp* should work.

You also need to implement a wrapper/interface for this pointer 
class in *restricted_interface.hpp*. For more information, see the header file.
