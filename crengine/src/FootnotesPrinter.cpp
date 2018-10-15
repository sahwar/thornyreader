//
// Created by Admin on 12/10/2018.
//

#include "crengine/include/crconfig.h"
#include "crengine/include/FootnotesPrinter.h"


void FootnotesPrinter::recurseNodesToPrint(ldomNode *node, LvDomWriter *writer)
{
    if (node->isNull())
    {
        CRLog::error("node is null");
        return;
    }

    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode *child = node->getChildNode(i);
        lString16 nodename = child->getNodeName();

        if (child->isText())
        {
            lString16 text = child->getText();
            //writer->OnTagOpen(L"", nodename.c_str());
            writer->OnTagOpen(L"", L"span");
            writer->OnText(text.c_str(), text.length(), 0);
            writer->OnTagClose(L"", L"span");
            //writer->OnTagClose(L"", nodename.c_str());

            //CRLog::error("<%s> text",LCSTR(child->getNodeName()));
            //CRLog::error("[%s]",LCSTR(text));
            //CRLog::error("</%s> text",LCSTR(child->getNodeName()));
            //CRLog::error("TEXT nodepath = %s",LCSTR(node->getXPath()));
        }
        else if (child->isNodeName("a"))
        {
            if (child->hasAttribute(attr_href))
            {
                lString16 href = child->getHRef();
                if (href != lString16::empty_str)
                {
                    writer->OnTagOpen(L"", nodename.c_str());
                    writer->OnAttribute(L"", L"href", href.c_str());
                    recurseNodesToPrint(child, writer);
                    writer->OnTagClose(L"", nodename.c_str());
                }
            }
            else
            {
                recurseNodesToPrint(child, writer);
            }
        }
        else if (child->isNodeName("title"))
        {
            //skip all title tags including their content
            continue;
        }
        else
        {
            //CRLog::error("<%s>",LCSTR(child->getNodeName()));
            writer->OnTagOpen(L"", nodename.c_str());
            recurseNodesToPrint(child, writer);
            writer->OnTagClose(L"", nodename.c_str());
            //CRLog::error("ELEMENT nodepath = %s",LCSTR(node->getXPath()));
            //CRLog::error("</%s>",LCSTR(child->getNodeName()));
        }
    }
}

ldomNode * FootnotesPrinter::FindTextInNode(ldomNode *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode *child = node->getChildNode(i);

        lString16 text = child->getText();
        //CRLog::error("node = %s",LCSTR(node->getXPath()));
        //CRLog::error("text = %s",LCSTR(text));

        if (text.length() == 0)
        {
            continue;
        }
        int num;
        if (text.atoi(num))
        {
            //CRLog::error("num = %d",num);
            continue;
        }
        //text is not a number
        return child;
    }
    return NULL;
}

ldomNode * FootnotesPrinter::FindTextInParents(ldomNode *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    //CRLog::error("node path = %s",LCSTR(node->getXPath()));

    int index = node->getNodeIndex();
    ldomNode *parent = node->getParentNode();
    if (parent == NULL)
    {
        return NULL;
    }
    for (int i = index + 1; i < parent->getChildCount(); i++)
    {
        ldomNode *child = parent->getChildNode(i);
        //CRLog::error("child path = %s",LCSTR(child->getXPath()));

        lString16 text = child->getText();
        //CRLog::error("text = %s",LCSTR(text));

        if (text.length() == 0)
        {
            continue;
        }
        int num;
        if (text.atoi(num))
        {
            //CRLog::error("num = %d",num);
            continue;
        }
        //text is not a number
        return child;
    }
    return FindTextInParents(parent);
}

bool FootnotesPrinter::NodeContainsNextNote(ldomNode *node, lString16 nextId)
{
    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode *child = node->getChildNode(i);
        if (child->isNodeName("a"))
        {
            if (nextId.empty())
            {
                return false;
            }
            if (child->hasAttribute(attr_id))
            {
                if (child->getAttributeValue(attr_id) == nextId)
                {
                    return true;
                }
            }
            if (child->getText().empty())
            {
                return true;
            }
        }
        else
        {
            return NodeContainsNextNote(child, nextId);
        }
    }
    return false;
}

bool FootnotesPrinter::NodeIsBreak(ldomNode *node, lString16 nextId)
{
    lString16 text = node->getText();
    if (text.empty())
    {
        return true;
    }
    int num;
    if (text.atoi(num))
    {
        return true;
    }
    if (node->isNodeName("pagebreak"))
    {
        return true;
    }
    if (NodeContainsNextNote(node, nextId))
    {
        return true;
    }
    return false;
}

