/*******************************************************

 Crengine

 lvdocview.cpp:  XML DOM tree rendering tools

 (c) Vadim Lopatin, 2000-2009
 This source code is distributed under the terms of
 GNU General Public License
 See LICENSE file for details

 *******************************************************/

#include <map>
#include "include/RectHelper.h"
#include "thornyreader/include/thornyreader.h"
#include "include/lvdocview.h"
#include "include/CreBridge.h"
#include "include/rtfimp.h"
#include "include/lvrend.h"
#include "include/epubfmt.h"
#include "include/chmfmt.h"
#include "include/wordfmt.h"
#include "include/pdbfmt.h"
#include "include/crcss.h"
#include "include/mobihandler.h"
#include "include/crconfig.h"
#include "include/fb2fmt.h"

// Yep, twice include single header with different define.
// Probably should be last in include list, to don't mess up with other includes.
#include "include/fb2def.h"
#define XS_IMPLEMENT_SCHEME 1
#include "include/fb2def.h"
//#undef XS_IMPLEMENT_SCHEME

#if 0
#define REQUEST_RENDER(caller) { CRLog::trace("RequestRender " caller); RequestRender(); }
#define CHECK_RENDER(caller) { CRLog::trace("CheckRender " caller); CheckRender(); }
#else
#define REQUEST_RENDER(caller) RequestRender();
#define CHECK_RENDER(caller) RenderIfDirty();
#endif

static const css_font_family_t DEF_FONT_FAMILY = css_ff_sans_serif;

LVDocView::LVDocView()
		: stream_(NULL),
		  cr_dom_(NULL),
		  viewport_mode_(MODE_PAGES),
		  page_(0),
		  offset_(0),
		  is_rendered_(false),
		  highlight_bookmarks_(1),
		  margins_(),
		  show_cover_(false),
		  background_tiled_(true),
		  position_is_set_(false),
		  doc_format_(DOC_FORMAT_NULL),
		  width_(200),
		  height_(400),
		  page_columns_(1),
		  background_color_(0xFFFFFFE0),
		  text_color_(0x000060),
		  cfg_margins_(),
		  cfg_font_size_(24),
		  cfg_interline_space_(100),
		  cfg_embeded_styles_(false),
		  cfg_embeded_fonts_(false),
		  cfg_enable_footnotes_(true),
		  cfg_firstpage_thumb_(false),
		  cfg_txt_smart_format_(true)
{
	cfg_font_face_ = lString8("Arial, Roboto");
	base_font_ = fontMan->GetFont(cfg_font_size_, 400, false, DEF_FONT_FAMILY, cfg_font_face_);
	doc_props_ = LVCreatePropsContainer();
	CreateEmptyDom();
}

LVDocView::~LVDocView()
{
	Clear();
}

void LVDocView::Clear()
{
	if (cr_dom_)
	{
		delete cr_dom_;
		cr_dom_ = NULL;
	}
	page_ = 0;
	offset_ = 0;
	position_is_set_ = false;
	show_cover_ = false;
	is_rendered_ = false;
	bookmark_ = ldomXPointer();
	bookmark_.clear();
	doc_props_->clear();
	if (!stream_.isNull())
	{
		stream_.Clear();
	}
}

void LVDocView::CreateEmptyDom()
{
	Clear();
	cr_dom_ = new CrDom();
	doc_format_ = DOC_FORMAT_NULL;
	cr_dom_->setProps(doc_props_);
	cr_dom_->setDocFlags(0);
	cr_dom_->setDocFlag(DOC_FLAG_ENABLE_FOOTNOTES, cfg_enable_footnotes_);
	cr_dom_->setDocFlag(DOC_FLAG_EMBEDDED_STYLES, cfg_embeded_styles_);
	cr_dom_->setDocFlag(DOC_FLAG_EMBEDDED_FONTS, cfg_embeded_fonts_);
	cr_dom_->setNodeTypes(fb2_elem_table);
	cr_dom_->setAttributeTypes(fb2_attr_table);
	cr_dom_->setNameSpaceTypes(fb2_ns_table);
	marked_ranges_.clear();
	bookmark_ranges_.clear();
}

void LVDocView::RenderIfDirty()
{
	if (is_rendered_)
	{
		return;
	}
	is_rendered_ = true;
	position_is_set_ = false;
	if (cr_dom_ && cr_dom_->getRootNode() != NULL)
	{
		int dx = page_rects_[0].width() - margins_.left - margins_.right;
		int dy = page_rects_[0].height() - margins_.top - margins_.bottom;
		CheckRenderProps(dx, dy);
		if (base_font_.isNull())
		{
			CRLog::error("RenderIfDirty base_font_.isNull()");
			return;
		}
		int y0 = show_cover_ ? dy + margins_.bottom * 4 : 0;
		cr_dom_->render(&pages_list_, dx, dy, show_cover_, y0, base_font_, cfg_interline_space_);
		fontMan->gc();
		is_rendered_ = true;
		UpdateSelections();
		UpdateBookmarksRanges();
	}
}

/// Invalidate formatted data, request render
void LVDocView::RequestRender()
{
	is_rendered_ = false;
	cr_dom_->clearRendBlockCache();
	cr_dom_->ApplyEmbeddedStyles();
}

/// Ensure current position is set to current bookmark value
void LVDocView::CheckPos()
{
	CHECK_RENDER("CheckPos()");
	if (position_is_set_)
	{
		return;
	}
	position_is_set_ = true;
	if (bookmark_.isNull())
	{
		if (IsPagesMode())
		{
			GoToPage(0);
		}
		else
		{
			GoToOffset(0, false);
		}
	}
	else
	{
		if (IsPagesMode())
		{
			GoToPage(GetPageForBookmark(bookmark_), false);
		}
		else
		{
			lvPoint pt = bookmark_.toPoint();
			GoToOffset(pt.y, false);
		}
	}
}

void LVDocView::UpdatePageMargins()
{
	int new_margin_left = cfg_margins_.left;
	int new_margin_right = cfg_margins_.right;
	if (gFlgFloatingPunctuationEnabled)
	{
		int align = 0;
		base_font_ = fontMan->GetFont(cfg_font_size_, 400, false, DEF_FONT_FAMILY, cfg_font_face_);
		base_font_->setFallbackFont(fontMan->GetFallbackFont(cfg_font_size_,400,false));
		align = base_font_->getVisualAligmentWidth() / 2;
		if (align > new_margin_right)
		{
			align = new_margin_right;
		}
        gTextLeftShift = align;
		//new_margin_left += align;
		//new_margin_right -= align;
	}
	else
	{
		gTextLeftShift = 0;
	}
	if (margins_.left != new_margin_left
	    || margins_.right != new_margin_right
	    || margins_.top != cfg_margins_.top
	    || margins_.bottom != cfg_margins_.bottom)
	{
		margins_.left = new_margin_left;
		margins_.right = new_margin_right;
		margins_.top = cfg_margins_.top;
		margins_.bottom = cfg_margins_.bottom;
		UpdateLayout();
		REQUEST_RENDER("UpdatePageMargins")
	}
}

void LVDocView::Resize(int width, int height)
{
	if (width < 80 || width > 5000)
	{
		width = 80;
	}
	if (height < 80 || height > 5000)
	{
		height = 80;
	}
	if (width == width_ && height == height_)
	{
		CRLog::trace("Size is not changed: %dx%d", width, height);
		return;
	}
	width_ = width;
	height_ = height;
	if (cr_dom_)
	{
		UpdateLayout();
		REQUEST_RENDER("resize")
		position_is_set_ = false;
		//goToBookmark(_posBookmark);
		//updateBookMarksRanges();
	}
}

void LVDocView::SetTextAlign(int align)
{
	// SHOULD BE CALLED ONLY AFTER setNodeTypes
	cr_dom_->setStylesheet(CR_CSS_BASE, true);
	if (align == 0)
	{
		cr_dom_->setStylesheet(CR_CSS_ALIGN_JUSTIFY, false);
	}
	else if (align == 1)
	{
		cr_dom_->setStylesheet(CR_CSS_ALIGN_LEFT, false);
	}
	else if (align == 2)
	{
		cr_dom_->setStylesheet(CR_CSS_ALIGN_CENTER, false);
	}
	else if (align == 3)
	{
		cr_dom_->setStylesheet(CR_CSS_ALIGN_RIGHT, false);
	}
	else
	{
		cr_dom_->setStylesheet(CR_CSS_ALIGN_JUSTIFY, false);
	}
}

static LVStreamRef ThResolveStream(int doc_format, const char *absolute_path_chars,
                                   uint32_t packed_size, bool smart_archive)
{
	lString16 absolute_path(absolute_path_chars);
	LVStreamRef stream = LVOpenFileStream(absolute_path.c_str(), LVOM_READ);
	if (stream.isNull())
	{
		CRLog::error("ThResolveStream open file fail %s", LCSTR(absolute_path));
		return LVStreamRef();
	}
	if (packed_size > 0)
	{
		LVContainerRef container = LVOpenArchive(stream);
		if (container.isNull())
		{
			CRLog::error("ThResolveStream read archive fail %s", LCSTR(absolute_path));
			return LVStreamRef();
		}
		stream = container->OpenStreamByPackedSize(packed_size);
		if (stream.isNull())
		{
			CRLog::error("ThResolveStream direct archive stream fail %s", LCSTR(absolute_path));
			return LVStreamRef();
		}
	}
	if (!smart_archive)
	{
		return stream;
	}
	LVContainerRef container = LVOpenArchive(stream);
	if (container.isNull())
	{
		CRLog::error("ThResolveStream smart_archive fail %s", LCSTR(absolute_path));
		return LVStreamRef();
	}
	int found_count = 0;
	lString16 entry_name;
	for (int i = 0; i < container->GetObjectCount(); i++)
	{
		const LVContainerItemInfo *item = container->GetObjectInfo(i);
		if (!item || item->IsContainer())
		{
			continue;
		}
		lString16 name(item->GetName());
		lString16 name_lowercase = name;
		name_lowercase.lowercase();
		if (doc_format == DOC_FORMAT_FB2)
		{
			if (name_lowercase.endsWith(".fb2"))
			{
				entry_name = name;
				found_count++;
			}
		}
		else if (doc_format == DOC_FORMAT_RTF)
		{
			if (name_lowercase.endsWith(".rtf"))
			{
				entry_name = name;
				found_count++;
			}
		}
		else if (doc_format == DOC_FORMAT_TXT)
		{
			if (name_lowercase.endsWith(".txt"))
			{
				entry_name = name;
				found_count++;
			}
		}
		else if (doc_format == DOC_FORMAT_MOBI)
		{
			if (name_lowercase.endsWith(".mobi"))
			{
				entry_name = name;
				found_count++;
			}
		}
		else if (doc_format == DOC_FORMAT_DOC)
		{
			if (name_lowercase.endsWith(".doc"))
			{
				entry_name = name;
				found_count++;
			}
		}
		else if (doc_format == DOC_FORMAT_CHM)
		{
			if (name_lowercase.endsWith(".chm"))
			{
				entry_name = name;
				found_count++;
			}
		}
		else if (doc_format == DOC_FORMAT_EPUB)
		{
			if (name_lowercase.endsWith(".epub"))
			{
				entry_name = name;
				found_count++;
			}
		}
        else if (doc_format == DOC_FORMAT_DOCX)
        {
            if (name_lowercase.endsWith(".docx"))
            {
                entry_name = name;
                found_count++;
            }
        }
	}
	if (found_count == 1)
	{
		return container->OpenStream(entry_name.c_str(), LVOM_READ);
	}
	else
	{
		CRLog::error("ThResolveStream smart_archive collision fail %s %d",
                     LCSTR(absolute_path), found_count);
		return LVStreamRef();
	}
}

bool LVDocView::LoadDoc(int doc_format, const char *absolute_path,
                        uint32_t compressed_size, bool smart_archive)
{
#ifdef TRDEBUG
	//cfg_firstpage_thumb_ = true;
	if (cfg_firstpage_thumb_)
	{
		CRLog::trace("LoadDoc: Partial parsing: %s %d", LCSTR(lString16(absolute_path)), doc_format);
	}
	else
	{
		CRLog::trace("LoadDoc: Full parsing: %s %d", LCSTR(lString16(absolute_path)), doc_format);
	}
#endif
    LVStreamRef stream = ThResolveStream(doc_format, absolute_path, compressed_size, smart_archive);
    if (!stream)
    {
        return false;
    }
    if (LoadDoc(doc_format, stream))
    {
        stream_.Clear();
        return true;
    }
    else
    {
        CreateEmptyDom();
        CRLog::error("Doc stream parsing fail");
        return false;
    }
}

