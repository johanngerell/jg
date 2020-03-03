<!-- slide -->

# evolution of a C++ mocking library

<!-- slide -->

we've just implemented this function

```c++ {.line-numbers}
void download_config() noexcept;
```

<!-- slide -->

with these requirements

  * download the default app configuration file from _\<remote place\>_
  * store the downloaded file at _\<local place\>_

and these definitions

_\<remote place\>_
: file server, http/ftp endpoints, etc.

_\<local place\>_
: file system, database, etc.

<!-- slide -->

now we want to unit test `download_config()` to make sure it does what it's supposed to do

<!-- slide -->

we want to specifically
  * _unit test_ it

because we want tests that
  * _build_ and _run_ quickly

with ideally
  * _no_ external dependencies

<!-- slide -->

> it's not my job to test the external dependencies
>
> / renegade thinker

<!-- slide -->

the external dependencies seem to be

  * the thing that gets the remote file
  * the thing that stores the local file

<!-- slide -->

we know that because the only "external" calls we
do in `download_config()` concern those things

<!-- slide -->

oh, but there's also code that validates that
the config file content is valid `json`

<!-- slide -->

let's move that to its own function,
which is separately unit tested

<!-- slide -->

because `download_config()` doesn't need to know anything about the config file format

<!-- slide -->

can we verify the correctness of `download_config()` by calling it in a unit test and check its side effects?

<!-- slide -->

it's a `void` function that doesn't throw an exception

```c++ {.line-numbers}
void download_config() noexcept;
```

<!-- slide -->

so it _seems_ like we can't tell if it "succeeds"

<!-- slide -->

also, it uses the real

  * "get the file thing"
  * "json validation thing"
  * "store the file thing"

<!-- slide -->

so it _seems_ like we can't call it in isolation in unit tests

<!-- slide -->

dude

<!-- slide -->

> just mock the external dependencies
>
> / renegade thinker

<!-- slide -->

sure, there are no _explicit_ observable side effects: a return value or an exception

<!-- slide -->

but we can observe the _implicit_ side effects: how external dependencies are used

<!-- slide -->

after all, even if there _was_ a success or failure return value that we checked in a unit test

<!-- slide -->

we wouldn't know for sure if `download_config()` actually did what it was supposed to do or not

<!-- slide -->

we would just know what `download_config()` told us it had done

<!-- slide -->

> trust is good
> verification is better
>
> / my military service drill sergeant

<!-- slide -->

we need a way to check if `download_config()` succeeded or failed without asking it

<!-- slide -->

`download_config()` has succeeded if there exists a file in a special local place which is identical to a file in a special remote place

<!-- slide -->

that's all

<!-- slide -->

the implementation

```c++ {.line-numbers}
void download_config() noexcept
{
    try
    {
        const std::string config = get_remote_config();

        if (is_valid_config(config))
            store_local_config(config);
    }
    catch(...) {}
}
```

<!-- slide -->

we can unit test this function if we mock

  * `get_remote_config()`  
  * `is_valid_config()`
  * `store_local_config()`

and check if a valid configuration returned from the first is passed to the last

<!-- slide -->

**attempt 1:** trivial mocks

```c++ {.line-numbers}
const std::string returned = "{\"foo\":\"bar\"}";
std::string       received;
bool              validated = false;

std::string get_remote_config()
{
    return returned;
}

bool is_valid_config(const std::string& config)
{
    validated = true;
    return true;
}

void store_local_config(const std::string& config)
{
    received = config;
}
```

<!-- slide -->

using them in a test case is super simple

```c++ {.line-numbers}
TEST_CASE()
{
    download_config();

    TEST_ASSERT(validated);
    TEST_ASSERT(returned == received);
}
```

<!-- slide -->

that's sometimes good enough, but more often it's not

<!-- slide -->

as soon as we need to change the mock behavior between test cases, we need something better, more flexible

<!-- slide -->

when `download_config()` returns, the external dependencies might have been acted on and reacted in a very specific set of ways

<!-- slide -->

again, the implementation

