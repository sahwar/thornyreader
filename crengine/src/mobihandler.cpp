//
// Created by Admin on 13/4/2018.
//

#include "include/mobihandler.h"
#include "include/crconfig.h"

#include <cctype>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

bool ImportMOBIDocNew(const char *absolute_path,const char* epubnewpath)
{
    CRLog::trace("ImportMobiDocNew start");
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
#if 0
    FILE *filedump = fopen("data/data/org.readera/files/mobidump.xml", "w");
    if (filedump == NULL) {
        mobi_free(m);
        CRLog::error("file == NULL, fullpath = %s",absolute_path);
        return false;
    }
    mobi_dump_rawml(m, filedump);
    fclose(filedump);
    //int a = dump_rawml_parts(rawml, FULL_PATH);
#endif
    ConvertMOBIDocToEpub(rawml,epubnewpath);
    mobi_free_rawml(rawml);
    mobi_free(m);
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
    return;
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
    if (mobi_ret != MOBI_SUCCESS) {
        CRLog::error("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        return a;
    }
    //GetMobiMeta(m);
    a = GetMobiMetaSummary(m);
    mobi_free(m);
    return a;
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

LVStreamRef GetMobiCoverPageToStream(const char *fullpath) {
    MOBIData *m = mobi_init();
    if (m == NULL) {
        CRLog::error("m == NULL");
        return LVStreamRef();
    }
    /* Open file for reading */
    FILE *file = fopen(fullpath, "rb");
    if (file == NULL) {
        mobi_free(m);
        CRLog::error("file == NULL, fullpath = %s",fullpath);
        return LVStreamRef();
    }
    MOBI_RET mobi_ret = mobi_load_file(m, file);
    fclose(file);
    if (mobi_ret != MOBI_SUCCESS) {
        CRLog::error("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        return LVStreamRef();
    }
    MOBIRawml *rawml = mobi_init_rawml(m);
    if (rawml == NULL) {
        CRLog::error("rawml == NULL");
        mobi_free(m);
        return LVStreamRef();
    }
    mobi_ret = mobi_parse_rawml(rawml, m);
    if (mobi_ret != MOBI_SUCCESS) {
        CRLog::error("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        mobi_free_rawml(rawml);
        return LVStreamRef();
    }
    int coverimageid = MOBIGetCoverImageId(m);
    if (rawml->resources != NULL)
    {
        MOBIPart *result = nullptr;
        MOBIPart *curr = rawml->resources;
        while (curr != NULL)
        {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            if (curr->size > 0)
            {
                if (file_meta.type == T_GIF
                    || file_meta.type == T_JPG
                    || file_meta.type == T_BMP
                    || file_meta.type == T_PNG)
                {
                    if (curr->uid == coverimageid)
                    {
                        result = curr;
                    }
                }
            }
            curr = curr->next;
        }
        LVStreamRef res = LVCreateMemoryStream(result->data, static_cast<int>(result->size));
        mobi_free_rawml(rawml);
        mobi_free(m);
        return res;
    }
    mobi_free_rawml(rawml);
    mobi_free(m);
    return LVStreamRef();
}

bool IsMobiDoc(const char* absolute_path)
{
    MOBI_RET mobi_ret;
    MOBIData *m = mobi_init();
    if (m == NULL)
    {
        CRLog::error("Memory allocation failed");
        return false;
    }
    mobi_parse_kf7(m);
    FILE *file = fopen(absolute_path, "rb");
    if (file == NULL)
    {
        CRLog::error("Error opening file: %s", absolute_path);
        mobi_free(m);
        return false;
    }
    mobi_ret = mobi_load_file(m, file);
    fclose(file);
    if (mobi_ret != MOBI_SUCCESS) {
        CRLog::error("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        return false;
    }
    mobi_free(m);
    return true;
}

/**
 @brief Print all loaded headers meta information
 @param[in] m MOBIData structure
 */
void GetMobiMeta(const MOBIData *m) {
    /* Full name stored at offset given in MOBI header */
    if (m->mh && m->mh->full_name) {
        char full_name[FULLNAME_MAX + 1];
        if (mobi_get_fullname(m, full_name, FULLNAME_MAX) == MOBI_SUCCESS) {
            CRLog::trace("\nFull name: %s\n", full_name);                       // <-
        }
    }

    /* Palm database header */
    if (m->ph) {
        CRLog::trace("\nPalm doc header:\n");
        CRLog::trace("name: %s\n", m->ph->name);
        CRLog::trace("attributes: %hu\n", m->ph->attributes);
        CRLog::trace("version: %hu\n", m->ph->version);
        struct tm * timeinfo = mobi_pdbtime_to_time(m->ph->ctime);
        CRLog::trace("ctime: %s", asctime(timeinfo));
        timeinfo = mobi_pdbtime_to_time(m->ph->mtime);
        CRLog::trace("mtime: %s", asctime(timeinfo));
        timeinfo = mobi_pdbtime_to_time(m->ph->btime);
        CRLog::trace("btime: %s", asctime(timeinfo));
        CRLog::trace("mod_num: %u\n", m->ph->mod_num);
        CRLog::trace("appinfo_offset: %u\n", m->ph->appinfo_offset);
        CRLog::trace("sortinfo_offset: %u\n", m->ph->sortinfo_offset);
        CRLog::trace("type: %s\n", m->ph->type);
        CRLog::trace("creator: %s\n", m->ph->creator);
        CRLog::trace("uid: %u\n", m->ph->uid);
        CRLog::trace("next_rec: %u\n", m->ph->next_rec);
        CRLog::trace("rec_count: %u\n", m->ph->rec_count);
    }
    /* Record 0 header */
    if (m->rh) {
        CRLog::trace("\nRecord 0 header:\n");
        CRLog::trace("compresion type: %u\n", m->rh->compression_type);
        CRLog::trace("text length: %u\n", m->rh->text_length);
        CRLog::trace("text record count: %u\n", m->rh->text_record_count);
        CRLog::trace("text record size: %u\n", m->rh->text_record_size);
        CRLog::trace("encryption type: %u\n", m->rh->encryption_type);
        CRLog::trace("unknown: %u\n", m->rh->unknown1);
    }
    /* Mobi header */
    if (m->mh) {
        CRLog::trace("MOBI header:");
        CRLog::trace("identifier: %s", m->mh->mobi_magic);
        if(m->mh->header_length) { CRLog::trace("header length: %u", *m->mh->header_length); }
        if(m->mh->mobi_type) { CRLog::trace("mobi type: %u", *m->mh->mobi_type); }
        if(m->mh->text_encoding) { CRLog::trace("text encoding: %u", *m->mh->text_encoding); }
        if(m->mh->uid) { CRLog::trace("unique id: %u", *m->mh->uid); }
        if(m->mh->version) { CRLog::trace("file version: %u", *m->mh->version); }
        if(m->mh->orth_index) { CRLog::trace("orth index: %u", *m->mh->orth_index); }
        if(m->mh->infl_index) { CRLog::trace("infl index: %u", *m->mh->infl_index); }
        if(m->mh->names_index) { CRLog::trace("names index: %u", *m->mh->names_index); }
        if(m->mh->keys_index) { CRLog::trace("keys index: %u", *m->mh->keys_index); }
        if(m->mh->extra0_index) { CRLog::trace("extra0 index: %u", *m->mh->extra0_index); }
        if(m->mh->extra1_index) { CRLog::trace("extra1 index: %u", *m->mh->extra1_index); }
        if(m->mh->extra2_index) { CRLog::trace("extra2 index: %u", *m->mh->extra2_index); }
        if(m->mh->extra3_index) { CRLog::trace("extra3 index: %u", *m->mh->extra3_index); }
        if(m->mh->extra4_index) { CRLog::trace("extra4 index: %u", *m->mh->extra4_index); }
        if(m->mh->extra5_index) { CRLog::trace("extra5 index: %u", *m->mh->extra5_index); }
        if(m->mh->non_text_index) { CRLog::trace("non text index: %u", *m->mh->non_text_index); }
        if(m->mh->full_name_offset) { CRLog::trace("full name offset: %u", *m->mh->full_name_offset); }
        if(m->mh->full_name_length) { CRLog::trace("full name length: %u", *m->mh->full_name_length); }
        if(m->mh->locale) {
            const char *locale_string = mobi_get_locale_string(*m->mh->locale);
            if (locale_string) {
                CRLog::trace("locale: %s (%u)", locale_string, *m->mh->locale);
            } else {
                CRLog::trace("locale: unknown (%u)", *m->mh->locale);
            }
        }
        if(m->mh->dict_input_lang) {
            const char *locale_string = mobi_get_locale_string(*m->mh->dict_input_lang);
            if (locale_string) {
                CRLog::trace("dict input lang: %s (%u)\n", locale_string, *m->mh->dict_input_lang);
            } else {
                CRLog::trace("dict input lang: unknown (%u)\n", *m->mh->dict_input_lang);
            }
        }
        if(m->mh->dict_output_lang) {
            const char *locale_string = mobi_get_locale_string(*m->mh->dict_output_lang);
            if (locale_string) {
                CRLog::trace("dict output lang: %s (%u)\n", locale_string, *m->mh->dict_output_lang);
            } else {
                CRLog::trace("dict output lang: unknown (%u)\n", *m->mh->dict_output_lang);
            }
        }
        if(m->mh->min_version) { CRLog::trace("minimal version: %u", *m->mh->min_version); }
        if(m->mh->image_index) { CRLog::trace("first image index: %u", *m->mh->image_index); }
        if(m->mh->huff_rec_index) { CRLog::trace("huffman record offset: %u", *m->mh->huff_rec_index); }
        if(m->mh->huff_rec_count) { CRLog::trace("huffman records count: %u", *m->mh->huff_rec_count); }
        if(m->mh->datp_rec_index) { CRLog::trace("DATP record offset: %u", *m->mh->datp_rec_index); }
        if(m->mh->datp_rec_count) { CRLog::trace("DATP records count: %u", *m->mh->datp_rec_count); }
        if(m->mh->exth_flags) { CRLog::trace("EXTH flags: %u", *m->mh->exth_flags); }
        if(m->mh->unknown6) { CRLog::trace("unknown: %u", *m->mh->unknown6); }
        if(m->mh->drm_offset) { CRLog::trace("drm offset: %u", *m->mh->drm_offset); }
        if(m->mh->drm_count) { CRLog::trace("drm count: %u", *m->mh->drm_count); }
        if(m->mh->drm_size) { CRLog::trace("drm size: %u", *m->mh->drm_size); }
        if(m->mh->drm_flags) { CRLog::trace("drm flags: %u", *m->mh->drm_flags); }
        if(m->mh->first_text_index) { CRLog::trace("first text index: %u", *m->mh->first_text_index); }
        if(m->mh->last_text_index) { CRLog::trace("last text index: %u", *m->mh->last_text_index); }
        if(m->mh->fdst_index) { CRLog::trace("FDST offset: %u", *m->mh->fdst_index); }
        if(m->mh->fdst_section_count) { CRLog::trace("FDST count: %u", *m->mh->fdst_section_count); }
        if(m->mh->fcis_index) { CRLog::trace("FCIS index: %u", *m->mh->fcis_index); }
        if(m->mh->fcis_count) { CRLog::trace("FCIS count: %u", *m->mh->fcis_count); }
        if(m->mh->flis_index) { CRLog::trace("FLIS index: %u", *m->mh->flis_index); }
        if(m->mh->flis_count) { CRLog::trace("FLIS count: %u", *m->mh->flis_count); }
        if(m->mh->unknown10) { CRLog::trace("unknown: %u", *m->mh->unknown10); }
        if(m->mh->unknown11) { CRLog::trace("unknown: %u", *m->mh->unknown11); }
        if(m->mh->srcs_index) { CRLog::trace("SRCS index: %u", *m->mh->srcs_index); }
        if(m->mh->srcs_count) { CRLog::trace("SRCS count: %u", *m->mh->srcs_count); }
        if(m->mh->unknown12) { CRLog::trace("unknown: %u", *m->mh->unknown12); }
        if(m->mh->unknown13) { CRLog::trace("unknown: %u", *m->mh->unknown13); }
        if(m->mh->extra_flags) { CRLog::trace("extra record flags: %u\n", *m->mh->extra_flags); }
        if(m->mh->ncx_index) { CRLog::trace("NCX offset: %u", *m->mh->ncx_index); }
        if(m->mh->unknown14) { CRLog::trace("unknown: %u", *m->mh->unknown14); }
        if(m->mh->unknown15) { CRLog::trace("unknown: %u", *m->mh->unknown15); }
        if(m->mh->fragment_index) { CRLog::trace("fragment index: %u", *m->mh->fragment_index); }
        if(m->mh->skeleton_index) { CRLog::trace("skeleton index: %u", *m->mh->skeleton_index); }
        if(m->mh->datp_index) { CRLog::trace("DATP index: %u", *m->mh->datp_index); }
        if(m->mh->unknown16) { CRLog::trace("unknown: %u", *m->mh->unknown16); }
        if(m->mh->guide_index) { CRLog::trace("guide index: %u", *m->mh->guide_index); }
        if(m->mh->unknown17) { CRLog::trace("unknown: %u", *m->mh->unknown17); }
        if(m->mh->unknown18) { CRLog::trace("unknown: %u", *m->mh->unknown18); }
        if(m->mh->unknown19) { CRLog::trace("unknown: %u", *m->mh->unknown19); }
        if(m->mh->unknown20) { CRLog::trace("unknown: %u", *m->mh->unknown20); }
    }
}

