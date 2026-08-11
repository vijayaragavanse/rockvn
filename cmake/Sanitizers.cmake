# AddressSanitizer + UndefinedBehaviorSanitizer, exposed as rockvn::sanitizers.
# The linux-debug preset enables this: memory bugs must be caught in the
# development loop, not discovered in production. MSVC's ASan still has
# rough edges with debug-runtime STL, so sanitized builds are a Linux
# concern until that changes — revisit when MSVC support matures.

option(ROCKVN_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(rockvn_sanitizers INTERFACE)
add_library(rockvn::sanitizers ALIAS rockvn_sanitizers)

if(ROCKVN_ENABLE_SANITIZERS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rockvn_sanitizers INTERFACE -fsanitize=address,undefined
                                                       -fno-omit-frame-pointer)
    target_link_options(rockvn_sanitizers INTERFACE -fsanitize=address,undefined)
  else()
    message(WARNING "ROCKVN_ENABLE_SANITIZERS is ON but compiler is not GCC/Clang; option ignored.")
  endif()
endif()