```c++ {.line-numbers}
void download_config() noexcept
{
    try
    {
        const std::string config = get_remote_config();

        if (is_valid_config(config))
            store_local_config(config);
    }
    catch(...) {}
}
```

<!-- slide -->

`get_remote_config()`

  * wasn't called at all
  * failed
  * succeeded with valid config file
  * succeeded with invalid config file

<!-- slide -->

`is_valid_config()`

  * wasn't called at all
  * called with valid config file
  * called with invalid config file

<!-- slide -->

`store_local_config()`

  * wasn't called at all
  * called with a config file

<!-- slide -->

combining them gives four test cases

<!-- slide -->

**test case 1**

| | |
|-|-|
|`get_remote_config()` | not called
|then
|`is_valid_config()`   | not called
|`store_local_config()`| not called

<!-- slide -->

**test case 2**

| | |
|-|-|
|`get_remote_config()` |fails
|then
|`is_valid_config()`   |not called
|`store_local_config()`|not called

<!-- slide -->

**test case 3**

| | |
|-|-|
|`get_remote_config()` |succeeds with valid config file
|then
|`is_valid_config()`   |succeeds
|`store_local_config()`|called

<!-- slide -->

**test case 4**

| | |
|-|-|
| `get_remote_config()` | succeeds with invalid config file
| then
| `is_valid_config()` | fails
| `store_local_config()` | not called

<!-- slide -->

the first `TEST_CASE` attempt earlier is
**test case 3**: the "happy path"

<!-- slide -->

the additional flexibility we need in the mocks to use them efficiently in all four test cases can be added using `std::function`

<!-- slide -->

**attempt 2:** not as trivial mocks, but more flexible

```c++ {.line-numbers}
std::function<std::string()> get_remote_config_func;
std::string get_remote_config()
{
    return get_remote_config_func();
}

std::function<bool(const std::string&)> is_valid_config_func;
bool is_valid_config(const std::string& config)
{
    return is_valid_config_func(config);
}

std::function<void(const std::string&)> store_local_config_func;
void store_local_config(const std::string& config)
{
    store_local_config_func(config);
}
```

<!-- slide -->

using them in a test case is straightforward

```c++ {.line-numbers}
TEST_CASE("3. happy path")
{
    const std::string returned = "{\"foo\":\"bar\"}";
    get_remote_config_func = [&] { return returned; };

    bool validated = false;
    is_valid_config_func = [&] (const std::string&) {
        validated = true; return true; };

    std::string received;
    store_local_config_func = [&] (const std::string& config) {
        received = config; };

    download_config();

    TEST_ASSERT(validated);
    TEST_ASSERT(returned == received);
}
```

<!-- slide -->

we still do the same test assertions on the same data, but with better locality of the mock logic

<!-- slide -->

after writing all four test cases with the new mock functions that use `std::function` and the auxiliary data that they use, that later gets test-asserted, a structural pattern emerges for the

  * mock functions
  * auxiliary data

<!-- slide -->

```c++ {.line-numbers}
std::function<std::string()> get_remote_config_func;
bool                         get_remote_config_called{};

std::string get_remote_config()
{
    get_remote_config_called = true;
    return get_remote_config_func();
}
```

<!-- slide -->

```c++ {.line-numbers}
std::function<bool(const std::string&)> is_valid_config_func;
bool                                    is_valid_config_called{};
std::string                             is_valid_config_param;

bool is_valid_config(const std::string& config)
{
    is_valid_config_called = true;
    is_valid_config_param = config;
    return is_valid_config_func(config);
}
```

<!-- slide -->

```c++ {.line-numbers}
std::function<void(const std::string&)> store_local_config_func;
bool                                    store_local_config_called{};
std::string                             store_local_config_param;

void store_local_config(const std::string& config)
{
    store_local_config_called = true;
    store_local_config_param = config;
    store_local_config_func(config);
}
```

<!-- slide -->

or, more generally for a mocked function `foo` that returns `T` and takes the parameters `P1`, ..., `PN`

