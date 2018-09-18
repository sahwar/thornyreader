//
// Created by Admin on 17/9/2018.
//

#ifndef CODE_THORNYREADER_PURE_RECTHELPER_H
#define CODE_THORNYREADER_PURE_RECTHELPER_H


#include <crengine/include/lvtinydom.h>
#include <crengine/include/lvtypes.h>

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

    bool NodeIndexFound_ = false;
    bool isInit          = false;

    void InitFinalNode(ldomNode *finalNode);

    ldomNode* GetFinalNode();

    //lvRect FinalNodeAbsRect();

    bool processRect(ldomXPointerEx xpointer, lvRect &rect);

    bool ifnull(ldomXPointerEx xpointer, lvRect &rect);

    int FindNodeIndex(ldomNode *node);

    void UpdateNode(ldomNode * Node);

public:
    RectHelper() {};

    lvRect getRect(ldomWord word);

    void Run(ldomXRange *range);

    void Invalidate();

};


#endif //CODE_THORNYREADER_PURE_RECTHELPER_H
