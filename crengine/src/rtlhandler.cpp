//
// Created by Admin on 23/11/2018.
//
// RTL arabic like texts related functions

#include "include/rtlhandler.h"

//shared
bool char_isRTL(const lChar16 c){
/*
    if(c==0x00A6) // BROKEN BAR
        return false;
    if((c>=0x0030)&&(c<=0x0039)) // digits
    {
        return false;
    }
    return true;
*/
    return ( //(c==0x00A6) ||  // UNICODE "BROKEN BAR"
                   (c>=0x0590)&&(c<=0x05FF)) ||
           (c == 0x05BE) || (c == 0x05C0) || (c == 0x05C3) || (c == 0x05C6) ||
           ((c>=0x05D0)&&(c<=0x05F4)) ||
           (c==0x0608) || (c==0x060B) ||
           (c==0x060D) ||
           ((c>=0x061B)&&(c<=0x064A)) ||
           ((c>=0x066D)&&(c<=0x066F)) ||
           ((c>=0x0671)&&(c<=0x06D5)) ||
           ((c>=0x06E5)&&(c<=0x06E6)) ||
           ((c>=0x06EE)&&(c<=0x06EF)) ||
           ((c>=0x06FA)&&(c<=0x0710)) ||
           ((c>=0x0712)&&(c<=0x072F)) ||
           ((c>=0x074D)&&(c<=0x07A5)) ||
           ((c>=0x07B1)&&(c<=0x07EA)) ||
           ((c>=0x07F4)&&(c<=0x07F5)) ||
           ((c>=0x07FA)&&(c<=0x0815)) ||
           (c==0x081A) || (c==0x0824) ||
           (c==0x0828) ||
           ((c>=0x0830)&&(c<=0x0858)) ||
           ((c>=0x085E)&&(c<=0x08AC)) ||
           (c==0x200F) || (c==0xFB1D) ||
           ((c>=0xFB1F)&&(c<=0xFB28)) ||
           ((c>=0xFB2A)&&(c<=0xFD3D)) ||
           ((c>=0xFD50)&&(c<=0xFDFC)) ||
           ((c>=0xFE70)&&(c<=0xFEFC)) ||
           ((c>=0x10800)&&(c<=0x1091B)) ||
           ((c>=0x10920)&&(c<=0x10A00)) ||
           ((c>=0x10A10)&&(c<=0x10A33)) ||
           ((c>=0x10A40)&&(c<=0x10B35)) ||
           ((c>=0x10B40)&&(c<=0x10C48)) ||
           ((c>=0x1EE00)&&(c<=0x1EEBB));
}

bool char_isPunct(const lChar16 c){
    return ((c>=33) && (c<=47)) ||
           ((c>=58) && (c<=64)) ||
           ((c>=91) && (c<=96)) ||
           ((c>=123)&& (c<=126))||
           (c == 0x00A6);
}



//hitboxes side
class TextRectGroup{
public:
    TextRectGroup(){};
    LVArray<TextRect> list_;
    bool is_rtl_ = false;

    int getWidth(LVFont *font)
    {
        int result = 0;
        if(!this->list_.empty())
        {
            for (int i = 0; i < list_.length(); i++)
            {
                result += font->getCharWidth(list_.get(i).getText().firstChar());
            }
        }
        return result;
    }

    lString16 getText()
    {
        lString16 text;
        for (int i = 0; i < this->list_.length(); i++)
        {
            text += this->list_.get(i).getText();
        }
        return text;
    }

    bool hasPunct(){
        if(list_.empty())
        {
            return false;
        }
        if(list_.length()>1)
        {
            return false;
        }
        lChar16 ch = list_[0].getText().firstChar();
        return char_isPunct(ch);
    }

    void addTextRect(TextRect textRect)
    {
        list_.add(textRect);
    }

