#ifndef CRCONFIG_H
#define CRCONFIG_H

#define TXT_SMART_HEADERS false
#define TXT_SMART_DESCRIPTION false
#define FIRSTPAGE_BLOCKS_MAX 250            //html + xml
#define FIRSTPAGE_BLOCKS_MAX_DOCX 500        //html + xml
#define FIRSTPAGE_BLOCKS_MAX_WORD 4000
#define FIRSTPAGE_BLOCKS_MAX_RTF 2000
#define FIRSTPAGE_BLOCKS_MAX_CHM 5

#define FALLBACK_FONT_ARRAY_SIZE 500
#define FALLBACK_CYCLE_MAX 5
#define FONT_FOLDER "/system/fonts/"
#define SYSTEM_FALLBACK_FONTS_ENABLE 1
#define RTL_DISPLAY_ENABLE           1
#define FALLBACK_FACE_DEFAULT lString8("Roboto") // lString8("NONE") to switch it off

#define META_MAX_LENGTH 2000
#define CHAR_HEIGHT_MIN 5
#define PARAEND_REPEAT_MAX 2
#define NOTES_HIDDEN_ID L"__notes_hidden__"
#define NOTES_HIDDEN_MAX_LEN 1000
#define TOC_ITEM_LENGTH_MAX 150

extern int gTextLeftShift;

#ifdef TRDEBUG
#define DUMP_DOMTREE 0
#define DEBUG_TREE_DRAW 0 // define to non-zero (1..5) to see block bounds
#define DEBUG_CRE_PARA_END_BLOCKS 0
#define DEBUG_DRAW_IMAGE_HITBOXES 0
#define DEBUG_GETRECT_LOGS 0
#define DEBUG_NOTES_HIDDEN_SHOW 0
#else
#define DUMP_DOMTREE 0
#define DEBUG_TREE_DRAW 0
#define DEBUG_CRE_PARA_END_BLOCKS 0
#define DEBUG_DRAW_IMAGE_HITBOXES 0
#define DEBUG_GETRECT_LOGS 0
#define DEBUG_NOTES_HIDDEN_SHOW 0
#endif // TRDEBUG

#endif //CRCONFIG_H