//
// Created by Admin on 16/5/2018.
//
#include "../include/docxhandler.h"
#include "../include/lvstring.h"
#include "../include/lvstream.h"
#include "../include/epubfmt.h"


lString16 DocxGetMainFilePath(LVContainerRef m_arc)
{
    LVStreamRef container_stream = m_arc->OpenStream(L"[Content_Types].xml", LVOM_READ);
    if (!container_stream.isNull())
    {
        CrDom *doc = LVParseXMLStream(container_stream);
        if (doc)
        {
            for (int i = 1; i < 50; i++)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("Types/Override[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                lString16 partname = item->getAttributeValue("PartName");
                lString16 contentType = item->getAttributeValue("ContentType");
                if (contentType.endsWith("document.main+xml"))
                {
                    return partname;
                }
            }
            delete doc;
        }
    }
    return lString16::empty_str;
}

DocxItems DocxParseContentTypes(LVContainerRef m_arc)
{
    LVStreamRef container_stream = m_arc->OpenStream(L"[Content_Types].xml", LVOM_READ);
    DocxItems docxItems_empty;
    if (!container_stream.isNull())
    {
        CrDom *doc = LVParseXMLStream(container_stream);
        if (doc)
        {
            DocxItems docxItems;
            for (int i = 1; i < 50; i++)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("Types/Override[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                lString16 partname = item->getAttributeValue("PartName");
                lString16 contentType = item->getAttributeValue("ContentType");

                DocxItem *docxItem = new DocxItem;
                docxItem->href = partname;
                docxItem->mediaType = contentType;
                docxItems.add(docxItem);
            }
            return docxItems;
        }
        delete doc;

    }
    return docxItems_empty;
}


bool ImportDocxDocument(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb)
{
    LVContainerRef arc = LVOpenArchive(stream);
    if (arc.isNull())
    {
        CRLog::error("This Docx is corrupted: not a ZIP archive");
        return false; // not a ZIP archive
    }
    // check if there is document.xml
    lString16 rootfilePath = DocxGetMainFilePath(arc);
    if (rootfilePath.empty())
    {
        CRLog::error("No main document file found! This is either not a docx file, or the file is corrupted!");
        return false;
    }
    EncryptedDataContainer *decryptor = new EncryptedDataContainer(arc);
    if (decryptor->open())
    {
        CRLog::debug("DOCX: encrypted items detected");
    }

    LVContainerRef m_arc = LVContainerRef(decryptor);

    if (decryptor->hasUnsupportedEncryption())
    {
        // DRM!!!
        return false;
    }

    m_doc->setDocParentContainer(m_arc);


    lString16 codeBase;
    //lString16 css;
    {
        codeBase = LVExtractPath(rootfilePath, false);
        CRLog::trace("codeBase=%s", LCSTR(codeBase));
    }

    LVStreamRef content_stream = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);

    if (content_stream.isNull())
    {
        CRLog::error("4");
        return false;
    }

    //lString16 ncxHref;
    //lString16 coverId;
    //bool CoverPageIsValid = false;

    LVStreamRef content_stream2 = m_arc->OpenStream(L"/word/_rels/document.xml.rels", LVOM_READ);

    if (content_stream2.isNull())
    {
        CRLog::error("5");
        return false;
    }

    CrDom *doc2 = LVParseXMLStream(content_stream2);
    if (!doc2)
    {
        return false;
    }
/*        // for debug
        {
            LVStreamRef out = LVOpenFileStream("/data/data/org.readera/files/document.xml.rels.xml", LVOM_WRITE);
            doc2->saveToStream(out, NULL, true);
        }
*/

    DocxItems docxItems;

    for (int i = 1; i < 50; i++)
    {
        ldomNode *item = doc2->nodeFromXPath(lString16("Relationships/Relationship[") << fmt::decimal(i) << "]");
        if (!item)
        {
            break;
        }
        lString16 id = item->getAttributeValue("Id");
        lString16 mediaType = item->getAttributeValue("Type");
        lString16 target = L"word/";
        target.append(item->getAttributeValue("Target"));
        if (mediaType.endsWith("/image"))
        {
            DocxItem *docxItem = new DocxItem;
            docxItem->href = target;
            docxItem->id = id;
            docxItem->mediaType = mediaType;
            docxItems.add(docxItem);
        }
    }
    CRPropRef m_doc_props = m_doc->getProps();

    LvDomWriter writer(m_doc);
#if 0
    m_doc->setNodeTypes( fb2_elem_table );
	m_doc->setAttributeTypes( fb2_attr_table );
	m_doc->setNameSpaceTypes( fb2_ns_table );
#endif
    //m_doc->setCodeBase( codeBase );

    class TrDocxWriter: public LvDocFragmentWriter {
    public:
        TrDocxWriter(LvXMLParserCallback *parentWriter)
                : LvDocFragmentWriter(parentWriter, cs16("body"), cs16("DocFragment"), lString16::empty_str)
        {
        }
    };

    TrDocxWriter appender(&writer);
    appender.setFlags(132);
    writer.OnStart(NULL);
    writer.OnTagOpenNoAttr(L"", L"body");
    int fragmentCount = 0;


    LVStreamRef stream2 = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);
    if (!stream2.isNull())
    {

        //LvXmlParser
        LvHtmlParser parser(stream2, &appender, firstpage_thumb);
        //if (parser.CheckFormat() && parser.ParseDocx())
        if (parser.ParseDocx(docxItems))
        {
            // valid
            fragmentCount++;
        }
        else
        {
            CRLog::error("Document type is not XML/XHTML for fragment %s", LCSTR(rootfilePath));
        }
    }
//			}
//		}
//	}

    writer.OnTagClose(L"", L"body");
    writer.OnStop();
    //CRLog::debug("EPUB: %d documents merged", fragmentCount);

    //if (fragmentCount == 0)
    {
        //	return false;
    }

#if 0 // set stylesheet
    //m_doc->getStylesheet()->clear();
	m_doc->setStylesheet( NULL, true );
	//m_doc->getStylesheet()->parse(m_stylesheet.c_str());
	if (!css.empty() && m_doc->getDocFlag(DOC_FLAG_ENABLE_INTERNAL_STYLES)) {
		m_doc->setStylesheet( "p.p { text-align: justify }\n"
			"svg { text-align: center }\n"
			"i { display: inline; font-style: italic }\n"
			"b { display: inline; font-weight: bold }\n"
			"abbr { display: inline }\n"
			"acronym { display: inline }\n"
			"address { display: inline }\n"
			"p.title-p { hyphenate: none }\n", false);
		m_doc->setStylesheet(UnicodeToUtf8(css).c_str(), false);
		//m_doc->getStylesheet()->parse(UnicodeToUtf8(css).c_str());
	} else {
		//m_doc->getStylesheet()->parse(m_stylesheet.c_str());
		//m_doc->setStylesheet( m_stylesheet.c_str(), false );
	}
#endif
    return true;
}