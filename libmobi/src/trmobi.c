#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#include "win32/getopt.h"
#else
#include <unistd.h>
#endif
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

#ifdef HAVE_SYS_RESOURCE_H
/* rusage */
# include <sys/resource.h>
# define PRINT_RUSAGE_ARG "u"
#else
# define PRINT_RUSAGE_ARG ""
#endif
/* encryption */
#ifdef USE_ENCRYPTION
# define PRINT_ENC_USG " [-p pid] [-P serial]"
# define PRINT_ENC_ARG "p:P:"
#else
# define PRINT_ENC_USG ""
# define PRINT_ENC_ARG ""
#endif
/* xmlwriter */

#ifdef USE_XMLWRITER
# define PRINT_EPUB_ARG "e"
#else
# define PRINT_EPUB_ARG ""
#endif

#define EPUB_CONTAINER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n\
  <rootfiles>\n\
    <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\" />\n\
  </rootfiles>\n\
</container>"
#define EPUB_MIMETYPE "application/epub+zip"

/* command line options */
int dump_rawml_opt = 0;
int create_epub_opt = 0;
int print_extended_meta_opt = 0;
int print_rec_meta_opt = 0;
int dump_rec_opt = 0;
int parse_kf7_opt = 0;
int dump_parts_opt = 0;
int print_rusage_opt = 0;
int outdir_opt = 0;
int extract_source_opt = 0;
#ifdef USE_ENCRYPTION
int setpid_opt = 0;
int setserial_opt = 0;
#endif

/* options values */
char outdir[FILENAME_MAX];
#ifdef USE_ENCRYPTION
char *pid = NULL;
char *serial = NULL;
#endif


#include <android/log.h>
void printlogcat(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    __android_log_vprint(ANDROID_LOG_ERROR, "thornyreader", msg, args);
    va_end(args);
}

#ifdef USE_XMLWRITER
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
    if (outdir_opt) {
        snprintf(zipfile, sizeof(zipfile), "%s%s", outdir, basename);
    } else {
        snprintf(zipfile, sizeof(zipfile), "%s%s", dirname, basename);
    }

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
#endif
/**
 @brief get coverpage or cover thumbnail offsets from EXTH headers
 @param[in] m MOBIData structure
 @Returns -1 if found none, image offset if found cover
 */

/*      https://wiki.mobileread.com/wiki/MOBI#Image_Records
        201	4	coveroffset	Add to first image field in Mobi Header to find PDB record containing the cover image	<EmbeddedCover>
        202	4	thumboffset	Add to first image field in Mobi Header to find PDB record containing the thumbnail cover image
 */

int GetExthCoverOffset(const MOBIData *m) {
    if (m->eh == NULL) {
        return -1;
    }
    /* Linked list of MOBIExthHeader structures holds EXTH records */
    const MOBIExthHeader *curr = m->eh;
    if (curr != NULL) {
        printlogcat("EXTH records:");
    }
    uint32_t val32;
    while (curr != NULL) {
        MOBIExthMeta tag = mobi_get_exthtagmeta_by_tag(curr->tag);
        if (tag.tag != 0) {
            size_t size = curr->size;
            unsigned char *data = curr->data;
            val32 = mobi_decode_exthvalue(data, size);

            if (tag.type == EXTH_COVEROFFSET)
            {
                printlogcat("COVEROFFSET FOUND 1!");
                return val32;
            }
            else if (tag.type == EXTH_THUMBOFFSET)
            {
                printlogcat("THUMBNAIL COVER OFFSET FOUND 1!");
                return val32;
            }
            else if (tag.type == EXTH_NUMERIC) //check all tags again
            {
                // printlogcat("%s (%i): %u\n", tag.name, tag.tag, val32);
                if (tag.tag == 202)
                {
                    printlogcat("COVEROFFSET FOUND 2!");
                    return val32;
                }
                if (tag.tag == 201)
                {
                    printlogcat("THUMBNAIL COVER OFFSET FOUND 2!");
                    return val32;
                }
            }
        }
        curr = curr->next;
    }
    return -1;
}