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
#include <lttoolbox/fst_processor.h>
#include <lttoolbox/trim_transducer.h>
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


std::pair<std::pair<wstring, Alphabet>, std::map<wstring, TrimTransducer> >
read_fst(FILE *bin_file)
{
  // like FSTProcessor::load, but return <<letters,Alphabet>,map::<name,Transducer>>
  Alphabet alphabet;

  // vector<wchar_t> alphabetic_chars;
  std::map<wstring, TrimTransducer> transducers;

  // letters
  int len = Compression::multibyte_read(bin_file);
  wchar_t alphabetic_chars[len];
  for(int i=0; i<len; i++) 
  {
    alphabetic_chars[i]= static_cast<wchar_t>(Compression::multibyte_read(bin_file));
  }
  wstring letters = wstring(alphabetic_chars, len);

  // symbols
  alphabet.read(bin_file);

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
  std::pair<wstring, Alphabet> letters_alph = std::pair<wstring, Alphabet>(letters, alphabet);
  return std::pair<std::pair<wstring, Alphabet>, map<wstring, TrimTransducer> >(letters_alph, transducers);
}


void checkValidity(FSTProcessor const &fstp)
{
  if(!fstp.valid())
  {
    exit(EXIT_FAILURE);
  }
}


int main(int argc, char *argv[])
{
  if(argc != 4)
  {
    endProgram(argv[0]);
  }

  LtLocale::tryToSetLocale();

  FILE *file_full = fopen(argv[1], "rb");
  if(file_full == NULL || ferror(file_full))
  {
    endProgram(argv[0]);
  }
  std::pair<std::pair<wstring, Alphabet>, map<wstring, TrimTransducer> > alph_trans_full = read_fst(file_full);
  std::pair<wstring, Alphabet> letters_alph_full = alph_trans_full.first;
  wstring letters_full = letters_alph_full.first;
  Alphabet alph_full = letters_alph_full.second;
  std::map<wstring, TrimTransducer> trans_full = alph_trans_full.second;
  fclose(file_full);

  FILE *file_trim_to = fopen(argv[2], "rb");
  if(file_trim_to == NULL || ferror(file_trim_to))
  {
    endProgram(argv[0]);
  }
  FSTProcessor trim_to;
  trim_to.load(file_trim_to);
  fclose(file_trim_to);

  trim_to.initBiltrans();
  checkValidity(trim_to);
  // TODO: how to deal with case conversions, ie. 'A' accepted as 'a'?


  std::map<wstring, Transducer> trans_trimmed = trans_trimmed;
  for(map<wstring, TrimTransducer>::iterator it = trans_full.begin(); it != trans_full.end(); it++)
  {
    trans_trimmed[it->first] = it->second.trim(alph_full, trim_to);
  }


  FILE *file_trimmed = fopen(argv[3], "wb");
  if(file_trimmed == NULL || ferror(file_trimmed))
  {
    endProgram(argv[0]);
  }

  // letters
  Compression::wstring_write(letters_full, file_trimmed);

  // symbols
  alph_full.write(file_trimmed);

  // transducers
  Compression::multibyte_write(trans_trimmed.size(), file_trimmed);

  int conta=0;
  for(map<wstring, Transducer>::iterator it = trans_trimmed.begin(); it != trans_trimmed.end(); it++)
  {
    conta++;
    wcout << it->first << " " << it->second.size();
    wcout << " " << it->second.numberOfTransitions() << endl;
    Compression::wstring_write(it->first, file_trimmed);
    it->second.write(file_trimmed);
  }

  fclose(file_trimmed);

  return 0;
}
