#include <stdlib.h>
#include <crengine/include/crconfig.h>
#include <crengine/include/RectHelper.h>

#include "thornyreader/include/thornyreader.h"
#include "thornyreader/include/StProtocol.h"
#include "thornyreader/include/StSocket.h"
#include "include/CreBridge.h"
#include "include/mobihandler.h"
#include "thornyreader_version.h"

static int CeilToEvenInt(int n)
{
    return (n + 1) & ~1;
}

static int FloorToEvenInt(int n)
{
    return n & ~1;
}

static uint32_t ExportPagesCount(int columns, int pages)
{
	if (columns == 2) {
		return (uint32_t) (CeilToEvenInt(pages) / columns);
	}
	return (uint32_t) pages;
}

static int ExportPage(int columns, int page)
{
	if (columns == 2) {
		return FloorToEvenInt(page) / columns;
	}
	return page;
}

static int ImportPage(int columns, int page)
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

void CreBridge::processConvert(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_CONVERT;
    CmdDataIterator iter(request.first);
    uint32_t src_format = 0;
    uint8_t* src_path_arg = NULL;
    uint32_t dst_format = 0;
    uint8_t* dst_path_arg = NULL;
    iter.getInt(&src_format).getByteArray(&src_path_arg)
            .getInt(&dst_format).getByteArray(&dst_path_arg);
    if (!iter.isValid() || !src_path_arg || !dst_path_arg) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (src_format != DOC_FORMAT_MOBI || dst_format != DOC_FORMAT_EPUB) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char* src_path = reinterpret_cast<const char*>(src_path_arg);
    const char* dst_path = reinterpret_cast<const char*>(dst_path_arg);
    if (!ImportMOBIDocNew(src_path, dst_path)) {
        response.result = RES_INTERNAL_ERROR;
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
        if  (FALLBACK_FACE_DEFAULT != lString8("NONE")) {
            fontMan->InitFallbackFontDefault();
        }
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
          //
        } else if (key == CONFIG_CRE_FONT_SIZE) {
            int int_val = atoi(val);
            int array_lenght = sizeof(ALLOWED_FONT_SIZES) / sizeof(int);
            int_val = GetClosestValueInArray(ALLOWED_FONT_SIZES, array_lenght, int_val);
            if (doc_view_->cfg_font_size_ != int_val) {
                doc_view_->cfg_font_size_ = int_val;
                fontMan->font_size_ = int_val;
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
            //doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_ENABLE_FOOTNOTES, bool_val);
            doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_ENABLE_FOOTNOTES, true);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_TEXT_ALIGN) {
            int int_val = atoi(val);
            if (int_val < 0 || int_val > 3) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            doc_view_->SetTextAlign(int_val);
            doc_view_->UpdatePageMargins();
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
            doc_view_->UpdatePageMargins();
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
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
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
#if SEND_FD_VIA_SOCKET == 1
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
#endif
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
}

void CreBridge::processPage(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE;
}

void CreBridge::processPageText(CmdRequest& request, CmdResponse& response)
{
#ifdef TRDEBUG
#define DEBUG_TEXT
#endif //TRDEBUG
    response.cmd = CMD_RES_PAGE_TEXT;
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid())
    {
        CRLog::error("processPageText bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);
#ifdef DEBUG_TEXT
    CRLog::debug("processPageText external_page=%d page=%d page_width=%d page_height=%d",
            external_page, page, doc_view_->GetWidth(), doc_view_->GetHeight());
#endif
    LVArray<Hitbox> hitboxes = doc_view_->GetPageHitboxes();
    for (int i = 0; i < hitboxes.length(); i++)
    {
        Hitbox currHitbox = hitboxes.get(i);
        response.addFloat(currHitbox.left_);
        response.addFloat(currHitbox.top_);
        response.addFloat(currHitbox.right_);
        response.addFloat(currHitbox.bottom_);
        responseAddString(response, currHitbox.text_);
    }
#undef DEBUG_TEXT
}


void CreBridge::processRTLHitboxes(CmdRequest &request, CmdResponse &response)
{
    //response.cmd = CMD_RES_RTL_TEXT;

   // CmdDataIterator iter(request.first);
   // uint8_t *temp_val;
   // iter.getByteArray(&temp_val);
   // if (!iter.isValid())
   // {
   //     CRLog::error("processPageXpaths bad request data");
   //     // response.result = RES_BAD_REQ_DATA;
   //     // return;
   // }
   // const char *val = reinterpret_cast<const char *>(temp_val);
    // lString16 key_input = lString16(val);


    lString16 key_input = lString16("0;"
                                    "0.115625:0.116250:0.188048:0.224303;"
                                    "0.516875:0.528750:0.296813:0.333068");

    lString16Collection keys;
    keys.split(key_input,lString16(";"));

    uint32_t external_page = keys.at(0).atoi();
    lString16 key_start = keys.at(1);
    lString16 key_end = keys.at(2);

    if(key_start == lString16("-") && key_end == lString16("-"))
    {
        responseAddString(response, lString16("-"));
        responseAddString(response, lString16("-"));
        return;
    }

    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    ldomWordMap m = doc_view_->GetldomWordMapFromPage(page);

    lString16 startstr = doc_view_->GetXpathByRectCoords(key_start, m);
    lString16 endstr = doc_view_->GetXpathByRectCoords(key_end, m);

    ldomXPointer start  = doc_view_->GetCrDom()->createXPointer(startstr);
    ldomXPointer end    = doc_view_->GetCrDom()->createXPointer(endstr);
    int startpage = doc_view_->GetPageForBookmark(start);
    int endpage = doc_view_->GetPageForBookmark(end);

    LVArray<Hitbox> BookmarkHitboxes;
    ldomXRange* range;

    // out of selection range
    if (page < startpage || page > endpage)
    {
        //CRLog::trace("Selection out of range");
        return;
    }
    //exactly on one current page
    if (startpage == page && endpage == page)
    {
        range = new ldomXRange(start, end);
    }
    //selection goes lower
    if (startpage == page && endpage > page)
    {
        ldomXPointer page_end = doc_view_->GetPageDocRange(page).get()->getEnd();
        range = new ldomXRange(start, page_end);
    }
    //selection goes upper
    if (startpage < page && endpage == page)
    {
        ldomXPointer page_start = doc_view_->GetPageDocRange(page).get()->getStart();
        range = new ldomXRange(page_start, end);
    }
    //selection goes upper and lower
    if (startpage < page && endpage > page)
    {
        ldomXPointer page_start = doc_view_->GetPageDocRange(page).get()->getStart();
        ldomXPointer page_end = doc_view_->GetPageDocRange(page).get()->getEnd();
        range = new ldomXRange(page_start, page_end);
    }

    BookmarkHitboxes = doc_view_->GetPageHitboxes(range,false);

    lString16 result;
    for (int i = 0; i < BookmarkHitboxes.length(); i++)
    {
            result+=BookmarkHitboxes.get(i).text_;
    }
    result = result.ReversePrettyLetters();
    //responseAddString(response,result);
}

//searches for the specified rectangles on page, returns xpaths if found.
void CreBridge::processPageXpaths(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_XPATH;
    //response.cmd = CMD_RES_PAGE_TEXT;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processPageXpaths bad request data");
       // response.result = RES_BAD_REQ_DATA;
       // return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
   // lString16 key_input = lString16(val);


   lString16 key_input = lString16("0;"
                                    "0.103750:0.121875:0.039841:0.062550;"
                                    "0.103750:0.121875:0.039841:0.062550");
  /*
    lString16 key_input = lString16("3;"
                                    "0.103750:0.120000:0.811155:0.833068");
   */

    lString16Collection keys;
    keys.split(key_input,lString16(";"));

    uint32_t external_page = keys.at(0).atoi();
    lString16 key_start = keys.at(1);
    lString16 key_end = keys.at(2);

    if(key_start == lString16("-") && key_end == lString16("-"))
    {
        responseAddString(response, lString16("-"));
        responseAddString(response, lString16("-"));
        return;
    }

    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    ldomWordMap m = doc_view_->GetldomWordMapFromPage(page);

    lString16 xp1 = doc_view_->GetXpathByRectCoords(key_start, m);
    responseAddString(response, xp1);
    lString16 xp2 = doc_view_->GetXpathByRectCoords(key_end, m);
    responseAddString(response, xp2);

}
//returns hitboxes of letters between the specified xpointers
void CreBridge::processPageRangeText(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_RANGE_HITBOX;
    //response.cmd = CMD_RES_PAGE_TEXT;

    /*CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processPageRangeText bad request data");
        // response.result = RES_BAD_REQ_DATA;
        // return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    // lString16 key_input = lString16(val);
*/

    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid())
    {
        CRLog::error("processPageText bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);

    lString16 key_input = lString16("0;"
                                    "/body/body[1]/p[11]/h6/text().0;"
                                    "/body/body[1]/p[44]/a/text().1");

    //temp_vals
    lString16Collection keys;
    keys.split(key_input,lString16(";"));

   // uint32_t external_page = keys.at(0).atoi();
    lString16 startstr  = keys.at(1);
    lString16 endstr    = keys.at(2);
    ldomXPointer start  = doc_view_->GetCrDom()->createXPointer(startstr);
    ldomXPointer end    = doc_view_->GetCrDom()->createXPointer(endstr);
    int startpage = doc_view_->GetPageForBookmark(start);
    int endpage = doc_view_->GetPageForBookmark(end);


    LVArray<Hitbox> BookmarkHitboxes;
    ldomXRange* range;

    // out of selection range
    if (page < startpage || page > endpage)
    {
        //CRLog::trace("Selection out of range");
        return;
    }
    //exactly on one current page
    if (startpage == page && endpage == page)
    {
        range = new ldomXRange(start, end);
    }
    //selection goes lower
    if (startpage == page && endpage > page)
    {
        ldomXPointer page_end = doc_view_->GetPageDocRange(page).get()->getEnd();
        range = new ldomXRange(start, page_end);
    }
    //selection goes upper
    if (startpage < page && endpage == page)
    {
        ldomXPointer page_start = doc_view_->GetPageDocRange(page).get()->getStart();
        range = new ldomXRange(page_start, end);
    }
    //selection goes upper and lower
    if (startpage < page && endpage > page)
    {
        ldomXPointer page_start = doc_view_->GetPageDocRange(page).get()->getStart();
        ldomXPointer page_end = doc_view_->GetPageDocRange(page).get()->getEnd();
        range = new ldomXRange(page_start, page_end);
    }

    BookmarkHitboxes = doc_view_->GetPageHitboxes(range);

    for (int i = 0; i < BookmarkHitboxes.length(); i++)
    {
        Hitbox currHitbox = BookmarkHitboxes.get(i);
        response.addFloat(currHitbox.left_);
        response.addFloat(currHitbox.top_);
        response.addFloat(currHitbox.right_);
        response.addFloat(currHitbox.bottom_);
        responseAddString(response, currHitbox.text_);
    }

}
//returns two (or one) hitboxes of letters for the specified xpointers
void CreBridge::processPageRects(CmdRequest &request, CmdResponse &response)
{
    //response.cmd = CMD_RES_PAGE_TEXT;
    response.cmd = CMD_RES_XPATH_HITBOX;
   /* CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processPageRects bad request data");
        //response.result = RES_BAD_REQ_DATA;
        //return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    //lString16 str = lString16(val);
*/

    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid())
    {
        CRLog::error("processPageText bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);

    lString16 key_input = lString16("0;"
                                    "/body/body[1]/p[11]/h6/text().0;"
                                    "/body/body[1]/p[44]/a/text().1");

    /*lString16 key_input = lString16("0;"
                                    "/body/body[1]/p[11]/h6/text().0;"
                                    "/body/body[1]/p[11]/h6/text().0");
    */
    lString16Collection keys;
    keys.split(key_input,lString16(";"));

    // uint32_t external_page = keys.at(0).atoi();
    lString16 startstr = keys.at(1);
    lString16 endstr = keys.at(2);

    ldomXPointer xPointer = doc_view_->GetCrDom()->createXPointer(startstr);
    ldomNode *node = xPointer.getNode();
    int xPointerPage = doc_view_->GetPageForBookmark(xPointer);
    int xPointerOffset = xPointer.getOffset() + 1;

    if (xPointerPage == page)
    {
        ldomXPointer xPointer2 = ldomXPointer(node, xPointerOffset);
        ldomXRange *range = new ldomXRange(xPointer, xPointer2);

        Hitbox currHitbox = doc_view_->GetPageHitboxes(range).get(0);
        response.addFloat(currHitbox.left_);
        response.addFloat(currHitbox.top_);
        response.addFloat(currHitbox.right_);
        response.addFloat(currHitbox.bottom_);
        responseAddString(response, currHitbox.text_);
    }
    if (startstr == endstr)
    {
        return;
    }

    xPointer = doc_view_->GetCrDom()->createXPointer(endstr);
    node = xPointer.getNode();
    xPointerPage = doc_view_->GetPageForBookmark(xPointer);
    xPointerOffset = xPointer.getOffset() - 1;

    if (xPointerPage == page)
    {
        ldomXPointer xPointer2 = ldomXPointer(node, xPointerOffset);
        ldomXRange *range = new ldomXRange(xPointer2, xPointer);

        Hitbox currHitbox = doc_view_->GetPageHitboxes(range).get(0);
        response.addFloat(currHitbox.left_);
        response.addFloat(currHitbox.top_);
        response.addFloat(currHitbox.right_);
        response.addFloat(currHitbox.bottom_);
        responseAddString(response, currHitbox.text_);
    }
    return;
}




void CreBridge::processPageLinks(CmdRequest& request, CmdResponse& response)
{
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
    LVArray<Hitbox> pageLinks = doc_view_->GetPageLinks();

    for (int i = 0; i < pageLinks.length(); i++)
    {
        uint16_t target_page = 0;
        Hitbox curr_link = pageLinks.get(i);
        float l = curr_link.left_;
        float t = curr_link.top_;
        float r = curr_link.right_;
        float b = curr_link.bottom_;
        lString16 href = curr_link.text_;

        if (href.length() > 1 && href[0] == '#')
        {
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
    }

    LVArray<TextRect> fnoteslist = doc_view_->GetCurrentPageFootnotesLinks();
    if(fnoteslist.empty())
    {
        return;
    }
    float width = doc_view_->GetWidth();
    float height = doc_view_->GetHeight();

    for (int i = 0; i < fnoteslist.length(); i++)
    {
        uint16_t target_page = 0;
        TextRect curr_link = fnoteslist.get(i);
        float l = curr_link.getRect().left   / width  ;
        float t = curr_link.getRect().top    / height ;
        float r = curr_link.getRect().right  / width  ;
        float b = curr_link.getRect().bottom / height ;
        lString16 href = curr_link.getText();

        if (href.length() > 1 && href[0] == '#')
        {
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
        //CRLog::error("ltrb = %f, %f, %f, %f , %s", l,t,r,b,LCSTR(href));
    }

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
    CRLog::trace("processOutline size: %d", outline.length());
#ifdef TRDEBUG
#if 0
    for (int i = 0; i < 60000; i++) {
        response.addWords((uint16_t) OUTLINE_TARGET_XPATH, 1);
        response.addInt((uint32_t) 0);
        responseAddString(response, lString16("chapter ").appendDecimal(i));
        responseAddString(response, lString16("xpath"));
    }
    return;
#endif
#if 0
    for (int i = 0; i < outline.length(); i++) {
        LvTocItem* row = outline[i];
        uint16_t row_page = (uint16_t) ExportPage(columns, row->getPage());
        CRLog::trace("%s, %d, %d, %s",
                     LCSTR(row->getName()), row_page, row->getLevel() - 1, LCSTR(row->getPath()));
    }
#endif
#endif
    for (int i = 0; i < outline.length(); i++) {
        LvTocItem* row = outline[i];
        uint16_t row_page = (uint16_t) ExportPage(columns, row->getPage());
        response.addWords((uint16_t) OUTLINE_TARGET_XPATH, row_page);
        // Crengine level is one-based while ThornyReader using zero-based levels
        response.addInt((uint32_t) row->getLevel() - 1);
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
#ifdef TRDEBUG
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
            //CRLog::trace("CreBridge: CMD_REQ_PDF_FONTS");
            processFonts(request, response);
            break;
        case CMD_REQ_SET_CONFIG:
            //CRLog::trace("CreBridge: CMD_REQ_SET_CONFIG");
            processConfig(request, response);
            break;
        case CMD_REQ_OPEN:
            //CRLog::trace("CreBridge: CMD_REQ_OPEN");
            processOpen(request, response);
            break;
        case CMD_REQ_PAGE:
            //CRLog::trace("CreBridge: CMD_REQ_PAGE");
            processPage(request, response);
            break;
        case CMD_REQ_PAGE_RENDER:
            //CRLog::trace("CreBridge: CMD_REQ_PAGE_RENDER");
            processPageRender(request, response);
            break;
        case CMD_REQ_LINKS:
            //CRLog::trace("CreBridge: CMD_REQ_LINKS");
            processPageLinks(request, response);
            break;
        case CMD_REQ_PAGE_TEXT:
            //CRLog::trace("CreBridge: CMD_REQ_PAGE_TEXT");
            processPageText(request, response);
            break;
        case CMD_REQ_XPATH:
            processPageXpaths(request, response);
            break;
        case CMD_REQ_RANGE_HITBOX:
            processPageRangeText(request, response);
            break;
        case CMD_REQ_XPATH_HITBOX:
            processPageRects(request, response);
            break;
        case CMD_REQ_OUTLINE:
            //CRLog::trace("CreBridge: CMD_REQ_OUTLINE");
            processOutline(request, response);
            break;
        case CMD_REQ_CRE_PAGE_BY_XPATH:
            //CRLog::trace("CreBridge: CMD_REQ_CRE_PAGE_BY_XPATH");
            processPageByXPath(request, response);
            break;
        case CMD_REQ_CRE_PAGE_XPATH:
            //CRLog::trace("CreBridge: CMD_REQ_CRE_PAGE_XPATH");
            processPageXPath(request, response);
            break;
        case CMD_REQ_CONVERT:
            //CRLog::trace("CreBridge: CMD_REQ_CONVERT");
            processConvert(request, response);
            break;
        case CMD_REQ_CRE_METADATA:
            //CRLog::trace("CreBridge: CMD_REQ_CRE_METADATA");
            processMetadata(request, response);
            break;
        case CMD_REQ_QUIT:
            //CRLog::trace("CreBridge: CMD_REQ_QUIT");
            processQuit(request, response);
            break;
        case CMD_REQ_VERSION:
            ThornyVersionResporse(THORNYREADER_BASE_VERSION, response);
            break;
        default:
            CRLog::error("Unknown request: %d", request.cmd);
            response.result = RES_UNKNOWN_CMD;
            break;
    }
    //response.print(LCTX);
}

int main(int argc, char *argv[])
{
    ThornyStart("crengine");
    CreBridge cre;
    return cre.main(argc, argv);
}