//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Intuition Engine WAD I/O functions.
//

#include "config.h"

#ifdef INTUITION_ENGINE

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "i_intuition.h"
#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

#define IE_MAX_WAD_FILE_SIZE (64u * 1024u * 1024u)

typedef struct
{
    wad_file_t wad;
    byte *data;
} intuition_wad_file_t;

static wad_file_t *W_Intuition_OpenFile(const char *path)
{
    intuition_wad_file_t *result;
    byte *data;
    uint32_t len;

    data = Z_Malloc(IE_MAX_WAD_FILE_SIZE, PU_STATIC, 0);

    if (!IE_FileReadAll(path, data, IE_MAX_WAD_FILE_SIZE, &len))
    {
        Z_Free(data);
        return NULL;
    }

    result = Z_Malloc(sizeof(*result), PU_STATIC, 0);
    result->wad.file_class = &intuition_wad_file;
    result->wad.mapped = data;
    result->wad.length = len;
    result->wad.path = M_StringDuplicate(path);
    result->data = data;

    return &result->wad;
}

static void W_Intuition_CloseFile(wad_file_t *wad)
{
    intuition_wad_file_t *intuition_wad = (intuition_wad_file_t *) wad;

    free((void *) wad->path);
    Z_Free(intuition_wad->data);
    Z_Free(intuition_wad);
}

static size_t W_Intuition_Read(wad_file_t *wad, unsigned int offset,
                               void *buffer, size_t buffer_len)
{
    size_t available;

    if (offset >= wad->length)
    {
        return 0;
    }

    available = wad->length - offset;
    if (buffer_len > available)
    {
        buffer_len = available;
    }

    memcpy(buffer, wad->mapped + offset, buffer_len);
    return buffer_len;
}

wad_file_class_t intuition_wad_file =
{
    W_Intuition_OpenFile,
    W_Intuition_CloseFile,
    W_Intuition_Read,
};

#endif
