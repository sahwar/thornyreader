#ifndef CODE_THORNYREADER_PURE_EPUBITEMS_H
#define CODE_THORNYREADER_PURE_EPUBITEMS_H

#include <crengine/include/lvstring.h>
#include <crengine/include/lvptrvec.h>
typedef std::map<lUInt32,lString16> LinksMap;

class EpubItem
{
public:
    lString16 href;
    lString16 mediaType;
    lString16 id;
    lString16 title;

    EpubItem() {}

    EpubItem(const EpubItem &v) : href(v.href), mediaType(v.mediaType), id(v.id) {}
    EpubItem(lString16 href_new,lString16 mediaType_new,lString16 id_new,lString16 title_new) : href(href_new), mediaType(mediaType_new), id(id_new), title(title_new) {}

    EpubItem &operator=(const EpubItem &v)
    {
        href = v.href;
        mediaType = v.mediaType;
        id = v.id;
        return *this;
    }
};

class EpubItems : public LVPtrVector<EpubItem>
{
public:
    EpubItem *findById(const lString16 &id)
    {
        if (id.empty())
        {
            return NULL;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id == id)
            {
                return get(i);
            }
        }
        return NULL;
    }

    EpubItem *findByHref(const lString16 &href)
    {
        if (href.empty())
        {
            return NULL;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->href == href)
            {
                return get(i);
            }
        }
        return NULL;
    }

    bool hrefCheck(lString16 string)
    {
        for (int i = 0; i < this->length(); i++)
        {
            if (string.pos(this->get(i)->href.c_str()) != -1)
            {
                return true;
            }
        }
        return false;
    }
};

class LinkStruct
{
public:
    int num_;
    lString16 href_ = lString16::empty_str;
    lString16 id_ = lString16::empty_str;
    LinkStruct(){}
    LinkStruct(int num, lString16 id, lString16 href):num_(num), id_(id), href_(href) {}
    ~LinkStruct(){}
};

class Epub3Notes
{
public:
    LinksMap AsidesMap_;
    lString16 FootnotesTitle_;
    void addTitle(lString16 title){FootnotesTitle_ = title; }
    void AddAside(lString16 href) {
        lUInt32 hash = href.getHash();
        AsidesMap_[hash] = "1";
    }
    int size() { return AsidesMap_.size(); }
};

#endif //CODE_THORNYREADER_PURE_EPUBITEMS_H
