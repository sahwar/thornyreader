//
// Created by Admin on 23/11/2018.
//

#ifndef CODE_THORNYREADER_PURE_RTLHANDLER_H
#define CODE_THORNYREADER_PURE_RTLHANDLER_H

#include "include/lvtinydom.h"

//shared
bool char_isRTL(const lChar16 c);

bool char_isPunct(const lChar16 c);

//hitboxes side
LVArray<TextRect> RTL_mix(LVArray<TextRect> in_list);

//txtfmt side
class WordItem
{
public:
    int x_;
    int y_;
    const lChar16* text_=0;
    int len_;
    bool flgHyphen_;
    src_text_fragment_t * srcline_;
    bool is_rtl_ = false;
    WordItem(){}
    WordItem(int x, int y, const lChar16* text, int len,bool flgHyphen,src_text_fragment_t * srcline ):
            x_(x),
            y_(y),
            text_(text),
            len_(len),
            flgHyphen_(flgHyphen),
            srcline_(srcline) {}

    WordItem(int x, int y, const lChar16* text, int len,bool flgHyphen,src_text_fragment_t * srcline, bool is_rtl ):
            x_(x),
            y_(y),
            text_(text),
            len_(len),
            flgHyphen_(flgHyphen),
            srcline_(srcline),
            is_rtl_ (is_rtl){}

    bool hasPunct(){
        if(this->len_>1)
        {
            return false;
        }
        lChar16 ch = this->text_[0];
        return char_isPunct(ch);
    }

    lString16 getText()
    {
        return lString16(text_,len_);
    }

};

void PrintRTL(LVArray<WordItem> WordItems, LVDrawBuf * buf, LVFont* font);

#endif //CODE_THORNYREADER_PURE_RTLHANDLER_H