bool LVDocView::LoadDoc(int doc_format, LVStreamRef stream)
{
    stream_ = stream;
	doc_format_ = doc_format;
	CheckRenderProps(0, 0);
	LVFileFormatParser *parser = nullptr;
	if (doc_format == DOC_FORMAT_FB2)
	{
		cr_dom_->setProps(doc_props_);
		CRLog::error("IMPORTING FB2");
		if (!ImportFb2Document(stream_, cr_dom_, cfg_firstpage_thumb_))
		{
			CRLog::error("IMPORTING FB2 FAILED");
			return false;
		}
		doc_props_ = cr_dom_->getProps();
	}
    else if (doc_format == DOC_FORMAT_DOCX)
    {
        cr_dom_->setProps(doc_props_);
        CRLog::error("IMPORTING DOCX");
        if (!ImportDocxDocument(stream_, cr_dom_, cfg_firstpage_thumb_))
        {
            CRLog::error("IMPORTING DOCX FAILED");
            return false;
        }
        doc_props_ = cr_dom_->getProps();
    }
	else if (doc_format == DOC_FORMAT_EPUB)
	{
		CRLog::error("IMPORTING EPUB");

		if (!DetectEpubFormat(stream_))
		{
			CRLog::error("DetectEpubFormat fail");
			return false;
		}
		cr_dom_->setProps(doc_props_);
		if (!ImportEpubDocument(stream_, cr_dom_, cfg_firstpage_thumb_))
		{
			CRLog::error("EPUB IMPORT FAILED");
			return false;
		}
		doc_props_ = cr_dom_->getProps();
	}
	else if (doc_format == DOC_FORMAT_MOBI)
	{
		doc_format_t pdb_format = doc_format_none;
		if (!DetectMOBIFormat(stream_, pdb_format))
		{
			return false;
		}
		cr_dom_->setProps(doc_props_);
		if (pdb_format != doc_format_mobi)
		{
			CRLog::error("pdb_format != doc_format_mobi");
		}
		if (!ImportMOBIDoc(stream_, cr_dom_, pdb_format, cfg_firstpage_thumb_))
		{
			if (pdb_format != doc_format_mobi)
			{
				CRLog::error("pdb_format != doc_format_mobi");
			}
			return false;
		}
	}
	else if (doc_format == DOC_FORMAT_DOC)
	{
#if ENABLE_ANTIWORD == 1
        if (DetectWordFormat(stream_))
        {
            cr_dom_->setProps(doc_props_);
            if (!ImportWordDocument(stream_, cr_dom_, cfg_firstpage_thumb_))
            {
                return false;
            }
        }
        else if(DetectRTFFormat(stream_))
        {
            doc_format = DOC_FORMAT_RTF;
        }
        else
        {
	        LvDomAutocloseWriter writer(cr_dom_, false, HTML_AUTOCLOSE_TABLE);
	        parser = new LvHtmlParser(stream_, &writer);
        }
#endif //ENABLE_ANTIWORD == 1
	}
	if (doc_format == DOC_FORMAT_RTF)
	{
		LvDomWriter writer(cr_dom_);
		parser = new LVRtfParser(stream_, &writer, cfg_firstpage_thumb_);
	}
	else if (doc_format == DOC_FORMAT_CHM)
	{
		if (!DetectCHMFormat(stream_))
		{
            CRLog::warn("!detectCHM");
			return false;
		}
		cr_dom_->setProps(doc_props_);
		if (!ImportCHMDocument(stream_, cr_dom_, cfg_firstpage_thumb_))
		{
            CRLog::warn("!importCHM");
			return false;
		}
	}
	else if (doc_format == DOC_FORMAT_TXT)
	{
		LvDomWriter writer(cr_dom_);
		parser = new LVTextParser(stream_, &writer, cfg_txt_smart_format_, cfg_firstpage_thumb_);
	}
	else if (doc_format == DOC_FORMAT_HTML)
	{
		LvDomAutocloseWriter writer(cr_dom_, false, HTML_AUTOCLOSE_TABLE);
		parser = new LvHtmlParser(stream_, &writer);
	}
	if (parser)
	{
		if (!parser->CheckFormat())
		{
            CRLog::trace("!parser->CheckFormat()");
			delete parser;
			return false;
		}
		if (!parser->Parse())
		{
            CRLog::trace("!parser->Parse()");
            delete parser;
            return false;
        }
        if (NeedCheckImage() && CheckImage())
        {
            CRLog::warn("Image found in shortened tree. Regenerating full tree.");
            cr_dom_->clear();
            parser->FullDom();
            if (!parser->Parse())
            {
                CRLog::warn("!parser->Parse()");
                delete parser;
                return false;
            }
        }
    }
    delete parser;
#ifdef TRDEBUG
if (DUMP_DOMTREE == 1)
{
    CRLog::error("dumping domtree");
    LVStreamRef out = LVOpenFileStream("/sdcard/download/temp.xml", LVOM_WRITE);
    //LVStreamRef out = LVOpenFileStream("/mnt/shell/emulated/10/Android/data/org.readera.trdevel/files/temp.xml", LVOM_WRITE);
    //LVStreamRef out = LVOpenFileStream("/data/data/org.readera.trdevel/files/temp.xml", LVOM_WRITE);
    if(cr_dom_->saveToStream(out, NULL, true))
    {
    	CRLog::error("dumped successfully");
    }
}
#if 0
    lString16 stylesheet = cr_dom_->createXPointer(L"/FictionBook/stylesheet").getText();
    if (!stylesheet.empty() && cfg_embeded_styles_) {
        cr_dom_->getStylesheet()->parse(UnicodeToUtf8(stylesheet).c_str());
        cr_dom_->setStylesheet(UnicodeToUtf8(stylesheet).c_str(), false);
    }
#endif
#endif
    offset_ = 0;
    page_ = 0;
	//show_cover_ = !getCoverPageImage().isNull();
	CheckRenderProps(0, 0);
	REQUEST_RENDER("LoadDoc")
	return true;
}

/// returns cover page image source, if any
LVImageSourceRef LVDocView::getCoverPageImage()
{
	lUInt16 path[] = {
			el_FictionBook,
			el_description,
			el_title_info,
			el_coverpage,
			//el_image,
			0};
	ldomNode *cover_el = cr_dom_->getRootNode()->findChildElement(path);
	if (cover_el)
	{
		ldomNode *cover_img_el = cover_el->findChildElement(LXML_NS_ANY, el_image, 0);
		if (cover_img_el)
		{
			LVImageSourceRef imgsrc = cover_img_el->getObjectImageSource();
			return imgsrc;
		}
	}
	// Not found: return NULL ref
	return LVImageSourceRef();
}

/// Draws coverpage to image buffer
void LVDocView::DrawCoverTo(LVDrawBuf *drawBuf, lvRect &rc)
{
	if (rc.width() < 130 || rc.height() < 130)
	{
		return;
	}
	lvRect imgrc = rc;
	LVImageSourceRef imgsrc = getCoverPageImage();
	if (!imgsrc.isNull() && imgrc.height() > 30)
	{
		int src_dx = imgsrc->GetWidth();
		int src_dy = imgsrc->GetHeight();
		int scale_x = imgrc.width() * 0x10000 / src_dx;
		int scale_y = imgrc.height() * 0x10000 / src_dy;
		if (scale_x < scale_y)
		{
			scale_y = scale_x;
		}
		else
		{
			scale_x = scale_y;
		}
		int dst_dx = (src_dx * scale_x) >> 16;
		int dst_dy = (src_dy * scale_y) >> 16;
		if (dst_dx > rc.width() * 6 / 8)
		{
			dst_dx = imgrc.width();
		}
		if (dst_dy > rc.height() * 6 / 8)
		{
			dst_dy = imgrc.height();
		}
		LVColorDrawBuf buf2(src_dx, src_dy, 32);
		buf2.Draw(imgsrc, 0, 0, src_dx, src_dy, true);
		drawBuf->DrawRescaled(
				&buf2,
				imgrc.left + (imgrc.width() - dst_dx) / 2,
				imgrc.top + (imgrc.height() - dst_dy) / 2,
				dst_dx,
				dst_dy,
				0);
	}
	else
	{
		imgrc.bottom = imgrc.top;
	}
	rc.top = imgrc.bottom;
}

int LVDocView::GetFullHeight()
{
	CHECK_RENDER("getFullHeight()");
	RenderRectAccessor rd(cr_dom_->getRootNode());
	return (rd.getHeight() + rd.getY());
}

#define FOOTNOTE_MARGIN 8
void LVDocView::DrawPageTo(LVDrawBuf *drawbuf, LVRendPageInfo &page, lvRect *pageRect)
{
	int start = page.start;
	int height = page.height;
	lvRect fullRect(0, 0, drawbuf->GetWidth(), drawbuf->GetHeight());
	if (!pageRect)
	{
		pageRect = &fullRect;
	}
	drawbuf->setHidePartialGlyphs(IsPagesMode());
	//int offset = (pageRect->height() - m_pageMargins.top - m_pageMargins.bottom - height) / 3;
	//if (offset>16)
	//    offset = 16;
	//if (offset<0)
	//    offset = 0;
	int offset = 0;
	lvRect clip;
	clip.left = pageRect->left + margins_.left;
	clip.top = pageRect->top + margins_.top + offset;
	clip.bottom = pageRect->top + margins_.top + height + offset;
	clip.right = pageRect->left + pageRect->width() - margins_.right + gTextLeftShift;
	if (page.type == PAGE_TYPE_COVER)
	{
		clip.top = pageRect->top + margins_.top;
	}
	drawbuf->SetClipRect(&clip);
	if (!cr_dom_)
	{
		drawbuf->SetClipRect(NULL);
		return;
	}
	if (page.type == PAGE_TYPE_COVER)
	{
		lvRect rc = *pageRect;
		drawbuf->SetClipRect(&rc);
		/*
		 if ( m_pageMargins.bottom > m_pageMargins.top )
			rc.bottom -= m_pageMargins.bottom - m_pageMargins.top;
		 rc.left += m_pageMargins.left / 2;
		 rc.top += m_pageMargins.bottom / 2;
		 rc.right -= m_pageMargins.right / 2;
		 rc.bottom -= m_pageMargins.bottom / 2;
		 */
		DrawCoverTo(drawbuf, rc);
		drawbuf->SetClipRect(NULL);
		return;
	}
	// draw main page text
	if (marked_ranges_.length())
	{
		CRLog::trace("Entering DrawDocument(): %d ranges", marked_ranges_.length());
	}
	if (page.height)
	{
		DrawDocument(*drawbuf,
		             cr_dom_->getRootNode(),
		             pageRect->left + margins_.left,
		             clip.top,
		             pageRect->width() - margins_.left - margins_.right,
		             height,
		             0,
		             -start + offset,
		             height_,
		             &marked_ranges_,
		             &bookmark_ranges_,
				     margins_,
                     GetColumns());
	}
	// draw footnotes
	int fny = clip.top + (page.height ? page.height + FOOTNOTE_MARGIN : FOOTNOTE_MARGIN);
    fny += (fontMan->font_size_ * 0.25);
	int fy = fny;
	bool footnoteDrawed = false;
	for (int fn = 0; fn < page.footnotes.length(); fn++)
	{
		int fstart = page.footnotes[fn].start;
		int fheight = page.footnotes[fn].height;
		clip.top = fy + offset;
		clip.left = pageRect->left + margins_.left;
		clip.right = pageRect->right - margins_.right + gTextLeftShift;
		clip.bottom = fy + offset + fheight;
		drawbuf->SetClipRect(&clip);
		DrawDocument(
				*drawbuf,
				cr_dom_->getRootNode(),
				pageRect->left + margins_.left,
				fy + offset,
				pageRect->width() - margins_.left - margins_.right,
				fheight,
				0,
				-fstart + offset,
				height_,
				&marked_ranges_);
		footnoteDrawed = true;
		fy += fheight;
	}
	if (footnoteDrawed)
	{ // && page.height
		fny -= FOOTNOTE_MARGIN / 2;
		drawbuf->SetClipRect(NULL);
		lUInt32 cl = drawbuf->GetTextColor();
		cl = (cl & 0xFFFFFF) | (0x55000000);
		drawbuf->FillRect(
				pageRect->left + margins_.left,
				fny,
				pageRect->right - margins_.right,
				fny + 1,
				cl);
	}
    if (DEBUG_DRAW_CLIP_REGION == 1)
    {
        lUInt32 cl = drawbuf->GetTextColor();
        cl = (cl & 0xFFFFFF) | (0x55000000);
        drawbuf->FillRect(
                clip,
                cl);
    }
	drawbuf->SetClipRect(NULL);
}

/// returns page count
int LVDocView::GetPagesCount()
{
    #if DEBUG_NOTES_HIDDEN_SHOW == 1
    return (pages_list_.length());
    #endif

    lUInt16 id = this->GetCrDom()->getAttrValueIndex(NOTES_HIDDEN_ID);
    if (this->cr_dom_->getNodeById(id) != NULL)
    {
        ldomNode * notes = this->cr_dom_->getNodeById(id);
        return GetPageForBookmark(ldomXPointer(notes,0));
    }
    else
    {
	    return (pages_list_.length());
    }
}

/// get vertical position of view inside document
int LVDocView::GetOffset()
{
	CheckPos();
	if (IsPagesMode() && page_ >= 0 && page_ < pages_list_.length())
	{
		return pages_list_[page_]->start;
	}
	return offset_;
}

int LVDocView::GetCurrPage()
{
	CheckPos();
	if (IsPagesMode() && page_ >= 0)
	{
		return page_;
	}
	return pages_list_.FindNearestPage(offset_, 0);
}

int LVDocView::GoToOffset(int offset, bool update_bookmark, bool allowScrollAfterEnd)
{
	position_is_set_ = true;
	CHECK_RENDER("setPos()")
	if (IsScrollMode())
	{
		if (offset > GetFullHeight() - height_ && !allowScrollAfterEnd)
		{
			offset = GetFullHeight() - height_;
		}
		if (offset < 0)
		{
			offset = 0;
		}
		offset_ = offset;
		int page = pages_list_.FindNearestPage(offset, 0);
		if (page >= 0 && page < pages_list_.length())
		{
			page_ = page;
		}
		else
		{
			page_ = -1;
		}
	}
	else
	{
		int page = pages_list_.FindNearestPage(offset, 0);
		//if (GetColumns() == 2) {
		//	page &= ~1;
		//}
		if (page < pages_list_.length())
		{
			offset_ = pages_list_[page]->start;
			page_ = page;
		}
		else
		{
			offset_ = 0;
			page_ = 0;
		}
	}
	if (update_bookmark)
	{
		bookmark_ = GetBookmark();
	}
	position_is_set_ = true;
	UpdateScrollInfo();
	return 1;
}

bool LVDocView::GoToPage(int page, bool update_bookmark)
{
	//CRLog::trace("LVDocView::GoToPage START page=%d, page_=%d", page, page_);
	CHECK_RENDER("GoToPage()")
	if (!pages_list_.length())
	{
		return false;
	}
	bool res = true;
	if (IsPagesMode())
	{
		if (page >= pages_list_.length())
		{
			page = pages_list_.length() - 1;
			res = false;
		}
		if (page < 0)
		{
			page = 0;
			res = false;
		}
		//if (GetColumns() == 2) {
		//	page &= ~1;
		//}
		if (page >= 0 && page < pages_list_.length())
		{
			offset_ = pages_list_[page]->start;
			page_ = page;
		}
		else
		{
			offset_ = 0;
			page_ = 0;
			res = false;
		}
	}
	else
	{
		if (page >= 0 && page < pages_list_.length())
		{
			offset_ = pages_list_[page]->start;
			page_ = page;
		}
		else
		{
			res = false;
			offset_ = 0;
			page_ = 0;
		}
	}
	// Нужно сначала выполнить position_is_set_ = true и только
	// затем вызывать GetBookmark(), иначе при первом вызове GoToPage,
	// CheckPos(), вызванный в GetBookmark() сбросит в 0
	// только что установленную нами page_
	position_is_set_ = true;
	if (update_bookmark)
	{
		bookmark_ = GetBookmark();
	}
	UpdateScrollInfo();
	if (res)
	{
		UpdateBookmarksRanges();
	}
	//CRLog::trace("LVDocView::GoToPage END page=%d, page_=%d", page, page_);
	return res;
}

