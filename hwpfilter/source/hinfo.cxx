/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include    "precompile.h"

#include    "hwplib.h"
#include    "hinfo.h"
#include    "hwpfile.h"

// Info Block


static bool HWPReadInfoBlock(void *ptr, int len, HWPFile & hwpf)
{
    hwpf.info_block_len = len;
    if (0 == len)
        return true;
    else
        return hwpf.ReadBlock(ptr, len) ? true : false;
}


// Document Information

HWPInfo::HWPInfo(void)
{
    info_block = 0;
     back_info.isset = false;
}


HWPInfo::~HWPInfo(void)
{
    if (info_block)
        delete[]info_block;
    info_block = 0;
}


/**
 * ���������� �о���̴� �Լ� ( 128 bytes )
 * ���������� �����ν�����( 30 bytes ) ������ ��ġ�� �����̴�.
 */
bool HWPInfo::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&cur_col, 1);                     /* ������ ������ ����� Ŀ���� ��ġ�� ���ܹ�ȣ */
    hwpf.Read2b(&cur_row, 1);                     /* ���� ĭ */

    hwpf.Read1b(&paper.paper_kind, 1);            /* ���� ���� */
    hwpf.Read1b(&paper.paper_direction, 1);       /* ���� ���� */

// paper geometry information
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.paper_height = tmp16;                   /* �� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.paper_width = tmp16;                    /* �� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.top_margin = tmp16;                     /* �� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.bottom_margin = tmp16;                  /* ��� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.left_margin = tmp16;                    /* �� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.right_margin = tmp16;                   /* ��� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.header_length = tmp16;                  /* ��� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.footer_length = tmp16;                  /* ��� �� */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.gutter_length = tmp16;                  /* ���� */
    hwpf.Read2b(&readonly, 1);                    /* ���� */
    hwpf.Read1b(reserved1, 4);                    /* ���� */
    hwpf.Read1b(&chain_info.chain_page_no, 1);    /* �� ��ȣ ���� 1-����, 0-���ν��� (�����μ⿡�� ���) */
    hwpf.Read1b(&chain_info.chain_footnote_no, 1);/* ���ֹ�ȣ ���� 1-���� 0-���ν��� */
                                                  /* �����μ��� ������ �̸� */
    hwpf.Read1b(chain_info.chain_filename, CHAIN_MAX_PATH);

    hwpf.Read1b(annotation, ANNOTATION_LEN);      /* �����̴� �� ( ���� ������ �� �����̴� ���� ������ ���� ) */
    hwpf.Read2b(&encrypted, 1);                   /* ��ȣ ���� 0-��������, �׿�-��ȣ�ɸ� ���� */
//hwpf.Read1b(reserved2, 6);                      /* �Ʒ� 3���ǰ����� �ٲ����. */
    hwpf.Read2b(&beginpagenum,1);                 /* ���������۹�ȣ */

// footnote
    hwpf.Read2b(&beginfnnum,1);                   /* ���� ���۹�ȣ */
    hwpf.Read2b(&countfn,1);                      /* ���� ���� */

    if (!hwpf.Read2b(tmp16))
        return false;
    splinetext = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    splinefn = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    spfnfn = tmp16;
    hwpf.Read1b(&fnchar, 1);
    hwpf.Read1b(&fnlinetype, 1);
// border layout
    for (int ii = 0; ii < 4; ++ii)
    {
        if (!hwpf.Read2b(tmp16))
            return false;
        bordermargin[ii] = tmp16;
    }
    hwpf.Read2b(&borderline, 1);

    hwpf.Read1b(&empty_line_hide, 1);
    hwpf.Read1b(&table_move, 1);
    hwpf.Read1b(&compressed, 1);

    hwpf.Read1b(&reserved3, 1);

    hwpf.Read2b(&info_block_len, 1);
    if (hwpf.State())
        return false;

/* ���� ����� �д´�. */
    if (!summary.Read(hwpf))
        return false;
    if (info_block_len > 0)
    {
        info_block = new unsigned char[info_block_len + 1];

        if (0 == info_block ||
            !HWPReadInfoBlock(info_block, info_block_len, hwpf))
            return false;
    }

/* hwpf�� ���� �缳�� �Ѵ�. */
    hwpf.compressed = compressed ? true : false;
    hwpf.encrypted = encrypted ? true : false;
    hwpf.info_block_len = info_block_len;
    hwpf.SetCompressed(hwpf.compressed);

    return (!hwpf.State());
}


// Document Summary

bool HWPSummary::Read(HWPFile & hwpf)
{
    hwpf.Read2b(title, 56);
    hwpf.Read2b(subject, 56);
    hwpf.Read2b(author, 56);
    hwpf.Read2b(date, 56);
    hwpf.Read2b(keyword[0], 56);
    hwpf.Read2b(keyword[1], 56);
    hwpf.Read2b(etc[0], 56);
    hwpf.Read2b(etc[1], 56);
    hwpf.Read2b(etc[2], 56);

    return (!hwpf.State());
}


bool ParaShape::Read(HWPFile & hwpf)
{
    pagebreak = 0;
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    left_margin = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    right_margin = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    indent = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    lspacing = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    pspacing_next = tmp16;

    hwpf.Read1b(&condense, 1);
    hwpf.Read1b(&arrange_type, 1);
    for (int ii = 0; ii < MAXTABS; ii++)
    {
        hwpf.Read1b(&tabs[ii].type, 1);
        hwpf.Read1b(&tabs[ii].dot_continue, 1);
        if (!hwpf.Read2b(tmp16))
            return false;
        tabs[ii].position = tmp16;
    }
    hwpf.Read1b(&coldef.ncols, 1);
    hwpf.Read1b(&coldef.separator, 1);
    if (!hwpf.Read2b(tmp16))
        return false;
    coldef.spacing = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    coldef.columnlen = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    coldef.columnlen0 = tmp16;
    hwpf.Read1b(&shade, 1);
    hwpf.Read1b(&outline, 1);
    hwpf.Read1b(&outline_continue, 1);
    if (!hwpf.Read2b(tmp16))
        return false;
    pspacing_prev = tmp16;

    hwpf.Read1b(reserved, 2);
    return (!hwpf.State());
}


bool CharShape::Read(HWPFile & hwpf)
{
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    size = tmp16;
    hwpf.Read1b(font, NLanguage);
    hwpf.Read1b(ratio, NLanguage);
    hwpf.Read1b(space, NLanguage);
    hwpf.Read1b(color, 2);
    hwpf.Read1b(&shade, 1);
    hwpf.Read1b(&attr, 1);
    hwpf.Read1b(reserved, 4);

    return (!hwpf.State());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
