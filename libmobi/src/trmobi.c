#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "trmobi.h"
/* miniz file is needed for EPUB creation */
#define USE_XMLWRITER
#ifdef USE_XMLWRITER
#define MINIZ_HEADER_FILE_ONLY
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "miniz.c"
#include "meta.h"
#endif

#define EPUB_CONTAINER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n\
  <rootfiles>\n\
    <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\" />\n\
  </rootfiles>\n\
</container>"
#define EPUB_MIMETYPE "application/epub+zip"

/* options values */
char outdir[FILENAME_MAX];

#include <android/log.h>
void printlogcat(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    __android_log_vprint(ANDROID_LOG_ERROR, "thornyreader", msg, args);
    va_end(args);
}

/**
 @brief Bundle recreated source files into EPUB container

 This function is a simple example.
 In real world implementation one should validate and correct all input
 markup to check if it conforms to OPF and HTML specifications and
 correct all the issues.

 @param[in] rawml MOBIRawml structure holding parsed records
 @param[in] fullpath File path will be parsed to build basenames of dumped records
 */
bool create_epub(const MOBIRawml *rawml, const char *fullpath) {
    if (rawml == NULL) {
        printf("Rawml structure not initialized\n");
        return false;
    }
    char dirname[FILENAME_MAX];
    char basename[FILENAME_MAX];
    split_fullpath(fullpath, dirname, basename);
    char zipfile[FILENAME_MAX];
    snprintf(zipfile, sizeof(zipfile), "%s%s", outdir, basename);

    printlogcat("Saving EPUB to %s\n", zipfile);
    /* create zip (epub) archive */
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(mz_zip_archive));
    mz_bool mz_ret = mz_zip_writer_init_file(&zip, zipfile, 0);
    if (!mz_ret) {
        printlogcat("Could not initialize zip archive\n");
        return false;
    }
    /* start adding files to archive */
    mz_ret = mz_zip_writer_add_mem(&zip, "mimetype", EPUB_MIMETYPE, sizeof(EPUB_MIMETYPE) - 1, MZ_NO_COMPRESSION);
    if (!mz_ret) {
        printlogcat("Could not add mimetype\n");
        mz_zip_writer_end(&zip);
        return false;
    }
    mz_ret = mz_zip_writer_add_mem(&zip, "META-INF/container.xml", EPUB_CONTAINER, sizeof(EPUB_CONTAINER) - 1, (mz_uint)MZ_DEFAULT_COMPRESSION);
    if (!mz_ret) {
        printlogcat("Could not add container.xml\n");
        mz_zip_writer_end(&zip);
        return false;
    }
    char partname[FILENAME_MAX];
    if (rawml->markup != NULL) {
        /* Linked list of MOBIPart structures in rawml->markup holds main text files */
        MOBIPart *curr = rawml->markup;
        while (curr != NULL) {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            snprintf(partname, sizeof(partname), "OEBPS/part%05zu.%s", curr->uid, file_meta.extension);
            mz_ret = mz_zip_writer_add_mem(&zip, partname, curr->data, curr->size, (mz_uint) MZ_DEFAULT_COMPRESSION);
            if (!mz_ret) {
                printlogcat("Could not add file to archive: %s\n", partname);
                mz_zip_writer_end(&zip);
                return false;
            }
            curr = curr->next;
        }
    }
    if (rawml->flow != NULL) {
        /* Linked list of MOBIPart structures in rawml->flow holds supplementary text files */
        MOBIPart *curr = rawml->flow;
        /* skip raw html file */
        curr = curr->next;
        while (curr != NULL) {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            snprintf(partname, sizeof(partname), "OEBPS/flow%05zu.%s", curr->uid, file_meta.extension);
            mz_ret = mz_zip_writer_add_mem(&zip, partname, curr->data, curr->size, (mz_uint) MZ_DEFAULT_COMPRESSION);
            if (!mz_ret) {
                printf("Could not add file to archive: %s\n", partname);
                mz_zip_writer_end(&zip);
                return false;
            }
            curr = curr->next;
        }
    }
    if (rawml->resources != NULL) {
        /* Linked list of MOBIPart structures in rawml->resources holds binary files, also opf files */
        MOBIPart *curr = rawml->resources;
        /* jpg, gif, png, bmp, font, audio, video, also opf, ncx */
        while (curr != NULL) {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            //CRLog::error("extension == %s",file_meta.extension);
            if (curr->size > 0) {
                //printlogcat("file.meta.type = %d", file_meta.type);
                if (file_meta.type == T_OPF)
                { //printlogcat("file.meta.type = T_OPF. Printing opf into file");
                    snprintf(partname, sizeof(partname), "OEBPS/content.opf");
                }
                else {
                    snprintf(partname, sizeof(partname), "OEBPS/resource%05zu.%s", curr->uid, file_meta.extension);
                }
                mz_ret = mz_zip_writer_add_mem(&zip, partname, curr->data, curr->size, (mz_uint) MZ_DEFAULT_COMPRESSION);
                if (!mz_ret) {
                    printlogcat("Could not add file to archive: %s\n", partname);
                    mz_zip_writer_end(&zip);
                    return false;
                }
            }
            curr = curr->next;
        }
    }
    /* Finalize epub archive */
    mz_ret = mz_zip_writer_finalize_archive(&zip);
    if (!mz_ret) {
        printlogcat("Could not finalize zip archive\n");
        mz_zip_writer_end(&zip);
        return false;
    }
    mz_ret = mz_zip_writer_end(&zip);
    if (!mz_ret) {
        printlogcat("Could not finalize zip writer\n");
        return false;
    }
    printlogcat("Create_epub: done!");
    return true;
}