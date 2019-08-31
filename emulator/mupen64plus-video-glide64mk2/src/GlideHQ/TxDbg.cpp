/*
 * Texture Filtering
 * Version:  1.0
 *
 * Copyright (C) 2007  Hiroshi Morii   All Rights Reserved.
 * Email koolsmoky(at)users.sourceforge.net
 * Web   http://www.3dfxzone.it/koolsmoky
 *
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Make; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define DBG_LEVEL 80

#include "TxDbg.h"
#include <string.h>
#include <stdarg.h>
#include <string>

TxDbg::TxDbg()
{
  _level = DBG_LEVEL;

#ifdef GHQCHK
  _dbgfile = fopen("ghqchk.txt", "w");
#else
  _dbgfile = fopen("glidehq.dbg", "w");
#endif
}

TxDbg::~TxDbg()
{
  if (_dbgfile) {
    fclose(_dbgfile);
    _dbgfile = 0;
  }

  _level = DBG_LEVEL;
}

void
TxDbg::output(const int level, const wchar_t *format, ...)
{
#if defined(_GLIBCXX_HAVE_BROKEN_VSWPRINTF) || defined(__clang__)
  wchar_t newformat[4095];
#else
  std::wstring newformat;
#endif

  va_list args;

  if (level > _level)
    return;

  va_start(args, format);
#if defined(_GLIBCXX_HAVE_BROKEN_VSWPRINTF) || defined(__clang__)
  #if defined(_GLIBCXX_HAVE_BROKEN_VSWPRINTF)
  swprintf(newformat, L"%d:\t", level);
  #else
  swprintf(newformat, 4095, L"%d:\t", level);
  #endif
  wcscat(newformat, format);
  vfwprintf(_dbgfile, newformat, args);
#else
  newformat = std::to_wstring(level) + L":\t" + format;
  vfwprintf(_dbgfile, newformat.c_str(), args);
#endif
  fflush(_dbgfile);
#ifdef GHQCHK
  //vwprintf(newformat, args);
  vwprintf(newformat.c_str(), args);
#endif
  va_end(args);
}
