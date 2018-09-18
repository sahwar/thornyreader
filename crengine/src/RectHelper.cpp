//
// Created by Admin on 17/9/2018.
//

#include "crengine/include/RectHelper.h"

void RectHelper::Invalidate()
{
    //Node_ = NULL;
    finalNode_ = NULL;
    absRect_ = lvRect();
    NodeIndex_ = -1;
    LineIndex_ = 0;

    //txtform clear
}

void RectHelper::Run(ldomXRange *range)
{
    Node_ = range->getStartNode();
    ldomNode * finalNode = GetFinalNode();
    //CRLog::error("NEW FINALNODE = [%s]",LCSTR(finalNode_->getXPath()));

    if (finalNode != finalNode_)
    {
        CRLog::error("Update FinalNode");
        InitFinalNode(finalNode);
    }
    CRLog::error("Update node");
    UpdateNode(Node_);
    //CRLog::error("Node_ Text = %s",LCSTR(Node_->getText()));
}


ldomNode *RectHelper::GetFinalNode()
{
    ldomNode * p = Node_->isElement() ? Node_: Node_->getParentNode() ;
    ldomNode * mainNode = p->getCrDom()->getRootNode();
    ldomNode * finalNode = NULL;
    for (; p; p = p->getParentNode())
    {
        int rm = p->getRendMethod();
        if (rm == erm_final || rm == erm_list_item)
        {
            finalNode = p; // found final block
        }
        else if (p->getRendMethod() == erm_invisible)
        {
            return NULL;
            //return false; // invisible !!!
        }
        if (p == mainNode)
        {
            break;
        }
    }
    return finalNode;
}

void RectHelper::InitFinalNode(ldomNode *finalNode)
{
    if (isInit)
    {
        Invalidate();
    }
    isInit = true;

    finalNode_ = finalNode;
    if (finalNode == NULL)
    {
        CRLog::error("INITFINALNODE NULL");
        return;
    }
    finalNode_->getAbsRect(absRect_);
    RenderRectAccessor r(finalNode_);
    finalNode_->renderFinalBlock(txtform_, &r, r.getWidth());
    //CRLog::error("txtform_->GetLineCount() = %d", txtform_->GetLineCount());
    //CRLog::error("finalnode = [%s]", LCSTR(finalNode_->getXPath()));
    //CRLog::error("finalnode childcount = %d", finalNode_->getChildCount());
}

void RectHelper::UpdateNode(ldomNode *Node)
{
    // text node
    srcIndex_   = -1;
    srcLen_     = -1;
    lastIndex_  = -1;
    lastLen_    = -1;
    lastOffset_ = -1;

    if (finalNode_ == NULL)
    {
        return;
    }

    int index = FindNodeIndex(Node);
    NodeIndex_ = index;

    int count = txtform_->GetSrcCount();
    if (index > 0)
    {
        const src_text_fragment_t *src = txtform_->GetSrcInfo(index);
        bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
        srcIndex_ = index;
        srcLen_ = isObject ? 0 : src->t.len;

        lastIndex_ = index - 1;
        const src_text_fragment_t *src2 = txtform_->GetSrcInfo(lastIndex_);
        bool isObject2 = (src2->flags & LTEXT_SRC_IS_OBJECT) != 0;
        lastLen_ = isObject2 ? 0 : src2->t.len;
        lastOffset_ = isObject2 ? 0 : src2->t.offset;
    }
    else if (index == 0)
    {
        srcIndex_ = index;
        const src_text_fragment_t *src = txtform_->GetSrcInfo(index);
        bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
        srcLen_ = isObject ? 0 : src->t.len;
    }
    else if ( count > 0 ) // srcindex < 0
    {
        lastIndex_ = count-1;
        const src_text_fragment_t *src = txtform_->GetSrcInfo(lastIndex_);
        bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
        srcIndex_ = lastIndex_;
        srcLen_ = isObject ? 0 : src->t.len;
        lastOffset_ = isObject ? 0 : src->t.offset;
    }
    else
    {
        CRLog::error("GetSrcCount: Final node contains no text nodes!");
    }
}

bool RectHelper::ifnull(ldomXPointerEx xpointer, lvRect &rect)
{
    // no base final node, using blocks
    //lvRect rc;
    ldomNode *node = xpointer.getNode();
    //CRLog::error("NEW NODE = [%s]",LCSTR(node->getXPath()));
    //CRLog::error("Node Text = %s",LCSTR(node->getText()));

    int offset = xpointer.getOffset();
    if (offset < 0 || node->getChildCount() == 0)
    {
        CRLog::error("Ifnull 1");
        node->getAbsRect(rect);
        return true;
        //return rc.topLeft();
    }
    if (offset < (int) node->getChildCount())
    {
        CRLog::error("Ifnull 2");
        node->getChildNode(offset)->getAbsRect(rect);
        return true;
        //return rc.topLeft();
    }
    CRLog::error("Ifnull 3");
    node->getChildNode(node->getChildCount() - 1)->getAbsRect(rect);
    return true;
    //return rc.bottomRight();
}
/*
lvRect RectHelper::FinalNodeAbsRect()
{
    lvRect rect;
    finalNode_->getAbsRect(rect);
    if (rect.height() == 0 && rect.width() > 0)
    {
        rect.bottom++;
    }
    return rect;
}
*/