/// Check whether resize or creation of buffer is necessary, ensure buffer is ok
static bool CheckBufferSize(LVRef<LVColorDrawBuf> &buf, int dx, int dy)
{
	if (buf.isNull() || buf->GetWidth() != dx || buf->GetHeight() != dy)
	{
		buf.Clear();
		buf = LVRef<LVColorDrawBuf>(new LVColorDrawBuf(dx, dy, 16));
		return false; // need redraw
	}
	else
	{
		return true;
	}
}

void LVDocView::SetBackgroundImage(LVImageSourceRef image, bool tiled)
{
	background_image = image;
	background_tiled_ = tiled;
	background_image_scaled_.Clear();
}

/// clears page background
void LVDocView::DrawBackgroundTo(LVDrawBuf &buf, int offsetX, int offsetY, int alpha)
{
	buf.SetBackgroundColor(background_color_);
	if (background_image.isNull())
	{
		// Solid color
		lUInt32 cl = background_color_;
		if (alpha > 0)
		{
			cl = (cl & 0xFFFFFF) | (alpha << 24);
			buf.FillRect(0, 0, buf.GetWidth(), buf.GetHeight(), cl);
		}
		else
		{
			buf.Clear(cl);
		}
	}
	else
	{
		// Texture
		int dx = buf.GetWidth();
		int dy = buf.GetHeight();
		if (background_tiled_)
		{
			if (!CheckBufferSize(background_image_scaled_,
			                     background_image->GetWidth(), background_image->GetHeight()))
			{
				// unpack
				background_image_scaled_->Draw(
						LVCreateAlphaTransformImageSource(background_image, alpha),
						0,
						0,
						background_image->GetWidth(),
						background_image->GetHeight(),
						false);
			}
			LVImageSourceRef src = LVCreateDrawBufImageSource(
					background_image_scaled_.get(),
					false);
			LVImageSourceRef tile = LVCreateTileTransform(src, dx, dy, offsetX, offsetY);
			buf.Draw(LVCreateAlphaTransformImageSource(tile, alpha), 0, 0, dx, dy);
			//CRLog::trace("draw completed");
		}
		else
		{
			if (IsScrollMode())
			{
				// scroll
				if (!CheckBufferSize(background_image_scaled_, dx, background_image->GetHeight()))
				{
					// unpack
					LVImageSourceRef resized = LVCreateStretchFilledTransform(
							background_image,
							dx,
							background_image->GetHeight(),
							IMG_TRANSFORM_STRETCH,
							IMG_TRANSFORM_TILE,
							0,
							0);
					background_image_scaled_->Draw(
							LVCreateAlphaTransformImageSource(resized, alpha),
							0,
							0,
							dx,
							background_image->GetHeight(),
							false);
				}
				LVImageSourceRef src = LVCreateDrawBufImageSource(
						background_image_scaled_.get(),
						false);
				LVImageSourceRef resized = LVCreateStretchFilledTransform(
						src,
						dx,
						dy,
						IMG_TRANSFORM_TILE,
						IMG_TRANSFORM_TILE,
						offsetX,
						offsetY);
				buf.Draw(LVCreateAlphaTransformImageSource(resized, alpha), 0, 0, dx, dy);
			}
			else if (GetColumns() != 2)
			{
				// single page
				if (!CheckBufferSize(background_image_scaled_, dx, dy))
				{
					// unpack
					LVImageSourceRef resized = LVCreateStretchFilledTransform(
							background_image,
							dx,
							dy,
							IMG_TRANSFORM_STRETCH,
							IMG_TRANSFORM_STRETCH,
							offsetX,
							offsetY);
					background_image_scaled_->Draw(
							LVCreateAlphaTransformImageSource(resized, alpha),
							0,
							0,
							dx,
							dy,
							false);
				}
				LVImageSourceRef src = LVCreateDrawBufImageSource(
						background_image_scaled_.get(),
						false);
				buf.Draw(LVCreateAlphaTransformImageSource(src, alpha), 0, 0, dx, dy);
			}
			else
			{
				// two pages
				int halfdx = (dx + 1) / 2;
				if (!CheckBufferSize(background_image_scaled_, halfdx, dy))
				{
					// unpack
					LVImageSourceRef resized = LVCreateStretchFilledTransform(
							background_image,
							halfdx,
							dy,
							IMG_TRANSFORM_STRETCH,
							IMG_TRANSFORM_STRETCH,
							offsetX,
							offsetY);
					background_image_scaled_->Draw(
							LVCreateAlphaTransformImageSource(resized, alpha),
							0,
							0,
							halfdx,
							dy,
							false);
				}
				LVImageSourceRef src = LVCreateDrawBufImageSource(
						background_image_scaled_.get(),
						false);
				buf.Draw(LVCreateAlphaTransformImageSource(src, alpha), 0, 0, halfdx, dy);
				buf.Draw(LVCreateAlphaTransformImageSource(src, alpha), dx / 2, 0, dx - halfdx, dy);
			}
		}
	}
	/*
	Рисуем разделительную полосу между страницами в двухколоночном режиме
	if (buf.GetBitsPerPixel() == 32 && GetPagesColumns() == 2) {
		int x = buf.GetWidth() / 2;
		lUInt32 cl = background_color_;
		cl = ((cl & 0xFCFCFC) + 0x404040) >> 1;
		buf.FillRect(x, 0, x + 1, buf.GetHeight(), cl);
	}
	*/
}

/*bool LVDocView::DrawImageTo(LVDrawBuf* buf, LVImageSourceRef img, int x, int y, int dx, int dy)
{
    if (img.isNull() || !buf)
        return false;
    // clear background
    DrawBackgroundTo(*buf, 0, 0);
    // draw image
    buf->Draw(img, x, y, dx, dy, true);
    return true;
}*/

/// draw current page to specified buffer
void LVDocView::Draw(LVDrawBuf &buf, bool auto_resize)
{
	int offset = -1;
	int page = -1;
	if (IsPagesMode())
	{
		page = page_;
		if (page < 0 || page >= pages_list_.length())
		{
			return;
		}
	}
	else
	{
		offset = offset_;
	}
	if (auto_resize)
	{
		buf.Resize(width_, height_);
	}
	buf.SetBackgroundColor(background_color_);
	buf.SetTextColor(text_color_);
	if (!is_rendered_ || !cr_dom_ || base_font_.isNull())
	{
		return;
	}
	if (IsScrollMode())
	{
		buf.SetClipRect(NULL);
		DrawBackgroundTo(buf, 0, offset);
		int cover_height = 0;
		if (pages_list_.length() > 0 && pages_list_[0]->type == PAGE_TYPE_COVER)
			cover_height = pages_list_[0]->height;
		if (offset < cover_height)
		{
			lvRect rc;
			buf.GetClipRect(&rc);
			rc.top -= offset;
			rc.bottom -= offset;
			rc.top += margins_.top;
			rc.bottom -= margins_.bottom;
			rc.left += margins_.left;
			rc.right -= margins_.right;
			DrawCoverTo(&buf, rc);
		}
		DrawDocument(buf,
		             cr_dom_->getRootNode(),
		             margins_.left,
		             0,
		             buf.GetWidth() - margins_.left - margins_.right,
		             buf.GetHeight(),
		             0,
		             -offset,
		             buf.GetHeight(),
		             &marked_ranges_,
		             &bookmark_ranges_);
	}
	else
	{
		if (page == -1)
		{
			page = pages_list_.FindNearestPage(offset, 0);
		}
		DrawBackgroundTo(buf, 0, 0);
		if (page >= 0 && page < pages_list_.length())
		{
			DrawPageTo(&buf, *pages_list_[page], &page_rects_[0]);
		}
		if (GetColumns() == 2 && page >= 0 && page + 1 < pages_list_.length())
		{
			DrawPageTo(&buf, *pages_list_[page + 1], &page_rects_[1]);
		}
	}
}

/// converts point from window to document coordinates, returns true if success
bool LVDocView::WindowToDocPoint(lvPoint &pt)
{
	CHECK_RENDER("windowToDocPoint()")
	if (IsScrollMode())
	{
		pt.y += offset_;
		pt.x -= margins_.left;
		return true;
	}
	else
	{
		int page = GetCurrPage();
		lvRect *rc = NULL;
		lvRect page1(page_rects_[0]);
		page1.left += margins_.left;
		page1.top += margins_.top;
		page1.right -= margins_.right;
		page1.bottom -= margins_.bottom;
		lvRect page2;
		if (page1.isPointInside(pt))
		{
			rc = &page1;
		}
		else if (GetColumns() == 2)
		{
			page2 = page_rects_[1];
			page2.left += margins_.left;
			page2.top += margins_.top;
			page2.right -= margins_.right;
			page2.bottom -= margins_.bottom;
			if (page2.isPointInside(pt))
			{
				rc = &page2;
				page++;
			}
		}
		if (rc && page >= 0 && page < pages_list_.length())
		{
			int page_y = pages_list_[page]->start;
			pt.x -= rc->left;
			pt.y -= rc->top;
			if (pt.y < pages_list_[page]->height)
			{
				//CRLog::debug(" point page offset( %d, %d )", pt.x, pt.y );
				pt.y += page_y;
				return true;
			}
		}
	}
	return false;
}

/// converts point from document to window coordinates, returns true if success
bool LVDocView::DocToWindowRect(lvRect &rect, bool modify)
{
    CHECK_RENDER("DocToWindowRect()")
    int page = GetCurrPage();
    if (page < 0 || page > pages_list_.length())
    {
        return false; // went out of bounds of pages array
    }
    int index = -1;

    if(rect.top+1 < pages_list_[page]->start)
    {
        return false;  //upper borderline check
    }
    else if (rect.top + 1 <= (pages_list_[page]->start + pages_list_[page]->height))
    {
        index = 0; // Left side of double-column page
    }
    else if (GetColumns() != 2 ||  page + 1 > pages_list_.length())
    {
        //CRLog::error("NOT PASSED BY VERTICAL CHECKS. Index = %d",index);
        //CRLog::error("rect.top+1 <= (pages_list_[page]->start + pages_list_[page]->height)");
        //CRLog::error("%d <= (%d )",rect.top+1, pages_list_[page]->start + pages_list_[page]->height);
        return false; // went out of bounds of pages array
    }
    else  if (rect.top + 1 <= (pages_list_[page + 1]->start + (pages_list_[page + 1]->height)))
    {
        index = 1; // Right side of double-column page
    }
    if (index < 0)
    {
        return false;
    }
	if(modify)
	{
		rect.left = rect.left + margins_.left + page_rects_[index].left;
		rect.right = rect.right + margins_.left + page_rects_[index].left;
		rect.top = rect.top + margins_.top - pages_list_[page + index]->start;
		rect.bottom = rect.bottom + margins_.top - pages_list_[page + index]->start;
	}
	return true;
}


void LVDocView::UpdateLayout()
{
	lvRect rc(0, 0, width_, height_);
	page_rects_[0] = rc;
	page_rects_[1] = rc;
	if (GetColumns() == 2)
	{
		int middle = (rc.left + rc.right) >> 1;
		page_rects_[0].right = middle; // - m_pageMargins.right;
		page_rects_[1].left = middle; // + m_pageMargins.left;
	}
}

/// Call setRenderProps(0, 0) to allow apply styles and rend method while loading.
void LVDocView::CheckRenderProps(int width, int height)
{
	if (!cr_dom_ || cr_dom_->getRootNode() == NULL)
	{
		return;
	}
	UpdateLayout();
	base_font_ = fontMan->GetFont(cfg_font_size_, 400, false, DEF_FONT_FAMILY, cfg_font_face_);
	if (!base_font_)
	{
		return;
	}
	cr_dom_->setRenderProps(width, height, base_font_, cfg_interline_space_);
	text_highlight_options_t h;
	h.bookmarkHighlightMode = highlight_mode_underline;
	h.selectionColor = 0xC0C0C0 & 0xFFFFFF;
	h.commentColor = 0xA08000 & 0xFFFFFF;
	h.correctionColor = 0xA00000 & 0xFFFFFF;
	cr_dom_->setHightlightOptions(h);
}

/// returns xpointer for specified window point
ldomXPointer LVDocView::getNodeByPoint(lvPoint pt)
{
	CHECK_RENDER("getNodeByPoint()")
	if (WindowToDocPoint(pt) && cr_dom_)
	{
		ldomXPointer ptr = cr_dom_->createXPointer(pt);
		//CRLog::debug("ptr (%d, %d) node=%08X offset=%d",
		//      pt.x, pt.y, (lUInt32)ptr.getNode(), ptr.getOffset() );
		return ptr;
	}
	return ldomXPointer();
}

/// returns image source for specified window point, if point is inside image
LVImageSourceRef LVDocView::getImageByPoint(lvPoint pt)
{
	LVImageSourceRef res = LVImageSourceRef();
	ldomXPointer ptr = getNodeByPoint(pt);
	if (ptr.isNull())
	{
		return res;
	}
	res = ptr.getNode()->getObjectImageSource();
	if (!res.isNull())
	{
		CRLog::debug("getImageByPoint(%d, %d): found image %d x %d",
		             pt.x, pt.y, res->GetWidth(), res->GetHeight());
	}
	return res;
}

/// sets selection for whole element, clears previous selection
void LVDocView::selectElement(ldomNode *elem)
{
	ldomXRangeList &sel = GetCrDom()->getSelections();
	sel.clear();
	sel.add(new ldomXRange(elem));
	UpdateSelections();
}

/// sets selection for list of words, clears previous selection
void LVDocView::selectWords(const LVArray<ldomWord> &words)
{
	ldomXRangeList &sel = GetCrDom()->getSelections();
	sel.clear();
	sel.addWords(words);
	UpdateSelections();
}

/// sets selections for ranges, clears previous selections
void LVDocView::selectRanges(ldomXRangeList &ranges)
{
	ldomXRangeList &sel = GetCrDom()->getSelections();
	if (sel.empty() && ranges.empty())
		return;
	sel.clear();
	for (int i = 0; i < ranges.length(); i++)
	{
		ldomXRange *item = ranges[i];
		sel.add(new ldomXRange(*item));
	}
	UpdateSelections();
}

/// sets selection for range, clears previous selection
void LVDocView::selectRange(const ldomXRange &range)
{
	// LVE:DEBUG
//    ldomXRange range2(range);
//    CRLog::trace("selectRange( %s, %s )",
//                LCSTR(range2.getStart().toString()), LCSTR(range2.getEnd().toString()) );
	ldomXRangeList &sel = GetCrDom()->getSelections();
	if (sel.length() == 1)
	{
		if (range == *sel[0])
			return; // the same range is set
	}
	sel.clear();
	sel.add(new ldomXRange(range));
	UpdateSelections();
}