    bool checkLineRTL()
    {
        if (list_.empty())
        {
            return false;
        }
        //CRLog::error("line length = %d",line.length());

        for (int i = 0; i < list_.length(); i++)
        {
            lChar16 ch = list_.get(i).getText().firstChar();
            if(char_isRTL(ch))
            {
                return true;
            }
        }
        return false;
    }
};

LVArray<TextRect> reverseWord(LVArray<TextRect> in_word)
{
    LVArray<TextRect> result;
    if(in_word.empty())
    {
        return result;
    }
    if(in_word.length()==1)
    {
        return in_word;
    }
    int first_left = in_word.get(0).getRect().left;
    for (int i = in_word.length()-1; i >=0 ; i--)
    {
        TextRect curr = in_word.get(i);
        lvRect curr_rect = curr.getRect();
        int width = curr_rect.width();
        lvRect new_rect(first_left,curr_rect.top,first_left+width,curr_rect.bottom);
        curr.setRect(new_rect);
        result.add(curr);
        first_left += width;
    }
    return result;
}

LVArray<TextRectGroup> reverseWordsOrder(LVArray<TextRectGroup> words)
{
    //CRLog::error("revvwordsorder START");
    LVArray<TextRectGroup> result;
    if(words.empty())
    {
        CRLog::error("words empty");
        return result;
    }
    result.reserve(words.length()+1);

    //use first word in line that contains symbols
    int count = 0;
    for (int i = 0; i < words.length() ; i++)
    {
        if (!words.get(i).list_.empty())
        {
            count=i;
            break;
        }
    }

    TextRect  firstword_txrect = words.get(count).list_.get(0);
    lString16 first_text = firstword_txrect.getText();
    lvRect    first_rect = firstword_txrect.getRect();
    //CRLog::error("first text = [%s], left = %d",LCSTR(first_text),first_rect.left);

    LVFont * font = firstword_txrect.getNode()->getParentNode()->getFont().get();

    int startx  = (first_rect.left>first_rect.right) ? first_rect.right : first_rect.right;
    startx -= first_rect.width();

    TextRectGroup last_word = words.get(words.length()-1);
    TextRect last = last_word.list_.get(last_word.list_.length()-1);

    bool line_isRTL = false;
    for (int i = 0; i < words.length(); i++)
    {
        if(words.get(i).is_rtl_)
        {
            line_isRTL = true;
            break;
        }
    }

    if(line_isRTL && last.getText().lastChar() == ' ')
    {
        startx -= font->getCharWidth(' ');
    }
    //CRLog::error("first left = %d",startx);
    //CRLog::error("rfirst left = %d",first_rect.left);
    //CRLog::error("rfirst right = %d",first_rect.right);
    LVArray<TextRectGroup> nonRTLBuffer;

    int buffwidth = 0;
    bool prev_state = words.get(words.length() - 1).is_rtl_;

    for (int w = words.length()-1; w >= 0; w--)
    {
        TextRectGroup currword = words.get(w);

        if(currword.is_rtl_ || (currword.hasPunct() && prev_state ))
        {
            if(nonRTLBuffer.length()>0)
            {
                /*
                CRLog::trace("nonrtlbuff start 1");
                for (int i = 0; i < nonRTLBuffer.length(); i++)
                {
                    CRLog::error("nonrtlbuffer %d , [%s]",i, LCSTR(nonRTLBuffer.get(i).getText()));
                }
                 */
                TextRectGroup firstitem = nonRTLBuffer.get(0);
                TextRectGroup lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

                if(firstitem.getText() == " ")
                {
                    nonRTLBuffer.remove(0);
                    nonRTLBuffer.add(firstitem);
                }
                if(lastitem.getText() == " ")
                {
                    nonRTLBuffer.remove(nonRTLBuffer.length()-1);
                    nonRTLBuffer.insert(0,lastitem);
                }
                int startx_nonrtlbuff = startx - buffwidth;
                for (int b = nonRTLBuffer.length()-1; b >=0  ; b--)
                {
                    TextRectGroup buff_word = nonRTLBuffer.get(b);
                    //CRLog::error("word from buff = [%s], is rtl = %d, startx = %d", LCSTR(buff_word.getText()), (int) buff_word.is_rtl_, startx_nonrtlbuff);
                    LVFont * font = buff_word.list_[0].getNode()->getParentNode()->getFont().get();
                    int wordwidth = 0;
                    for (int c = 0; c < buff_word.list_.length(); c++)
                    {
                        TextRect  curr_txrect = buff_word.list_.get(c);
                        lString16 curr_txrect_text = curr_txrect.getText();
                        lvRect    curr_txrect_rect = curr_txrect.getRect();
                        int charwidth = font->getCharWidth(curr_txrect_text.firstChar());

                        int height = font->getHeight();

                        lvRect new_rect(startx_nonrtlbuff,curr_txrect_rect.top,startx_nonrtlbuff + charwidth,curr_txrect_rect.top+height);

                        curr_txrect.setRect(new_rect);
                        buff_word.list_.set(c, curr_txrect);
                        wordwidth += charwidth;
                        startx_nonrtlbuff +=charwidth;
                    }
                    result.add(buff_word);
                }
                buffwidth = 0;
                nonRTLBuffer.clear();
            }
            //CRLog::error("rtl word after buff = [%s], is rtl = %d, startx = %d", LCSTR(currword.getText()), (int) currword.is_rtl_, startx);

            for (int c = 0; c < currword.list_.length(); c++)
            {
                TextRect curr = currword.list_.get(c);
                lvRect curr_rect = curr.getRect();
                lString16 currwordtext = curr.getText();
                int charwidth = font->getCharWidth(currwordtext.firstChar());
                int height = font->getHeight();

                curr_rect.right = curr_rect.left + charwidth;
                curr_rect.bottom = curr_rect.top + height;

                int width = curr_rect.width();
                lvRect new_rect(startx, curr_rect.top, startx + width, curr_rect.bottom);
                curr.setRect(new_rect);
                currword.list_.set(c, curr);
                startx += width;
            }
            result.add(currword);
        }
        else //!currword.is_rtl_
        {
            // CRLog::error("word to buff = [%s], is rtl = %d, startx = %d", LCSTR(currword.getText()), (int) currword.is_rtl_, startx);
            nonRTLBuffer.add(currword);
            buffwidth +=currword.getWidth(font);
            startx+=currword.getWidth(font);
        }
        prev_state = currword.is_rtl_;
    }
    if(nonRTLBuffer.length()>0)
    {
        /*
        CRLog::trace("nonrtlbuff start 2");
        for (int i = 0; i < nonRTLBuffer.length(); i++)
        {
            CRLog::error("nonrtlbuffer %d , [%s]",i, LCSTR(nonRTLBuffer.get(i).getText()));
        }
         */
        TextRectGroup firstitem = nonRTLBuffer.get(0);
        TextRectGroup lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

        if(line_isRTL && firstitem.getText() == " ")
        {
            nonRTLBuffer.remove(0);
            nonRTLBuffer.add(firstitem);
        }
        if(line_isRTL && lastitem.getText() == " ")
        {
            nonRTLBuffer.remove(nonRTLBuffer.length()-1);
            nonRTLBuffer.insert(0,lastitem);
        }
        int startx_nonrtlbuff = startx - buffwidth;
        for (int b = nonRTLBuffer.length()-1; b >=0  ; b--)
        {
            TextRectGroup buff_word = nonRTLBuffer.get(b);
            //CRLog::error("nonrtl word in end from buff = [%s], is rtl = %d, startx = %d", LCSTR(buff_word.getText()), (int) buff_word.is_rtl_, startx_nonrtlbuff);

            int wordwidth = 0;

            for (int c = 0; c < buff_word.list_.length(); c++)
            {
                TextRect  curr_txrect = buff_word.list_.get(c);
                lString16 curr_txrect_text = curr_txrect.getText();
                lvRect    curr_txrect_rect = curr_txrect.getRect();
                int charwidth = font->getCharWidth(curr_txrect_text.firstChar());

                int height = font->getHeight();

                lvRect new_rect(startx_nonrtlbuff,curr_txrect_rect.top,startx_nonrtlbuff + charwidth,curr_txrect_rect.top+height);

                curr_txrect.setRect(new_rect);
                buff_word.list_.set(c, curr_txrect);
                wordwidth += charwidth;
                startx_nonrtlbuff +=charwidth;
            }
            result.add(buff_word);
        }
        buffwidth = 0;
        nonRTLBuffer.clear();
    }
    //CRLog::error("revvwordsorder END");

    return result;
}