int RectHelper::FindNodeIndex(ldomNode *node)
{
    int start = ( NodeIndex_ < 0 ) ? 0 : NodeIndex_;
    int count = txtform_->GetSrcCount();
    for (int i = start; i < count; i++)
    {
        const src_text_fragment_t *src = txtform_->GetSrcInfo(i);
        if (src->object == node)
        {
            return i;
        }
    }
    CRLog::error("start = %d, count = %d",start,count);
    CRLog::error("NOT FOUND NODE INDEX RETRYING");
    //nodes in txtform appear to be able not to be in order,
    // so we retry search cycle from zero, to find node index again
    for (int i = 0; i < count; i++)
    {
        const src_text_fragment_t *src = txtform_->GetSrcInfo(i);
        if (src->object == node)
        {
            return i;
        }
    }
    CRLog::error("start = %d, count = %d",start,count);
    return -1;

    /*
     for (int i = 0; i < txtform->GetSrcCount(); i++)
        {
            const src_text_fragment_t *src = txtform->GetSrcInfo(i);
            bool isObject = (src->flags & LTEXT_SRC_IS_OBJECT) != 0;
            if (src->object == node)
            {
                srcIndex = i;
                srcLen = isObject ? 0 : src->t.len;
                break;
            }
            lastIndex = i;
            lastLen = isObject ? 0 : src->t.len;
            lastOffset = isObject ? 0 : src->t.offset;
            // Проблемный участок, от которого одновременно зависит
            // генерация оглавлений (Гаррисон), и генерация хитбоксов в некоторых случаях (Storia della mafia)
            // К функции GetRect добавлен параметр forlvpoint, который при вызове в генерации оглавлений, включает этот кусок.
            if (forlvpoint )
            {
                ldomXPointerEx xp2((ldomNode *) src->object, lastOffset);
                if(xp2.compare(xp) > 0)
                {
                srcIndex = i;
                srcLen = lastLen;
                offset = lastOffset;
                break;
                }
            }
            //конец проблемного участка кода
        }

     */
}


lvRect RectHelper::getRect(ldomWord word)
{
    CRLog::trace("GetRect");
    lvRect rect;
    if (word.isNull())
    {
        return rect;
    }
    // get start and end rects
    lvRect rc1;
    lvRect rc1old;
    lvRect rc2;
    lvRect rc2old;
    ldomXPointerEx xp1 = word.getStartXPointer();
    ldomXPointerEx xp2 = word.getEndXPointer();
    CRLog::error("Nodetext = [%s] , LETTER = [%s]",LCSTR(word.getNode()->getText()),LCSTR(word.getText()));
    bool a1 = this->processRect(xp1, rc1);
    //bool a1old = xp1.getRect(rc1old,false);
    bool a2 = this->processRect(xp2, rc2);
    //bool a2old = xp2.getRect(rc2old,false);
    //if(rc1 != rc1old)
    //{
    //    CRLog::error("RC1 New rect != old rect! ^ [%d:%d][%d:%d] != [%d:%d][%d:%d]",rc1.left,rc1.right,rc1.top,rc1.bottom,rc1old.left,rc1old.right,rc1old.top,rc1old.bottom);
    //}
    //if (rc2 != rc2old)
    //{
    //    CRLog::error("RC2 New rect != old rect! ^ [%d:%d][%d:%d] != [%d:%d][%d:%d]",rc2.left,rc2.right,rc2.top,rc2.bottom,rc2old.left,rc2old.right,rc2old.top,rc2old.bottom);
    //}

    //if (!xp1.getRect(rc1,false) || !xp2.getRect(rc2,false)) //OLD OLD
    //if (!xp1.getRect(rc1,false) || !this->processRect(xp2, rc2)) //OLD NEW
    //if (!this->processRect(xp1, rc1) || !this->processRect(xp2, rc2)) //NEW NEW
    if (!a1 || !a2)
    {
        return rect;
    }
    if (rc1.top == rc2.top && rc1.bottom == rc2.bottom)
    {
        // on same line
        rect.left = rc1.left;
        rect.top = rc1.top;
        rect.right = rc2.right;
        rect.bottom = rc2.bottom;
        return rect;
    }
    // on different lines
    ldomXRange range(xp1, xp2);
    ldomNode *parent = range.getNearestCommonParent();
    if (!parent)
    {
        return rect;
    }
    parent->getAbsRect(rect);
    rect.top = rc1.top;
    rect.bottom = rc2.bottom;
    rect.left = rc1.left < rc2.left ? rc1.left : rc2.left;
    rect.right = rc1.right > rc2.right ? rc1.right : rc2.right;
    return rect;

}

