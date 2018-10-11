//
// Created by Admin on 10/10/2018.
//

#include "crengine/include/fb2fmt.h"


void recurseNodesToPrint(ldomNode * node, LvDomWriter* writer)
{
    if (node->isNull())
    {
        CRLog::error("node is null");
        return;
    }

    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        if (child->isText())
        {
            lString16 text = child->getText();
            writer->OnTagOpen(L"",child->getNodeName().c_str());
            writer->OnText(text.c_str(),text.length(),0);
            writer->OnTagClose(NULL,child->getNodeName().c_str());

            //CRLog::error("<%s> text",LCSTR(child->getNodeName()));
            //CRLog::error("[%s]",LCSTR(text));
            //CRLog::error("</%s> text",LCSTR(child->getNodeName()));
            //CRLog::error("TEXT nodepath = %s",LCSTR(node->getXPath()));
        }
        else
        {
            lString16 nodename = child->getNodeName();
            //CRLog::error("<%s>",LCSTR(child->getNodeName()));

            writer->OnTagOpen(L"",nodename.c_str());
            recurseNodesToPrint(child,writer);
            writer->OnTagClose(L"",nodename.c_str());

            //CRLog::error("ELEMENT nodepath = %s",LCSTR(node->getXPath()));
            //CRLog::error("</%s>",LCSTR(child->getNodeName()));
        }
    }
}

bool AppendLinksToDoc(CrDom *m_doc,LVArray<LinkStruct> LinksList)
{
    LvDomWriter writer(m_doc);

    writer.OnTagOpenNoAttr(L"", L"FictionBook");
    writer.OnTagOpenNoAttr(L"", L"FictionBook");
    writer.OnTagOpen(L"", L"body");
    writer.OnAttribute(L"",L"name",L"notes_hidden");

    lString16 hdr("Footnotes");
    writer.OnTagOpenNoAttr(L"", L"h3");
    writer.OnText(hdr.c_str(), hdr.length(), 0);
    writer.OnTagClose(NULL, L"h3");

    for (int i = 0; i < LinksList.length(); i++)
    {
        //CRLog::error("New node to print");
        LinkStruct currlink = LinksList.get(i);
        lString16 num = lString16::itoa(currlink.num_) + lString16("  ");
        lString16 href = (currlink.href_.startsWith("#"))?currlink.href_.substr(1):currlink.href_;
        ldomNode * node = m_doc->getElementById(href.c_str());
        if(node==NULL)
        {
            continue;
        }
        href = href + lString16("_note");

        if(node->isElement())
        {
            writer.OnTagOpen(L"",node->getNodeName().c_str());
            writer.OnAttribute(L"",L"id",href.c_str());
            recurseNodesToPrint(node,&writer);
            writer.OnTagClose(L"",node->getNodeName().c_str());
        }
    }

    writer.OnTagClose(L"", L"body");
    writer.OnTagOpen(L"", L"NoteFragment");

    return true;
}

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
    writer.OnStart(&parser);
    AppendLinksToDoc(m_doc,LinksList);
    writer.OnStop();

    return true;
}

