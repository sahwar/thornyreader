//
// Created by Admin on 17/9/2018.
//

#ifndef CODE_THORNYREADER_PURE_RECTHELPER_H
#define CODE_THORNYREADER_PURE_RECTHELPER_H


#include <crengine/include/lvtinydom.h>
#include <crengine/include/lvtypes.h>
extern bool templogs;

class RectHelper
{
    ldomNode *Node_ = NULL;
    ldomNode *finalNode_ = NULL;
    LFormattedTextRef txtform_  = LFormattedTextRef();
    lvRect absRect_= lvRect();

    int srcIndex_      = -1;
    int srcLen_        = -1;
    int lastIndex_     = -1;
    int lastLen_       = -1;
    int lastOffset_    = -1;
    int NodeIndex_     = -1;
    int LineIndex_     =  0;
    int NodeLineIndex_ =  0;
    bool NodeIsInvisible_ = false;
    bool isInit           = false;

    ldomNode* GetFinalNode();

    void InitFinalNode(ldomNode *finalNode);

    void InitNode(ldomNode *Node);

    int FindNodeIndex(ldomNode *node, int start);

    int FindLineIndex(ldomNode *node , int start);

    int FindLastIndex(ldomNode *node);

    bool ifnull(ldomXPointerEx xpointer, lvRect &rect);

    void Invalidate();

    bool NodeIsInvisible(ldomNode *node);

public:

    RectHelper() {};

    /*~RectHelper() {
      free(Node_);
      free(finalNode_);
      free(txtform_.get());
      free(&absRect_);
    };*/

    void Init(ldomNode *Node);

    void Init(ldomXRange *range);

    lvRect getRect(ldomWord word);

    lvRect getRect(ldomWord word, bool init);

    lvRect getRect(ldomXPointer xPointer);

    bool processRect(ldomXPointerEx xpointer, lvRect &rect);

    //FOR TOC
    bool FindLastIndexEnable_ = false;

    void ResetLineIndex();
};


#endif //CODE_THORNYREADER_PURE_RECTHELPER_H
