/**
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef libtrmobi_mobi_h
#define libtrmobi_mobi_h

// mobi_config_h difine disables inclusion of config.h, that generates error
#define mobi_config_h
#include "mobi.h"
#include "../tools/common.h"

#ifdef __cplusplus
extern "C"
{
#endif
    MOBI_EXPORT bool create_epub(const MOBIRawml *rawml, const char *fullpath);
#ifdef __cplusplus
}
#endif

#endif //libtrmobi_mobi_h
