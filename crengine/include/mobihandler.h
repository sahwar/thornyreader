//
// Created by Admin on 13/4/2018.
//
#include "lvtinydom.h"
#include "../../libmobi/src/mobi.h"
#include "trlog.h"
#include <cctype>
#include "../../libmobi/tools/common.h"

struct mobiresponse
{
    lString16 title;
    lString16 author;
    lString16 series=L"NULL";
    int series_number=0;
    lString16 language;
};

#define FULLNAME_MAX 1024

bool ImportMOBIDocNew(const char *absolute_path, MOBIRawml* rawmlret ,MOBIData* mobidataret);
bool ConvertMOBIDocToEpub(MOBIRawml* rawml, const char* epubnewpath);
void FreeMOBIStructures(MOBIRawml* rawml, MOBIData* m);
void GetMobiMeta(const MOBIData *m);
mobiresponse GetMobiMetaSummary(const MOBIData *m);
mobiresponse GetMobiMetaFromFile(const char *fullpath);