void LVDocView::ClearSelection()
{
	ldomXRangeList &sel = GetCrDom()->getSelections();
	sel.clear();
	UpdateSelections();
}

/*
/// selects first link on page, if any. returns selected link range, null if no links.
ldomXRange *LVDocView::SelectFirstPageLink()
{
	ldomXRangeList list;
	GetCurrentPageLinks(list);
	if (!list.length())
		return NULL;
	selectRange(*list[0]);
	ldomXRangeList &sel = GetCrDom()->getSelections();
	UpdateSelections();
	return sel[0];
}
*/

/// update selection ranges
void LVDocView::UpdateSelections()
{
	CHECK_RENDER("updateSelections()")
	ldomXRangeList ranges(cr_dom_->getSelections(), true);
	//CRLog::trace("updateSelections() : selection count = %d", cr_dom_->getSelections().length());
	ranges.getRanges(marked_ranges_);
}

void LVDocView::UpdateBookmarksRanges()
{
	CHECK_RENDER("UpdateBookmarksRanges()")
	ldomXRangeList ranges;
	//TODO AXY
	if (highlight_bookmarks_)
	{
		for (int i = 0; i < bookmarks_.length(); i++)
		{
			CRBookmark *bmk = bookmarks_[i];
			int t = bmk->getType();
			if (t != bmkt_lastpos)
			{
				ldomXPointer p = cr_dom_->createXPointer(bmk->getStartPos());
				if (p.isNull())
					continue;
				lvPoint pt = p.toPoint();
				if (pt.y < 0)
					continue;
				ldomXPointer ep = (t == bmkt_pos)
				                  ? p
				                  : cr_dom_->createXPointer(bmk->getEndPos());
				if (ep.isNull())
					continue;
				lvPoint ept = ep.toPoint();
				if (ept.y < 0)
					continue;
				ldomXRange *n_range = new ldomXRange(p, ep);
				if (!n_range->isNull())
				{
					int flags = 1;
					if (t == bmkt_pos)
						flags = 2;
					if (t == bmkt_comment)
						flags = 4;
					if (t == bmkt_correction)
						flags = 8;
					n_range->setFlags(flags);
					ranges.add(n_range);
				}
				else
					delete n_range;
			}
		}
	}
	ranges.getRanges(bookmark_ranges_);
}

int LVDocView::GetColumns()
{
	return viewport_mode_ == MODE_SCROLL ? 1 : page_columns_;
}

/// sets current bookmark
void LVDocView::SetBookmark(ldomXPointer bm)
{
	bookmark_ = bm;
}

/// get view height
int LVDocView::GetHeight()
{
	return height_;
}

/// get view width
int LVDocView::GetWidth()
{
	return width_;
}

/// returns XPointer to middle paragraph of current page
ldomXPointer LVDocView::getCurrentPageMiddleParagraph()
{
	CheckPos();
	ldomXPointer ptr;
	if (!cr_dom_)
		return ptr;

	if (IsScrollMode())
	{
		// SCROLL mode
		int starty = offset_;
		int endy = offset_ + height_;
		int fh = GetFullHeight();
		if (endy >= fh)
			endy = fh - 1;
		ptr = cr_dom_->createXPointer(lvPoint(0, (starty + endy) / 2));
	}
	else
	{
		// PAGES mode
		int pageIndex = GetCurrPage();
		if (pageIndex < 0 || pageIndex >= pages_list_.length())
			pageIndex = GetCurrPage();
		LVRendPageInfo *page = pages_list_[pageIndex];
		if (page->type == PAGE_TYPE_NORMAL)
			ptr = cr_dom_->createXPointer(lvPoint(0, page->start + page->height / 2));
	}
	if (ptr.isNull())
		return ptr;
	ldomXPointerEx p(ptr);
	if (!p.isVisibleFinal())
		if (!p.ensureFinal())
			if (!p.prevVisibleFinal())
				if (!p.nextVisibleFinal())
					return ptr;
	return ldomXPointer(p);
}

/// returns bookmark
ldomXPointer LVDocView::GetBookmark()
{
	CheckPos();
	ldomXPointer ptr;
	if (cr_dom_)
	{
		if (IsPagesMode())
		{
			if (page_ >= 0 && page_ < pages_list_.length())
			{
				ptr = cr_dom_->createXPointer(lvPoint(0, pages_list_[page_]->start));
			}
		}
		else
		{
			ptr = cr_dom_->createXPointer(lvPoint(0, offset_));
		}
	}
	return ptr;
}

/// returns bookmark for specified page
ldomXPointer LVDocView::getPageBookmark(int page)
{
	CHECK_RENDER("getPageBookmark()")
	if (page < 0 || page >= pages_list_.length())
	{
		return ldomXPointer();
	}
	ldomXPointer ptr = cr_dom_->createXPointer(lvPoint(0, pages_list_[page]->start));
	return ptr;
}

static lString16 GetSectionHeader(ldomNode *section)
{
	lString16 header;
	if (!section || section->getChildCount() == 0)
	{
		return header;
	}
	ldomNode *child = section->getChildElementNode(0, L"title");
	if (!child)
	{
		return header;
	}
	header = child->getText(L' ', 1024);
	return header;
}

/// get bookmark position text
bool LVDocView::getBookmarkPosText(ldomXPointer bm, lString16 &titleText, lString16 &posText)
{
	CHECK_RENDER("getBookmarkPosText")
	titleText = posText = lString16::empty_str;
	if (bm.isNull())
		return false;
	ldomNode *el = bm.getNode();
	if (el->isText())
	{
		lString16 txt = bm.getNode()->getText();
		int startPos = bm.getOffset();
		int len = txt.length() - startPos;
		if (len > 0)
			txt = txt.substr(startPos, len);
		if (startPos > 0)
			posText = "...";
		posText += txt;
		el = el->getParentNode();
	}
	else
	{
		posText = el->getText(L' ', 1024);
	}
	bool inTitle = false;
	do
	{
		while (el && el->getNodeId() != el_section && el->getNodeId()
		                                              != el_body)
		{
			if (el->getNodeId() == el_title || el->getNodeId() == el_subtitle)
				inTitle = true;
			el = el->getParentNode();
		}
		if (el)
		{
			if (inTitle)
			{
				posText.clear();
				if (el->getChildCount() > 1)
				{
					ldomNode *node = el->getChildNode(1);
					posText = node->getText(' ', 8192);
				}
				inTitle = false;
			}
			if (el->getNodeId() == el_body && !titleText.empty())
				break;
			lString16 txt = GetSectionHeader(el);
			lChar16 lastch = !txt.empty() ? txt[txt.length() - 1] : 0;
			if (!titleText.empty())
			{
				if (lastch != '.' && lastch != '?' && lastch != '!')
					txt += ".";
				txt += " ";
			}
			titleText = txt + titleText;
			el = el->getParentNode();
		}
		if (titleText.length() > 50)
			break;
	}
	while (el);
	limitStringSize(titleText, 70);
	limitStringSize(posText, 120);
	return true;
}

/// moves position to bookmark
void LVDocView::GoToBookmark(ldomXPointer bm)
{
	CHECK_RENDER("goToBookmark()")
	position_is_set_ = false;
	bookmark_ = bm;
}

/// get page number by bookmark
int LVDocView::GetPageForBookmark(ldomXPointer bm)
{
	CHECK_RENDER("getBookmarkPage()")
	if (bm.isNull())
	{
		return 0;
	}
	else
	{
		lvPoint pt = bm.toPoint();
		if (pt.y < 0)
		{
			return 0;
		}
		return pages_list_.FindNearestPage(pt.y, 0);
	}
}

void LVDocView::UpdateScrollInfo()
{
	CheckPos();
	if (viewport_mode_ == MODE_SCROLL)
	{
		int npos = offset_;
		int fh = GetFullHeight();
		int shift = 0;
		int npage = height_;
		while (fh > 16384)
		{
			fh >>= 1;
			npos >>= 1;
			npage >>= 1;
			shift++;
		}
		if (npage < 1)
			npage = 1;
		scroll_info_.pos = npos;
		scroll_info_.maxpos = fh - npage;
		scroll_info_.pagesize = npage;
		scroll_info_.scale = shift;
		char str[32];
		sprintf(str, "%d%%", fh > 0 ? (100 * npos / fh) : 0);
		scroll_info_.posText = lString16(str);
	}
	else
	{
		int page = GetCurrPage();
		int vpc = GetColumns();
		scroll_info_.pos = page / vpc;
		scroll_info_.maxpos = (pages_list_.length() + vpc - 1) / vpc - 1;
		scroll_info_.pagesize = 1;
		scroll_info_.scale = 0;
		char str[32] = "";
		if (pages_list_.length() > 1)
		{
			if (page <= 0)
			{
				sprintf(str, "cover");
			}
			else
			{
				sprintf(str, "%d / %d", page, pages_list_.length() - 1);
			}
		}
		scroll_info_.posText = lString16(str);
	}
}

/// move to position specified by scrollbar
bool LVDocView::goToScrollPos(int pos)
{
	if (viewport_mode_ == MODE_SCROLL)
	{
		GoToOffset(scrollPosToDocPos(pos));
		return true;
	}
	else
	{
		int vpc = this->GetColumns();
		int curPage = GetCurrPage();
		pos = pos * vpc;
		if (pos >= GetPagesCount())
			pos = GetPagesCount() - 1;
		if (pos < 0)
			pos = 0;
		if (curPage == pos)
			return false;
		GoToPage(pos);
		return true;
	}
}

/// converts scrollbar pos to doc pos
int LVDocView::scrollPosToDocPos(int scrollpos)
{
	if (viewport_mode_ == MODE_SCROLL)
	{
		int n = scrollpos << scroll_info_.scale;
		if (n < 0)
			n = 0;
		int fh = GetFullHeight();
		if (n > fh)
			n = fh;
		return n;
	}
	else
	{
		int vpc = GetColumns();
		int n = scrollpos * vpc;
		if (!pages_list_.length())
			return 0;
		if (n >= pages_list_.length())
			n = pages_list_.length() - 1;
		if (n < 0)
			n = 0;
		return pages_list_[n]->start;
	}
}

/// get page document range, -1 for current page
LVRef<ldomXRange> LVDocView::GetPageDocRange(int page_index)
{
	CHECK_RENDER("getPageDocRange()")
	LVRef<ldomXRange> res(NULL);
	if (IsScrollMode())
	{
		// SCROLL mode
		int starty = offset_;
		int endy = offset_ + height_;
		int fh = GetFullHeight();
		if (endy >= fh)
			endy = fh - 1;
		ldomXPointer start = cr_dom_->createXPointer(lvPoint(0, starty));
		ldomXPointer end = cr_dom_->createXPointer(lvPoint(0, endy));
		if (start.isNull() || end.isNull())
			return res;
		res = LVRef<ldomXRange>(new ldomXRange(start, end));
	}
	else
	{
		// PAGES mode
		if (page_index < 0 || page_index >= pages_list_.length())
		{
			page_index = GetCurrPage();
		}
		if(pages_list_.empty())
		{
			CRLog::error("Fatal error: no pages in pages list found!");
			return res;
		}
		LVRendPageInfo *page = pages_list_[page_index];
		if (page->type != PAGE_TYPE_NORMAL)
		{
			return res;
		}
        int columns = GetColumns();
        int end_y = page->start + (height_ * columns);
        if (page_index + columns < pages_list_.length())
        {
            LVRendPageInfo *nextpage = pages_list_[page_index + columns];
            end_y = nextpage->start;
        }
        ldomXPointer start = cr_dom_->createXPointer(lvPoint(0, page->start));
        ldomXPointer end = cr_dom_->createXPointer(lvPoint(0, end_y), 1);
		if (start.isNull() || end.isNull())
		{
			return res;
		}
		res = LVRef<ldomXRange>(new ldomXRange(start, end));
	}
	return res;
}

void LVDocView::GetImageScaleParams(ldomNode* node, int &imgheight, int &imgwidth)
{
    lvRect margins = this->cfg_margins_;
    int width = (viewport_mode_ == MODE_PAGES && GetColumns() > 1)?(this->GetWidth()/2):this->GetWidth();
    int maxheight = this->GetHeight() - (margins.bottom + margins.top);
    int	maxwidth  = width - (margins.left + margins.right);
    LVImageSource *image = node->getObjectImageSource().get();
    if(image)
    {
	    imgheight = image->GetHeight();
	    imgwidth = image->GetWidth();
	    int pscale_x = 1000 * maxwidth / imgwidth;
	    int pscale_y = 1000 * maxheight / imgheight;
	    int pscale = pscale_x < pscale_y ? pscale_x : pscale_y;
	    int maxscale = 3 * 1000;
	    if (pscale > maxscale)
		    pscale = maxscale;
	    imgheight = imgheight * pscale / 1000;
	    imgwidth = imgwidth * pscale / 1000;
    }
}

LVArray<TextRect> LVDocView::GetPageFootnotesText(int page, bool rightpage)
{
	CHECK_RENDER("GetPageFootnotesText()");
    LVArray<TextRect> result;
	if (!cr_dom_)
	{
		CRLog::error("No crdom found!");
		return result;
	}
	LVArray<FootNoteInfo>  fnotes = this->pages_list_.get(page)->footnotes_info;
	if (fnotes.empty())
	{
		//CRLog::trace("No footnotes on page %d",page);
		return result;
	}

	int shift = 0;
	if (rightpage)
	{
		shift = width_ /2;
	}

	//CRLog::error("Footnotes page = %d", page);
    FootNoteInfo first_fnote = fnotes.get(0);
    FootNoteInfo last_fnote = fnotes.get(fnotes.length() - 1);

	int start_y = first_fnote.start;
	int end_y = last_fnote.start + last_fnote.height;

	int y_zero = 10 + margins_.top + this->pages_list_[page]->height + (fontMan->font_size_ * 0.25);

	lvPoint start_point(0, start_y);
	lvPoint end_point(0, end_y);
	ldomXPointer xp1 = cr_dom_->createXPointer(start_point);
	ldomXPointer xp2 = cr_dom_->createXPointer(end_point);
    //CRLog::error("xp1 node = %s",LCSTR(xp1.getNode()->getXPath()));
    //CRLog::error("xp2 node = %s",LCSTR(xp2.getNode()->getXPath()));
	ldomXRange range = ldomXRange(xp1, xp2);
	LVArray<TextRect> word_chars;
	int width = (this->page_columns_==1)?this->GetWidth() : this->GetWidth()/2;
    int line_width = width - this->cfg_margins_.left - this->cfg_margins_.right;
	range.getRangeChars(word_chars,line_width);


	if (word_chars.empty())
	{
		CRLog::error("No footnotes rects on current page, but footnotes exist!");
		return result;
	}
	int abs_top = word_chars.get(0).getRect().top;
	int curr_top = y_zero;
	LVFont *font = this->GetBaseFont().get();
	lvRect last = word_chars.get(0).getRect();
	for (int i = 0; i < word_chars.length(); i++)
	{
		TextRect word = word_chars.get(i);
		lvRect rect = word.getRect();
		int curr_height = rect.height();
//		CRLog::error("before rect = [%d:%d][%d:%d]",rect.left,rect.right,rect.top,rect.bottom);
		if (rect.top != abs_top)
		{
			curr_top = curr_top + rect.height();
		}
		abs_top = rect.top;
		rect.top = curr_top;
		rect.bottom = curr_top + curr_height;
		rect.left += margins_.left + shift;
		rect.right += margins_.left + shift;


		int lastheight = last.height();
		if (rect.height() >= lastheight * 1.5)
		{
			int curwidth = font->getCharWidth(word.getText().firstChar());
			rect.bottom = rect.top + lastheight;
			rect.right = rect.right + curwidth;
		}

		word.setRect(rect);
		word_chars.set(i, word);
		last = rect;
	}
	result.add(word_chars);
    return result;
}