bool RectHelper::processRect(ldomXPointerEx xpointer, lvRect &rect)
{
    //return xpointer.getRect(rect, false);
    if (finalNode_ == NULL)
    {
        bool res = ifnull(xpointer, rect);
        //CRLog::error("Rect ifnull = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
        return res;
    }

    lvRect rc;
    finalNode_->getAbsRect(rc);
    //CRLog::error("absrect = [%d:%d][%d:%d]",rc.left,rc.right,rc.top,rc.bottom);
    if (rc.height() == 0 && rc.width() > 0)
    {
        rect = rc;
        rect.bottom++;
        CRLog::error("1");
        return true;
    }
    if (NodeIndex_ < 0 && lastIndex_ < 0)
    {
        CRLog::error("LastIndex < 0");
        return false;
    }
    ldomNode *node = xpointer.getNode();
     CRLog::error("NEW FINAL NODE = [%s]",LCSTR(finalNode_->getXPath()));
    // CRLog::error("FINAL NODE TEXT = [%s]",LCSTR(finalNode_->getText()));
     CRLog::error("NODE      = [%s]",LCSTR(Node_->getXPath()));
    // CRLog::error("NODE TEXT = [%s]",LCSTR(Node_->getText()));

    int offset = xpointer.getOffset();
    //CRLog::error("NEW offset = %d",offset);
    //CRLog::error("NEW lastoffset = %d",lastOffset_);
    if (NodeIndex_ < 0)
    {
        CRLog::error("NEW replacing offset with last offset");
        offset = lastOffset_;
    }
    CRLog::error("srcIndex = %d srcLen = %d lastIndex = %d lastLen = %d lastOffset = %d nodeIndex =%d ",srcIndex_,srcLen_,lastIndex_,lastLen_,lastOffset_,NodeIndex_);
    int count = txtform_->GetLineCount();
    int start = ( LineIndex_< count) ? 0 : LineIndex_;
    for (int l = start; l < txtform_->GetLineCount(); l++)
    {
        const formatted_line_t *frmline = txtform_->GetLineInfo(l);
        for (int w = 0; w < (int) frmline->word_count; w++)
        {
            const formatted_word_t *word = &frmline->words[w];
            bool lastWord = (l == txtform_->GetLineCount() - 1 && w == frmline->word_count - 1);
            if (word->src_text_index >= srcIndex_ || lastWord)
            {

                CRLog::error("l = %d, w = %d line_index = %d",l,w,LineIndex_);
                //CRLog::error("word->src_text_index > srcIndex || offset <= word->t.start");
                //CRLog::error("%d>%d || %d <= %d",word->src_text_index,srcIndex_,offset,word->t.start);
                //CRLog::error("(offset < word->t.start + word->t.len) || (offset == srcLen && offset == word->t.start + word->t.len)");
                //CRLog::error("(%d < %d + %d) || (%d == %d && %d == %d + %d)",offset, word->t.start , word->t.len,offset , srcLen_ ,offset , word->t.start , word->t.len);

                // found word from same src line
                if (word->src_text_index > srcIndex_ || offset <= word->t.start)
                {

                    // before this word
                    rect.left = word->x + rc.left + frmline->x;
                    //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                    rect.top = rc.top + frmline->y;
                    rect.right = rect.left + 1;
                    rect.bottom = rect.top + frmline->height;
                    //              CRLog::error("word->x    = %d",word->x);
                    //              CRLog::error("rc.left    = %d",rc.left);
                    //              CRLog::error("frmline->x = %d",frmline->x);
                    //CRLog::error("Rect1 = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
                    LineIndex_ = l;
                    //CRLog::error("LINEINDEX END = %d",LineIndex_);

                    return true;
                }
                else if ((offset < word->t.start + word->t.len) || (offset == srcLen_ && offset == word->t.start + word->t.len))
                {
                     // pointer inside this word
                    LVFont *font = (LVFont *) txtform_->GetSrcInfo(srcIndex_)->t.font;
                    lUInt16 widths[512];
                    lUInt8 flg[512];
                    lString16 str = node->getText();
                    font->measureText(str.c_str() + word->t.start,
                            offset - word->t.start,
                            widths,
                            flg,
                            word->width + 50,
                            '?',
                            txtform_->GetSrcInfo(srcIndex_)->letter_spacing);
                    int chx = widths[offset - word->t.start - 1];
                    rect.left = word->x + rc.left + frmline->x + chx;
                    //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                    rect.top = rc.top + frmline->y;
                    rect.right = rect.left + 1;
                    rect.bottom = rect.top + frmline->height;
                    //CRLog::error("Rect2 = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
                    LineIndex_ = l;
                    //CRLog::error("LINEINDEX END = %d",LineIndex_);

                    return true;
                }
                else if (lastWord)
                {
                    // after last word
                    rect.left = word->x + rc.left + frmline->x + word->width;
                    //rect.top = word->y + rc.top + frmline->y + frmline->baseline;
                    rect.top = rc.top + frmline->y;
                    rect.right = rect.left + 1;
                    rect.bottom = rect.top + frmline->height;
                    //CRLog::error("Rect3 = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
                    LineIndex_ = l;
                    //CRLog::error("LINEINDEX END = %d",LineIndex_);

                    return true;
                }
            }
        }
    }
    CRLog::error("2");
    return false;
}