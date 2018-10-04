#ifndef READERA_CREBRIDGE_H
#define READERA_CREBRIDGE_H

#include "thornyreader/include/StBridge.h"
#include "include/lvdocview.h"
typedef std::map<int, ldomWord> ldomWordMap;

class CreBridge : public StBridge
{
private:
    LVDocView* doc_view_;

public:
    CreBridge();
    ~CreBridge();

    void process(CmdRequest& request, CmdResponse& response);

protected:
    void responseAddString(CmdResponse& response, lString16 str16);
    void convertBitmap(LVColorDrawBuf* bitmap);
    void responseAddLinkUnknown(CmdResponse& response, lString16 href,
                                float l, float t, float r, float b);
    void processConvert(CmdRequest& request, CmdResponse& response);
    void processFonts(CmdRequest& request, CmdResponse& response);
    void processConfig(CmdRequest& request, CmdResponse& response);
    void processOpen(CmdRequest& request, CmdResponse& response);
    void processQuit(CmdRequest& request, CmdResponse& response);
    void processOutline(CmdRequest& request, CmdResponse& response);
    void processPage(CmdRequest& request, CmdResponse& response);
    void processPageLinks(CmdRequest& request, CmdResponse& response);
    void processPageText(CmdRequest& request, CmdResponse& response);
    void processPageRender(CmdRequest& request, CmdResponse& response);
    void processPageByXPath(CmdRequest& request, CmdResponse& response);
    void processPageXPath(CmdRequest& request, CmdResponse& response);
    void processMetadata(CmdRequest& request, CmdResponse& response);
    void processPageXpaths(CmdRequest &request, CmdResponse &response);
    void processPageRangeText(CmdRequest &request, CmdResponse &response);
    void processPageRects(CmdRequest &request, CmdResponse &response);
};

#endif //READERA_CREBRIDGE_H