LVArray<TextRect> LVDocView::GetPageFootnotesLinks(int page, bool rightpage)
{
    LVArray<TextRect> result;
	LVArray<TextRect> word_chars = GetPageFootnotesText(page, rightpage);

    if(word_chars.empty())
	{
		return result;
	}
    //Uncomment to pass all text through
    //return word_chars;
    //temp

    ldomNode *last_node = word_chars.get(0).getNode();
	lString16 last_href = last_node->getHRef();
	LVArray<TextRect> link_list;
	for (int i = 0; i < word_chars.length(); i++)
	{
		TextRect word = word_chars.get(i);
		ldomNode *node = word.getNode();
		lString16 href = (node == last_node) ? last_href : node->getHRef();
		last_href = href;
		last_node = word.getNode();
		if (href.empty())
		{
			continue;
		}

		word.setText(href);
		link_list.add(word);
	}

	if (link_list.empty())
	{
		return result;
	}

	TextRect  lastword = link_list.get(0);
	lvRect    lastrect = lastword.getRect();
	lString16 lasthref = lastword.getText();

	for (int i = 0; i < link_list.length(); i++)
	{
		TextRect  currword = link_list.get(i);
		lvRect    currrect = currword.getRect();
		lString16 currhref = currword.getText();
        if (currrect.top == lastrect.top && currrect.bottom == lastrect.bottom && currhref == lasthref )
		{
			lastrect.right = currrect.right;
		}
		else
		{
			result.add(TextRect(NULL, lastrect, lasthref));
			lastrect = currword.getRect();
		}
		lasthref = currhref;
	}
	//last link
	result.add(TextRect(NULL, lastrect, lasthref));
	return result;
}

LVArray<TextRect> LVDocView::GetCurrentPageFootnotesLinks()
{
    LVArray<TextRect> result;
	if (page_columns_ == 2)
	{
		result.add(GetPageFootnotesLinks(page_));
		if (pages_list_.length() > page_ + 1)
		{
            result.add(GetPageFootnotesLinks(page_ + 1, true));
		}
        return result;
    }
	//page_columns = 1
    return GetPageFootnotesLinks(page_);
}

void LVDocView::GetCurrentPageLinks(LVArray<TextRect>& links_list)
{
	links_list.clear();
	LVRef<ldomXRange> page_range = GetPageDocRange();
	if (page_range.isNull())
	{
		return;
	}
	class LinkKeeper : public ldomNodeCallback
	{
	    LVDocView * doc_view_;
        LVArray<TextRect> &list_;
        RectHelper rectHelper_;
		void ProcessFinalNode(ldomNode *node)
        {
        	//CRLog::error("final node = %s",LCSTR(node->getXPath()));
            ldomXRange nodeRange = ldomXRange(node);
            ldomNode *parent_node = node->getParentNode();
            if (parent_node == nullptr)
            {
                return;
            }
            rectHelper_.Init(node);
            lString16 text = node->getText();
            int pos = nodeRange.getStart().getOffset();
            int len = text.length();
            int end = nodeRange.getEnd().getOffset();
            if (len > end)
            {
                len = end;
            }
            lString16 href = node->getHRef();
            if(href.empty())
            {
	            return;
            }
	        LVFont *font = doc_view_->GetBaseFont().get();
            LVArray<TextRect> pre;
            for (; pos < len; pos++)
            {
                ldomWord domword = ldomWord(node, pos, pos + 1);
                //old implementation
                //lvRect rect = domword.getRect();
	            //new implementation
	            lvRect rect = rectHelper_.getRect(domword);
	            lString16 string = domword.getText();
                pre.add(TextRect(node, rect, string));
            }
            if(pre.empty())
            {
	            return;
            }
            TextRect word = pre.get(0);
            lvRect rect = word.getRect();


            lvRect last = rect;
            for (int i = 0; i < pre.length(); i++)
            {

                TextRect word = pre.get(i);
                lvRect raw_rect = word.getRect();
                lString16 string = word.getText();
                int lastheight = last.bottom - last.top;
                if (raw_rect.bottom - raw_rect.top >= lastheight * 1.5)
                {
                    int curwidth = font->getCharWidth(string.firstChar());
                    raw_rect.bottom = raw_rect.top + lastheight;
                    raw_rect.right = raw_rect.right + curwidth;
                }
                if (raw_rect.top == rect.top && raw_rect.bottom == rect.bottom)
                {
                    rect.right = raw_rect.right;
                }
                else
                {
                    list_.add(TextRect(node, rect, href));
                    rect = word.getRect();
                }
                last = raw_rect;
            }
            list_.add(TextRect(node, rect, href));
        }

        void ProcessImageNode(ldomNode *node)
        {
            if (!node->isImage())
            {
                return;
            }

            //CRLog::error("in node = %s", LCSTR(node->getNodeName()));

            int end_index = node->getText().length();
            ldomXPointerEx end = ldomXPointerEx(node, end_index);
            ldomXPointer xp = ldomXPointer(node, end_index);

            int imgheight = 0;
            int imgwidth = 0;
            doc_view_->GetImageScaleParams(node, imgheight, imgwidth);

            lvRect imgrect;
            if (!xp.getRect(imgrect))
            {
                CRLog::error("Unable to get imagerect!");
            }

            css_style_rec_t *style = node->getStyle().get();
            css_style_rec_t *parent_style = node->getParentNode()->getStyle().get();

            imgrect.right=imgrect.left+imgwidth;
            imgrect.bottom=imgrect.top+imgheight;

            lString16 href = node->getHRef();
            if(href.empty())
            {
                return;
            }
            list_.add(TextRect(node, imgrect, href));
        }

        void ProcessLinkNode(ldomNode *node)
		{
			for (lUInt32 i = 0; i < node->getChildCount(); i++)
			{
				ldomNode *child = node->getChildNode(i);
				if (child->isText())
				{
					ProcessFinalNode(child);
				}
				else if (child->isImage())
				{
					ProcessImageNode(child);
				}
				else
				{
					ProcessLinkNode(child);
				}
			}
		}
	public:
		LinkKeeper(LVArray<TextRect> &list, LVDocView* doc_view) : list_(list),doc_view_(doc_view) {}
		// Called for each text fragment in range
		virtual void onText(ldomXRange *node_range)
		{
			ldomNode *node = node_range->getStart().getNode();
			ldomNode *element_node = node->getParentNode();
			if (element_node->isNull() || node->getHRef() == lString16::empty_str)
			{
                return;
            }
			ProcessLinkNode(element_node);
#ifdef TRDEBUG
			lString16 text = element_node->getText();
			int start = node_range->getStart().getOffset();
			int end = node_range->getEnd().getOffset();
			if (start < end)
			{
				text = text.substr(start, end - start);
			}
			ldomNode *end_node = node_range->getEnd().getNode();
			CRLog::debug("GetCurrentPageLinks first text on page: %d-%d %s",
			             node->getDataIndex(), end_node->getDataIndex(), LCSTR(text));
#endif
		}
		// Called for each node in range
        virtual bool onElement(ldomNode *node)
		{
			if (node->getNodeId() != el_a)
			{
				return true;
			}
#ifdef TRDEBUG
			if (node->getChildCount() == 0)
			{
				// Empty link in malformed doc, example: <a name="sync_on_demand"></a>
				CRLog::trace("GetCurrentPageLinks empty link in malformed doc");
			}
#endif
			ProcessLinkNode(node);
			return true;
		}


		bool processElement(ldomNode* node, ldomXRange * range)
		{
			for (lUInt32 i = 0; i < node->getChildCount(); i++)
			{
				ldomNode *child = node->getChildNode(i);

				//Text node is processed only when range includes, in processLinkNode
				/*
				if (child->isText())
				{
					//CRLog::error("_______TEXT nodepath = %s",LCSTR(child->getXPath()));
					processText(child,range);
				}
				else
				{*/

				if (this->onElement(child))
				{
					if (processElement(child, range))
					{
						return true;
					}
				}
				//}
				if (child == range->getEndNode())
				{
					return true;
				}
			}
			return false;
		}
	};
	LinkKeeper callback(links_list,this);
    page_range->forEach2(&callback);
	if (viewport_mode_ == MODE_PAGES && GetColumns() > 1)
	{
		// Process second page
		int page_index = GetCurrPage();
		page_range = GetPageDocRange(page_index + 1);
		if (!page_range.isNull())
		{
            page_range->forEach2(&callback);
		}
	}
}

LVArray<lvRect> LVDocView::GetCurrentPageParas()
{
	LVArray<lvRect> result;
	LVRef<ldomXRange> page_range = GetPageDocRange();
	if (page_range.isNull())
	{
		return result;
	}
	class ParaKeeper : public ldomNodeCallback
	{
		LVArray<lvRect> para_rect_array;
		bool endnode_found_ = false;
		int unused_;
		RectHelper rectHelper_;

		bool NodeIsAllowed(ldomNode * node)
        {
            LVArray<lString16> allowed;
            allowed.add(lString16("p"));
            //allowed.add(lString16("ul"));
            allowed.add(lString16("li"));
            allowed.add(lString16("h1"));
            allowed.add(lString16("h2"));
            allowed.add(lString16("h3"));
            allowed.add(lString16("h4"));
            allowed.add(lString16("h5"));
            allowed.add(lString16("h6"));
            allowed.add(lString16("subtitle"));
            allowed.add(lString16("blockquote"));
            allowed.add(lString16("autoBoxing"));
            allowed.add(lString16("br"));
            allowed.add(lString16("date"));
            //allowed.add(lString16("code"));
            //allowed.add(lString16("pagebreak"));

            lString16 nodename = node->getNodeName();
            if(node->getParentNode() != NULL)
            {
                css_display_t display = node->getParentNode()->getStyle().get()->display;
                if(display == css_d_run_in)
                {
                    return false;
                }
            }

            for (int i = 0; i < allowed.length(); i++)
            {
	            //if (node->isText()))
	            //{
	            if (nodename.compare(allowed.get(i))==0)
	            {
		            return true;
	            }
	            //}
            }
            return false;
        }

		lvRect ProcessNodeParaends(ldomNode *node, ldomXRange* range)
		{
            //CRLog::error("in node = %s",LCSTR(node->getXPath()));
            lvRect paraend;
            int childcount = node->getChildCount();
            if (childcount == 0 && node->isNodeName("br"))
            {
                //CRLog::error("BR IN");
            	//int index = (node->getNodeIndex() > 0) ? node->getNodeIndex() - 1 : 0;
                int index = node->getNodeIndex();
                if (index <= 0)
                {
                    //CRLog::error("BR1");
	                return paraend;
                }
                ldomNode *prevNode = node->getParentNode()->getChildNode(index-1);
                if (prevNode == NULL)
                {
                    //CRLog::error("BR2");
                    return paraend;
                }
                ldomNode *TxtNode = (prevNode->isText()) ? prevNode : NULL ; //prevNode->getLastTextChild();
                if (TxtNode == NULL)
                {
                    //CRLog::error("prevnode = [%s]",LCSTR(prevNode->getXPath()));
                    //CRLog::error("BR3");
                    return paraend;
                }
                //CRLog::error("BR OK");
                paraend = ProcessFinalNode_GetNodeEnd(TxtNode);
            }
			for (lUInt32 i = 0; i < childcount; i++)
			{
				ldomNode *child = node->getChildNode(i);
				if (child->isText())
				{
					paraend = ProcessFinalNode_GetNodeEnd(child);
                  //  CRLog::error("Final node processed");
                  //  CRLog::error("final node = %s",LCSTR(child->getNodeName()));
                  //  CRLog::error("final text = %s",LCSTR(child->getText()));
				}
				else
				{
                //    CRLog::error("diving deeper");
					paraend = ProcessNodeParaends(child,range);
                }
              //  CRLog::error("node ended");
                if (child == range->getEndNode())
                {
                    //CRLog::error("endnode found for processnodeparaends");
                    endnode_found_ = true;
                    break;
                }
                if (endnode_found_)
                {
                    break;
                }
			}
            if(NodeIsAllowed(node))
            {
             //   CRLog::error("Node added");
                para_rect_array.add(paraend);
             //   CRLog::error("paraend = [%d:%d][%d:%d]",paraend.left,paraend.right,paraend.top,paraend.bottom);
            }
            return paraend;
		}

		lvRect ProcessFinalNode_GetNodeEnd(ldomNode *node)
		{
			lvRect empty_rect(0, 0, 0, 0);
			ldomXPointerEx end;
			if (node->isText())
			{
				int end_index = node->getText().length();
				end = ldomXPointerEx(node, end_index);
			}
			else
			{
				return empty_rect;
			}

            lvRect end_rect;

            rectHelper_.Init(node);
#if 0
            //debug test: New getrect vs old getrect
            {
                lvRect oldrect;
                lvRect newrect;
                end.getRect(oldrect);
                rectHelper_.processRect(end,newrect);
                if(oldrect!=newrect)
                {
                    CRLog::warn("new rect != old rect [%d:%d][%d:%d] != [%d:%d][%d:%d]",newrect.left,newrect.right,newrect.top,newrect.bottom,oldrect.left,oldrect.right,oldrect.top,oldrect.bottom);
                }
            }
#endif
			//old implementation
			//if (!end.getRect(end_rect))

            //new implementation
			if(!rectHelper_.processRect(end,end_rect))
			{
				CRLog::warn("Unable to get node end coordinates. Ignoring");
				//para_rect_array.add(empty_rect);
				return empty_rect;
			}
            //CRLog::error("node = %s",LCSTR(node->getNodeName()));
            //CRLog::error("text = %s",LCSTR(node->getText()));
            //CRLog::error("rect = [%d:%d][%d:%d]",end_rect.left,end_rect.right,end_rect.top,end_rect.bottom);
			//para_rect_array.add(end_rect);
            end_rect.left += gTextLeftShift;
            end_rect.right += gTextLeftShift;
            end_rect.right -=1;
			return end_rect;
		}

	public:

		ParaKeeper(int &unused) : unused_(unused)	{	}

        // Called for each text fragment in range
        void processText(ldomNode *node1, ldomXRange *range)
        {
            ldomXRange node_range = ldomXRange(node1);
            ldomNode *node = node_range.getStart().getNode();
            ldomNode *txtnode = node_range.getStart().getNode();
            if (node->isNull())
            {
                return;
            }
           int index = node->getNodeIndex();
           int lastindex = node->getParentNode()->getChildCount()-1;
           if (index < lastindex)
           {
               return;
           }
            while (node != NULL && node->getParentNode() != NULL)
            {
                node = node->getParentNode();
                if (NodeIsAllowed(node))
                {
                    break;
                }
                int index = node->getNodeIndex();
                int lastindex = node->getParentNode()->getChildCount()-1;
                if (index < lastindex)
                {
                    return;
                }
            }
            if (node->getParentNode() == NULL)
            {
                return;
            }
	        para_rect_array.add(ProcessFinalNode_GetNodeEnd(txtnode));
	        //ProcessNodeParaends(node, range);
        }

        virtual bool processElement(ldomNode *node, ldomXRange *range)
        {
            ProcessNodeParaends(node, range);
            return (endnode_found_) ? true : false;
        }

        LVArray<lvRect> GetParaArray()
        {
            return para_rect_array;
        }
    };

	int unused = 0;
	ParaKeeper callback(unused);
    page_range->forEach2(&callback);
	result = callback.GetParaArray();

	if (viewport_mode_ == MODE_PAGES && GetColumns() > 1)
	{
		// Process second page
		int page_index = GetCurrPage();
		page_range = GetPageDocRange(page_index + 1);
		if (!page_range.isNull())
		{
            page_range->forEach2(&callback);
			result.add(callback.GetParaArray());
		}
	}
	return result;
}

