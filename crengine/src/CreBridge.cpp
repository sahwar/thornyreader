#include <stdlib.h>
#include <stdio.h>
#include "include/thornyreader.h"
#include "include/StProtocol.h"
#include "include/StSocket.h"
#include "include/CreBridge.h"

static inline int CeilToEvenInt(int n)
{
    return (n + 1) & ~1;
}

static inline int FloorToEvenInt(int n)
{
    return n & ~1;
}

static inline uint32_t ExportPagesCount(int columns, int pages)
{
	if (columns == 2) {
		return (uint32_t) (CeilToEvenInt(pages) / columns);
	}
	return (uint32_t) pages;
}

static inline int ExportPage(int columns, int page)
{
	if (columns == 2) {
		return FloorToEvenInt(page) / columns;
	}
	return page;
}

static inline int ImportPage(int columns, int page)
{
	return page * columns;
}

static const int ALLOWED_INTERLINE_SPACES[] =
{
        70, 75, 80, 85, 90, 95, 100, 105, 110, 115,
        120, 125, 130, 135, 140, 145, 150, 160, 180, 200
};

static const int ALLOWED_FONT_SIZES[] =
{
		12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
		31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		42, 44, 46, 48, 50,
		52, 54, 56, 58, 60,
		62, 64, 66, 68, 70,
        72, 74, 76, 78, 80,
        82, 84, 86, 88, 90,
        92, 94, 96, 98, 100,
        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,
        125, 130, 135, 140, 145, 150, 155, 160
};

static int GetClosestValueInArray(const int array[], int array_lenght, int value)
{
	int closest_value = -1;
	int smallest_delta = -1;
	for (int i = 0; i < array_lenght; i++) {
		int delta = array[i] - value;
		if (delta < 0) {
			delta = -delta;
		}
		if (smallest_delta == -1 || smallest_delta > delta) {
			smallest_delta = delta;
			closest_value = array[i];
		}
	}
	return closest_value;
}

void CreBridge::responseAddLinkUnknown(CmdResponse& response, lString16 href,
                                       float l, float t, float r, float b)
{
    response.addWords(LINK_TARGET_UNKNOWN, 0);
    responseAddString(response, href);
    response.addFloat(l);
    response.addFloat(t);
    response.addFloat(r);
    response.addFloat(b);
}

void CreBridge::responseAddString(CmdResponse& response, lString16 str16)
{
    lString8 str8 = UnicodeToUtf8(str16);
    uint32_t size = (uint32_t) str8.size();
    // We will place null-terminator at the string end
    size++;
    CmdData* cmd_data = new CmdData();
    unsigned char* str_buffer = cmd_data->newByteArray(size);
    memcpy(str_buffer, str8.c_str(), (size - 1));
    str_buffer[size - 1] = 0;
    response.addData(cmd_data);
}

void CreBridge::convertBitmap(LVColorDrawBuf* bitmap)
{
    if (bitmap->GetBitsPerPixel() == 32) {
        // Convert Cre colors to Android
        int size = bitmap->GetWidth() * bitmap->GetHeight();
        for (lUInt8* p = bitmap->GetData(); --size >= 0; p+=4) {
            // Invert A
            p[3] ^= 0xFF;
            // Swap R and B
            lUInt8 temp = p[0];
            p[0] = p[2];
            p[2] = temp;
        }
    }
}

