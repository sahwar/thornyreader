#ifndef EPUBFMT_H
#define EPUBFMT_H

#include "lvtinydom.h"

class EncryptedItem;

class EncryptedItemCallback
{
public:
    virtual void addEncryptedItem(EncryptedItem* item) = 0;
    virtual ~EncryptedItemCallback() {}
};

class EncryptedDataContainer : public LVContainer, public EncryptedItemCallback
{
    LVContainerRef _container;
    LVPtrVector<EncryptedItem> _list;
    LVArray<lUInt8> _fontManglingKey;
public:
    EncryptedDataContainer(LVContainerRef baseContainer);
    virtual LVContainer* GetParentContainer();
    //virtual const LVContainerItemInfo* GetObjectInfo(const wchar_t * pname);
    virtual const LVContainerItemInfo* GetObjectInfo(int index);
    virtual int GetObjectCount() const;
    /// returns object size (file size or directory entry count)
    virtual lverror_t GetSize(lvsize_t* pSize);
    virtual LVStreamRef OpenStream(const lChar16* fname, lvopen_mode_t mode);
    virtual LVStreamRef OpenStreamByPackedSize(uint32_t size);
    /// returns stream/container name, may be NULL if unknown
    virtual const lChar16* GetName();
    /// sets stream/container name, may be not implemented for some objects
    virtual void SetName(const lChar16* name);
    virtual void addEncryptedItem(EncryptedItem* item);
    EncryptedItem* findEncryptedItem(const lChar16* name);
    bool isEncryptedItem(const lChar16* name);
    bool setManglingKey(lString16 key);
    bool hasUnsupportedEncryption();
    bool open();
};

bool DetectEpubFormat(LVStreamRef stream);
bool ImportEpubDocument(LVStreamRef stream, CrDom * doc,  bool firstpage_thumb);
lString16 EpubGetRootFilePath(LVContainerRef m_arc);

bool ImportDocxDocument(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb);
lString16 DocxGetRootFilePath(LVContainerRef m_arc);

#endif // EPUBFMT_H
