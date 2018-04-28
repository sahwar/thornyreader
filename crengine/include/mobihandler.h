//
// Created by Admin on 13/4/2018.
//

#include "lvtinydom.h"
#include "libmobi/src/trmobi.h"

struct mobiresponse
{
    lString16 title;
    lString16 author;
    lString16 series=L"NULL";
    int series_number=0;
    lString16 language;
};

// Main interface
bool ImportMOBIDocNew(const char *absolute_path,const char *epubnewpath);
mobiresponse GetMobiMetaFromFile(const char *fullpath);
LVStreamRef GetMobiCoverPageToStream(const char *fullpath);
bool IsMobiDoc(const char *absolute_path);
// Local helpers
bool ConvertMOBIDocToEpub(MOBIRawml* rawml, const char* epubnewpath);
void FreeMOBIStructures(MOBIRawml* rawml, MOBIData* m);
void GetMobiMeta(const MOBIData *m);
mobiresponse GetMobiMetaSummary(const MOBIData *m);
int GetMobiCoverPageFile(const MOBIRawml *rawml, const char *fullpath);
