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

    int srcIndex_   = -1;
    int srcLen_     = -1;
    int lastIndex_  = -1;
    int lastLen_    = -1;
    int lastOffset_ = -1;
    int NodeIndex_  = -1;
    int LineIndex_  =  0;
    bool NodeIsInvisible_ = false;
    bool isInit           = false;

    ldomNode* GetFinalNode();

    void InitFinalNode(ldomNode *finalNode);

    bool ifnull(ldomXPointerEx xpointer, lvRect &rect);

    void UpdateNode(ldomNode * Node);

    int FindNodeIndex(ldomNode *node, int start);

    int FindLineIndex(ldomNode *node , int start);

    int FindLastIndex(ldomNode *node);

    void Invalidate();

    bool NodeIsInvisible(ldomNode *node);

public:

    RectHelper() {};

    void Run(ldomNode *Node);

    void Run(ldomXRange *range);

    lvRect getRect(ldomWord word);

    bool processRect(ldomXPointerEx xpointer, lvRect &rect);

    bool FindLastIndexEnable_ = false;

};


#endif //CODE_THORNYREADER_PURE_RECTHELPER_H
