#ifndef IMPORT_EXPORT_HPP_INCLUDED
#   define IMPORT_EXPORT_HPP_INCLUDED

#   if defined(_MSC_VER) // MSVC
#       define DIMPORT                      __declspec(dllimport)
#       define DEXPORT                      __declspec(dllexport)
#       define DLL_USE_EXE_VAR(expr)
#   elif defined(__APPLE__) // LLVM
#       define DIMPORT
#       define DEXPORT
#       define DLL_USE_EXE_VAR(expr)        
#   elif defined(__GNUC__) // gcc
#       define DIMPORT
#       define DEXPORT
#       define DLL_USE_EXE_VAR(expr)        expr
#   else // unknown
#       error "Unknown compiler"
#   endif

#endif
