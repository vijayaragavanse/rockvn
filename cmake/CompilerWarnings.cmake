# Central warning policy, exposed as the interface target rockvn::warnings.
# First-party targets link against it; third-party code obtained through
# vcpkg is never subjected to it. Warnings are errors by default: a warning
# that is allowed to persist is a warning that will eventually be ignored.

option(ROCKVN_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

add_library(rockvn_warnings INTERFACE)
add_library(rockvn::warnings ALIAS rockvn_warnings)

if(MSVC)
  target_compile_options(
    rockvn_warnings
    INTERFACE /W4
              /permissive-
              /utf-8
              /Zc:__cplusplus
              /w14265 # class has virtual functions but non-virtual destructor
              /w14640 # thread-unsafe static local initialization
  )
  if(ROCKVN_WARNINGS_AS_ERRORS)
    target_compile_options(rockvn_warnings INTERFACE /WX)
  endif()
else()
  target_compile_options(
    rockvn_warnings
    INTERFACE -Wall
              -Wextra
              -Wpedantic
              -Wshadow
              -Wconversion
              -Wsign-conversion
              -Wnon-virtual-dtor
              -Wold-style-cast
              -Wcast-align
              -Woverloaded-virtual
              -Wnull-dereference
              -Wdouble-promotion
              -Wimplicit-fallthrough)
  if(ROCKVN_WARNINGS_AS_ERRORS)
    target_compile_options(rockvn_warnings INTERFACE -Werror)
  endif()
endif()
