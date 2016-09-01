#define _notdef NULL

const unsigned short pdf_doc_encoding[256] =
{
	// EBD: undefine nonprintable >>>
	/* 0x0 to 0x17 except \t, \n and \r are really undefined */
	0x0, ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', 0x09, 0x0A, ' ', ' ', 0x0D, ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	// EBD: undefine nonprintable <<<
	0x02d8, 0x02c7, 0x02c6, 0x02d9, 0x02dd, 0x02db, 0x02da, 0x02dc,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x0000,
	0x2022, 0x2020, 0x2021, 0x2026, 0x2014, 0x2013, 0x0192, 0x2044,
	0x2039, 0x203a, 0x2212, 0x2030, 0x201e, 0x201c, 0x201d, 0x2018,
	0x2019, 0x201a, 0x2122, 0xfb01, 0xfb02, 0x0141, 0x0152, 0x0160,
	0x0178, 0x017d, 0x0131, 0x0142, 0x0153, 0x0161, 0x017e, 0x0000,
	0x20ac, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
	0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x0000, 0x00ae, 0x00af,
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
	0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
	0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
	0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

const char * const pdf_standard[256] = { _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
	"ampersand", "quoteright", "parenleft", "parenright", "asterisk",
	"plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon",
	"semicolon", "less", "equal", "greater", "question", "at", "A",
	"B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
	"O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
	"quoteleft", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
	"l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
	"y", "z", "braceleft", "bar", "braceright", "asciitilde", _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, "exclamdown", "cent", "sterling",
	"fraction", "yen", "florin", "section", "currency", "quotesingle",
	"quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright",
	"fi", "fl", _notdef, "endash", "dagger", "daggerdbl", "periodcentered",
	_notdef, "paragraph", "bullet", "quotesinglbase", "quotedblbase",
	"quotedblright", "guillemotright", "ellipsis", "perthousand",
	_notdef, "questiondown", _notdef, "grave", "acute", "circumflex",
	"tilde", "macron", "breve", "dotaccent", "dieresis", _notdef,
	"ring", "cedilla", _notdef, "hungarumlaut", "ogonek", "caron",
	"emdash", _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, "AE",
	_notdef, "ordfeminine", _notdef, _notdef, _notdef, _notdef,
	"Lslash", "Oslash", "OE", "ordmasculine", _notdef, _notdef,
	_notdef, _notdef, _notdef, "ae", _notdef, _notdef,
	_notdef, "dotlessi", _notdef, _notdef, "lslash", "oslash",
	"oe", "germandbls", _notdef, _notdef, _notdef, _notdef
};

const char * const pdf_mac_roman[256] = { _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
	"ampersand", "quotesingle", "parenleft", "parenright", "asterisk",
	"plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon",
	"semicolon", "less", "equal", "greater", "question", "at", "A",
	"B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
	"O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
	"grave", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
	"l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
	"y", "z", "braceleft", "bar", "braceright", "asciitilde", _notdef,
	"Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis",
	"Udieresis", "aacute", "agrave", "acircumflex", "adieresis", "atilde",
	"aring", "ccedilla", "eacute", "egrave", "ecircumflex", "edieresis",
	"iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute",
	"ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave",
	"ucircumflex", "udieresis", "dagger", "degree", "cent", "sterling",
	"section", "bullet", "paragraph", "germandbls", "registered",
	"copyright", "trademark", "acute", "dieresis", _notdef, "AE",
	"Oslash", _notdef, "plusminus", _notdef, _notdef, "yen", "mu",
	_notdef, _notdef, _notdef, _notdef, _notdef, "ordfeminine",
	"ordmasculine", _notdef, "ae", "oslash", "questiondown", "exclamdown",
	"logicalnot", _notdef, "florin", _notdef, _notdef, "guillemotleft",
	"guillemotright", "ellipsis", "space", "Agrave", "Atilde", "Otilde",
	"OE", "oe", "endash", "emdash", "quotedblleft", "quotedblright",
	"quoteleft", "quoteright", "divide", _notdef, "ydieresis",
	"Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright",
	"fi", "fl", "daggerdbl", "periodcentered", "quotesinglbase",
	"quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", "Aacute",
	"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave",
	"Oacute", "Ocircumflex", _notdef, "Ograve", "Uacute", "Ucircumflex",
	"Ugrave", "dotlessi", "circumflex", "tilde", "macron", "breve",
	"dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron"
};

const char * const pdf_mac_expert[256] = { _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	"space", "exclamsmall", "Hungarumlautsmall", "centoldstyle",
	"dollaroldstyle", "dollarsuperior", "ampersandsmall", "Acutesmall",
	"parenleftsuperior", "parenrightsuperior", "twodotenleader",
	"onedotenleader", "comma", "hyphen", "period", "fraction",
	"zerooldstyle", "oneoldstyle", "twooldstyle", "threeoldstyle",
	"fouroldstyle", "fiveoldstyle", "sixoldstyle", "sevenoldstyle",
	"eightoldstyle", "nineoldstyle", "colon", "semicolon", _notdef,
	"threequartersemdash", _notdef, "questionsmall", _notdef,
	_notdef, _notdef, _notdef, "Ethsmall", _notdef, _notdef,
	"onequarter", "onehalf", "threequarters", "oneeighth", "threeeighths",
	"fiveeighths", "seveneighths", "onethird", "twothirds", _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, "ff", "fi",
	"fl", "ffi", "ffl", "parenleftinferior", _notdef, "parenrightinferior",
	"Circumflexsmall", "hypheninferior", "Gravesmall", "Asmall", "Bsmall",
	"Csmall", "Dsmall", "Esmall", "Fsmall", "Gsmall", "Hsmall", "Ismall",
	"Jsmall", "Ksmall", "Lsmall", "Msmall", "Nsmall", "Osmall", "Psmall",
	"Qsmall", "Rsmall", "Ssmall", "Tsmall", "Usmall", "Vsmall", "Wsmall",
	"Xsmall", "Ysmall", "Zsmall", "colonmonetary", "onefitted", "rupiah",
	"Tildesmall", _notdef, _notdef, "asuperior", "centsuperior",
	_notdef, _notdef, _notdef, _notdef, "Aacutesmall",
	"Agravesmall", "Acircumflexsmall", "Adieresissmall", "Atildesmall",
	"Aringsmall", "Ccedillasmall", "Eacutesmall", "Egravesmall",
	"Ecircumflexsmall", "Edieresissmall", "Iacutesmall", "Igravesmall",
	"Icircumflexsmall", "Idieresissmall", "Ntildesmall", "Oacutesmall",
	"Ogravesmall", "Ocircumflexsmall", "Odieresissmall", "Otildesmall",
	"Uacutesmall", "Ugravesmall", "Ucircumflexsmall", "Udieresissmall",
	_notdef, "eightsuperior", "fourinferior", "threeinferior",
	"sixinferior", "eightinferior", "seveninferior", "Scaronsmall",
	_notdef, "centinferior", "twoinferior", _notdef, "Dieresissmall",
	_notdef, "Caronsmall", "osuperior", "fiveinferior", _notdef,
	"commainferior", "periodinferior", "Yacutesmall", _notdef,
	"dollarinferior", _notdef, _notdef, "Thornsmall", _notdef,
	"nineinferior", "zeroinferior", "Zcaronsmall", "AEsmall", "Oslashsmall",
	"questiondownsmall", "oneinferior", "Lslashsmall", _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, "Cedillasmall",
	_notdef, _notdef, _notdef, _notdef, _notdef, "OEsmall",
	"figuredash", "hyphensuperior", _notdef, _notdef, _notdef,
	_notdef, "exclamdownsmall", _notdef, "Ydieresissmall", _notdef,
	"onesuperior", "twosuperior", "threesuperior", "foursuperior",
	"fivesuperior", "sixsuperior", "sevensuperior", "ninesuperior",
	"zerosuperior", _notdef, "esuperior", "rsuperior", "tsuperior",
	_notdef, _notdef, "isuperior", "ssuperior", "dsuperior",
	_notdef, _notdef, _notdef, _notdef, _notdef, "lsuperior",
	"Ogoneksmall", "Brevesmall", "Macronsmall", "bsuperior", "nsuperior",
	"msuperior", "commasuperior", "periodsuperior", "Dotaccentsmall",
	"Ringsmall", _notdef, _notdef, _notdef, _notdef };

const char * const pdf_win_ansi[256] = { _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, _notdef,
	_notdef, _notdef, _notdef, _notdef, _notdef, "space",
	"exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand",
	"quotesingle", "parenleft", "parenright", "asterisk", "plus",
	"comma", "hyphen", "period", "slash", "zero", "one", "two", "three",
	"four", "five", "six", "seven", "eight", "nine", "colon", "semicolon",
	"less", "equal", "greater", "question", "at", "A", "B", "C", "D",
	"E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q",
	"R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
	"backslash", "bracketright", "asciicircum", "underscore", "grave",
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	"braceleft", "bar", "braceright", "asciitilde", "bullet", "Euro",
	"bullet", "quotesinglbase", "florin", "quotedblbase", "ellipsis",
	"dagger", "daggerdbl", "circumflex", "perthousand", "Scaron",
	"guilsinglleft", "OE", "bullet", "Zcaron", "bullet", "bullet",
	"quoteleft", "quoteright", "quotedblleft", "quotedblright", "bullet",
	"endash", "emdash", "tilde", "trademark", "scaron", "guilsinglright",
	"oe", "bullet", "zcaron", "Ydieresis", "space", "exclamdown", "cent",
	"sterling", "currency", "yen", "brokenbar", "section", "dieresis",
	"copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen",
	"registered", "macron", "degree", "plusminus", "twosuperior",
	"threesuperior", "acute", "mu", "paragraph", "periodcentered",
	"cedilla", "onesuperior", "ordmasculine", "guillemotright",
	"onequarter", "onehalf", "threequarters", "questiondown", "Agrave",
	"Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE",
	"Ccedilla", "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave",
	"Iacute", "Icircumflex", "Idieresis", "Eth", "Ntilde", "Ograve",
	"Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply", "Oslash",
	"Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn",
	"germandbls", "agrave", "aacute", "acircumflex", "atilde", "adieresis",
	"aring", "ae", "ccedilla", "egrave", "eacute", "ecircumflex",
	"edieresis", "igrave", "iacute", "icircumflex", "idieresis", "eth",
	"ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis",
	"divide", "oslash", "ugrave", "uacute", "ucircumflex", "udieresis",
	"yacute", "thorn", "ydieresis"
};