LVArray<ImgRect> LVDocView::GetCurrentPageImages()
{

	LVArray<ImgRect> result;
	LVRef<ldomXRange> page_range = GetPageDocRange();
	if (page_range.isNull())
	{
		return result;
	}
	class ImageKeeper : public ldomNodeCallback
	{
		LVArray<ImgRect> img_rect_array;
		LVDocView* doc_view_;
		RectHelper rectHelper_;
	public:

		ImageKeeper(LVDocView* doc_view) : doc_view_(doc_view) {	}
		// Called for each text fragment in range
		virtual void onText(ldomXRange *node_range)
		{
			return;
		}
		// Called for each node in range
		virtual bool onElement(ldomNode *node)
        {
            if (!node->isImage())
            {
                return true;
            }

            int end_index = node->getText().length();
            ldomXPointerEx end = ldomXPointerEx(node, end_index);
            ldomXPointer xp = ldomXPointer(node, end_index);

            css_style_rec_t *style = node->getStyle().get();
            css_style_rec_t *parent_style = node->getParentNode()->getStyle().get();
            lvRect imgrect;
            rectHelper_.Init(node);
	        #if 0
	        //debug test: New getrect vs old getrect
            {
                lvRect oldrect;
                lvRect newrect;
                xp.getRect(oldrect);
                rectHelper_.processRect(xp,newrect);
                if(oldrect!=newrect)
                {
                    CRLog::warn("new rect != old rect [%d:%d][%d:%d] != [%d:%d][%d:%d]",newrect.left,newrect.right,newrect.top,newrect.bottom,oldrect.left,oldrect.right,oldrect.top,oldrect.bottom);
                }
            }
            #endif

            //old implementation
            //if (!xp.getRect(imgrect))
	        //new implementation
            if (!rectHelper_.processRect(xp,imgrect))
            {
                CRLog::error("Unable to get imagerect!");
            }

	        int imgheight = 0;
	        int imgwidth = 0;
	        doc_view_->GetImageScaleParams(node, imgheight, imgwidth);

	        if(node->getHRef() != lString16::empty_str && style->display != css_d_inline)
            {
                imgrect.left  = imgrect.left  + gTextLeftShift;
                imgrect.right = imgrect.right + gTextLeftShift;
            }
            else if (style->display == css_d_block)
            {
                imgrect.top = imgrect.top + style->font_size.value;
                //cite p image fix
                if (node->getParentNode()->getNodeName()=="p")
                {
                    if (node->getParentNode()->getParentNode()->getNodeName() == "cite")
                    {
                        imgrect.left = imgrect.left + style->font_size.value;
                    }
                }
	            if (node->getParentNode()->getNodeName()=="div")
	            {
		            if (node->getParentNode()->getParentNode()->getNodeName() == "span")
		            {
			            imgrect.left = imgrect.left + gTextLeftShift;
                        imgrect.top = imgrect.top - style->font_size.value;
		            }
	            }
            }

            imgrect.right=imgrect.left+imgwidth;
            imgrect.bottom=imgrect.top+imgheight;

            if(style->display == css_d_inline)
            {
                imgrect.left  = imgrect.left  + gTextLeftShift;
                imgrect.right = imgrect.right + gTextLeftShift;
            }
            img_rect_array.add(ImgRect(node,imgrect));

			return false;
		}
		LVArray<ImgRect> GetImgArray()
		{
			return img_rect_array;
		}
	};


	ImageKeeper callback(this);
    page_range->forEach2(&callback);
	result = callback.GetImgArray();

	if (viewport_mode_ == MODE_PAGES && GetColumns() > 1)
	{
		// Process second page
		int page_index = GetCurrPage();
		page_range = GetPageDocRange(page_index + 1);
		if (!page_range.isNull())
		{
            page_range->forEach2(&callback);
			result.add(callback.GetImgArray());
		}
	}
	return result;
}

/// sets new list of bookmarks, removes old values
void LVDocView::SetBookmarks(LVPtrVector<CRBookmark> &bookmarks)
{
	bookmarks_.clear();
	for (int i = 0; i < bookmarks.length(); i++)
	{
		bookmarks_.add(new CRBookmark(*bookmarks[i]));
	}
	UpdateBookmarksRanges();
}

static void UpdateOutline(LVDocView *doc_view,
                          LVPtrVector<LvTocItem, false> &list,
                          LvTocItem *item)
{
	list.add(item);
	if (!item->getXPointer().isNull())
	{
		int page = doc_view->GetPageForBookmark(item->getXPointer());
		if (page >= 0 && page < doc_view->GetPagesCount())
		{
			item->setPage(page);
		}
		else
		{
			item->setPage(-1);
		}
	}
	else
	{
		item->setPage(-1);
	}
	for (int i = 0; i < item->getChildCount(); i++)
	{
		UpdateOutline(doc_view, list, item->getChild(i));
	}
}

void LVDocView::GetOutline(LVPtrVector<LvTocItem, false> &outline)
{
	outline.clear();
	if (cr_dom_)
	{
		LvTocItem *outline_root = cr_dom_->getToc();
		if (outline_root->getChildCount() == 0)
		{
			CRLog::error("outline root childcount = 0, no table of contents generated");
		}
		else
		{
			// First item its just dummy container, so we skip it
			for (int i = 0; i < outline_root->getChildCount(); i++)
			{
			    LvTocItem * child = outline_root->getChild(i);
                UpdateOutline(this, outline, child);
			}
		}
	}
}

static int CalcBookmarkMatch(lvPoint pt, lvRect &rc1, lvRect &rc2, int type)
{
	if (pt.y < rc1.top || pt.y >= rc2.bottom)
		return -1;
	if (type == bmkt_pos)
	{
		return abs(pt.x - 0);
	}
	if (rc1.top == rc2.top)
	{
		// single line
		if (pt.y >= rc1.top && pt.y < rc2.bottom && pt.x >= rc1.left && pt.x < rc2.right)
		{
			return abs(pt.x - (rc1.left + rc2.right) / 2);
		}
		return -1;
	}
	else
	{
		// first line
		if (pt.y >= rc1.top && pt.y < rc1.bottom && pt.x >= rc1.left)
		{
			return abs(pt.x - (rc1.left + rc1.right) / 2);
		}
		// last line
		if (pt.y >= rc2.top && pt.y < rc2.bottom && pt.x < rc2.right)
		{
			return abs(pt.x - (rc2.left + rc2.right) / 2);
		}
		// middle line
		return abs(pt.y - (rc1.top + rc2.bottom) / 2);
	}
}

/// Find bookmark by window point, return NULL if point doesn't belong to any bookmark
CRBookmark *LVDocView::FindBookmarkByPoint(lvPoint pt)
{
	if (!WindowToDocPoint(pt))
		return NULL;
	CRBookmark *best = NULL;
	int bestMatch = -1;
	for (int i = 0; i < bookmarks_.length(); i++)
	{
		CRBookmark *bmk = bookmarks_[i];
		int t = bmk->getType();
		if (t == bmkt_lastpos)
			continue;
		ldomXPointer p = cr_dom_->createXPointer(bmk->getStartPos());
		if (p.isNull())
			continue;
		lvRect rc;
		if (!p.getRect(rc))
			continue;
		ldomXPointer ep = (t == bmkt_pos) ? p : cr_dom_->createXPointer(bmk->getEndPos());
		if (ep.isNull())
			continue;
		lvRect erc;
		if (!ep.getRect(erc))
			continue;
		int match = CalcBookmarkMatch(pt, rc, erc, t);
		if (match < 0)
			continue;
		if (match < bestMatch || bestMatch == -1)
		{
			bestMatch = match;
			best = bmk;
		}
	}
	return best;
}

void LVPageWordSelector::UpdateSelection()
{
	LVArray<ldomWord> list;
	if (words_.getSelWord())
	{
		list.add(words_.getSelWord()->getWord());
	}
	if (list.length())
	{
		doc_view_->selectWords(list);
	}
	else
	{
		doc_view_->ClearSelection();
	}
}

LVPageWordSelector::~LVPageWordSelector()
{
	doc_view_->ClearSelection();
}

LVPageWordSelector::LVPageWordSelector(LVDocView *doc_view) : doc_view_(doc_view)
{
	LVRef<ldomXRange> range = doc_view_->GetPageDocRange();
	if (!range.isNull())
	{
		words_.addRangeWords(*range, true);
		if (doc_view_->IsPagesMode() && doc_view_->GetColumns() > 1)
		{
			// process second page
			int page_index = doc_view_->GetCurrPage();
			range = doc_view_->GetPageDocRange(page_index + 1);
			if (!range.isNull())
				words_.addRangeWords(*range, true);
		}
		words_.selectMiddleWord();
		UpdateSelection();
	}
}

void LVPageWordSelector::MoveBy(MoveDirection dir, int distance)
{
	words_.selectNextWord(dir, distance);
	UpdateSelection();
}

void LVPageWordSelector::SelectWord(int x, int y)
{
	ldomWordEx *word = words_.findNearestWord(x, y, DIR_ANY);
	words_.selectWord(word, DIR_ANY);
	UpdateSelection();
}

// append chars to search pattern
ldomWordEx *LVPageWordSelector::AppendPattern(lString16 chars)
{
	ldomWordEx *res = words_.appendPattern(chars);
	if (res)
	{
		UpdateSelection();
	}
	return res;
}

// remove last item from pattern
ldomWordEx *LVPageWordSelector::ReducePattern()
{
	ldomWordEx *res = words_.reducePattern();
	if (res)
	{
		UpdateSelection();
	}
	return res;
}