LVArray<TextRect> reverseLine(TextRectGroup group)
{
    LVArray<TextRect> result;
    LVArray<TextRect> line = group.list_;
    LVArray<TextRectGroup> words;
    words.reserve(line.length());

    if (line.empty())
    {
        CRLog::error("Line is empty.");
        return result;
    }
    //CRLog::error("line length = %d",line.length());


    //CRLog::error("WORDS BREAKUP START");
    int start = 0;
    bool last_space = false;
    bool last_punct = false;

    lString16 last_text = line.get(0).getText();
    bool last_state = char_isRTL(last_text.firstChar());

    for (int c = 0; c < line.length(); c++)
    {
        TextRect curr = line.get(c);
        lString16 curr_text = curr.getText();

        lChar16 ch = curr_text.firstChar();
        bool is_space = ch == ' ';
        bool is_punct = char_isPunct(ch);

        //bool curr_state = (is_space)? last_state : char_isRTL(ch);
        bool curr_state;
        if(is_space)
        {
            curr_state = last_state;
        }
        else if(is_punct)
        {
            curr_state = (last_space)? false : last_state ;
        }
        else
        {
            curr_state = char_isRTL(ch);
        }
        //curr_state = (is_punct && last_space)? : curr_state
        bool break_char = (is_space || last_space || is_punct || last_punct);

        //CRLog::error("letter = [%s]",LCSTR(curr_text));
        if (curr_state != last_state || break_char )
        {
            int len = c-start;
            if(len>0)
            {
                TextRectGroup word;
                word.list_.reserve(c - start + 1);
                for (int i = start; i < c; i++)
                {
                    word.list_.add(line.get(i));
                }

                //word.is_rtl_ = (is_punct)? ( (last_space)? true : curr_state) : last_state;
                word.is_rtl_ = last_state;
                words.add(word);
                start = c;
            }
        }
        last_state = curr_state;
        last_space = is_space;
        last_punct = is_punct;
    }
    TextRectGroup word;
    word.list_.reserve(line.length() - start + 1);
    for (int i = start; i < line.length(); i++)
    {
        word.list_.add(line.get(i));
    }
    word.is_rtl_ = last_state;
    words.add(word);
    //CRLog::error("added word = [%s]  (%s)",LCSTR(word.getText()),(word.is_rtl_)?"RTL":"NOT RTL");
    //CRLog::error("WORDS BREAKUP END");

    //to avoid first space problems with text formatter
    if(words.get(0).getText() == " ")
    {
        words.add(words.get(0));
        words.remove(0);
    }
    words = reverseWordsOrder(words);

    for (int w = 0; w < words.length(); w++)
    {
        if(words[w].is_rtl_)// rtl word
        {
            words[w].list_ = reverseWord(words[w].list_);
        }
    }

    for (int i = 0; i < words.length(); i++)
    {
        for (int j = 0; j < words.get(i).list_.length(); j++)
        {
            TextRect curr = words.get(i).list_.get(j);
            result.add(curr);
        }
    }

    return result;
}

