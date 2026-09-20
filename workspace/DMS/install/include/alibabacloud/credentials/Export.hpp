#ifndef ALIBABACLOUD_CREDENTIALS_EXPORT_HPP_
#define ALIBABACLOUD_CREDENTIALS_EXPORT_HPP_

// Export/import macro for Windows DLLs and default visibility on other platforms
#if defined(_WIN32) || defined(__CYGWIN__)
  #if defined(ALIBABACLOUD_CREDENTIALS_BUILD)
    #define ALIBABACLOUD_CREDENTIALS_EXPORT __declspec(dllexport)
  #else
    #define ALIBABACLOUD_CREDENTIALS_EXPORT __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #define ALIBABACLOUD_CREDENTIALS_EXPORT __attribute__((visibility("default")))
  #else
    #define ALIBABACLOUD_CREDENTIALS_EXPORT
  #endif
#endif

// Macros to suppress MSVC warnings for STL types in exported classes
// C4251: STL types need dll-interface (safe when using same compiler/runtime)
// C4275: Non dll-interface class used as base for dll-interface class (safe for STL base classes)
#ifdef _MSC_VER
  #define ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4251)) \
    __pragma(warning(disable: 4275))
  
  #define ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP \
    __pragma(warning(pop))
#else
  #define ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_PUSH
  #define ALIBABACLOUD_CREDENTIALS_SUPPRESS_STL_WARNING_POP
#endif

#endif // ALIBABACLOUD_CREDENTIALS_EXPORT_HPP_
