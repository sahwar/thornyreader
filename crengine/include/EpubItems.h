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

enum epub_CSS_attr_t_align {ta_inherit = 0, ta_left, ta_right, ta_center, ta_justify};

class EpubCSSClass
{
public:
    lString16 source_line_;
    lString16 name_;
    lString16 margin_top_;
    lString16 margin_bottom_;
    bool rtl_       = false;
    bool bold_      = false;
    bool italic_    = false;
    bool underline_ = false;
    int text_align_ = ta_inherit;
    lString16 style_string_;
    EpubCSSClass() {};

    inline bool empty() { return (name_.empty() || source_line_.empty());}

    lString16 getAttrval(lString16 in, lString16 attrname)
    {
        lString16 result;
        int attr_start = in.pos(attrname);
        if(attr_start == -1)
        {
            return result;
        }

        int attr_end = -1;
        for (int i = attr_start; i < in.length(); i++)
        {
            lChar16 curr = in.at(i);
            if (curr == ';')
            {
                attr_end = i;
                break;
            }
        }

        lString16 attrstr = in.substr(attr_start, attr_end - attr_start);
        //CRLog::error("attrstr = [%s]", LCSTR(attrstr));

        if(attrstr.pos(":")!=-1)
        {
            int colonpos = attrstr.pos(":")+1;
            result = attrstr.substr(colonpos,attrstr.length()-colonpos);
        }
        return result.trimDoubleSpaces(false, false, false);
    }

    lString16 formatCSSstring()
    {
        lString16 text_align;

        //switch (text_align_)
        //{
        //    case ta_left    : text_align = lString16("left");    break;
        //    case ta_right   : text_align = lString16("right");   break;
        //    case ta_center  : text_align = lString16("center");  break;
        //    case ta_justify : text_align = lString16("justify"); break;
        //    case ta_inherit : text_align = lString16::empty_str; break;
        //}
        //if(!text_align.empty())     { CSS_class_string += "text-align: "    + text_align      + "; "; }

        lString16 CSS_class_string;
        if(bold_)                  { CSS_class_string += "font-weight: bold; ";}
        if(italic_)                { CSS_class_string += "font-style: italic; ";}
        if(underline_)             { CSS_class_string += "text-decoration: underline; ";}
        if(!margin_top_.empty())    { CSS_class_string += "margin-top: "    + margin_top_    + "; "; }
        if(!margin_bottom_.empty()) { CSS_class_string += "margin-bottom: " + margin_bottom_ + ";"; }

        if(CSS_class_string.empty())
        {
            //CRLog::error("class [%s] malformed",LCSTR(name_));
            return lString16::empty_str;
        }
        CSS_class_string = "." + name_ + " { " + CSS_class_string + "}";
        //CRLog::error("class = [%s]", LCSTR(CSS_class_string));
        return CSS_class_string;
    }

    EpubCSSClass(lString16 in)
    {
        source_line_ = in;
        lString16 rest;
        for (int i = 0; i < in.length(); i++)
        {
            lChar16 curr = in.at(i);
            if(curr!='{')
            {
                continue;
            }
            else
            {
                name_ = in.substr(0,i).trimDoubleSpaces(false,false,false);
                if(name_.startsWith("."))
                {
                    name_ = name_.substr(1,name_.length()-1);
                }
                rest = in.substr(i,in.length()-i);
                break;
            }
        }
        if(!rest.empty())
        {
            lString16 attrval = getAttrval(rest, lString16("direction"));
            if(attrval.empty())
            {
                attrval = getAttrval(rest, lString16("dir"));
            }
            rtl_ = (attrval==lString16("rtl"));

            attrval = getAttrval(rest,lString16("text-align"));
            if (attrval == lString16("left"))
            {
                text_align_ = ta_left;
            }
            else if (attrval == lString16("right"))
            {
                text_align_ = ta_right;
            }
            else if (attrval == lString16("center"))
            {
                text_align_ = ta_center;
            }
            else if (attrval == lString16("justify"))
            {
                text_align_ = ta_justify;
            }
            else
            {
                text_align_ = ta_inherit;
            }
            attrval     = getAttrval(rest,lString16("font-weight"));
            bold_       = (attrval == "bold");
            attrval     = getAttrval(rest,lString16("font-style"));
            italic_     = (attrval == "italic");
            attrval     = getAttrval(rest,lString16("text-decoration"));
            underline_  = (attrval == "underline");
            margin_top_    = getAttrval(rest,lString16("margin-top"));
            margin_bottom_ = getAttrval(rest,lString16("margin-bottom"));

            style_string_ = formatCSSstring();
        }
    };
};

typedef std::map<lUInt32,EpubCSSClass> EpubCSSMap;

class EpubStylesManager
{
private:
    EpubCSSMap classes_map_;