void CreBridge::processFonts(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PDF_FONTS;
    CmdDataIterator iter(request.first);
    while (iter.hasNext()) {
        uint32_t font_family;
        if (!iter.getInt(&font_family).isValid()) {
            CRLog::error("No font family found");
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        uint8_t* fonts[4];
        iter.getByteArray(&fonts[0])
                .getByteArray(&fonts[1])
                .getByteArray(&fonts[2])
                .getByteArray(&fonts[3]);
        if (!iter.isValid()) {
            CRLog::error("processFonts no fonts");
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        for (int i = 0; i < 4; i++) {
            const char* const_font = reinterpret_cast<const char*>(fonts[i]);
            if (strlen(const_font) == 0) {
                continue;
            }
            lString16 font = lString16(const_font);
            fontMan->RegisterFont(UnicodeToUtf8(font));
        }
    }
}

void CreBridge::processConfig(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_SET_CONFIG;
    CmdDataIterator iter(request.first);
    if (!doc_view_) {
        doc_view_ = new LVDocView();
    }
    while (iter.hasNext()) {
        uint32_t key;
        uint8_t* temp_val;
        iter.getInt(&key).getByteArray(&temp_val);
        if (!iter.isValid()) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        const char* val = reinterpret_cast<const char*>(temp_val);
        if (key == CONFIG_CRE_PAGE_WIDTH) {
            int int_val = atoi(val);
            doc_view_->Resize(int_val, doc_view_->height_);
        } else if (key == CONFIG_CRE_PAGE_HEIGHT) {
            int int_val = atoi(val);
            doc_view_->Resize(doc_view_->width_, int_val);
        } else if (key == CONFIG_CRE_FONT_ANTIALIASING) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 2) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            fontMan->SetAntialiasMode(int_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FONT_GAMMA) {
            double gamma = 1.0;
            if (sscanf(val, "%lf", &gamma) == 1) {
                fontMan->SetGamma(gamma);
            }
        } else if (key == CONFIG_CRE_PAGES_COLUMNS) {
            int int_val = atoi(val);
            if (int_val < 1 || int_val > 2) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            if (doc_view_->page_columns_ != int_val) {
                doc_view_->page_columns_ = int_val;
                doc_view_->UpdateLayout();
                doc_view_->RequestRender();
                doc_view_->position_is_set_ = false;
            }
        } else if (key == CONFIG_CRE_FONT_COLOR) {
            doc_view_->text_color_ = (lUInt32) (atoi(val) & 0xFFFFFF);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_BACKGROUND_COLOR) {
            doc_view_->background_color_ = (lUInt32) (atoi(val) & 0xFFFFFF);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_MARGIN_LEFT) {
            int margin = atoi(val);
            doc_view_->cfg_margins_.left = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_MARGIN_TOP) {
            int margin = atoi(val);
            doc_view_->cfg_margins_.top = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_MARGIN_RIGHT) {
            int margin = atoi(val);
            doc_view_->cfg_margins_.right = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_MARGIN_BOTTOM) {
            int margin = atoi(val);
            doc_view_->cfg_margins_.bottom = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_FONT_FACE_MAIN) {
            doc_view_->cfg_font_face_ = UnicodeToUtf8(lString16(val));
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FONT_FACE_FALLBACK) {
            fontMan->SetFallbackFontFace(UnicodeToUtf8(lString16(val)));
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FONT_SIZE) {
            int int_val = atoi(val);
            int array_lenght = sizeof(ALLOWED_FONT_SIZES) / sizeof(int);
            int_val = GetClosestValueInArray(ALLOWED_FONT_SIZES, array_lenght, int_val);
            if (doc_view_->cfg_font_size_ != int_val) {
                doc_view_->cfg_font_size_ = int_val;
                doc_view_->UpdatePageMargins();
                doc_view_->RequestRender();
            }
        } else if (key == CONFIG_CRE_INTERLINE) {
            int int_val = atoi(val);
            int array_lenght = sizeof(ALLOWED_INTERLINE_SPACES) / sizeof(int);
            int_val = GetClosestValueInArray(ALLOWED_INTERLINE_SPACES, array_lenght, int_val);
            if (doc_view_->cfg_interline_space_ != int_val) {
                    doc_view_->cfg_interline_space_ = int_val;
                    doc_view_->RequestRender();
                    doc_view_->position_is_set_ = false;
            }
        } else if (key == CONFIG_CRE_EMBEDDED_STYLES) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_embeded_styles_ = bool_val;
            doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_EMBEDDED_STYLES, bool_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_EMBEDDED_FONTS) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_embeded_fonts_ = bool_val;
            doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_EMBEDDED_FONTS, bool_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FOOTNOTES) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_enable_footnotes_ = bool_val;
            doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_ENABLE_FOOTNOTES, bool_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_TEXT_ALIGN) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 3) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            doc_view_->SetTextAlign(int_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_HYPHENATION) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            if (int_val == 0) {
                HyphMan::activateDictionary(lString16(HYPH_DICT_ID_NONE));
            } else {
                HyphMan::activateDictionary(lString16(HYPH_DICT_ID_ALGORITHM));
            }
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FLOATING_PUNCTUATION) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            if (bool_val != gFlgFloatingPunctuationEnabled) {
                gFlgFloatingPunctuationEnabled = bool_val;
            }
        } else if (key == CONFIG_CRE_FIRSTPAGE_THUMB) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_firstpage_thumb_ = bool_val;
        } else {
            CRLog::warn("processConfig unknown key: key=%d, val=%s", key, val);
        }
    }
    doc_view_->RenderIfDirty();
    response.addInt(ExportPagesCount(doc_view_->GetColumns(), doc_view_->GetPagesCount()));
}

