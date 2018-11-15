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
            lString16 text = child->getText();
            if(text.startsWith("["))
            {
                text = text.substr(1);
            }
            if( text.endsWith("]"))
            {
                text = text.substr(0,text.length()-1);
            }
            int num;
            if (text.atoi(num))
            {
                continue;
            }

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
        if(text.startsWith("["))
        {
            text = text.substr(1);
        }
        if( text.endsWith("]"))
        {
            text = text.substr(0,text.length()-1);
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
        lString16 text = child->getText();
        if (text.length() == 0)
        {
            continue;
        }
        if(text.startsWith("["))
        {
            text = text.substr(1);
        }
        if( text.endsWith("]"))
        {
            text = text.substr(0,text.length()-1);
        }
        int num;
        if (text.atoi(num))
        {
            CRLog::error("num = %d",num);
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

void FootnotesPrinter::PrintNum(lString16 num)
{
    writer_->OnTagOpen(L"", L"title");
    writer_->OnText(num.c_str(), num.length(), 0);
    writer_->OnTagClose(L"", L"title");
}

void FootnotesPrinter::PrintLinkNum(lString16 num, lString16 id)
{

    writer_->OnTagOpen(L"", L"title");
    writer_->OnTagOpen(L"", L"a");
    writer_->OnAttribute(L"",L"class",L"link_valid");
    writer_->OnAttribute(L"", L"href", ("#"+id).c_str());
    writer_->OnText(num.c_str(), num.length(), 0);
    writer_->OnTagClose(L"", L"a");
    writer_->OnTagClose(L"", L"title");
}

bool FootnotesPrinter::PrintLinksList(LVArray<LinkStruct> LinksList)
{
    //CRLog::error("PRINTER");
    StrMap map;
    PrintHeader();

    for (int i = 0; i < LinksList.length(); i++)
    {
        //CRLog::error("New node to print");
        LinkStruct currlink = LinksList.get(i);
        LinkStruct nextlink = (i+1<LinksList.length())? LinksList.get(i+1) : LinkStruct();
        lString16 nextid;

        if(!this->PrintIsAllowed(currlink.href_))
        {
            continue;
        }
        if (!nextlink.href_.empty())
        {
            nextid = (nextlink.href_.startsWith("#")) ? nextlink.href_.substr(1) : nextlink.href_;
        }
        lString16 num = lString16::itoa(currlink.num_) + lString16("  ");
        lString16 href = (currlink.href_.startsWith("#")) ? currlink.href_.substr(1) : currlink.href_;
        if(map.find(href.getHash())!=map.end())
        {
            continue;
        }
        map[href.getHash()]=href;

        ldomNode *node = doc_->getElementById(href.c_str());
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
        if(node->isNodeName("DocFragment"))
        {
            continue;
        }
        if(this->hidden_)
        {
            href = href + lString16("_note");
        }
        ldomNode *found;
        if (node->isNodeName("section"))
        {
            //fb2, epub structure
            writer_->OnTagOpen(L"", L"section");
            writer_->OnAttribute(L"", L"id", href.c_str());

            if(hidden_)
            {
                PrintNum(num);
            }
            else
            {
                PrintLinkNum(num, currlink.id_);
            }
            recurseNodesToPrint(node, writer_);

            writer_->OnTagClose(L"", L"section");
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
            writer_->OnTagOpen(L"", L"section");
            writer_->OnAttribute(L"", L"id", href.c_str());

            if(hidden_)
            {
                PrintNum(num);
            }
            else
            {
                PrintLinkNum(num, currlink.id_);
            }

            if(found->isText())
            {
                lString16 text = found->getText();
                writer_->OnTagOpen(L"", L"p");
                writer_->OnTagOpen(L"", L"span");
                writer_->OnText(text.c_str(), text.length(),0);
                writer_->OnTagClose(L"", L"span");
                writer_->OnTagClose(L"", L"p");
            }
            else
            {
                writer_->OnTagOpen(L"", found->getNodeName().c_str());
                recurseNodesToPrint(found, writer_);
                writer_->OnTagClose(L"", found->getNodeName().c_str());
            }

            for (int i = index + 1; i < parent->getChildCount(); i++)
            {
                ldomNode *child = parent->getChildNode(i);
                //CRLog::error("child path = %s",LCSTR(child->getXPath()));
                if (NodeIsBreak(child,nextid))
                {
                    break;
                }
                //text is not a number
                writer_->OnTagClose(L"", child->getNodeName().c_str());
                recurseNodesToPrint(child, writer_);
                writer_->OnTagClose(L"", child->getNodeName().c_str());
            }
            writer_->OnTagClose(L"", L"section");
        }
    }

    writer_->OnTagClose(L"", L"body");
    //writer.OnTagOpen(L"", L"NoteFragment");

    return true;
}

void FootnotesPrinter::PrintHeader()
{
    writer_->OnTagOpenNoAttr(L"", L"FictionBook");
    writer_->OnTagOpenNoAttr(L"", L"FictionBook");
    writer_->OnTagOpen(L"", L"body");
    lString16 space("\u200b");

    if(hidden_)
    {
        writer_->OnAttribute(L"", L"name", L"notes_hidden");
        writer_->OnAttribute(L"", L"id", NOTES_HIDDEN_ID); // used in LDocView::GetpagesCount() for hiding all the footnotes pages.
        writer_->OnTagOpenNoAttr(L"", L"h1");
        writer_->OnText(space.c_str(), space.length(), 0); // h1 adds new page break
        writer_->OnTagClose(L"", L"h1");

        writer_->OnTagOpenNoAttr(L"", L"div");
        writer_->OnText(space.c_str(), space.length(), 0); // h1 adds new page break
        writer_->OnTagClose(L"", L"div");
    }
    else
    {
        writer_->OnAttribute(L"", L"name", L"notes");
    }

    writer_->OnTagOpenNoAttr(L"", L"h1");
    writer_->OnText(title_.c_str(), title_.length(), 0); // footnotes header text
    writer_->OnTagClose(L"", L"h1");
}

bool Epub3NotesPrinter::PrintIsAllowed(lString16 href)
{
    if (AsidesMap_.find(href.getHash()) != AsidesMap_.end())
    {
        return true;
    }
    return false;
}
