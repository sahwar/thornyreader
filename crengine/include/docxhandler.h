//
// Created by Admin on 15/5/2018.
//

#ifndef CODE_THORNYREADER_PURE_DOCXHANDLER_H
#define CODE_THORNYREADER_PURE_DOCXHANDLER_H

#include "lvstring.h"
#include "lvptrvec.h"

class DocxItem
{
public:
    lString16 href;
    lString16 mediaType;
    lString16 id;

    DocxItem() {}

    DocxItem(const DocxItem &v) : href(v.href), mediaType(v.mediaType), id(v.id) {}

    DocxItem &operator=(const DocxItem &v)
    {
        href = v.href;
        mediaType = v.mediaType;
        id = v.id;
        return *this;
    }

    ~DocxItem() {}
};

class DocxItems : public LVPtrVector<DocxItem>
{
public:
    DocxItem *findById(const lString16 &id)
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

    DocxItems &operator=(const DocxItems &v)
    {
        for (int i = 0; i < this->length(); ++i)
        {
            this->set(i,v.get(i));
        }
        return *this;
    }

    lString16 findHrefById(const lString16 &id)
    {
        if (id.empty())
        {
            return lString16::empty_str;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id == id)
            {
                return get(i)->href;
            }
        }
        return lString16::empty_str;
    }
};

class DocxLink
{
public:
    lString16 id_;
    lString16 type_;
    lString16 target_;
    lString16 targetmode_;

    DocxLink() {}
    DocxLink(const DocxLink &v) : id_(v.id_), type_(v.type_), target_(v.target_),targetmode_(v.targetmode_) {}
    DocxLink &operator=(const DocxLink &v)
    {
        id_ = v.id_;
        type_ = v.type_;
        target_ = v.target_;
        targetmode_ = v.targetmode_;
        return *this;
    }

    ~DocxLink() {}
};

class DocxLinks : public LVPtrVector<DocxLink>
{
public:
    DocxLink *findById(const lString16 &id)
    {
        if (id.empty())
        {
            return NULL;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id_ == id)
            {
                return get(i);
            }
        }
        return NULL;
    }

    DocxLinks &operator=(const DocxLinks &v)
    {
        for (int i = 0; i < this->length(); ++i)
        {
            this->set(i,v.get(i));
        }
        return *this;
    }

    lString16 findTargetById(const lString16 &id)
    {
        if (id.empty())
        {
            return lString16::empty_str;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id_ == id)
            {
                return get(i)->target_;
            }
        }
        return lString16::empty_str;
    }
};

#endif //CODE_THORNYREADER_PURE_DOCXHANDLER_H
