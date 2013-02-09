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
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/lttoolbox_config.h>

#include <lttoolbox/my_stdio.h>
#include <lttoolbox/lt_locale.h>

#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <string>

void endProgram(char *name)
{
  if(name != NULL)
  {
    cout << basename(name) << " v" << PACKAGE_VERSION <<": trim a transducer to another transducer" << endl;
    cout << "USAGE: " << basename(name) << " analyser_bin_file bidix_bin_file output_bin_file " << endl;
  }
  exit(EXIT_FAILURE);
}

std::pair<Alphabet, std::map<wstring, Transducer> >
read_fst(FILE *bin_file)
{
  Alphabet new_alphabet;
  set<wchar_t> alphabetic_chars;

  std::map<wstring, Transducer> transducers;

  // letters
  int len = Compression::multibyte_read(bin_file);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(bin_file)));
    len--;
  }

  // symbols
  new_alphabet.read(bin_file);

  len = Compression::multibyte_read(bin_file);

  while(len > 0)
  {
    int len2 = Compression::multibyte_read(bin_file);
    wstring name = L"";
    while(len2 > 0)
    {
      name += static_cast<wchar_t>(Compression::multibyte_read(bin_file));
      len2--;
    }
    transducers[name].read(bin_file);

    len--;
  }
  return std::pair<Alphabet, std::map<wstring, Transducer> > (new_alphabet, transducers);
}

void
trim(FILE *file_a, FILE *file_b)
{
  std::pair<Alphabet, std::map<wstring, Transducer> > alph_trans_a = read_fst(file_a);
  Alphabet alph_a = alph_trans_a.first;
  std::map<wstring, Transducer> trans_a = alph_trans_a.second;
  std::pair<Alphabet, std::map<wstring, Transducer> > alph_trans_b = read_fst(file_b);
  Alphabet alph_b = alph_trans_b.first;
  std::map<wstring, Transducer> trans_b = alph_trans_b.second;

  for(std::map<wstring, Transducer>::iterator it = trans_a.begin(); it != trans_a.end(); it++)
  {
    it->second.trim(alph_a, alph_b, trans_b);
  }
}


int main(int argc, char *argv[])
{
  if(argc != 4)
  {
    endProgram(argv[0]);
  }

  LtLocale::tryToSetLocale();

  FILE *analyser = fopen(argv[1], "rb");
  FILE *bidix = fopen(argv[2], "rb");
  FILE *output = fopen(argv[3], "wb");

  trim(analyser, bidix);
  // TODO write

  fclose(analyser);
  fclose(bidix);
  fclose(output);

  return 0;
}
