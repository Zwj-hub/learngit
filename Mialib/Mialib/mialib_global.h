#ifndef MIALIB_GLOBAL_H
#define MIALIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MIALIB_LIBRARY)
#  define MIALIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MIALIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MIALIB_GLOBAL_H
