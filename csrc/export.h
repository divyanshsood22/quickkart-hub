/* export.h */
#ifndef EXPORT_H
#define EXPORT_H

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#endif
