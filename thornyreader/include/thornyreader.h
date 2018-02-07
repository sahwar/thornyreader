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

#ifndef _THORNYREADER_H_
#define _THORNYREADER_H_

#define THORNYREADER_VERSION "18.02.07+00"
#define THORNYREADER_LOG_TAG "thornyreader"

const bool ThornyReaderIsDebugBuild();
void ThornyReaderStart(const char* name);

#define DOC_FORMAT_NULL 0
#define DOC_FORMAT_EPUB 1
#define DOC_FORMAT_FB2 2
#define DOC_FORMAT_PDF 3
#define DOC_FORMAT_MOBI 4
#define DOC_FORMAT_DJVU 5
#define DOC_FORMAT_DJV 6
#define DOC_FORMAT_DOC 7
#define DOC_FORMAT_RTF 8
#define DOC_FORMAT_TXT 9
#define DOC_FORMAT_XPS 10
#define DOC_FORMAT_OXPS 11
#define DOC_FORMAT_CHM 12
#define DOC_FORMAT_HTML 13

#define CONFIG_CRE_FOOTNOTES 100
#define CONFIG_CRE_EMBEDDED_STYLES 101
#define CONFIG_CRE_EMBEDDED_FONTS 102
#define CONFIG_CRE_FONT_FACE_MAIN 103
#define CONFIG_CRE_FONT_FACE_FALLBACK 104
#define CONFIG_CRE_FONT_COLOR 105
#define CONFIG_CRE_FONT_SIZE 106
/**
 * Supported: 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.5, 1.9
 * See gammatbl.h
 */
#define CONFIG_CRE_FONT_GAMMA 107
/**
 * 0 - no antialias, 1 - antialias big fonts, 2 antialias all
 */
#define CONFIG_CRE_FONT_ANTIALIASING 108
/**
 * Supported: 80, 85, 90, 95, 100, 105, 110, 115, 120, 130, 140, 150, 160, 180, 200
 */
#define CONFIG_CRE_INTERLINE 110
#define CONFIG_CRE_BACKGROUND_COLOR 111
/**
 * 1 for one-column mode, 2 for two-column mode in landscape orientation
 */
#define CONFIG_CRE_PAGES_COLUMNS 112
#define CONFIG_CRE_MARGIN_TOP 113
#define CONFIG_CRE_MARGIN_BOTTOM 114
#define CONFIG_CRE_MARGIN_LEFT 115
#define CONFIG_CRE_MARGIN_RIGHT 116
#define CONFIG_CRE_PAGE_WIDTH 117
#define CONFIG_CRE_PAGE_HEIGHT 118
#define CONFIG_CRE_TEXT_ALIGN 119
#define CONFIG_CRE_HYPHENATION 120
#define CONFIG_CRE_FLOATING_PUNCTUATION 121
#define CONFIG_CRE_FIRSTPAGE_THUMB 122

#define CONFIG_MUPDF_INVERT_IMAGES 200

#define HARDCONFIG_DJVU_RENDERING_MODE 0
#define HARDCONFIG_MUPDF_SLOW_CMYK 0

#endif /* _THORNYREADER_H_ */
