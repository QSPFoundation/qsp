/* Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru) */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "../../declarations.h"

#ifdef _ANDROID

#include "../../text.h"

#include <jni.h>
#include <string.h>
#include <android/log.h>

static int qspUTF8_mbtowc(int *pwc, unsigned char *s, int n)
{
	unsigned char c = s[0];
	if (c < 0x80)
	{
		*pwc = c;
		return 1;
	}
	else if (c < 0xc2)
		return 0;
	else if (c < 0xe0)
	{
		if (n < 2) return 0;
		if (!((s[1] ^ 0x80) < 0x40)) return 0;
		*pwc = ((int)(c & 0x1f) << 6)
			| (int)(s[1] ^ 0x80);
		return 2;
	}
	else if (c < 0xf0)
	{
		if (n < 3) return 0;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
			&& (c >= 0xe1 || s[1] >= 0xa0)))
			return 0;
		*pwc = ((int)(c & 0x0f) << 12)
			| ((int)(s[1] ^ 0x80) << 6)
			| (int)(s[2] ^ 0x80);
		return 3;
	}
	else if (c < 0xf8)
	{
		if (n < 4) return 0;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
			&& (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90)))
			return 0;
		*pwc = ((int)(c & 0x07) << 18)
			| ((int)(s[1] ^ 0x80) << 12)
			| ((int)(s[2] ^ 0x80) << 6)
			| (int)(s[3] ^ 0x80);
		return 4;
	}
	else if (c < 0xfc)
	{
		if (n < 5) return 0;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
			&& (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
			&& (c >= 0xf9 || s[1] >= 0x88)))
			return 0;
		*pwc = ((int)(c & 0x03) << 24)
			| ((int)(s[1] ^ 0x80) << 18)
			| ((int)(s[2] ^ 0x80) << 12)
			| ((int)(s[3] ^ 0x80) << 6)
			| (int)(s[4] ^ 0x80);
		return 5;
	}
	else if (c < 0xfe)
	{
		if (n < 6) return 0;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
			&& (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
			&& (s[5] ^ 0x80) < 0x40
			&& (c >= 0xfd || s[1] >= 0x84)))
			return 0;
		*pwc = ((int)(c & 0x01) << 30)
			| ((int)(s[1] ^ 0x80) << 24)
			| ((int)(s[2] ^ 0x80) << 18)
			| ((int)(s[3] ^ 0x80) << 12)
			| ((int)(s[4] ^ 0x80) << 6)
			| (int)(s[5] ^ 0x80);
		return 6;
	}
	else
		return 0;
}

static int qspUTF8_wctomb(unsigned char *r, int wc, int n)
{
	int count;
	if (wc < 0x80)
		count = 1;
	else if (wc < 0x800)
		count = 2;
	else if (wc < 0x10000)
		count = 3;
	else if (wc < 0x200000)
		count = 4;
	else if (wc < 0x4000000)
		count = 5;
	else if (wc <= 0x7fffffff)
		count = 6;
	else
		return 0;
	if (n < count) return 0;
	switch (count)
	{
	case 6: r[5] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x4000000;
	case 5: r[4] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x200000;
	case 4: r[3] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x10000;
	case 3: r[2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
	case 2: r[1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
	case 1: r[0] = wc;
	}
	return count;
}

char *qspW2C(QSP_CHAR *src)
{
	int ret;
	char *dst = (char *)malloc((qspStrLen(src) * 3) + 1);
	char *s = dst;
	while ((ret = qspUTF8_wctomb(s, *src, 3)) && *s)
	{
		++src;
		s += ret;
	}
	*s = 0;
	return dst;
}

QSP_CHAR *qspC2W(char *src)
{
	int ret, ch;
	QSP_CHAR *dst = (QSP_CHAR *)malloc((strlen(src) + 1) * sizeof(QSP_CHAR));
	QSP_CHAR *s = dst;
	while ((ret = qspUTF8_mbtowc(&ch, src, 3)) && ch)
	{
		*s++ = ch;
		src += ret;
	}
	*s = 0;
	return dst;
}

char *qspToSysString(QSP_CHAR *s)
{
	return qspW2C(s);
}

#endif