    bool CheckClassName(lString16 name)
    {
        //CRLog::error("check classname = [%s]",LCSTR(name));
        if(name.empty())
        {
            return false;
        }
        for (int i = 0; i < name.length(); i++)
        {
            lChar16 ch = name.at(i);
            if ((ch >= 45 && ch <= 57)     || //0-9
                (ch >= 65 && ch <= 90)     || //A-Z
                (ch >= 97 && ch <= 122)    || //a-z
                (ch == ' ') || (ch == '.') ||
                (ch == ',') || (ch == '=') ||
                (ch == '_') || (ch == '"') ||
                (ch == '<') || (ch == '>') ||
                (ch == '[') || (ch == ']'))
            {
                continue;
            }
            else
            {
                //CRLog::error("Found illegal character in CSS class name: [%s] -> [%lc]",LCSTR(name),ch);
                return false;
            }
        }

        lChar16 last = '.';
        int q_count = 0;
        bool br_open = false;
        for (int i = 0; i < name.length(); i++)
        {
            lChar16 ch = name.at(i);
            if (last == '.' || last == ' ' || last == ',')
            {
                if ((ch >= 45 && ch <= 57) || ch == '-')
                {
                    //CRLog::error("Illegal character combination in css class name: [%s] -> [%lc][%lc]",LCSTR(name),last,ch);
                    return false;
                }
            }
            if(ch == '[')
            {
                if(br_open)
                {
                    //CRLog::error("brackets error 1");
                    return false;
                }
                br_open = true;
            }
            else if(ch == ']')
            {
                if(!br_open)
                {
                    //CRLog::error("brackets error 2");
                    return false;
                }
                br_open = false;
            }
            else if(ch=='"')
            {
                q_count++;
            }

            last = ch;
        }
        if(q_count %2 != 0 || br_open)
        {
            //CRLog::error("unpaired quotes or br_open");
            return false;
        }

        return true;
    }


    void addCSSClass(EpubCSSClass css)
    {
        if( css.empty() && !css.rtl_ )
        {
            //CRLog::error("EpubCSSclass is empty [%s].",LCSTR(css.name_));
            return;
        }
        if(css.style_string_.empty() && !css.rtl_)
        {
            //CRLog::error("EpubCSSclass style string is empty and class is not rtl [%s].",LCSTR(css.name_));
            return;
        }
        if(classes_map_.find(css.name_.getHash())!=classes_map_.end())
        {
            //CRLog::error("EpubCSSclass already exists [%s].",LCSTR(css.name_));
            return;
        }
        if(!CheckClassName(css.name_))
        {
            //CRLog::error("EpubCSSclass is invalid [%s].",LCSTR(css.name_));
            return;
        }
        //CRLog::trace("EpubCSSclass added [%s] ",LCSTR(css.name_));
        classes_map_.insert(std::make_pair(css.name_.getHash(),css));
    }

    lString16Collection SplitToClasses(lString16 in)
    {
        lString16Collection result;
        int classstart = -1;
        int classend = -1;
        bool comment_skip = false;
        for (int i = 0; i < in.length(); i++)
        {
            lChar16 curr = in.at(i);

            if(comment_skip && !(curr =='*' && in.at(i+1) == '/'))
            {
                continue;
            }
            if(curr == '*' && in.at(i+1) == '/')
            {
                comment_skip = false;
                continue;
            }
            if(curr == '/' && in.at(i+1) == '*')
            {
                comment_skip = true;
                continue;
            }

            if (classstart == -1 && curr== '.')
            {
                classstart = i;
            }
            if(classstart!=-1)
            {
                if(curr!='}')
                {
                    continue;
                }
                else
                {
                    classend = i+1;
                    lString16 CSSclass = in.substr(classstart,classend-classstart);
                    result.add(CSSclass);
                    classstart=-1;
                    classend=-1;
                    continue;
                }
            }
            if (curr != L'.')
            {
                continue;
            }
        }
        for (int i = 0; i < result.length(); i++)
        {
            if(result.at(i).pos("{")==-1 || result.at(i).pos("}")==-1)
            {
                result.erase(i,1);
            }
        }
        return result;
    }

public:
    EpubStylesManager() {};

    inline bool empty() { return classes_map_.empty();}
    LVArray<lString16> char_CSS_classes_;

    void parseString(lString16 in)
    {
        {
            in = in.trimDoubleSpaces(false,false,false);
            lString16Collection classes_str_coll = SplitToClasses(in);
            for (int i = 0; i < classes_str_coll.length(); i++)
            {
                EpubCSSClass cssClass = EpubCSSClass(classes_str_coll.at(i));
                this->addCSSClass(cssClass);
            }
        }
    }

    void ConvertClassesMap()
    {
        std::map<lUInt32 , EpubCSSClass>::iterator it = classes_map_.begin();
        while (it != classes_map_.end())
        {
            //lUInt32 hash = it->first;
            EpubCSSClass curr = it->second;
            char_CSS_classes_.add(curr.style_string_);
            it++;
        }
    }

    EpubCSSClass getCSSClass(lString16 name) //array scan
    {
        if(classes_map_.empty())
        {
            return EpubCSSClass();
        }
        if(classes_map_.find(name.getHash())!=classes_map_.end())
        {
            return classes_map_.at(name.getHash());
        }
        return EpubCSSClass();
    }
};
#endif //CODE_THORNYREADER_PURE_EPUBITEMS_H
