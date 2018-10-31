//
// Created by Admin on 10/10/2018.
//

#include "crengine/include/fb2fmt.h"
#include "crengine/include/FootnotesPrinter.h"


bool ImportFb2Document(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb)
{
    LvDomWriter writer(m_doc);
    LvXmlParser parser(stream, &writer, false, true, firstpage_thumb);
    LVArray<LinkStruct> LinksList;
    EpubItems epubItems;

    if (!parser.CheckFormat())
    {
        CRLog::trace("!parser->CheckFormat()");
        // delete parser;
        return false;
    }
    parser.setLinksList(LinksList);
    parser.setEpubNotes(epubItems);
    if (!parser.Parse())
    {
        CRLog::trace("!parser->Parse()");
        // delete parser;
        return false;
    }
    LinksList = parser.getLinksList();
    //CRLog::error("Linkslist length = %d",LinksList.length());
    //for (int i = 0; i < LinksList.length(); i++)
    //{
    //	CRLog::error("LinksList %d = %s = %s",LinksList.get(i).num_,LCSTR(LinksList.get(i).id_),LCSTR(LinksList.get(i).href_));
    //}
    if (LinksList.length() > 0)
    {
        writer.OnStart(&parser);
        FootnotesPrinter printer(m_doc);
        printer.PrintLinksList(LinksList);
        writer.OnStop();
    }
    return true;
}

