/*
 *      Copyright (C) 2010-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#version 100

attribute vec2 m_attrpos;
attribute vec4 m_attrcol;
attribute vec2 m_attrcord0;
attribute vec2 m_attrcord1;
varying vec4 m_cord0;
varying vec4 m_cord1;
varying lowp vec4 m_colour;
uniform vec2 m_size;
uniform vec2 m_position;

void main ()
{
  vec2 pos = m_size * m_attrpos + m_position;
  gl_Position = vec4(pos*(1920./2.),28.,1920./2.);
  m_colour = m_attrcol;
  m_cord0     = vec4(m_attrcord0, 0., 0.);
  m_cord1     = vec4(m_attrcord1, 0., 0.);
}