void CreBridge::processMetadata(CmdRequest &request, CmdResponse &response)
{
	response.cmd = CMD_RES_CRE_METADATA;
	CmdDataIterator iter(request.first);
	uint32_t doc_format = 0;
	uint8_t *absolute_path_arg;
	uint32_t packed_size = 0;
	uint32_t smart_archive_arg = 0;
	iter.getInt(&doc_format)
			.getByteArray(&absolute_path_arg)
			.getInt(&packed_size)
			.getInt(&smart_archive_arg);
	if (!iter.isValid())
	{
		CRLog::error("processMetadata: iterator invalid data");
		response.result = RES_BAD_REQ_DATA;
		return;
	}
	const char *absolute_path = reinterpret_cast<const char *>(absolute_path_arg);
	bool smart_archive = (bool) smart_archive_arg;
	LVStreamRef stream = ThResolveStream(doc_format, absolute_path, packed_size, smart_archive);
	if (!stream)
	{
		if (packed_size > 0)
		{
			response.result = RES_ARCHIVE_COLLISION;
		}
		else
		{
			response.result = RES_INTERNAL_ERROR;
		}
		return;
	}
	LVStreamRef thumb_stream;
	lString16 title;
	lString16 authors;
	lString16 series;
	int series_number = 0;
	lString16 lang;
	if (doc_format == DOC_FORMAT_EPUB)
	{
		LVContainerRef container = LVOpenArchive(stream);
		if (container.isNull())
		{
			CRLog::error("processMetadata: EPUB is not in ZIP");
			response.result = RES_INTERNAL_ERROR;
			return;
		}
		// Check root media type
		lString16 root_file_path = EpubGetRootFilePath(container);
		if (root_file_path.empty())
		{
			CRLog::error("processMetadata: malformed EPUB");
			response.result = RES_INTERNAL_ERROR;
			return;
		}
		EncryptedDataContainer *decryptor = new EncryptedDataContainer(container);
		if (decryptor->open())
		{
			CRLog::debug("processMetadata: EPUB encrypted items detected");
		}
		container = LVContainerRef(decryptor);
		lString16 code_base = LVExtractPath(root_file_path, false);
		LVStreamRef content_stream = container->OpenStream(root_file_path.c_str(), LVOM_READ);
		if (content_stream.isNull())
		{
			CRLog::error("processMetadata: malformed EPUB");
			response.result = RES_INTERNAL_ERROR;
			return;
		}
		CrDom *dom = LVParseXMLStream(content_stream);
		if (!dom)
		{
			CRLog::error("processMetadata: malformed EPUB");
			response.result = RES_INTERNAL_ERROR;
			return;
		}
		lString16 thumb_id;
		for (int i = 1; i < 20; i++)
		{
			lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
			ldomNode *item = dom->nodeFromXPath(xpath);
			if (!item)
			{
				break;
			}
			lString16 name = item->getAttributeValue("name");
			lString16 content = item->getAttributeValue("content");
			if (name == "cover")
			{
				thumb_id = content;
			}
		}
		for (int i = 1; i < 50000; i++)
		{
			lString16 xpath = lString16("package/manifest/item[") << fmt::decimal(i) << "]";
			ldomNode *item = dom->nodeFromXPath(xpath);
			if (!item)
			{
				break;
			}
			lString16 href = item->getAttributeValue("href");
			lString16 id = item->getAttributeValue("id");
			if (!href.empty() && !id.empty())
			{
				if (id == thumb_id)
				{
					lString16 thumbnail_file_name = code_base + href;
					thumb_stream = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
				}
				href = DecodeHTMLUrlString(href);
                if(id.endsWith("cover.jpg") || id.endsWith("cover.jpeg")
                   || href.endsWith("cover.jpg") || href.endsWith("cover.jpeg"))
				{
					CRLog::trace("EPUB structure is malformed...BUT Found coverpage image! Yay!");
                    lString16 thumbnail_file_name = code_base + href;
					CRLog::trace("_______EPUB coverpage file: %s", LCSTR(thumbnail_file_name));
                    thumb_stream = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
                }
			}
		}
		authors = dom->textFromXPath(lString16("package/metadata/creator")).trim();
		title = dom->textFromXPath(lString16("package/metadata/title")).trim();
		lang = dom->textFromXPath(lString16("package/metadata/language")).trim();
		for (int i = 1; i < 20; i++)
		{
			lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
			ldomNode *item = dom->nodeFromXPath(xpath);
			if (!item)
			{
				break;
			}
			lString16 name = item->getAttributeValue("name");
			lString16 content = item->getAttributeValue("content");
			if (name == "calibre:series")
			{
				series = content.trim();
			}
			else if (name == "calibre:series_index")
			{
				series_number = content.trim().atoi();
			}
		}
		delete dom;
	}
	else if (doc_format == DOC_FORMAT_FB2)
	{
        thumb_stream = GetFB2Coverpage(stream);
		CrDom dom;
		LvDomWriter writer(&dom, true);
		dom.setNodeTypes(fb2_elem_table);
		dom.setAttributeTypes(fb2_attr_table);
		dom.setNameSpaceTypes(fb2_ns_table);
		LvXmlParser parser(stream, &writer);
        if (parser.CheckFormat() && parser.Parse())
		{
			authors = ExtractDocAuthors(&dom, lString16("|"));
			title = ExtractDocTitle(&dom);
			lang = ExtractDocLanguage(&dom);
			series = ExtractDocSeries(&dom, &series_number);
            //coverimage = ExtractDocThumbImageName(&dom);
            //CRLog::error("coverimage extracted == %s", LCSTR(coverimage));
		}
		else
		{
			CRLog::error("processMetadata: !parser.CheckFormat() || !parser.Parse()");
			response.result = RES_INTERNAL_ERROR;
			return;
		}
#ifdef TRDEBUG
#if 0
		LVStreamRef out = LVOpenFileStream("/data/data/org.readera/files/metatemp.xml", LVOM_WRITE);
        dom.saveToStream(out, NULL, true);
#endif
#endif
	}
	else if (doc_format == DOC_FORMAT_MOBI)
    {
        //thumb_stream = GetMOBICover(stream); //old implementation
        thumb_stream = GetMobiCoverPageToStream(absolute_path);
        mobiresponse mobiresponse = GetMobiMetaFromFile(absolute_path);
        title = mobiresponse.title;
        authors = mobiresponse.author;
        lang = mobiresponse.language;
        if (mobiresponse.series!= L"NULL")
        {
            series.append(mobiresponse.series);
            series_number = 0;
        }
    }
	CmdData *doc_thumb = new CmdData();
	int thumb_width = 0;
	int thumb_height = 0;
	doc_thumb->type = TYPE_ARRAY_POINTER;
	if (!thumb_stream.isNull())
	{
		LVImageSourceRef thumb_image = LVCreateStreamCopyImageSource(thumb_stream);
		if (!thumb_image.isNull() && thumb_image->GetWidth() > 0 && thumb_image->GetHeight() > 0)
		{
			if (thumb_image->GetWidth() * thumb_image->GetHeight() * 4 <= 20e6) { // 20 mb
				thumb_width = thumb_image->GetWidth();
				thumb_height = thumb_image->GetHeight();
				unsigned char *pixels = doc_thumb->newByteArray(thumb_width * thumb_height * 4);
				LVColorDrawBuf *buf = new LVColorDrawBuf(thumb_width, thumb_height, pixels, 32);
				buf->Draw(thumb_image, 0, 0, thumb_width, thumb_height, false);
				convertBitmap(buf);
				delete buf;
				thumb_image.Clear();
			} else {
				CRLog::warn("Ignoring large doc thumb");
			}
		}
	}
	if(title.length() > META_MAX_LENGTH)
	{
		title = title.substr(0, META_MAX_LENGTH);
	}
	if(authors.length() > META_MAX_LENGTH)
	{
		authors = authors.substr(0, META_MAX_LENGTH);
	}
	if(series.length() > META_MAX_LENGTH)
	{
		series = series.substr(0, META_MAX_LENGTH);
	}
	if(lang.length() > META_MAX_LENGTH)
	{
		lang = lang.substr(0, META_MAX_LENGTH);
	}
	response.addData(doc_thumb);
	response.addInt((uint32_t) thumb_width);
	response.addInt((uint32_t) thumb_height);
	responseAddString(response, title);
	responseAddString(response, authors);
	responseAddString(response, series);
	response.addInt((uint32_t) series_number);
	responseAddString(response, lang);
}

bool LVDocView::CheckImage()
{
    ldomXPointer a = cr_dom_->createXPointer(L"/FictionBook/description/coverpage/image");
    ldomXPointer b = cr_dom_->createXPointer(L"/FictionBook/body/image");
    ldomXPointer c = cr_dom_->createXPointer(L"/FictionBook/body/section/image");
    ldomXPointer d = cr_dom_->createXPointer(L"/FictionBook/body/section/img");
    if (a.isNull() && b.isNull() && c.isNull() && d.isNull())
    {
        return false;
    }
    return true; //returns true if image is found
}

bool LVDocView::NeedCheckImage()
{
    if (!cfg_firstpage_thumb_)
    {
        return false;
    }
    if (doc_format_ == DOC_FORMAT_RTF)
    {
        return true;
    }
    if (doc_format_ == DOC_FORMAT_FB2)
    {
        return true;
    }
    return false;
}

unsigned long int getkey(lvRect rect)
{
    lString16 key;
	unsigned long int a;
    a = abs(((rect.topLeft().x*rect.bottomRight().x)-(rect.topLeft().y*rect.bottomRight().y)) % 1000000);
    return a;
}

float LVDocView::CalcRightSide(TextRect textrect)
{
    float result;
    lvRect rect = textrect.getRect();
    DocToWindowRect(rect);
    lString16 word = textrect.getText();
    ldomNode *node = textrect.getNode();

    int page_width_int = this->GetWidth();
    float halfwidth = page_width_int / 2;
    lvRect page_margins = this->cfg_margins_;
    //lvRect nodemargins = node->getFullMargins();
    //CRLog::error("cfg  margins [%d:%d][%d:%d]",page_margins.left,page_margins.right,page_margins.top,page_margins.bottom);
    //CRLog::error("node margins [%d:%d][%d:%d]",nodemargins.left,nodemargins.right,nodemargins.top,nodemargins.bottom);
    bool two_columns = this->GetColumns() > 1;
    bool right_side = two_columns ? rect.left > halfwidth : false;
    int right_line;
    bool nomargins = (page_margins.right < 20) ? true : false;  //todo take nomargins from docview, set nomargins in docview from bridge

    LVFont *font = this->base_font_.get();
    LVFont::glyph_info_t glyph;
    int hyphwidth = font->getCharWidth(UNICODE_SOFT_HYPHEN_CODE);
    int curwidth = font->getCharWidth(word.firstChar());
    css_style_rec_t *style = node->getParentNode()->getStyle().get();
    css_text_align_t align = style->text_align;

    //CRLog::error("letter = %s",LCSTR(word));
    if (two_columns)
    {
        right_line = right_side ? page_width_int : halfwidth;
    }
    else
    {
        right_line = page_width_int;
    }
    //return right_line - right_line - ( hyphwidth / 2 );
    int leftshift = right_side ? rect.left - halfwidth : rect.left;
    lString16 mainname = node->getMainParentName();

    // variants of right lines
    if (mainname == "epigraph") {
        result = right_line * 0.9;
    }
    if (mainname == "td")
    {
        result = rect.right + hyphwidth;
    }
    else if (align == css_ta_right)
    {
        result = right_line - page_margins.right + (hyphwidth / 2);
    }
    else if (align == css_ta_center || align == css_ta_left)
    {
        result = rect.right + curwidth + (hyphwidth / 2);
    }

    else if ((mainname == "poem" || mainname == "stanza" || mainname == "blockquote") )
    {
        result = right_line - leftshift + gTextLeftShift + (hyphwidth / 2);
    }
    else if(nomargins)
    {
        result = right_line - ( hyphwidth / 2 );
    }
    //else if( mainname == "annotation" )
    //{
	//    result = right_line - ( hyphwidth * 2 );
    //}
    else if (mainname == "li")
    {
        result = right_line - hyphwidth;
    }
	else if (mainname == "ul")
	{
        result = right_line - leftshift + (hyphwidth * 2);
	}
	else //css_ta_justify AND margins ON
	{
        result = right_line - leftshift + hyphwidth;
	}

    //some fixes:
	if (result < rect.right) // Plan B
	{
        if (nomargins)
        {
            result = right_line - (hyphwidth / 2);
        }
        else
        {
            result = right_line - (hyphwidth * 2);
        }
	}
	if (result > right_line) // plan C
	{
		result = right_line - (hyphwidth / 2);
	}
	return result;
}

LVArray<lvRect> LVDocView::GetPageParaEnds()
{
	LVArray<lvRect> result;
	LVArray<lvRect> raw_para_array = this->GetCurrentPageParas();

	int offset = this->GetOffset();
	int page_height_int = this->GetHeight();
	bool two_columns = this->GetColumns() > 1;
	unsigned long long int key;
	lvRect margins = this->cfg_margins_;

	typedef std::map< unsigned long long int, int> Rectmap;
	Rectmap m;

	for (int i = 0; i < raw_para_array.length(); i++)
	{
		lvRect rawrect = raw_para_array.get(i);
		//CRLog::error("raw_pararect = [%d:%d][%d:%d]",rawrect.left,rawrect.right,rawrect.top,rawrect.bottom);
		if (rawrect == lvRect(0, 0, 0, 0))
		{
			continue;
		}
		if (rawrect.top < offset)
		{
			continue;
		}
		int bottom = (two_columns) ? offset + page_height_int + page_height_int - margins.bottom : offset + page_height_int - margins.bottom;
		if (rawrect.bottom > bottom)
		{
			continue;
		}
		if( rawrect.bottom - rawrect.top <= CHAR_HEIGHT_MIN)
		{
			continue;
		}
		key = getkey(rawrect);

		if (m.find(key) != m.end())
		{
			continue;
		}
		m[key]= i;

		if( !this->DocToWindowRect(rawrect,false))
		{
			continue;
		}
		//if there's double paraends ion one line - write reverse cycle map filtering
		//CRLog::error("pararect: [%d:%d] [%d:%d]", rawrect.left, rawrect.right, rawrect.top-offset, rawrect.bottom-offset);
		result.add(rawrect);
	}
	raw_para_array.clear();

	return result;
}

LVArray<ImgRect> LVDocView::GetPageImages(image_display_t type)  //display type :0 == all; 1 == css_d_block; 2 == inline;
{
	LVArray<ImgRect> result ;
	lvRect margins = this->cfg_margins_;

    int width = (viewport_mode_ == MODE_PAGES && GetColumns() > 1)?(this->GetWidth()/2):this->GetWidth();
    int maxheight = this->GetHeight() - (margins.bottom + margins.top);
    int	maxwidth  = width - (margins.left + margins.right);

    LVArray<ImgRect> raw_image_array= this->GetCurrentPageImages();

	typedef std::map< unsigned long long int, int> Rectmap;
	Rectmap m;
	unsigned long long int key;

	for (int i = 0; i < raw_image_array.length(); i++)
	{
		lvRect rawrect = raw_image_array.get(i).getRect();
		css_display_t curtype =  raw_image_array.get(i).getNode()->getStyle().get()->display;
        //display type :0 == all; 1 == css_d_block; 2 == inline;

		if(curtype == css_d_none)
        {
            continue;
        }
		if(type == img_block && curtype != css_d_block)
		{
			continue;
		}
		if(type == img_inline && curtype != css_d_inline && curtype != css_d_inline_table)
		{
			continue;
		}
		if (rawrect == lvRect(0,0,0,0))
        {
            continue;
        }
		key = getkey(rawrect);

		if (m.find(key) != m.end())
		{
			continue;
		}
		m[key]= i;
		result.add(raw_image_array.get(i));
	}
	raw_image_array.clear();

	return result;
}