bool FootnotesPrinter::AppendLinksToDoc(CrDom *m_doc, LVArray<LinkStruct> LinksList)
{
   // CRLog::error("PRINTER");
    LvDomWriter writer(m_doc);

    writer.OnTagOpenNoAttr(L"", L"FictionBook");
    writer.OnTagOpenNoAttr(L"", L"FictionBook");
    writer.OnTagOpen(L"", L"body");
    writer.OnAttribute(L"", L"name", L"notes_hidden");
    writer.OnAttribute(L"", L"id", L"__notes_hidden__"); // used in LDocView::GetpagesCount() for hiding all the footnotes pages.

    lString16 hdr("Footnotes");
    lString16 space("\u200b");
    writer.OnTagOpenNoAttr(L"", L"h1");
    writer.OnText(space.c_str(), space.length(), 0); // h1 adds new page break
    writer.OnTagClose(L"", L"h1");
    writer.OnText(space.c_str(), space.length(), 0); // h1 adds new page break

    writer.OnTagOpenNoAttr(L"", L"h1");
    writer.OnText(hdr.c_str(), hdr.length(), 0); // footnotes header text
    writer.OnTagClose(L"", L"h1");

    for (int i = 0; i < LinksList.length(); i++)
    {
        //CRLog::error("New node to print");
        LinkStruct currlink = LinksList.get(i);
        LinkStruct nextlink = (i+1<LinksList.length())? LinksList.get(i+1) : LinkStruct();
        lString16 nextid;
        if (!nextlink.href_.empty())
        {
            nextid = (nextlink.href_.startsWith("#")) ? nextlink.href_.substr(1) : nextlink.href_;
        }
        lString16 num = lString16::itoa(currlink.num_) + lString16("  ");
        lString16 href = (currlink.href_.startsWith("#")) ? currlink.href_.substr(1) : currlink.href_;
        ldomNode *node = m_doc->getElementById(href.c_str());
        if (node == NULL)
        {
            CRLog::error("Failed to get node from href = %s, skipping", LCSTR(href));
            continue;
        }
        if (node->isText())
        {
            CRLog::error("Node is Text, skipping");
            continue;
        }
        href = href + lString16("_note");
        ldomNode *found;
        if (node->isNodeName("section"))
        {
            //fb2, epub structure
            found = node;

            writer.OnTagOpen(L"", L"section");
            writer.OnAttribute(L"", L"id", href.c_str());

            writer.OnTagOpen(L"", L"title");
            writer.OnText(num.c_str(), num.length(), 0);
            writer.OnTagClose(L"", L"title");

            writer.OnTagOpen(L"", found->getNodeName().c_str());
            recurseNodesToPrint(found, &writer);
            writer.OnTagClose(L"", found->getNodeName().c_str());

            writer.OnTagClose(L"", L"section");
        }
        else
        {
            found = FindTextInNode(node);
            if (found == NULL)
            {
                found = FindTextInParents(node);
            }
            if (found == NULL)
            {
                continue;
            }
            int index = found->getNodeIndex();
            ldomNode *parent = found->getParentNode();
            if (parent == NULL)
            {
                return NULL;
            }
            writer.OnTagOpen(L"", L"section");
            writer.OnAttribute(L"", L"id", href.c_str());

            writer.OnTagOpen(L"", L"title");
            writer.OnText(num.c_str(), num.length(), 0);
            writer.OnTagClose(L"", L"title");
            writer.OnTagOpen(L"", found->getNodeName().c_str());
            recurseNodesToPrint(found, &writer);
            writer.OnTagClose(L"", found->getNodeName().c_str());

            for (int i = index + 1; i < parent->getChildCount(); i++)
            {
                ldomNode *child = parent->getChildNode(i);
                //CRLog::error("child path = %s",LCSTR(child->getXPath()));
                if (NodeIsBreak(child,nextid))
                {
                    break;
                }
                //text is not a number
                writer.OnTagClose(L"", child->getNodeName().c_str());
                recurseNodesToPrint(child, &writer);
                writer.OnTagClose(L"", child->getNodeName().c_str());
            }
            writer.OnTagClose(L"", L"section");
        }
    }

    writer.OnTagClose(L"", L"body");
    //writer.OnTagOpen(L"", L"NoteFragment");

    return true;
}

