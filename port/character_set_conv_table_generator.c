// SPDX-License-Identifier: MIT
// Copyright 2024 Even Rouault

#include <assert.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>

#define ENCODING_MAX_LEN 256

static void launder_name(const char *srcEncoding,
                         char srcEncodingLaundered[ENCODING_MAX_LEN])
{
    snprintf(srcEncodingLaundered, ENCODING_MAX_LEN, "%s", srcEncoding);
    for (int i = 0; srcEncodingLaundered[i]; ++i)
    {
        if (srcEncodingLaundered[i] == '-')
            srcEncodingLaundered[i] = '_';
    }
}

static void generate(FILE *c_file, FILE *h_file, const char *srcEncoding,
                     const char *comment)
{
    iconv_t sConv = iconv_open("UTF-8", srcEncoding);
    if (sConv == (iconv_t)(-1))
    {
        fprintf(stderr, "iconv_open(%s) failed\n", srcEncoding);
        exit(1);
    }
    char srcEncodingLaundered[ENCODING_MAX_LEN];
    launder_name(srcEncoding, srcEncodingLaundered);
    fprintf(c_file, "/* %s */\n", comment);
    fprintf(c_file, "static const CPLCodePageConvTable CPL_%s_to_UTF8 = {\n",
            srcEncodingLaundered);
    for (int i = 0; i <= 255; ++i)
    {
        unsigned char c = (unsigned char)i;
        size_t size_in = 1;
        unsigned char out[4] = {0, 0, 0, 0};
        size_t size_out = sizeof(out);
        char *p_in = (char *)&c;
        char *p_out = (char *)out;
        size_t nConverted = iconv(sConv, &p_in, &size_in, &p_out, &size_out);
        if (i <= 127)
        {
            assert(out[0] == i);
            continue;
        }
        if (nConverted != (size_t)-1)
        {
            const size_t needed = sizeof(out) - size_out;
            assert(needed <= 3);
            fprintf(c_file, "  {0x%02X, 0x%02X, 0x%02X},\n", out[0], out[1],
                    out[2]);
        }
        else
        {
            fprintf(c_file, "  {0, 0, 0}, /* invalid */\n");
        }
    }
    fprintf(c_file, "};\n\n");
    iconv_close(sConv);
}

int main()
{
    FILE *c_file = fopen("cpl_character_sets.c", "wb");
    FILE *h_file = fopen("cpl_character_sets.h", "wb");
    fprintf(c_file, "/* This file has been generated by "
                    "generate_character_set_conv_tables.c */\n");
    fprintf(c_file, "/* DO NOT EDIT !*/\n\n");
    fprintf(c_file, "/* clang-format off */\n");
    fprintf(c_file, "#include \"cpl_port.h\"\n");
    fprintf(c_file, "#include \"cpl_character_sets.h\"\n\n");

    fprintf(h_file, "/* This file has been generated by "
                    "generate_character_set_conv_tables.c */\n");
    fprintf(h_file, "/* DO NOT EDIT !*/\n\n");
    fprintf(h_file, "/* clang-format off */\n");
    fprintf(h_file, "typedef unsigned char CPLCodePageConvTable[128][3];\n");

    const struct
    {
        const char *name;
        const char *comment;
    } encodings[] = {
        {"CP437", "Character set of original IBM PC"},
        {"CP1250", "Central and eastern Europe languages"},
        {"CP1251", "Cyrillic script"},
        {"CP1252",
         "Legacy Windows single-byte character set used in a lot of countries"},
        {"ISO-8859-2", "Central Europe languages"},
        {"ISO-8859-15", "New Western Europe"},
        {NULL, NULL}};

    for (int i = 0; encodings[i].name; ++i)
    {
        generate(c_file, h_file, encodings[i].name, encodings[i].comment);
    }
    fprintf(h_file, "\n");
    fprintf(h_file, "const CPLCodePageConvTable* "
                    "CPLGetConversionTableToUTF8(const char* pszEncoding);\n");

    fprintf(c_file, "\nconst CPLCodePageConvTable* "
                    "CPLGetConversionTableToUTF8(const char* pszEncoding)\n");
    fprintf(c_file, "{\n");
    for (int i = 0; encodings[i].name; ++i)
    {
        char srcEncodingLaundered[ENCODING_MAX_LEN];
        launder_name(encodings[i].name, srcEncodingLaundered);
        fprintf(c_file, "    if (EQUAL(pszEncoding, \"%s\"))\n",
                encodings[i].name);
        fprintf(c_file, "        return &CPL_%s_to_UTF8;\n",
                srcEncodingLaundered);
    }
    fprintf(c_file, "    return CPL_NULLPTR;\n");
    fprintf(c_file, "}\n");
    fprintf(c_file, "/* clang-format on */\n");
    fprintf(h_file, "/* clang-format on */\n");
    fclose(c_file);
    fclose(h_file);
    return 0;
}