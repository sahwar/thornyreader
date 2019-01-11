/*
 * Copyright (C) 2016 ThornyReader
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>
#include "thornyreader.h"
#include "thornyreader_version.h"

const bool ThornyBuildDebug() {
#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(TRDEBUG)
    return true;
#else
    return false;
#endif
}

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

std::string ThornyVersion(std::string base_version)
{
#ifdef TR_BUILD_TYPE
    base_version += "+";
    base_version += STRINGIZE_VALUE_OF(TR_BUILD_TYPE);
#endif
    if (ThornyBuildDebug()) {
        base_version += "+DEBUG";
    }
    return base_version;
}

void ThornyStart(const char* name) {
    std::string defines;
#ifdef NDEBUG
    defines += " NDEBUG";
#endif
#ifdef DEBUG
    defines += " DEBUG";
#endif
#ifdef _DEBUG
    defines += " _DEBUG";
#endif
#ifdef TRDEBUG
    defines += " TRDEBUG";
#endif
    if (defines.size() > 0) {
        defines = ". Defines:" + defines;
    }
    __android_log_print(ANDROID_LOG_INFO, THORNYREADER_LOG_TAG,
                        "Start %s v%s%s",
                        name,
                        ThornyVersion(THORNYREADER_BASE_VERSION).c_str(),
                        defines.c_str());
}

void ThornyVersionResporse(const char* base_version, CmdResponse& response)
{
    response.cmd = CMD_RES_VERSION;
    response.addIpcString(ThornyVersion(base_version).c_str(), true);
}