void CreBridge::processOpen(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_OPEN;
    CmdDataIterator iter(request.first);
    uint32_t doc_format = 0;
    uint8_t* socket_name = NULL;
    uint8_t* absolute_path_arg = NULL;
    uint32_t compressed_size = 0;
    uint32_t smart_archive_arg = 0;
    iter.getInt(&doc_format)
            .getByteArray(&socket_name)
            .getByteArray(&absolute_path_arg)
            .getInt(&compressed_size)
            .getInt(&smart_archive_arg);
    if (!iter.isValid() || !socket_name || !absolute_path_arg) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    StSocketConnection connection((const char*) socket_name);
    if (!connection.isValid()) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    int fd;
    bool received = connection.receiveFileDescriptor(fd);
    if (!received) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char* absolute_path = reinterpret_cast<const char*>(absolute_path_arg);
    bool smart_archive = (bool) smart_archive_arg;
    bool result = doc_view_->LoadDoc(doc_format, absolute_path, compressed_size, smart_archive);
    if (result) {
        doc_view_->RenderIfDirty();
        response.addInt(ExportPagesCount(doc_view_->GetColumns(), doc_view_->GetPagesCount()));
    } else if (compressed_size > 0) {
        response.result = RES_ARCHIVE_COLLISION;
    } else {
        response.result = RES_INTERNAL_ERROR;
    }
}