```c++ {.line-numbers}
T foo(P1 p1, ..., PN pN)
```

<!-- slide -->

the pattern is

```c++ {.line-numbers}
std::function<T(P1, ..., PN)> foo_func;
bool                          foo_called{};
P1                            foo_param1;
...
PN                            foo_paramN;

T foo(P1 p1, ..., PN pN)
{
    foo_called = true;
    foo_param1 = p1;
    ...
    foo_paramN = pN;
    return foo_func(p1, ..., pN);
}
```

<!-- slide -->

we can also...

<!-- slide -->

...add a "call counter" - `foo_called` is just the special case `foo_count > 0`

```c++ {.line-numbers}
...
size_t                        foo_count{};
...

T foo(P1 p1, ..., PN pN)
{
    foo_count++;
    ...
}
```

<!-- slide -->

...and the ability to just set a return value instead of assigning a callable to the `std::function`, if we don't need any complex logic

```c++ {.line-numbers}
...
std::optional<T>              foo_result;
...

T foo(P1 p1, ..., PN pN)
{
    ...
    return foo_result.has_value() ?
           foo_result.value() :
           foo_func(p1, ..., pN);
}
```

<!-- slide -->

...and make a dedicated type `foo_aux` for the auxiliary data and create an instance `foo_` of it to use in tests

```c++ {.line-numbers}
struct foo_aux final
{ 
    std::optional<T>              result;
    std::function<T(P1, ..., PN)> func;
    size_t                        count{};
    bool                          called{};
    P1                            param1;
    ...
    PN                            paramN;
} foo_;
```

<!-- slide -->

the mock function would then use its auxiliary data like this

```c++ {.line-numbers}
T foo(P1 p1, ..., PN pN)
{
    foo_.count++;
    foo_.called = true;
    foo_.param1 = p1;
    ...
    foo_.paramN = pN;
    return foo_.result.has_value() ?
           foo_.result.value() :
           foo_.func(p1, ..., pN);
}
```

<!-- slide -->

since the auxiliary data instance `foo_` is global, we also need a way to reset its state between tests

```c++ {.line-numbers}
struct foo_aux final
{
    ...
    void reset();
    ...
} foo_;
```

<!-- slide -->

how would our four test cases look if this kind of rich mocks with auxiliary data existed for the external dependecies?

<!-- slide -->

like this

```c++ {.line-numbers}
TEST_CASE("3. happy path")
{
    const std::string returned = "{\"foo\":\"bar\"}";
    get_remote_config_.reset();
    get_remote_config_.result = returned;

    is_valid_config_.reset();
    is_valid_config_.result = true;

    store_local_config_.reset();

    download_config();

    TEST_ASSERT(is_valid_config_.called);
    TEST_ASSERT(store_local_config_.param1 == returned);
}
```

<!-- slide -->

it gets even cleaner if the test framework supports test case initialization

```c++ {.line-numbers}
TEST_CASE_INIT() // runs before every test case body
{
    get_remote_config_.reset();
    is_valid_config_.reset();
    store_local_config_.reset();
}

TEST_CASE("3. happy path")
{
    const std::string returned = "{\"foo\":\"bar\"}";
    get_remote_config_.result = returned;
    is_valid_config_.result = true;

    download_config();

    TEST_ASSERT(is_valid_config_.called);
    TEST_ASSERT(store_local_config_.param1 == returned);
}
```

<!-- slide -->

test cases like that, with all inputs and outputs close to the action, are easier to write, read, and reason about

<!-- slide -->

however...

<!-- slide -->

