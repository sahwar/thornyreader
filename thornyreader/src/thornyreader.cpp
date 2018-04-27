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
#include "StProtocol.h"

const bool ThornyReaderIsDebugBuild() {
#if !defined(NDEBUG) || defined(TRDEBUG) || defined(DEBUG) || defined(_DEBUG)
    return true;
#else
    return false;
#endif
}

void ThornyReaderStart(const char *name) {
#ifdef TRDEBUG
    __android_log_print(ANDROID_LOG_DEBUG,
            THORNYREADER_LOG_TAG,
            "\n\n___________________________________________________________");
#endif
    __android_log_print(ANDROID_LOG_INFO,
                        THORNYREADER_LOG_TAG,
                        "Start %s v%s%s",
                        name,
                        THORNYREADER_VERSION,
                        ThornyReaderIsDebugBuild() ? "-DEBUG" : "");
    if (ThornyReaderIsDebugBuild()) {
        const char *ndebug;
#ifdef NDEBUG
        ndebug = "NDEBUG=def";
#else
        ndebug = "NDEBUG=ndef";
#endif
        const char *axy_debug;
#ifdef TRDEBUG
        axy_debug = "TRDEBUG=def";
#else
        axy_debug = "TRDEBUG=ndef";
#endif
        __android_log_print(ANDROID_LOG_DEBUG,
                            THORNYREADER_LOG_TAG,
                            "DEFINITIONS: %s %s",
                            ndebug,
                            axy_debug);
    }
}

void ThornyReaderVersion(const char* version, CmdResponse& response)
{
    response.cmd = CMD_RES_VERSION;
    response.addIpcString(version, false);
}