void CreBridge::processPageRender(CmdRequest& request, CmdResponse& response)
{
    //CRLog::trace("processPageRender START");
    response.cmd = CMD_RES_PAGE_RENDER;
    CmdDataIterator iter(request.first);
    uint32_t page;
    uint32_t width;
    uint32_t height;
    iter.getInt(&page).getInt(&width).getInt(&height);
    if (!iter.isValid()) {
        CRLog::error("processPageRender bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc_view_ == NULL) {
        CRLog::error("processPageRender doc not opened");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc_view_->height_ != height || doc_view_->width_ != width) {
        CRLog::error("processPageRender wanted page size mismatch");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    doc_view_->GoToPage(ImportPage(page, doc_view_->GetColumns()));
    CmdData* resp = new CmdData();
    unsigned char* pixels = resp->newByteArray(width * height * 4);
    LVColorDrawBuf* buf = new LVColorDrawBuf(width, height, pixels, 32);
    doc_view_->Draw(*buf);
    convertBitmap(buf);
    delete buf;
    response.addData(resp);
    //CRLog::trace("processPageRender END");
}

void CreBridge::processPage(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE;
}

void CreBridge::processPageLinks(CmdRequest& request, CmdResponse& response)
{
#define DEBUG_LINKS
    response.cmd = CMD_RES_LINKS;
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid()) {
        CRLog::error("processPageLinks bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);
#ifdef DEBUG_LINKS
    CRLog::debug("processPageLinks external_page=%d page=%d page_width=%d page_height=%d",
                 external_page, page, doc_view_->GetWidth(), doc_view_->GetHeight());
    if (0) {
        CRLog::debug("processPageLinks text: %s", LCSTR(doc_view_->GetPageText(page)));
    }
#endif
    ldomXRangeList list;
    doc_view_->GetCurrentPageLinks(list);
    if (list.empty()) {
        return;
    }
    float page_width = doc_view_->GetWidth();
    float page_height = doc_view_->GetHeight();
    for (int i = 0; i < list.length(); i++) {
        ldomXRange* link = list[i];
        lvRect raw_rect;
        link->getRect(raw_rect);
        lvRect rect = lvRect(raw_rect.left, raw_rect.top, raw_rect.right, raw_rect.bottom);
        if (!doc_view_->DocToWindowRect(rect)) {
#ifdef DEBUG_LINKS
            ldomNode* start_node = link->getStart().getNode();
            ldomNode* end_node = link->getEnd().getNode();
            CRLog::warn("processPageLinks DocToWindowRect fail %s\n  %d:%d-%d:%d\n  %s %d\n  %s %d",
                        LCSTR(link->getHRef()),
                        raw_rect.left, raw_rect.right, raw_rect.top, raw_rect.bottom,
                        LCSTR(link->getStart().toString()), start_node->getDataIndex(),
                        LCSTR(link->getEnd().toString()), end_node->getDataIndex());
#endif
            continue;
        }
        float l = rect.left / page_width;
        float t = rect.top / page_height;
        float r = rect.right / page_width;
        float b = rect.bottom / page_height;
        lString16 href = link->getHRef();
        uint16_t target_page = 0;
        if (href.length() > 1 && href[0] == '#') {
            lString16 ref = href.substr(1, href.length() - 1);
            lUInt16 id = doc_view_->GetCrDom()->getAttrValueIndex(ref.c_str());
            ldomNode* node = doc_view_->GetCrDom()->getNodeById(id);
            if (node) {
                ldomXPointer position(node, 0);
                target_page = (uint16_t) doc_view_->GetPageForBookmark(position);
                target_page = (uint16_t) ExportPage(doc_view_->GetColumns(), target_page);
                response.addWords(LINK_TARGET_PAGE, target_page);
                response.addFloat(l);
                response.addFloat(t);
                response.addFloat(r);
                response.addFloat(b);
                response.addFloat(.0F);
                response.addFloat(.0F);
            } else {
                responseAddLinkUnknown(response, href, l, t, r, b);
            }
        } else if (href.startsWith("http:") || href.startsWith("https:")) {
            response.addWords(LINK_TARGET_URI, 0);
            responseAddString(response, href);
            response.addFloat(l);
            response.addFloat(t);
            response.addFloat(r);
            response.addFloat(b);
        } else {
            responseAddLinkUnknown(response, href, l, t, r, b);
        }
#ifdef DEBUG_LINKS
        ldomNode* start_node = link->getStart().getNode();
        ldomNode* end_node = link->getEnd().getNode();
        CRLog::trace("processPageLinks %s %d\n  %d:%d-%d:%d %d:%d-%d:%d\n  %s %d\n  %s %d",
                     LCSTR(href), target_page, rect.left, rect.right, rect.top, rect.bottom,
                     raw_rect.left, raw_rect.right, raw_rect.top, raw_rect.bottom,
                     LCSTR(link->getStart().toString()), start_node->getDataIndex(),
                     LCSTR(link->getEnd().toString()), end_node->getDataIndex());
#endif
    }
#undef DEBUG_LINKS
}

void CreBridge::processPageByXPath(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_CRE_PAGE_BY_XPATH;
    CmdDataIterator iter(request.first);
    uint8_t* xpath_string;
    iter.getByteArray(&xpath_string);
    if (!iter.isValid()) {
        CRLog::error("processPageByXPath invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    lString16 xpath(reinterpret_cast<const char*>(xpath_string));
    ldomXPointer bm = doc_view_->GetCrDom()->createXPointer(xpath);
    if (bm.isNull()) {
        CRLog::error("processPageByXPath bad xpath bm.isNull()");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    doc_view_->GoToBookmark(bm);
    int current_page = doc_view_->GetCurrPage();
    if (current_page < 0) {
        CRLog::error("processPageByXPath bad xpath current_page < 0");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    response.addInt((uint32_t) ExportPage(doc_view_->GetColumns(), current_page));
}

void CreBridge::processPageXPath(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_CRE_PAGE_XPATH;
    CmdDataIterator iter(request.first);
    uint32_t page;
    iter.getInt(&page);
    if (!iter.isValid()) {
        CRLog::error("processPageXPath invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    ldomXPointer xptr = doc_view_->getPageBookmark(ImportPage(page, doc_view_->GetColumns()));
    if (xptr.isNull()) {
        CRLog::error("processPageXPath null ldomXPointer");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    responseAddString(response, xptr.toString());
}

void CreBridge::processOutline(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_OUTLINE;

    response.addInt((uint32_t) 0);

    int columns = doc_view_->GetColumns();
    LVPtrVector<LvTocItem, false> outline;
    doc_view_->GetOutline(outline);
    for (int i = 0; i < outline.length(); i++) {
        LvTocItem* row = outline[i];
        response.addWords(
                (uint16_t) OUTLINE_TARGET_XPATH,
                (uint16_t) ExportPage(columns, row->getPage()));
        response.addInt((uint32_t) row->getLevel());
        responseAddString(response, row->getName());
        responseAddString(response, row->getPath());
    }
}

void CreBridge::processQuit(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_QUIT;
}

CreBridge::CreBridge() : StBridge(THORNYREADER_LOG_TAG)
{
    doc_view_ = NULL;
#ifdef AXYDEBUG
    CRLog::setLevel(CRLog::TRACE);
#else
    CRLog::setLevel(CRLog::FATAL);
#endif
    InitFontManager(lString8::empty_str);
    // 0 - disabled, 1 - bytecode, 2 - auto
    fontMan->SetHintingMode(HINTING_MODE_BYTECODE_INTERPRETOR);
    fontMan->setKerning(true);
    HyphMan::init();
}

CreBridge::~CreBridge()
{
    if (doc_view_) {
        delete doc_view_;
    }
    HyphMan::uninit();
    ShutdownFontManager();
}

void CreBridge::process(CmdRequest& request, CmdResponse& response)
{
    response.reset();
    switch (request.cmd)
    {
        case CMD_REQ_PDF_FONTS:
            //CRLog::trace("CreBridge :CMD_REQ_PDF_FONTS");
            processFonts(request, response);
            break;
        case CMD_REQ_SET_CONFIG:
            //CRLog::trace("CreBridge :CMD_REQ_SET_CONFIG");
            processConfig(request, response);
            break;
        case CMD_REQ_OPEN:
            //CRLog::trace("CreBridge :CMD_REQ_OPEN");
            processOpen(request, response);
            break;
        case CMD_REQ_PAGE:
            //CRLog::trace("CreBridge :CMD_REQ_PAGE");
            processPage(request, response);
            break;
        case CMD_REQ_LINKS:
            //CRLog::trace("CreBridge :CMD_REQ_LINKS");
            processPageLinks(request, response);
            break;
        case CMD_REQ_PAGE_RENDER:
            //CRLog::trace("CreBridge :CMD_REQ_PAGE_RENDER");
            processPageRender(request, response);
            break;
        case CMD_REQ_OUTLINE:
            //CRLog::trace("CreBridge :CMD_REQ_OUTLINE");
            processOutline(request, response);
            break;
        case CMD_REQ_CRE_PAGE_BY_XPATH:
            //CRLog::trace("CreBridge :CMD_REQ_CRE_PAGE_BY_XPATH");
            processPageByXPath(request, response);
            break;
        case CMD_REQ_CRE_PAGE_XPATH:
            //CRLog::trace("CreBridge :CMD_REQ_CRE_PAGE_XPATH");
            processPageXPath(request, response);
            break;
        case CMD_REQ_CRE_METADATA:
            //CRLog::trace("CreBridge :CMD_REQ_CRE_METADATA");
            processMetadata(request, response);
            break;
        case CMD_REQ_ALIVE:
            //CRLog::trace("CreBridge :CMD_REQ_ALIVE");
            response.cmd = CMD_RES_ALIVE;
            break;
        case CMD_REQ_QUIT:
            //CRLog::trace("CreBridge :CMD_REQ_QUIT");
            processQuit(request, response);
            break;
        default:
            CRLog::error("Unknown request: %d", request.cmd);
            //CRLog::trace("CreBridge :RES_UNKNOWN_CMD");
            response.result = RES_UNKNOWN_CMD;
            break;
    }
    //response.print(LCTX);
}