LVArray<Hitbox> LVDocView::GetPageHitboxes(ldomXRange* in_range, bool rtl_enable)
{
	//CRLog::trace("GETPAGEHITBOXES start");
    LVArray<Hitbox> result;
    float page_width    = this->GetWidth();
    float page_height   = this->GetHeight();

    LVFont * font = this->base_font_.get();
	int footnotesheightr = 0;
	//checking whether this is the last page in document, to prevent sigsegv crashes
    if(this->page_ < this->pages_list_.length()-1)
    {
	    int footnoteslength = pages_list_[this->page_ + 1]->footnotes.length();
	    for (int fn = 0; fn < footnoteslength; fn++)
        {
            footnotesheightr = footnotesheightr + pages_list_[this->page_+1]->footnotes[fn].height;
        }
    }

#if DEBUG_DRAW_IMAGE_HITBOXES
    LVArray<ImgRect> images_array = this->GetPageImages();
#else
	LVArray<ImgRect> images_array = this->GetPageImages(img_inline);
#endif //DEBUG_DRAW_IMAGE_HITBOXES
	lvRect margins = this->cfg_margins_;
	LVArray<lvRect> para_array;
	ldomXRange pagerange;
	if(in_range->isNull())
    {
    	//allow paraends when full range
	    para_array = this->GetPageParaEnds();

	    LVRef<ldomXRange> range = this->GetPageDocRange();
	    pagerange= *range;
    }
	else
	{
		pagerange = *in_range;
	}

    int strheight_last = 0;
    int para_counter = 0;
    int img_counter = 0;
    int img_top = 0;
    int img_left = 0;
    int img_right = 0;


    LVArray<TextRect> word_chars;
    //CRLog::trace("RANGECHARS start");
    int width = (this->page_columns_==1)?this->GetWidth() : this->GetWidth()/2;
    int clip_width =  width - margins.left - margins.right;
    pagerange.getRangeChars(word_chars,clip_width,rtl_enable);

    //CRLog::trace("RANGECHARS end");
    for (int i = 0; i < word_chars.length(); ++i)
    {
	    ldomWord word = word_chars.get(i).getWord();
        lString16 text = word_chars.get(i).getText();
        //CRLog::error("letter = %s",LCSTR(text));
        lvRect rect = word_chars.get(i).getRect();
        ldomNode* node = word_chars.get(i).getNode();
        int strheight_curr = rect.bottom - rect.top;

        //paragraph breaks implementation
        if(!this->DocToWindowRect(rect,false))
        {
            continue;
        }

	    int para_repeat_counter = 0;
	    int last_top = 0 ;
	    while (para_counter < para_array.length() && rect.top > para_array.get(para_counter).top)
	    {
		    lvRect para_rect = para_array.get(para_counter);

		    this->DocToWindowRect(para_rect);
		    if(para_repeat_counter < PARAEND_REPEAT_MAX && para_rect.top != last_top)
		    {
				#if DEBUG_CRE_PARA_END_BLOCKS
			    float l = (para_rect.left + (strheight_curr / 2)) / page_width;
                float r = (para_rect.right + (strheight_curr * 2)) / page_width;
                float t = (para_rect.top + 10) / page_height;
                float b = (para_rect.bottom - 10) / page_height;
                #else
                float l = para_rect.right / page_width;
                float r = (para_rect.right + (strheight_curr / 4)) / page_width;
			    if (i >= 1)
			    {
				    ldomNode *last_node = word_chars.get(i - 1).getNode();
				    if (last_node->isRTL() || node->isRTL())
				    {
					    int startx = para_rect.left;
					    int leftspace = startx - margins.left;
					    startx = (margins.left + clip_width) - leftspace - (strheight_curr/4);

					    r = startx / page_width;
					    l = (startx + (strheight_curr / 4)) / page_width;
				    }
			    }
			    float t = para_rect.top / page_height;
			    float b = para_rect.bottom / page_height;
			    #endif // DEBUG_PARA_END_BLOCKS

			    #ifdef TRDEBUG
			    //CRLog::error("[%s%s%s%s%s]",
			    //        LCSTR(word_chars.get(i).getText()),
			    //        LCSTR(word_chars.get(i+1).getText()),
			    //        LCSTR(word_chars.get(i+2).getText()),
			    //        LCSTR(word_chars.get(i+3).getText()),
			    //        LCSTR(word_chars.get(i+4).getText()));
				#endif //TRDEBUG
			    lString16 para_end = lString16("\n");// + lString16::itoa(para_counter);
			    Hitbox *hitbox = new Hitbox(l, r, t, b, para_end);
			    result.add(*hitbox);
			    para_repeat_counter++;
			    last_top = para_rect.top;
		    }
		para_counter++;
	    }

        while (img_counter < images_array.length())
        {
            lvRect img_rect = images_array.get(img_counter).getRect();

            if (rect.top < img_rect.top)
            {
                break;
            }

            img_counter++;
            if (rect.top == img_rect.top)
            {
                //CRLog::error("letter       = %s",LCSTR(text));
                //CRLog::error("rect.bottom     = %d",rect.bottom);
                //CRLog::error("img_rect.bottom = %d",img_rect.bottom);
                if (rect.bottom-strheight_curr < img_rect.bottom)
                {
                	lvRect img_rect2 = img_rect;
                	DocToWindowRect(img_rect2);
                    img_top = img_rect2.top;
                    img_left = img_rect2.left;
                    img_right = img_rect2.right;
                    //CRLog::error("img_top      = %d",img_top);
                }
                break;
            }
        }

	    this->DocToWindowRect(rect);

        //usual line break implementation
        if (strheight_last != 0 && strheight_curr >= strheight_last + (strheight_last/2))
        {
            float pre_r = this->CalcRightSide(word_chars.get(i));

	        float l = rect.right / page_width;
	        float r = pre_r / page_width;
	        float t = rect.top / page_height;
	        float b = (rect.top + strheight_last) / page_height;

	        //l = rect.left    / page_width;
	        //r = rect.right   / page_width;
	        //t = rect.top     / page_height;
	        //b = rect.bottom  / page_height;

	        //text=text+lString16("+00");
            // на случай второго переноса в stanza
	        int rightzone = (this->GetColumns()==1)? (GetWidth()*3/4) : (GetWidth()*7/8);
            if(rect.left > rightzone)
            {
                l = rect.left / page_width;
                //text=text+lString16("+001");
            }

            //инлайновые изображения
            if (rect.top == img_top && word_chars.length() > i + 1)
            {
                int charwidth = font->getCharWidth(text.firstChar());
                int height = node->getParentNode()->getStyle().get()->font_size.value;
                height = height + (height / 4);
                l = rect.left / page_width;
                r = rect.right / page_width;
                t = (rect.bottom - height) / page_height;
                b = rect.bottom / page_height;

                lvRect lastrect = word_chars.get(i - 1).getRect();
                DocToWindowRect(lastrect);
                lvRect nextrect = word_chars.get(i + 1).getRect();
                DocToWindowRect(nextrect);
	            //text=text+lString16("+0");
                if (nextrect.top > img_top) //последний символ в строке с инлайновым изображением
                {
                    //text=text+lString16("+1");
                    l = (rect.left)    / page_width;
                    r = (rect.right)   / page_width;
                    t = (lastrect.bottom - height) / page_height;
                    b = (lastrect.bottom) / page_height;

                    if(rect.left <= img_left)
                    {
	                    //   text=text+lString16("+2");
                        l = (rect.right) / page_width;
                        r = pre_r / page_width;
                    }

                    if(rect.right <= img_right)
                    {
	                    //   text=text+lString16("+3");
                        l = rect.right / page_width;
                        r = rect.left / page_width;
                    }

                    if (lastrect.top != img_top ) //первый И последний символ в строке с инлайновым изображением
                    {
	                    //    text=text+lString16("+4");
                        l = (rect.left)    / page_width;
                        r = (rect.right)   / page_width;
                        b = (nextrect.top) / page_height;
                        t = (nextrect.top - height) / page_height;
                    }
                    img_top = 0;
                }
            }

	        if (rect.top < margins.top)
	        {
		        t = (rect.top - strheight_last) / page_height;
		        b = (rect.top + 10) / page_height;
	        }

	        //CRLog::error("linebreak letter = %s",LCSTR(text));
	        Hitbox *hitbox = new Hitbox(l, r, t, b, text, word);
	        result.add(*hitbox);
        }
        else
        { //usual single-line words
            if (strheight_curr > CHAR_HEIGHT_MIN)
            {
	            strheight_last = strheight_curr;
            }
            else
            {
                #ifdef TRDEBUG
                //CRLog::error("invalid character [%s] %x : too short height!",LCSTR(text),text.firstChar());
                #endif
                continue;
            }
            // filling between-nodes empty space (right direction)
            if(text == " " && word_chars.length() > i + 1)
            {
                lvRect nextrect = word_chars.get(i + 1).getRect();
                DocToWindowRect(nextrect);
                if (nextrect.height() < strheight_curr * 1.5 && rect.right < nextrect.left && rect.top == nextrect.top)
                {
                    rect.right = nextrect.left;
                }
            }
            // filling between-nodes empty space (left direction)
            if(text == " " && i > 0)
            {
                lvRect lastrect = word_chars.get(i - 1 ).getRect();
                DocToWindowRect(lastrect);
                if (rect.left > lastrect.right && lastrect.height() < strheight_curr * 1.5 && rect.top == lastrect.top)
                {
                    rect.left = lastrect.right;
                }
            }
            float l = rect.left / page_width;
            float t = rect.top / page_height;
            float r = rect.right / page_width;
            float b = rect.bottom / page_height;
	        //text=text+lString16("+01");
            if (rect.top == img_top && word_chars.length() > i + 1)
            {
	            //	text=text+lString16("+11");
                int charheight = node->getParentNode()->getStyle().get()->font_size.value;
                charheight = charheight + (charheight / 4);
                l = rect.left / page_width;
                r = rect.right / page_width;
                t = (rect.bottom - charheight) / page_height;
                b = rect.bottom / page_height;

                lvRect nextrect = word_chars.get(i + 1).getRect();
                DocToWindowRect(nextrect);
                if (nextrect.top > img_top) //последний символ в строке с инлайновым изображением
                {
                    //text = text + lString16("+12");
                    int charwidth = font->getCharWidth(text.firstChar());
                    l = (rect.right)                / page_width;
                    r = (rect.right + charwidth)    / page_width;
                    t = (nextrect.top - charheight) / page_height;
                    b = (nextrect.top)              / page_height;
                }
            }

            //CRLog::error("usual letter = %s rect [%f:%f]:[%f:%f]", LCSTR(text),l*page_width,t*page_height,r*page_width,b*page_height);
            //CRLog::error("usual letter = %s rect %d;%f:%f:%f:%f", LCSTR(text),page_,l,r,t,b);

            Hitbox *hitbox = new Hitbox(l, r, t, b, text, word);
            result.add(*hitbox);
        }
    }
    //adding last para end on page if needed
    if (para_counter < para_array.length())
    {
        lvRect rect = para_array.get(para_counter);

        if (this->DocToWindowRect(rect))
        {
            if( rect.bottom - rect.top > CHAR_HEIGHT_MIN)
            {
				#if DEBUG_CRE_PARA_END_BLOCKS
                float l = (rect.right + (strheight_last / 2)) / page_width;
                float r = (rect.right + (strheight_last)) / page_width;
                float t = (rect.top + 10) / page_height;
                float b = (rect.bottom - 10) / page_height;
				#else
                float l = rect.right / page_width;
                float r = (rect.right + (strheight_last / 4)) / page_width;
	            if (word_chars.length()>=1)
	            {
                    int counter = word_chars.length() - 1;
                    ldomNode *last_node = word_chars.get(counter).getNode();
                    while (last_node == NULL && counter >= 0)
                    {
                        last_node = word_chars.get(counter).getNode();
                        counter--;
                    }
                    if (last_node->isRTL())
		            {
			            int startx = rect.right;
			            int leftspace = startx - margins.left;
			            startx = (margins.left + clip_width) - leftspace - (strheight_last/4);

			            r = startx / page_width;
			            l = (startx + (strheight_last / 4)) / page_width;
		            }
	            }
                float t = rect.top / page_height;
                float b = rect.bottom / page_height;
				#endif // DEBUG_PARA_END_BLOCKS
                lString16 para_end = lString16("\n");
                Hitbox *hitbox = new Hitbox(l, r, t, b, para_end);
                result.add(*hitbox);
            }
        }
    }
#ifdef TRDEBUG
#if DEBUG_DRAW_IMAGE_HITBOXES
	for (int i = 0; i < images_array.length(); i++)
	{
		lvRect imgrect = images_array.get(i).getRect();
		if (this->DocToWindowRect(imgrect))
		{
			float l = imgrect.left / page_width;
			float r = imgrect.right / page_width;
			float t = (imgrect.top) / page_height;
			float b = (imgrect.bottom) / page_height;
			lString16 imagestring = lString16("IMAGE");
			Hitbox *hitbox = new Hitbox(l, r, t, b, imagestring);
			result.add(*hitbox);
		}
	}
#endif
#endif //TRDEBUG
	//CRLog::trace("GETPAGEHITBOXES end");

	return result;
}

LVArray<Hitbox> LVDocView::GetPageLinks()
{
	LVArray<Hitbox> result;
	float page_width  = GetWidth();
	float page_height = GetHeight();

    LVArray<TextRect> list;
	GetCurrentPageLinks(list);
	if (list.empty()) {
		return result;
	}

    for (int i = 0; i < list.length(); i++)
	{
		TextRect curr = list.get(i);
		lString16 href = curr.getText();
		lvRect rect = curr.getRect();
		if (!DocToWindowRect(rect))
		{
			continue;
		}
		float l = (rect.left + gTextLeftShift) / page_width;
		float t = rect.top / page_height;
		float r = (rect.right + gTextLeftShift) / page_width;
		float b = rect.bottom / page_height;
		Hitbox *hitbox = new Hitbox(l, r, t, b, href);
		result.add(*hitbox);
	}
	return result;
 }
font_ref_t LVDocView::GetBaseFont() { return base_font_; };

lString16 LVDocView::GetHitboxHash(float l,float r,float t,float b)
{
	lString16 result;
	result<<std::to_string(l).c_str();
	result<<":";
	result<<std::to_string(r).c_str();
	result<<":";
	result<<std::to_string(t).c_str();
	result<<":";
	result<<std::to_string(b).c_str();
	return result;
}

ldomWord LVDocView::FindldomWordFromMap(ldomWordMap m, lString16 key)
{
	lUInt32 in_key = key.getHash();
	ldomWord result;
	if (m.find(in_key) != m.end())
	{
		result = m.at(in_key);
	}
	return result;
}

ldomWordMap LVDocView::GetldomWordMapFromPage(int page)
{
	ldomWordMap m;

	this->GoToPage(page);

	LVArray<Hitbox> hitboxes = this->GetPageHitboxes();
	for (int i = 0; i < hitboxes.length(); i++)
	{
		Hitbox curr = hitboxes.get(i);
		lUInt32 key = GetHitboxHash(curr.left_,curr.right_,curr.top_,curr.bottom_).getHash();
		m[key] = curr.word_;
	}
	return m;
}

lString16 LVDocView::GetXpathByRectCoords(lString16 key, ldomWordMap m)
{
	if (key == lString16("-"))
	{
		return lString16("-");
	}
	else
	{
		ldomWord word = this->FindldomWordFromMap(m, key);
		if (word.isNull())
		{
			return lString16("-");
		}
		ldomXPointer xPointer = word.getStartXPointer();
		return xPointer.toString();
		///CRLog::error("xpointer1 for key [%d__%s] = [%s]",page,LCSTR(key_start),LCSTR(xPointer.toString()));
	}
}