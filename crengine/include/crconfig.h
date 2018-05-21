#ifndef CRCONFIG_H
#define CRCONFIG_H

#define TXT_SMART_HEADERS false
#define TXT_SMART_DESCRIPTION false
#define FIRSTPAGE_BLOCKS_MAX 250        //html + xml
#define FIRSTPAGE_BLOCKS_MAX_WORD 4000
#define FIRSTPAGE_BLOCKS_MAX_RTF 2000
#define FIRSTPAGE_BLOCKS_MAX_CHM 5

#define FALLBACK_FONT_ARRAY_SIZE 500
#define FALLBACK_CYCLE_MAX 5
#define FONT_FOLDER "/system/fonts/"
#define FALLBACK_FONTS_ENABLE 1

#define META_MAX_LENGTH 2000

#ifdef TRDEBUG
#define DUMP_DOMTREE 0
#define DEBUG_TREE_DRAW 0 // define to non-zero (1..5) to see block bounds
#else
#define DUMP_DOMTREE 0
#define DEBUG_TREE_DRAW 0
#endif // TRDEBUG

#endif //CRCONFIG_H