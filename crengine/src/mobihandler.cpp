//
// Created by Admin on 13/4/2018.
//

#include "include/mobihandler.h"
#if 1

bool ImportMOBIDocNew(const char *absolute_path, MOBIRawml* rawmlret ,MOBIData* mobidataret)
{
    CRLog::error("ImportMobiDocNew start");
    /* Initialize main MOBIData structure */
/* Must be deallocated with mobi_free() when not needed */
    MOBIData *m = mobi_init();
    if (m == NULL) {
        CRLog::error("m == NULL");
        return false;
    }
/* Open file for reading */
    FILE *file = fopen(absolute_path, "rb");
    if (file == NULL) {
        mobi_free(m);
        CRLog::error("file == NULL, fullpath = %s",absolute_path);
        return false;
    }
/* Load file into MOBIData structure */
/* This structure will hold raw data/metadata from mobi document */
    MOBI_RET mobi_ret = mobi_load_file(m, file);
    fclose(file);
    if (mobi_ret != MOBI_SUCCESS) {
        CRLog::error("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        return false;
    }
/* Initialize MOBIRawml structure */
/* Must be deallocated with mobi_free_rawml() when not needed */
/* In the next step this structure will be filled with parsed data */
    MOBIRawml *rawml = mobi_init_rawml(m);
    if (rawml == NULL) {
        CRLog::error("rawml == NULL");
        mobi_free(m);
        return false;
    }
/* Raw data from MOBIData will be converted to html, css, fonts, media resources */
/* Parsed data will be available in MOBIRawml structure */
    mobi_ret = mobi_parse_rawml(rawml, m);
    if (mobi_ret != MOBI_SUCCESS) {
        CRLog::error("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        mobi_free_rawml(rawml);
        return false;
    }
    rawmlret=rawml;
    mobidataret=m;

    ConvertMOBIDocToEpub(rawml,"/data/data/org.readera/files/epub.epub");
    return true;
}

bool ConvertMOBIDocToEpub(MOBIRawml* rawml, const char* epubnewpath)
{
    CRLog::trace("EPUB creation begin!");
    if (!create_epub(rawml,epubnewpath))
    {
        CRLog::error("Epub not created!");
        return false;
    }
    CRLog::trace("EPUB created successfully!");
    return true; // SUCCESS;
}

void FreeMOBIStructures(MOBIRawml* rawml, MOBIData* m)
{
/* Free MOBIRawml structure */
    mobi_free_rawml(rawml);

/* Free MOBIData structure */
    mobi_free(m);
    return; // SUCCESS;
}

mobiresponse GetMobiMetaFromFile(const char *fullpath)
{
    mobiresponse a;
    MOBI_RET mobi_ret;
    /* Initialize main MOBIData structure */
    MOBIData *m = mobi_init();
    if (m == NULL)
    {
        CRLog::error("Memory allocation failed\n");
        return a;
    }
        mobi_parse_kf7(m);
    FILE *file = fopen(fullpath, "rb");
    if (file == NULL)
    {
        CRLog::error("Error opening file: %s", fullpath);
        mobi_free(m);
        return a;
    }
    mobi_ret = mobi_load_file(m, file);
    fclose(file);
    //GetMobiMeta(m);
    a = GetMobiMetaSummary(m);
    return a;
}


#include <iostream>
#include <string>
#include <locale>
#include <codecvt>


/**
 @brief Print all loaded headers meta information
 @param[in] m MOBIData structure
 */
void GetMobiMeta(const MOBIData *m) {
    /* Full name stored at offset given in MOBI header */
    if (m->mh && m->mh->full_name) {
        char full_name[FULLNAME_MAX + 1];
        if (mobi_get_fullname(m, full_name, FULLNAME_MAX) == MOBI_SUCCESS) {
            CRLog::error("\nFull name: %s\n", full_name);                       // <-
        }
    }

    /* Palm database header */
    if (m->ph) {
        CRLog::error("\nPalm doc header:\n");
        CRLog::error("name: %s\n", m->ph->name);
        CRLog::error("attributes: %hu\n", m->ph->attributes);
        CRLog::error("version: %hu\n", m->ph->version);
        struct tm * timeinfo = mobi_pdbtime_to_time(m->ph->ctime);
        CRLog::error("ctime: %s", asctime(timeinfo));
        timeinfo = mobi_pdbtime_to_time(m->ph->mtime);
        CRLog::error("mtime: %s", asctime(timeinfo));
        timeinfo = mobi_pdbtime_to_time(m->ph->btime);
        CRLog::error("btime: %s", asctime(timeinfo));
        CRLog::error("mod_num: %u\n", m->ph->mod_num);
        CRLog::error("appinfo_offset: %u\n", m->ph->appinfo_offset);
        CRLog::error("sortinfo_offset: %u\n", m->ph->sortinfo_offset);
        CRLog::error("type: %s\n", m->ph->type);
        CRLog::error("creator: %s\n", m->ph->creator);
        CRLog::error("uid: %u\n", m->ph->uid);
        CRLog::error("next_rec: %u\n", m->ph->next_rec);
        CRLog::error("rec_count: %u\n", m->ph->rec_count);
    }
    /* Record 0 header */
    if (m->rh) {
        CRLog::error("\nRecord 0 header:\n");
        CRLog::error("compresion type: %u\n", m->rh->compression_type);
        CRLog::error("text length: %u\n", m->rh->text_length);
        CRLog::error("text record count: %u\n", m->rh->text_record_count);
        CRLog::error("text record size: %u\n", m->rh->text_record_size);
        CRLog::error("encryption type: %u\n", m->rh->encryption_type);
        CRLog::error("unknown: %u\n", m->rh->unknown1);
    }
    /* Mobi header */
    if (m->mh) {
        CRLog::error("MOBI header:");
        CRLog::error("identifier: %s", m->mh->mobi_magic);
        if(m->mh->header_length) { CRLog::error("header length: %u", *m->mh->header_length); }
        if(m->mh->mobi_type) { CRLog::error("mobi type: %u", *m->mh->mobi_type); }
        if(m->mh->text_encoding) { CRLog::error("text encoding: %u", *m->mh->text_encoding); }
        if(m->mh->uid) { CRLog::error("unique id: %u", *m->mh->uid); }
        if(m->mh->version) { CRLog::error("file version: %u", *m->mh->version); }
        if(m->mh->orth_index) { CRLog::error("orth index: %u", *m->mh->orth_index); }
        if(m->mh->infl_index) { CRLog::error("infl index: %u", *m->mh->infl_index); }
        if(m->mh->names_index) { CRLog::error("names index: %u", *m->mh->names_index); }
        if(m->mh->keys_index) { CRLog::error("keys index: %u", *m->mh->keys_index); }
        if(m->mh->extra0_index) { CRLog::error("extra0 index: %u", *m->mh->extra0_index); }
        if(m->mh->extra1_index) { CRLog::error("extra1 index: %u", *m->mh->extra1_index); }
        if(m->mh->extra2_index) { CRLog::error("extra2 index: %u", *m->mh->extra2_index); }
        if(m->mh->extra3_index) { CRLog::error("extra3 index: %u", *m->mh->extra3_index); }
        if(m->mh->extra4_index) { CRLog::error("extra4 index: %u", *m->mh->extra4_index); }
        if(m->mh->extra5_index) { CRLog::error("extra5 index: %u", *m->mh->extra5_index); }
        if(m->mh->non_text_index) { CRLog::error("non text index: %u", *m->mh->non_text_index); }
        if(m->mh->full_name_offset) { CRLog::error("full name offset: %u", *m->mh->full_name_offset); }
        if(m->mh->full_name_length) { CRLog::error("full name length: %u", *m->mh->full_name_length); }
        if(m->mh->locale) {
            const char *locale_string = mobi_get_locale_string(*m->mh->locale);
            if (locale_string) {
                CRLog::error("locale: %s (%u)", locale_string, *m->mh->locale);
            } else {
                CRLog::error("locale: unknown (%u)", *m->mh->locale);
            }
        }
        if(m->mh->dict_input_lang) {
            const char *locale_string = mobi_get_locale_string(*m->mh->dict_input_lang);
            if (locale_string) {
                CRLog::error("dict input lang: %s (%u)\n", locale_string, *m->mh->dict_input_lang);
            } else {
                CRLog::error("dict input lang: unknown (%u)\n", *m->mh->dict_input_lang);
            }
        }
        if(m->mh->dict_output_lang) {
            const char *locale_string = mobi_get_locale_string(*m->mh->dict_output_lang);
            if (locale_string) {
                CRLog::error("dict output lang: %s (%u)\n", locale_string, *m->mh->dict_output_lang);
            } else {
                CRLog::error("dict output lang: unknown (%u)\n", *m->mh->dict_output_lang);
            }
        }
        if(m->mh->min_version) { CRLog::error("minimal version: %u", *m->mh->min_version); }
        if(m->mh->image_index) { CRLog::error("first image index: %u", *m->mh->image_index); }
        if(m->mh->huff_rec_index) { CRLog::error("huffman record offset: %u", *m->mh->huff_rec_index); }
        if(m->mh->huff_rec_count) { CRLog::error("huffman records count: %u", *m->mh->huff_rec_count); }
        if(m->mh->datp_rec_index) { CRLog::error("DATP record offset: %u", *m->mh->datp_rec_index); }
        if(m->mh->datp_rec_count) { CRLog::error("DATP records count: %u", *m->mh->datp_rec_count); }
        if(m->mh->exth_flags) { CRLog::error("EXTH flags: %u", *m->mh->exth_flags); }
        if(m->mh->unknown6) { CRLog::error("unknown: %u", *m->mh->unknown6); }
        if(m->mh->drm_offset) { CRLog::error("drm offset: %u", *m->mh->drm_offset); }
        if(m->mh->drm_count) { CRLog::error("drm count: %u", *m->mh->drm_count); }
        if(m->mh->drm_size) { CRLog::error("drm size: %u", *m->mh->drm_size); }
        if(m->mh->drm_flags) { CRLog::error("drm flags: %u", *m->mh->drm_flags); }
        if(m->mh->first_text_index) { CRLog::error("first text index: %u", *m->mh->first_text_index); }
        if(m->mh->last_text_index) { CRLog::error("last text index: %u", *m->mh->last_text_index); }
        if(m->mh->fdst_index) { CRLog::error("FDST offset: %u", *m->mh->fdst_index); }
        if(m->mh->fdst_section_count) { CRLog::error("FDST count: %u", *m->mh->fdst_section_count); }
        if(m->mh->fcis_index) { CRLog::error("FCIS index: %u", *m->mh->fcis_index); }
        if(m->mh->fcis_count) { CRLog::error("FCIS count: %u", *m->mh->fcis_count); }
        if(m->mh->flis_index) { CRLog::error("FLIS index: %u", *m->mh->flis_index); }
        if(m->mh->flis_count) { CRLog::error("FLIS count: %u", *m->mh->flis_count); }
        if(m->mh->unknown10) { CRLog::error("unknown: %u", *m->mh->unknown10); }
        if(m->mh->unknown11) { CRLog::error("unknown: %u", *m->mh->unknown11); }
        if(m->mh->srcs_index) { CRLog::error("SRCS index: %u", *m->mh->srcs_index); }
        if(m->mh->srcs_count) { CRLog::error("SRCS count: %u", *m->mh->srcs_count); }
        if(m->mh->unknown12) { CRLog::error("unknown: %u", *m->mh->unknown12); }
        if(m->mh->unknown13) { CRLog::error("unknown: %u", *m->mh->unknown13); }
        if(m->mh->extra_flags) { CRLog::error("extra record flags: %u\n", *m->mh->extra_flags); }
        if(m->mh->ncx_index) { CRLog::error("NCX offset: %u", *m->mh->ncx_index); }
        if(m->mh->unknown14) { CRLog::error("unknown: %u", *m->mh->unknown14); }
        if(m->mh->unknown15) { CRLog::error("unknown: %u", *m->mh->unknown15); }
        if(m->mh->fragment_index) { CRLog::error("fragment index: %u", *m->mh->fragment_index); }
        if(m->mh->skeleton_index) { CRLog::error("skeleton index: %u", *m->mh->skeleton_index); }
        if(m->mh->datp_index) { CRLog::error("DATP index: %u", *m->mh->datp_index); }
        if(m->mh->unknown16) { CRLog::error("unknown: %u", *m->mh->unknown16); }
        if(m->mh->guide_index) { CRLog::error("guide index: %u", *m->mh->guide_index); }
        if(m->mh->unknown17) { CRLog::error("unknown: %u", *m->mh->unknown17); }
        if(m->mh->unknown18) { CRLog::error("unknown: %u", *m->mh->unknown18); }
        if(m->mh->unknown19) { CRLog::error("unknown: %u", *m->mh->unknown19); }
        if(m->mh->unknown20) { CRLog::error("unknown: %u", *m->mh->unknown20); }
    }
}

mobiresponse GetMobiMetaSummary(const MOBIData *m) {

    mobiresponse ret;
    char *title = mobi_meta_get_title(m);
    if (title) {
        ret.title.append(Utf8ToUnicode(title));
        CRLog::trace("Title %s",title);
        free(title);
    }
    char *author = mobi_meta_get_author(m);
    if (author) {
        ret.author.append(Utf8ToUnicode(author));
        CRLog::trace("Author: %s\n", author);
        //free(author);
    }
    char *language = mobi_meta_get_language(m);
    if (language) {
        ret.language.append(Utf8ToUnicode(language));
        CRLog::trace("Language: %s", language);
        //free(language);
    }
    return ret;
}


#endif