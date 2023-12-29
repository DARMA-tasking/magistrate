\page ckpt_learn_serialize How to Serialize Data

*Serialization* is the process of recursively traversing C++ objects into a
simple format that can be stored or transmitted and reconstructed later.
*checkpoint* translates the object into a set of contiguous bits and provides
the steps to reverse the process, i.e. to reconstitute the object from the set
of bits.

The extraction of the data from a set of bytes is called *deserialization*.

\section serialize_builtin Serialization of built-in types

This action is straightforward with the ```|``` operator.

When the variable `a` has a built-in type and it needs to be serialized
into the serializer object `s`, we simply write

\code{.cpp}
s | a;
\endcode

\section serialize_stl Serialization of C++ standard library

The `|` operator has been overloaded for many of the C++ STL data structures:
  - `std::array`
  - `std::deque`
  - `std::list`
  - `std::map`
  - `std::multimap`
  - `std::multiset`
  - `std::queue`
  - `std::set`
  - `std::string`
  - `std::unordered_map`
  - `std::unordered_multimap`
  - `std::unordered_multiset`
  - `std::unordered_set`
  - `std::vector`
  - `std::tuple`
  - `std::pair`
  - `std::unique_ptr`

When the variable `c` is such an STL object, whose template parameter(s) can
be directly serialized into a serializer object `s`, we write

\code{.cpp}
s | c;
\endcode

\section serialize_class Serialization of classes

When a class (or structure) has to be serialized, the user must provide a
reconstruction method for the class and a serialization method or free function
to actually perform the serialization.

\subsection reconstruct_class Class reconstruction

There are several ways to allow *checkpoint* to reconstruct a
class. *checkpoint* will try to detect a reconstruction strategy in the
following resolution order:
  1. Tagged constructor: `MyClass(checkpoint::SERIALIZE_CONSTRUCT_TAG) {}`
  1. Reconstruction `MyClass::reconstruct(buf)` or `reconstruct(MyClass, buf)`
  1. Default constructor: `MyClass()`

If no reconstruct strategy is detected with type traits, *checkpoint* will fail
at compile-time with a static assertion indicating that *checkpoint* can't
reconstruct the class.

The example in \ref ckpt_learn_ex1 illustrates the reconstruct method.

\subsection serialize_class Class serializers

Users may provide a serializer for a class in one of two forms: a `serialize`
method on the class (intrusive) or a free function `serialize` that takes a
reference to the class as an argument (non-intrusive). Note that if a
free-function serialization strategy is employed, one may be required to friend
the serialize function so it can access private/protected data inside the class,
depending on what data members the function needs to access for correct
serialization of the class state.

\section serialize_polymorphic Serialization of polymorphic classes

To serialize polymorphic class hierarchies, one must write serializers for each
class in the hierarchy. Then, the user should either insert macros
`checkpoint_virtual_serialize_root()` and
`checkpoint_virtual_serialize_derived_from(T)` to inform *checkpoint* of the
hierarchy so it can automatically traverse the hierarchy. Alternatively, the
user may use the inheritance wrappers `checkpoint::SerializableBase<T>` and
`checkpoint::SerializableDerived<T, U>` to achieve the same effect.

The example in \ref ckpt_learn_ex6_polymorphic_macro illustrates the
approach uses the macros. The example in \ref ckpt_learn_example_polymorphic
illustrates this approach.

\subsection serialize_polymorphic_step Allocation and reconstruction

- If one has a `std::unique_ptr<T> x`, where `T` is polymorphic serializable
  `s | x` will correctly serialize and reconstruct `x` based on the concrete
  type.

- If one has a raw pointer, `Teuchos::RCP<T>`, or `std::shared_ptr<T>`,
   `checkpoint::allocateConstructForPointer<SerializerT,T>(s, ptr)` can be
   invoked to properly allocate and construct the concrete class depending on
   runtime type.
