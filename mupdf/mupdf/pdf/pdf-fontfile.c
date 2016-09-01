#include <mupdf/pdf.h>

#include <sys/stat.h>

// EBD: global changes

char font_DroidSansMono[] = "/system/fonts/DroidSansMono.ttf";
char font_DroidSans[] = "/system/fonts/DroidSans.ttf";
char font_DroidSansBold[] = "/system/fonts/DroidSans-Bold.ttf";
char font_DroidSansItalic[] = "/system/fonts/Roboto-Italic.ttf";
char font_DroidSansBoldItalic[] = "/system/fonts/Roboto-BoldItalic.ttf";
char font_DroidSerifRegular[] = "/system/fonts/DroidSerif-Regular.ttf";
char font_DroidSerifBold[] = "/system/fonts/DroidSerif-Bold.ttf";
char font_DroidSerifItalic[] = "/system/fonts/DroidSerif-Italic.ttf";
char font_DroidSerifBoldItalic[] = "/system/fonts/DroidSerif-BoldItalic.ttf";
char font_StandardSym[] = "/sdcard/.org.ebookdroid/StandardSymL.cff";
char font_Dingbats[] = "/sdcard/.org.ebookdroid/Dingbats.cff";
char font_DroidSansFallback[] = "/system/fonts/DroidSansFallback.ttf";

