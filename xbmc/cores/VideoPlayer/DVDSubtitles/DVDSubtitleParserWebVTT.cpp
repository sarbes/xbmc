/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "DVDSubtitleParserWebVTT.h"
#include "DVDCodecs/Overlay/DVDOverlayText.h"
#include "cores/VideoPlayer/Interface/Addon/TimingConstants.h"
#include "utils/StringUtils.h"
#include "DVDSubtitleTagSami.h"

CDVDSubtitleParserWebVTT::CDVDSubtitleParserWebVTT(std::unique_ptr<CDVDSubtitleStream> && pStream, const std::string& strFile)
    : CDVDSubtitleParserText(std::move(pStream), strFile)
{
}

CDVDSubtitleParserWebVTT::~CDVDSubtitleParserWebVTT()
{
  Dispose();
}

bool CDVDSubtitleParserWebVTT::Open(CDVDStreamInfo &hints)
{
  if (!CDVDSubtitleParserText::Open())
    return false;

  CDVDSubtitleTagSami TagConv;
  if (!TagConv.Init())
    return false;

  char line[1024];
  std::string strLine;

  while (m_pStream->ReadLine(line, sizeof(line)))
  {
    strLine = line;
    StringUtils::Trim(strLine);

    if (strLine.length() > 0)
    {
      char from[13], to[13], alignment[128];
      int c;
      char sep;
      int aa, bb, cc, dd;
      int hh1, mm1, ss1, ms1, hh2, mm2, ss2, ms2;

      c = sscanf(strLine.c_str(), "%12s --> %12s %127s", from, to, alignment);
      if (c == 1) continue;

      c = sscanf(from, "%d%c%d%c%d%c%d", &aa, &sep, &bb, &sep, &cc, &sep, &dd);
      if (c == 5)
      {
        hh1 = 0;
        mm1 = aa;
        ss1 = bb;
        ms1 = cc;
      }
      else if (c == 7)
      {
        hh1 = aa;
        mm1 = bb;
        ss1 = cc;
        ms1 = dd;
      }
      else continue;

      c = sscanf(to, "%d%c%d%c%d%c%d", &aa, &sep, &bb, &sep, &cc, &sep, &dd);
      if (c == 5)
      {
        hh2 = 0;
        mm2 = aa;
        ss2 = bb;
        ms2 = cc;
      }
      else if (c == 7)
      {
        hh2 = aa;
        mm2 = bb;
        ss2 = cc;
        ms2 = dd;
      }
      else continue;      

      CDVDOverlayText* pOverlay = new CDVDOverlayText();
      pOverlay->Acquire(); // increase ref count with one so that we can hold a handle to this overlay

      pOverlay->iPTSStartTime = ((double)(((hh1 * 60 + mm1) * 60) + ss1) * 1000 + ms1) * (DVD_TIME_BASE / 1000);
      pOverlay->iPTSStopTime  = ((double)(((hh2 * 60 + mm2) * 60) + ss2) * 1000 + ms2) * (DVD_TIME_BASE / 1000);

      while (m_pStream->ReadLine(line, sizeof(line)))
      {
        strLine = line;
        StringUtils::Trim(strLine);

        // empty line, next subtitle is about to start
        if (strLine.length() <= 0) break;

        TagConv.ConvertLine(pOverlay, strLine.c_str(), strLine.length());
      }
      TagConv.CloseTag(pOverlay);
      m_collection.Add(pOverlay);
    }
  }
  m_collection.Sort();
  return true;
}