TextRect trimFirstSpace(TextRectGroup *line)
{
    TextRect zero;
    TextRect first = line->list_.get(0);
    if( (first.getText().firstChar() == ' ') && line->checkLineRTL())
    {
        int spacewidth = first.getRect().width();
        zero = first;
        line->list_.remove(0);
        for (int c = 0; c < line->list_.length(); c++)
        {
            lvRect newrect = line->list_.get(c).getRect();
            newrect.left -= spacewidth;
            newrect.right -= spacewidth;
            line->list_[c].setRect(newrect);
        }
    }
    return zero;
}

TextRect fixFirstSpace(TextRectGroup *line,TextRect zero)
{
    int firstleft = line->list_.get(0).getRect().left;
    lvRect zerorect = zero.getRect();
    int width = zerorect.width();
    zerorect.right = firstleft;
    zerorect.left = firstleft - width;
    zero.setRect(zerorect);
    return zero;
}

LVArray<TextRect> RTL_mix(LVArray<TextRect> in_list)
{
    LVArray<TextRect> result_list;
    if(in_list.empty())
    {
        return result_list;
    }
    LVArray<TextRectGroup> lines;
    TextRect first = in_list.get(0);
    lvRect first_rect = first.getRect();

    TextRectGroup line;
    ldomNode * last_node = first.getNode();
    bool last_state = last_node->isRTL();
    bool curr_state;

    for (int i = 0; i < in_list.length(); i++)
    {
        TextRect curr = in_list[i];
        curr.setIndex(i);
        lvRect curr_rect = curr.getRect();
        ldomNode * curr_node = curr.getNode();

        curr_state = ( curr_node != last_node)? curr_node->isRTL() : last_state;

        if(curr_rect.top > first_rect.top)
        {
            lines.add(line);
            line = TextRectGroup();
            first_rect = curr_rect;
        }
        line.addTextRect(curr);
        last_node = curr_node;
        last_state = curr_state;
        if(curr_state)
        {
            line.is_rtl_ = true;
        }
    }
    lines.add(line);

    for (int l = 0; l < lines.length(); l++)
    {
        TextRect zero;
        if(lines[l].is_rtl_)// && lines[l].checkLineRTL())
        {
            zero = trimFirstSpace(&lines[l]);
            CRLog::trace("reverse line text = [%s]",LCSTR(lines[l].getText()));
            lines[l].list_ = reverseLine(lines[l]);
        }

        if(zero.getIndex() >= 0 ) //zero is initialized
        {
            zero = fixFirstSpace(&lines[l],zero);
            result_list.add(zero);
        }
        for (int c = 0; c < lines[l].list_.length(); c++)
        {
            TextRect curr = lines[l].list_.get(c);
            result_list.add(curr);
        }
    }
    return result_list;
}



