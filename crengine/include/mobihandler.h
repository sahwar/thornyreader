//
// Created by Admin on 13/4/2018.
//

#include <cctype>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

#include "lvtinydom.h"
#include "trlog.h"
#include "lvxml.h"

#include "libmobi/src/mobi.h"
#include "libmobi/tools/common.h"
//#include "libmobi/src/meta.h"

#define EPUB_CONTAINER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n\
  <rootfiles>\n\
    <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n\
  </rootfiles>\n\
</container>"
#define EPUB_MIMETYPE "application/epub+zip"


struct mobiresponse
{
    lString16 title;
    lString16 author;
    lString16 series=L"NULL";
    int series_number=0;
    lString16 language;
};

#define FULLNAME_MAX 1024

bool ImportMOBIDocNew(const char *absolute_path,const char *epubnewpath);
bool ConvertMOBIDocToEpub(MOBIRawml* rawml, const char* epubnewpath);
void FreeMOBIStructures(MOBIRawml* rawml, MOBIData* m);
void GetMobiMeta(const MOBIData *m);
mobiresponse GetMobiMetaSummary(const MOBIData *m);
mobiresponse GetMobiMetaFromFile(const char *fullpath);
int GetMobiCoverPageFile(const MOBIRawml *rawml, const char *fullpath);
LVStreamRef GetMobiCoverPageToStream(const char *fullpath);