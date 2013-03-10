/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#include <cstdio>
#include <map>
#include <set>

#include <lttoolbox/transducer.h>

/**
 * Class to represent a letter transducer during the dictionary trimming
 */
class TrimTransducer : public Transducer
{

public:
  /**
   * Destructively Trim right-sides of this transducer down to
   * left-sides of another.
   *
   * TODO: words with spaces, +, # 
   *
   * @param alph alphabet of this transducer
   * @param trim_to other transducer to trim to, typically a bidix
   * @param epsilon_tag the tag to take as epsilon
   */
  Transducer trim(Alphabet const &alph, FSTProcessor const &trim_to, int const epsilon_tag = 0);

};