//textfmt side

void PrintRTL(LVArray<WordItem> WordItems, LVDrawBuf * buf, LVFont * font)
{
    if(WordItems.empty() || buf == NULL || font == NULL)
    {
        return;
    }
    int startx = WordItems.get(0).x_;
    bool line_isRTL = false;
    for (int i = 0; i < WordItems.length(); i++)
    {
        if(WordItems.get(i).is_rtl_)
        {
            line_isRTL = true;
            break;
        }
    }
    WordItem last = WordItems.get(WordItems.length()-1);
    if(line_isRTL && last.getText().lastChar() == ' ')
    {
        startx -= font->getCharWidth(' ');
    }
    LVArray<WordItem> nonRTLBuffer;
    int buffwidth = 0;
    bool prev_state = WordItems.get(WordItems.length() - 1).is_rtl_;

    lString16 text;
//    for (int w = WordItems.length() - 1; w >= 0; w--)
    for (int w = 0 ; w <  WordItems.length(); w++)
    {
        WordItem curr = WordItems.get(w);
        text += lString16(curr.text_,curr.len_);
    }
    CRLog::error("fmt line = [%s]",LCSTR(text));


    for (int w = WordItems.length() - 1; w >= 0; w--)
    {
        //WordItem prev = WordItems.get((w>0)?w-1:0);
        WordItem curr = WordItems.get(w);
        //WordItem next = WordItems.get((w<WordItems.length()-1)?w+1:(w<WordItems.length()-1));

        LVFont *font = (LVFont *) curr.srcline_->t.font;
        lUInt32 width = font->getTextWidth(curr.text_, curr.len_);
        //CRLog::error("curr = %d , isrtl = %d",w, (int)curr.is_rtl_);

        if( curr.is_rtl_ || (curr.hasPunct() && prev_state))
        {
            if(nonRTLBuffer.length()>0)
            {
                WordItem firstitem = nonRTLBuffer.get(0);
                WordItem lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

                if(firstitem.getText() == " ")
                {
                    nonRTLBuffer.remove(0);
                    nonRTLBuffer.add(firstitem);
                }
                if(lastitem.getText() == " ")
                {
                    nonRTLBuffer.remove(nonRTLBuffer.length()-1);
                    nonRTLBuffer.insert(0,lastitem);
                }
                int startx_nonrtlbuff = startx - buffwidth;
                for (int k =nonRTLBuffer.length()-1; k >=0 ; k--)
                {
                    WordItem curr = nonRTLBuffer.get(k);
                    LVFont *font = (LVFont *) curr.srcline_->t.font;

                    //CRLog::error("nonrtlbuf1 , word = [%s]",LCSTR(lString16(curr.text_,curr.len_)));

                    font->DrawTextString(buf,
                            startx_nonrtlbuff,
                            curr.y_,
                            curr.text_,
                            curr.len_,
                            '?',
                            NULL,
                            curr.flgHyphen_,
                            curr.srcline_->flags & 0x0F00,
                            curr.srcline_->letter_spacing,
                            false);
                    lUInt32 width = font->getTextWidth(curr.text_, curr.len_);
                    startx_nonrtlbuff += width;
                }
                buffwidth = 0;
                nonRTLBuffer.clear();
            }

            //CRLog::error("rtl after buf, word = [%s]",LCSTR(lString16(curr.text_,curr.len_)));

            font->DrawTextString(buf,
                    startx,
                    curr.y_,
                    curr.text_,
                    curr.len_,
                    '?',
                    NULL,
                    curr.flgHyphen_,
                    curr.srcline_->flags & 0x0F00,
                    curr.srcline_->letter_spacing,
                    true);

            startx += width;
        }
        else
        {
            nonRTLBuffer.add(WordItem(startx,
                    curr.y_,
                    curr.text_,
                    curr.len_,
                    curr.flgHyphen_,
                    curr.srcline_));
            startx += width;
            buffwidth +=width;
        }
        prev_state = curr.is_rtl_;
    }
    if(nonRTLBuffer.length()>0)
    {
        WordItem firstitem = nonRTLBuffer.get(0);
        WordItem lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

        if(line_isRTL && firstitem.getText() == " ")
        {
            nonRTLBuffer.remove(0);
            nonRTLBuffer.add(firstitem);
        }
        if(line_isRTL && lastitem.getText() == " ")
        {
            nonRTLBuffer.remove(nonRTLBuffer.length()-1);
            nonRTLBuffer.insert(0,lastitem);
        }
        int startx_nonrtlbuff = startx - buffwidth;
        for (int k =nonRTLBuffer.length()-1; k >=0 ; k--)
        {

            WordItem curr = nonRTLBuffer.get(k);
            //CRLog::error(" nonrtl no rtl in front , word = [%s]",LCSTR(lString16(curr.text_,curr.len_)));
            LVFont *font = (LVFont *) curr.srcline_->t.font;
            font->DrawTextString(buf,
                    startx_nonrtlbuff,
                    curr.y_,
                    curr.text_,
                    curr.len_,
                    '?',
                    NULL,
                    curr.flgHyphen_,
                    curr.srcline_->flags & 0x0F00,
                    curr.srcline_->letter_spacing,
                    false);
            lUInt32 width = font->getTextWidth(curr.text_, curr.len_);
            startx_nonrtlbuff += width;
        }
        nonRTLBuffer.clear();
    }
}
