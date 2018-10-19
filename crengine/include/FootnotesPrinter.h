//
// Created by Admin on 12/10/2018.
//

#ifndef CODE_THORNYREADER_PURE_FOOTNOTESPRINTER_H
#define CODE_THORNYREADER_PURE_FOOTNOTESPRINTER_H

#include "lvtinydom.h"
#include "fb2def.h"

class FootnotesPrinter
{
private:
    static ldomNode *FindTextInNode(ldomNode *node);

    static ldomNode *FindTextInParents(ldomNode *node);

    static bool NodeContainsNextNote(ldomNode *node, lString16 nextId);

    static bool NodeIsBreak(ldomNode *node, lString16 nextId);

    static void recurseNodesToPrint(ldomNode *node, LvDomWriter *writer);
public:

    static bool AppendLinksToDoc(CrDom *m_doc, LVArray<LinkStruct> LinksList);
};


#endif //CODE_THORNYREADER_PURE_FOOTNOTESPRINTER_H