char ext_font_Courier[1024]; //= "/sdcard/.org.ebookdroid/fonts/FreeMono.ttf";
char ext_font_CourierBold[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeMonoBold.ttf";
char ext_font_CourierOblique[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeMonoOblique.ttf";
char ext_font_CourierBoldOblique[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeMonoBoldOblique.ttf";
char ext_font_Helvetica[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSans.ttf";
char ext_font_HelveticaBold[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSansBold.ttf";
char ext_font_HelveticaOblique[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSansOblique.ttf";
char ext_font_HelveticaBoldOblique[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSans.ttf";
char ext_font_TimesRoman[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSerif.ttf";
char ext_font_TimesBold[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSerifBold.ttf";
char ext_font_TimesItalic[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSerifItalic.ttf";
char ext_font_TimesBoldItalic[1024]; // = "/sdcard/.org.ebookdroid/fonts/FreeSerifBoldItalic.ttf";
char ext_font_Symbol[1024]; // = "/sdcard/.org.ebookdroid/fonts/StandardSymL.cff";
char ext_font_ZapfDingbats[1024]; // = "/sdcard/.org.ebookdroid/fonts/Dingbats.cff";


char ext_system_fonts[100][2][512];
int  ext_system_fonts_count;


struct fontsubst_s {
	const char* name;
	char* ext_font;
	char* def_font;
	struct fontsubst_s* subst;
};

typedef struct fontsubst_s fontsubst;

struct fontfamily_s {
	const char* name;
	fontsubst regular;
	fontsubst bold;
	fontsubst italic;
	fontsubst bold_italic;
};

typedef struct fontfamily_s fontfamily;

fontfamily font_Courier = {
	"Courier",
	{"Courier", ext_font_Courier, font_DroidSansMono},
	{"Courier-Bold", ext_font_CourierBold, font_DroidSansMono},
	{"Courier-Oblique", ext_font_CourierOblique, font_DroidSansMono},
	{"Courier-BoldOblique", ext_font_CourierBoldOblique, font_DroidSansMono}
};

fontsubst subst_SansBold = {font_DroidSansBold, font_DroidSans};
fontsubst subst_SansItalic = {font_DroidSansItalic, font_DroidSans};
fontsubst subst_SansBoldItalic = {font_DroidSansItalic, font_DroidSansBold};

fontfamily font_Helvetica = {
	"Helvetica",
	{"Helvetica", ext_font_Helvetica, font_DroidSans},
	{"Helvetica-Bold", ext_font_HelveticaBold, font_DroidSansBold, &subst_SansBold},
	{"Helvetica-Oblique", ext_font_HelveticaOblique, font_DroidSansItalic, &subst_SansItalic},
	{"Helvetica-BoldOblique", ext_font_HelveticaBoldOblique, font_DroidSansBoldItalic, &subst_SansBoldItalic}
};

fontsubst subst_SerifBold = {font_DroidSerifBold, font_DroidSerifRegular};
fontsubst subst_SerifItalic = {font_DroidSerifItalic, font_DroidSerifRegular};
fontsubst subst_SerifBoldItalic = {font_DroidSerifItalic, font_DroidSerifBold};

fontfamily font_Times = {
	"Times",
	{"Times-Roman", ext_font_TimesRoman, font_DroidSerifRegular},
	{"Times-Bold", ext_font_TimesBold, font_DroidSerifBold, &subst_SerifBold},
	{"Times-Italic", ext_font_TimesItalic, font_DroidSerifItalic, &subst_SerifItalic},
	{"Times-BoldItalic", ext_font_TimesBoldItalic, font_DroidSerifBoldItalic, &subst_SerifBoldItalic}
};

fontsubst font_Symbol = {"Symbol", ext_font_Symbol, font_StandardSym};

fontsubst font_ZapfDingbats = {"ZapfDingbats", ext_font_ZapfDingbats, font_Dingbats};

fontsubst* fonts[] = {
	&font_Courier.regular,
	&font_Courier.bold,
	&font_Courier.italic,
	&font_Courier.bold_italic,
	&font_Helvetica.regular,
	&font_Helvetica.bold,
	&font_Helvetica.italic,
	&font_Helvetica.bold_italic,
	&font_Times.regular,
	&font_Times.bold,
	&font_Times.italic,
	&font_Times.bold_italic,
	&font_Symbol,
	&font_ZapfDingbats
};

size_t fonts_count = sizeof(fonts)/sizeof(fontsubst*);

int file_exists(char * filename)
{
    struct stat info;
    int i = stat(filename, &info);
    /* File found */
    if (i == 0)
        return 1;
    return 0;
}

unsigned char* get_ext_font(fontsubst* fs)
{
	char* font = fs->ext_font;
	char* def = fs->def_font;

	if (font && font[0])
	{
		if (file_exists(font))
		{
			LOGI("Load ext font: %s", font);
			return (unsigned char*)font;
		}
		LOGI("No ext font found: %s", font);
	}
	else
	{
		LOGI("No ext font defined");
	}

	if (fs->subst)
	{
		if (def && def[0])
		{
			if (file_exists(def))
			{
				LOGI("Load def font: %s", def);
				return (unsigned char*)def;
			}
			else
			{
				LOGI("No def font found: %s", def);
			}
		}

		LOGI("Load substitution: %s %s", fs->subst->ext_font, fs->subst->def_font);
		return get_ext_font(fs->subst);
	}

	if (def)
	{
		LOGI("Load def font: %s", def);
	}
	else
	{
		LOGI("No def font defined");
	}
	return (unsigned char*)def;
}

unsigned char *
pdf_lookup_builtin_font(char *name, unsigned int *len)
{
	*len = 0;
	int index;
	for(index = 0; index < fonts_count; index++)
	{
		if (!strcmp(fonts[index]->name, name))
		{
			LOGI("Built-in font found: %s", name);
			return get_ext_font(fonts[index]);
		}
	}

	LOGI("No Built-in font found");
	return NULL ;
}


unsigned char *
pdf_lookup_substitute_fontfamily(fontfamily* font, int bold, int italic)
{
	LOGI("Substitute font family found: %s", font->name);

	if (bold) {
		if (italic) {
			return get_ext_font(&font->bold_italic);
		} else {
			return get_ext_font(&font->bold);
		}
	}

	if (italic) {
		return get_ext_font(&font->italic);
	} else {
		return get_ext_font(&font->regular);
	}
}

unsigned char *
pdf_lookup_substitute_font(int mono, int serif, int bold, int italic, unsigned int *len)
{
	*len = 0;

	if (mono)
	{
		return pdf_lookup_substitute_fontfamily(&font_Courier, bold, italic);
	}

	if (serif)
	{
		return pdf_lookup_substitute_fontfamily(&font_Times, bold, italic);
	}

	return pdf_lookup_substitute_fontfamily(&font_Helvetica, bold, italic);
}

unsigned char *
pdf_lookup_substitute_cjk_font(int ros, int serif, unsigned int *len)
{
	*len = 0;
	return (unsigned char*) font_DroidSansFallback;
}

unsigned char *
pdf_lookup_system_font(char *name, unsigned int *len)
{
	*len = 0;
	int index;
	for(index = 0; index < ext_system_fonts_count; index++)
	{
		if (!strcmp(ext_system_fonts[index][0], name))
		{
			LOGI("System font found: %s", name);
			if (file_exists(ext_system_fonts[index][1])) {
				return (unsigned char *)ext_system_fonts[index][1];
			}
		}
	}

	LOGI("No System font found");
	return NULL;
}
