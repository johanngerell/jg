# Checking the size of any instance or type

## C++

The idea is to define a macro that declares a `char` array of the same size as the type that is checked, and then initialize it with the return value of a function that takes an `int` parameter, but it's passed the address of the array that is initialized, which fails to compile. The compiler error will show the array size, which is the same as the size that was being checked:

```c++
#define CHECK_SIZE(instance_or_type) \
    namespace \
    { \
        char checker(int); \
        char checkSize[sizeof(instance_or_type)] = { checker(&checkSize) }; \
    }
```

The size of any instance or type, like `std::string` below, can be checked with:

```c++
CHECK_SIZE(std::string);
```

In a reasonable IDE that "compiles code" behind your back while it's being written, just hover over the `CHECK_SIZE` invocation to see the error message, which includes the size:

> argument of type "char (*)[40]" is incompatible with parameter of type "int"

This means that the size of `std::string` for that particular compiler configuration is 40 bytes. If this error message isn't shown by the IDE, then it will be shown at compile time.

## C

The basic idea is the same as for the C++ case above, but following the limitations set by the language:

```c++
#define CHECK_SIZE(type) \
    char char_from_int(int); \
    void check_size_of(void) { \
        char char_array[sizeof(type)] = { char_from_int(&char_array) }; \
        (void)char_array; \
    }
```

The size of any type, like `size_t` below, can be checked with:

```c++
CHECK_SIZE(size_t);
```

In a reasonable IDE that "compiles code" behind your back while it's being written, just hover over the `CHECK_SIZE` invocation to see the error message, which includes the size:

> 'int' differs in levels of indirection from 'char (*)[8]'

This means that the size of `size_t` for that particular compiler configuration is 8 bytes. If this error message isn't shown by the IDE, then it will be shown at compile time.