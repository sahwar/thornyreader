//
// Created by Admin on 12/10/2018.
//

#ifndef CODE_THORNYREADER_PURE_FOOTNOTESPRINTER_H
#define CODE_THORNYREADER_PURE_FOOTNOTESPRINTER_H

#include "lvtinydom.h"
#include "fb2def.h"

class FootnotesPrinter
{
protected:
    CrDom *doc_ = NULL;
    LvDomWriter* writer_ = NULL;
    lString16 title_ = lString16("Footnotes");
    bool hidden_ = true;
public:

    FootnotesPrinter(){}

    FootnotesPrinter(CrDom *m_doc){
        writer_ = new LvDomWriter(m_doc);
        doc_ = m_doc;
    }

    ldomNode *FindTextInNode(ldomNode *node);

    ldomNode *FindTextInParents(ldomNode *node);

    bool NodeContainsNextNote(ldomNode *node, lString16 nextId);

    bool NodeIsBreak(ldomNode *node, lString16 nextId);

    void recurseNodesToPrint(ldomNode *node, LvDomWriter *writer);

    bool PrintLinksList(LVArray<LinkStruct> LinksList);

    void PrintHeader();

    void PrintNum(lString16 num);

    void PrintLinkNum(lString16 num, lString16 id);

    lString16 StripDocFragment(lString16 in);
};

class VisiNotesPrinter : public  FootnotesPrinter
{
public:
    VisiNotesPrinter(){}

    VisiNotesPrinter(CrDom *m_doc){
        writer_ = new LvDomWriter(m_doc);
        doc_ = m_doc;
        hidden_ = false;
    }
    void SetTitle(lString16 title){title_ = title;}

    void PrintNum(lString16 num, lString16 id);
};

#endif //CODE_THORNYREADER_PURE_FOOTNOTESPRINTER_H