manually creating auxiliary data structures for every function we want to mock, and using all that data correctly in the mock functions will quickly become a hassle :(

<!-- slide -->

macros! :D

<!-- slide -->

this is the `is_valid_config()` external dependency

```c++ {.line-numbers}
bool is_valid_config(const std::string& config);
```

<!-- slide -->

wouldn't it be great if the mock function and its auxiliary data structure were generated automatically for us if we just wrote something like

```c++ {.line-numbers}
MOCK(bool, is_valid_config, const std::string&);
```

<!-- slide -->

that's exactly what the `jg::mock` library does with its `JG_MOCK` macro. some template metaprogramming makes sure that auxiliary data only includes

  * `result` if a non-void return type is declared
  * `param<N>` if any parameter type is declared

<!-- slide -->

`JG_MOCK` adds three optional parameters

```c++ {.line-numbers}
JG_MOCK(prefix, suffix, overload_suffix,
        bool, is_valid_config, const std::string&);
```

`prefix`
: adds things "to the left" of the mock function prototype

`suffix`
: adds things "to the right" of the mock function prototype

`overload_suffix`
: adds an arbitrary tag to the auxiliary data name for overloaded functions

<!-- slide -->

they can all be omitted if unused

```c++ {.line-numbers}
JG_MOCK(,,, bool, is_valid_config, const std::string&);
```

<!-- slide -->

we can create a "mock class" by mocking virtual functions of an abstract base class

```c++ {.line-numbers}
class logger
{
public:
    virtual void log_info(const char* text, size_t level) = 0;
    virtual ~logger() {}
}

class mock_logger final : public logger
{
public:
    JG_MOCK(,,, void, log_info, const char*, size_t);
}
```

<!-- slide -->

this is functionally equivalent

```c++ {.line-numbers}
class mock_logger final : public logger
{
public:
    JG_MOCK(virtual, override,, void, log_info, const char*, size_t);
}
```

but since `virtual` and `override` aren't required when declaring a virtual function override, omitting them makes mocks easier to write and read

<!-- slide -->

when a "mock class" is used in tests, it's usually instantiated in every test case, which means that all auxiliary data in it is already reset

```c++ {.line-numbers}
TEST_CASE("The tested entity logs when created")
{
    mock_logger logger; // no logger.log_info_.reset() is needed

    some_tested_entity sut(logger);

    TEST_ASSERT(logger.log_info_.called());
    TEST_ASSERT(logger.log_info_.param<1>() != nullptr);
}
```

<!-- slide -->

if the abstract base class has overloaded virtual
functions, we need to use the `overload_suffix`
macro parameter

```c++ {.line-numbers}
class logger
{
public:
    virtual void log_info(const char* text, size_t level) = 0;
    virtual void log_info(const std::string& text) = 0;
    virtual ~logger() {}
}

class mock_logger final : public logger
{
public:
    JG_MOCK(,, PCHAR, void, log_info, const char*, size_t);
    JG_MOCK(,, STRING, void, log_info, const std::string&);
}
```

<!-- slide -->

using auxiliary data for overloaded functions is done like this

```c++ {.line-numbers}
TEST_CASE("The tested entity logs when created")
{
    mock_logger logger;

    some_tested_entity sut(logger);

    TEST_ASSERT(logger.log_info_PCHAR.called());
    TEST_ASSERT(!logger.log_info_STRING.called());
}
```

<!-- slide -->

sometimes we want to mock free functions and use the mocks in more than one translation unit

<!-- slide -->

adding the same `JG_MOCK(...)` declaration in multiple translation units defines the mock function multiple times

<!-- slide -->

but a non-static function that has multiple definitions in a program will at best cause linker errors and at worst exhibit undefined behavior

<!-- slide -->

`jg::mock` helps in this scenario by offering the `JG_MOCK_REF` macro, which creates an extern declaration of an identical `JG_MOCK` declaration made in another module or translation unit

<!-- slide -->

the recommendation is to add

  * free function mocks with `JG_MOCK` in dedicated `.cpp` files and reference them with `JG_MOCK_REF` in dedicated `.h` files that test case `.cpp` files can include
  * virtual function mocks with `JG_MOCK` in dedicated `.h` files that test case `.cpp` files can include
  
<!-- slide -->

compilation flags that affect `jg::mock`

`JG_MOCK_ENABLE_SHORT_NAMES`
: if defined, macro names without the `JG_` prefix are enabled, so that `MOCK` and `MOCK_REF` can be used instead of `JG_MOCK` and `JG_MOCK_REF` 
