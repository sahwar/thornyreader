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

class DocxStyle
{
public:
    lString16 type_;
    lString16 styleId_;
    int fontSize_ = -1;
    bool isDefault_ = false;
    lString16 name_;

    DocxStyle() {}

    DocxStyle(const DocxStyle &v) : type_(v.type_), fontSize_(v.fontSize_), styleId_(v.styleId_),isDefault_(v.isDefault_),name_(v.name_) {}
    DocxStyle(lString16 type,int fontSize,lString16 styleId,bool isDefault,lString16 name) : type_(type)   , fontSize_(fontSize)   , styleId_(styleId)   ,isDefault_(isDefault),name_(name) {}

    DocxStyle &operator=(const DocxStyle &v)
    {
        type_ = v.type_;
        styleId_ = v.styleId_;
        fontSize_ = v.fontSize_;
        isDefault_ = v.isDefault_;
        return *this;
    }

    ~DocxStyle() {}
};

class DocxStyles : public LVPtrVector<DocxStyle>
{
private:

    bool updateMiniMax()
    {
        for (int i = 0; i < this->length(); i++)
        {
            DocxStyle* curr = this->get(i);
            if(curr->fontSize_>max_)
                max_ = curr->fontSize_;
            if(curr->fontSize_<min_)
                min_ = curr->fontSize_;
        }
        return !(this->min_ == 500 || this->max_ == -1);
    };

    bool h1isset = false;
    bool h2isset = false;
    bool h3isset = false;
    bool h4isset = false;
    bool h5isset = false;
    bool h6isset = false;

public:
    int min_ = 500;
    int max_ = -1;
    int default_size_ = -1;
    int h6min_ = -1;
    int h5min_ = -1;
    int h4min_ = -1;
    int h3min_ = -1;
    int h2min_ = -1;
    int h1min_ = -1;

    lString16 h1id_;
    lString16 h2id_;
    lString16 h3id_;
    lString16 h4id_;
    lString16 h5id_;
    lString16 h6id_;

    void checkForHeaders()
    {
        for (int i = 0; i < this->length(); i++)
        {
            DocxStyle* curr = this->get(i);
            lString16 curr_name = curr->name_;
            if(curr_name.pos("head")!=-1)
            {
                //CRLog::error("curr name = [%s]",LCSTR(curr_name));
                lString16Collection namefrags;
                namefrags.parse(curr_name,' ', false);
                for (int i = 0; i < namefrags.length(); i++)
                {
                    lString16 namefrag  = namefrags.at(i);
                    if(namefrag.DigitsOnly())
                    {
                        int headnum = namefrag.atoi();
                        switch (headnum)
                        {
                            case 1: h1id_ = (h1isset)? h1id_: curr->styleId_; break;
                            case 2: h2id_ = (h2isset)? h2id_: curr->styleId_; break;
                            case 3: h3id_ = (h3isset)? h3id_: curr->styleId_; break;
                            case 4: h4id_ = (h4isset)? h4id_: curr->styleId_; break;
                            case 5: h5id_ = (h5isset)? h5id_: curr->styleId_; break;
                            case 6: h6id_ = (h6isset)? h6id_: curr->styleId_; break;
                            default:
                                CRLog::error("Error: Unknown header style found! [%s]",LCSTR(curr_name));
                                break;
                        }
                    }
                }
            }
        }
    }

    bool generateHeaderFontSizes()
    {
        checkForHeaders();
        updateMiniMax();
        if (this->default_size_ < 0 )
        {
            return false;
        }
        int range_size = this->max_ - this->default_size_;
        if (range_size < 0)
        {
            return false;
        }
        int step = range_size / 6;
        h6min_ = default_size_;
        h5min_ = default_size_ + step;
        h4min_ = default_size_ + (step * 2);
        h3min_ = default_size_ + (step * 3);
        h2min_ = default_size_ + (step * 4);
        h1min_ = default_size_ + (step * 5);
        return true;
    }

    DocxStyle *findById(const lString16 &id)
    {
        if (id.empty())
        {
            return NULL;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->styleId_ == id)
            {
                return get(i);
            }
        }
        return NULL;
    }

    DocxStyles &operator=(const DocxStyles &v)
    {
        for (int i = 0; i < this->length(); ++i)
        {
            this->set(i,v.get(i));
        }
        return *this;
    }

    int getSizeById(const lString16 &id)
    {
        if (id.empty())
        {
            return -1;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->styleId_ == id)
            {
                return get(i)->fontSize_;
            }
        }
        return -1;
    }
};

#endif //CODE_THORNYREADER_PURE_DOCXHANDLER_H
