# Code style
## General
* Macros are not allowed for constants and functions, use `constexpr` instead.
* No include guards allowed, `#pragma once` is used instead.
* Name spaces don't indent code.
* No `using namespace` allowed.
* Code should be formatted using `clang-format`.
* Spaces are used instead of tabs all the time.

## File names
### Headers
* Header _files_ are put in the `include` directory.
* Header _filenames_ end with **.hpp**.
* Header _filenames_ follow the **snake_case** or **lowercase** style.

### Sources
* Source _files_ are put in the `src` directory.
* Source _filenames_ end with **.cpp**.
* Source _filenames_ follow the **snake_case** or **lowercase** style.

## Naming
* _Variables_ and _name spaces_ follow the **snake_case** style.
* _Constants_ follow the **SCREAMING_SNAKE_CASE** style.
* _Type aliases_ follow the **snake_case** style.
* _Class names_ follow the **PascalCase** style.
* _Structure names_ follow the **snake_case** style.
* _Function_ and _method names_ follow the **camelCase** style.

## Namespaces
* Namespace blocks are allowed in headers.
* In source files, the full name is required (eg. `uvre::GLRenderContext::doSomething()`).

## Brace wrapping
Opening braces are placed on the next line for:
* Name spaces
* Functions
* Scopes

## Include blocks
Headers are sorted in alphabetical order regardless of the subdirectory and organized into two blocks:
* Headers that belong to the project _itself_ must be placed first.
* Headers that do not belong to the project (_standard_ or _third-party_) must be placed second.
