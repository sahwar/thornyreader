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

    virtual bool PrintIsAllowed(lString16 href){ return true;};
};

class Epub3NotesPrinter : public  FootnotesPrinter
{
private:
    LinksMap AsidesMap_;
public:
    Epub3NotesPrinter(){}

    Epub3NotesPrinter(CrDom *m_doc, Epub3Notes Epub3Notes){
        AsidesMap_ = Epub3Notes.AsidesMap_;
        title_ = Epub3Notes.FootnotesTitle_;
        writer_ = new LvDomWriter(m_doc);
        doc_ = m_doc;
        hidden_ = false;
    }
    bool PrintIsAllowed(lString16 href);
};

#endif //CODE_THORNYREADER_PURE_FOOTNOTESPRINTER_H
