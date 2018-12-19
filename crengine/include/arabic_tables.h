//
// Created by Tarasus on 14/12/2018.
//

#ifndef CODE_THORNYREADER_PURE_ARABIC_TABLES_H
#define CODE_THORNYREADER_PURE_ARABIC_TABLES_H

#include "lvtinydom.h"
#include <array>
typedef std::array<lChar16,4> CharArr;
typedef std::map<lChar16 , CharArr> LetterMap;
enum charType {arabic_end = 0, arabic_mid = 1,arabic_start = 2, arabic_isolated = 3};

LetterMap ArabicLetterMap()
{
    LVArray<lChar16> list;
    lString16 temp;
    LetterMap LetterMap;
            //key   //end    //mid   //start //isolated
    CharArr x0627 = {0xFE8E, 0xFE8E, 0xFE8D, 0xFE8D};
    CharArr x0628 = {0xFE90, 0xFE92, 0xFE91, 0xFE8F};
    CharArr x062A = {0xFE96, 0xFE98, 0xFE97, 0xFE95};
    CharArr x062B = {0xFE9A, 0xFE9C, 0xFE9B, 0xFE99};
    CharArr x062C = {0xFE9E, 0xFEA0, 0xFE9F, 0xFE9D};
    CharArr x062D = {0xFEA2, 0xFEA4, 0xFEA3, 0xFEA1};
    CharArr x062E = {0xFEA6, 0xFEA8, 0xFEA7, 0xFEA5};
    CharArr x062F = {0xFEAA, 0xFEAA, 0xFEA9, 0xFEA9};
    CharArr x0630 = {0xFEAC, 0xFEAC, 0xFEAB, 0xFEAB};
    CharArr x0631 = {0xFEAE, 0xFEAE, 0xFEAD, 0xFEAD};
    CharArr x0632 = {0xFEB0, 0xFEB0, 0xFEAF, 0xFEAF};
    CharArr x0633 = {0xFEB2, 0xFEB4, 0xFEB3, 0xFEB1};
    CharArr x0634 = {0xFEB6, 0xFEB8, 0xFEB7, 0xFEB5};
    CharArr x0635 = {0xFEBA, 0xFEBC, 0xFEBB, 0xFEB9};
    CharArr x0636 = {0xFEBE, 0xFEC0, 0xFEBF, 0xFEBD};
    CharArr x0637 = {0xFEC2, 0xFEC4, 0xFEC3, 0xFEC1};
    CharArr x0638 = {0xFEC6, 0xFEC8, 0xFEC7, 0xFEC5};
    CharArr x0639 = {0xFECA, 0xFECC, 0xFECB, 0xFEC9};
    CharArr x063A = {0xFECE, 0xFED0, 0xFECF, 0xFECD};
    CharArr x0641 = {0xFED2, 0xFED4, 0xFED3, 0xFED1};
    CharArr x0642 = {0xFED6, 0xFED8, 0xFED7, 0xFED5};
    CharArr x0643 = {0xFEDA, 0xFEDC, 0xFEDB, 0xFED9};
    CharArr x0644 = {0xFEDE, 0xFEE0, 0xFEDF, 0xFEDD};
    CharArr x0645 = {0xFEE2, 0xFEE4, 0xFEE3, 0xFEE1};
    CharArr x0646 = {0xFEE6, 0xFEE8, 0xFEE7, 0xFEE5};
    CharArr x0647 = {0xFEEA, 0xFEEC, 0xFEEB, 0xFEE9};
    CharArr x0648 = {0xFEEE, 0xFEEE, 0xFEED, 0xFEED};
    CharArr x064A = {0xFEF2, 0xFEF4, 0xFEF3, 0xFEF1};
    CharArr x0622 = {0xFE82, 0xFE82, 0xFE81, 0xFE81};
    CharArr xFEFB = {0xFEFC, 0xFEFC, 0xFEFB, 0xFEFB}; // LAM-ALEF LIGATURE //fefb comes from LigatureCheck() function
    CharArr x0623 = {0xFE84, 0xFE84, 0xFE83, 0xFE83}; // ALEF WITH HAMZA ABOVE
    CharArr x0625 = {0xFE88, 0xFE88, 0xFE87, 0xFE87}; // ALEF WITH HAMZA BELOW
    CharArr x0624 = {0xFE86, 0xFE86, 0xFE85, 0xFE85}; // WAW WITH HAMZA ABOVE
    CharArr x0626 = {0xFE8A, 0xFE8C, 0xFE8B, 0xFE89}; // YEH WITH HAMZA ABOVE
    CharArr x0629 = {0xFE94, 0xFE94, 0xFE93, 0xFE93}; // TEH MARBUTA
    CharArr x0649 = {0xFEF0, 0xFEF0, 0xFEEF, 0xFEEF}; // ALEF MAKSURA

    LetterMap.insert(std::make_pair(0x0627,x0627));
    LetterMap.insert(std::make_pair(0x0628,x0628));
    LetterMap.insert(std::make_pair(0x062A,x062A));
    LetterMap.insert(std::make_pair(0x062B,x062B));
    LetterMap.insert(std::make_pair(0x062C,x062C));
    LetterMap.insert(std::make_pair(0x062D,x062D));
    LetterMap.insert(std::make_pair(0x062E,x062E));
    LetterMap.insert(std::make_pair(0x062F,x062F));
    LetterMap.insert(std::make_pair(0x0630,x0630));
    LetterMap.insert(std::make_pair(0x0631,x0631));
    LetterMap.insert(std::make_pair(0x0632,x0632));
    LetterMap.insert(std::make_pair(0x0633,x0633));
    LetterMap.insert(std::make_pair(0x0634,x0634));
    LetterMap.insert(std::make_pair(0x0635,x0635));
    LetterMap.insert(std::make_pair(0x0636,x0636));
    LetterMap.insert(std::make_pair(0x0637,x0637));
    LetterMap.insert(std::make_pair(0x0638,x0638));
    LetterMap.insert(std::make_pair(0x0639,x0639));
    LetterMap.insert(std::make_pair(0x063A,x063A));
    LetterMap.insert(std::make_pair(0x0641,x0641));
    LetterMap.insert(std::make_pair(0x0642,x0642));
    LetterMap.insert(std::make_pair(0x0643,x0643));
    LetterMap.insert(std::make_pair(0x0644,x0644));
    LetterMap.insert(std::make_pair(0x0645,x0645));
    LetterMap.insert(std::make_pair(0x0646,x0646));
    LetterMap.insert(std::make_pair(0x0647,x0647));
    LetterMap.insert(std::make_pair(0x0648,x0648));
    LetterMap.insert(std::make_pair(0x064A,x064A));
    LetterMap.insert(std::make_pair(0x0622,x0622));
    LetterMap.insert(std::make_pair(0xFEFB,xFEFB));
    LetterMap.insert(std::make_pair(0x0623,x0623));
    LetterMap.insert(std::make_pair(0x0625,x0625));
    LetterMap.insert(std::make_pair(0x0624,x0624));
    LetterMap.insert(std::make_pair(0x0626,x0626));
    LetterMap.insert(std::make_pair(0x0629,x0629));
    LetterMap.insert(std::make_pair(0x0649,x0649));

 return LetterMap;
}
#endif //CODE_THORNYREADER_PURE_ARABIC_TABLES_H