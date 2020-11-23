\page ckpt_learn_serialize How to Serialize Data

*Serialization* is the process of converting an object into a simple format
that can be stored or transmitted and reconstructed later.
checkpoint translates the object into a set of contiguous bits
and provides the steps to reverse the process, i.e. to reconstitute
the object from the set of bits.

The extraction of the data from a series of bytes is called
*deserialization*.

\section serialize_builtin Serialization of built-in types

This action is straightforward with the ```|``` operator.

When the variable `a` has a built-in type and it needs to be serialized
into the serializer object `s`, we simply write

\code{.cpp}
s | a;
\endcode

\section serialize_stl Serialization of STL containers

The `|` operator has been overloaded for the STL containers:
`std::array`, `std::deque`, `std::list`,
`std::map`, `std::multimap`, `std::multiset`,
`std::queue`,
`std::set`, `std::string`,
`std::unordered_map`, `std::unordered_multimap`,
`std::unordered_multiset`, `std::unordered_set`,
`std::vector`.

The `|` operator is also overloaded for `std::pair` and `std::unique_ptr`.

When the variable `c` is such an STL object, whose template parameter(s) can
be directly serialized into a serializer object `s`, we write

\code{.cpp}
s | c;
\endcode

\section serialize_class Serialization of custom class

When a class (or structure) has to be serialized, the following steps are recommended:
- define a default constructor;
- define a templated function `template< class Serializer > void serialize(Serializer &s)` with
  - a serialization step for all the member variables that need to be serialized.

The example in \ref ckpt_learn_ex1 illustrates this situation.

When the class (or structure) is not amenable to having a default constructor,
the class must contains a static reconstructor method that serialization can use
to construct the class.
For a class to be serializable, it must have a valid reconstruction method
(either a default constructor or a static reconstructor method).

The example in \ref ckpt_learn_ex1 illustrates this situation.

\section serialize_virtual Serialization of virtual class

Consider the situation where the class `BaseVC` is abstract (i.e. it has a pure virtual method)
and the classe `CI` is one concrete class derived from `BaseVC`.

\subsection serialize_virtual_prep Preparatory steps

\subsubsection serialize_virtual_prep_macro Macro-based approach (preferred)

The following steps should be taken:
- Insert checkpoint macros in your virtual class hierarchy for derived and
  base classes with the corresponding macros:
  - \c checkpoint_virtual_serialize_root()
  - \c checkpoint_virtual_serialize_derived_from( BaseVC )

The example in \ref ckpt_learn_example_virtual_macro
illustrates this approach.

\subsubsection serialize_virtual_prep_hier Hierarchy-based approach

In this case,
- Make your virtual class hierarchy you want to serialize all inherit from
  \c SerializableBase< BaseVC > and \c SerializableDerived< CI, BaseVC > in the whole
  hierarchy.

The example in \ref ckpt_learn_example_virtual
illustrates this approach.

\subsection serialize_virtual_step Actual serialization

- If you have a \c std::unique_ptr< BaseVC >, where `BaseVC` is virtually serializable (by
  using the macros or inheriting from \c SerializableBase< BaseVC > and
  \c SerializableDerived< CI, BaseVC > ), they will automatically be virtually
  serialized.

- If you have a raw pointer, \c Teuchos::RCP, or \c std::shared_ptr<T>,
  you must invoke: \c checkpoint::allocateConstructForPointer<SerializerT,T>

  Example with raw pointer:

  \code{.cpp}
     template <typename T>
     struct MyObjectWithRawPointer {
       T* raw_ptr;

       template <typename SerializerT>
       void serialize(SerializerT& s) {
         bool is_null = raw_ptr == nullptr;
         s | is_null; // serialize whether we have a null pointer not
         if (!is_null) {
           // During size/pack, save the actual derived type of raw_ptr;
           // During unpack, allocate/construct raw_ptr with correct virtual type
           checkpoint::allocateConstructForPointer(s, raw_ptr);
           s | *raw_ptr;
         }
       }
     };
  \endcode

