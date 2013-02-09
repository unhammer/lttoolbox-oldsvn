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
#include <lttoolbox/alphabet.h>
#include <lttoolbox/lttoolbox_config.h>
#include <lttoolbox/my_stdio.h>
#include <lttoolbox/state.h>

#include <cstdlib>
#include <iostream>
#include <vector>


void
TrimTransducer::trim(Alphabet const &alph, FSTProcessor const &trim_to, int const epsilon_tag)
{
  joinFinals(epsilon_tag);      // TODO necessary?

  typedef std::pair<int, State*> untrPair;
  std::list<untrPair> untrimmed; // used last-in-first-out / depth-first to avoid memory blow-up

  untrimmed.push_front(untrPair(getInitial(), trim_to.getInitial()));
  while(untrimmed.size() > 0)
  {
    untrPair auxest = untrimmed.back();
    untrimmed.pop_back();

    int current = auxest.first;
    //wcout << L"from " << current << L"\ttransitions.size(): " << transitions.size() << endl;

    std::map<int, multimap<int, int> >::const_iterator it = transitions.find(current);
    if(it == transitions.end())
    {
      wcerr << L"ERROR: Didn't find referenced state " << current << endl;
      exit(EXIT_FAILURE);
    }
    std::multimap<int, int> edges = it->second;

    State* current_trim_to = auxest.second;
    //wcout <<L"analysed so far: " <<current_trim_to->getReadableString(trim_to.getAlphabet()) << endl;

    for(std::multimap<int, int>::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
      int label = edge->first;
      std::pair<int, int> p = alph.decode(label);
      int to_state = edge->second;

      // There's no functional step method, copy manually :-/
      State* next_trim_to = new State(*current_trim_to);
      next_trim_to->step(p.second);

      //wcout<<L"from "<<current<<L" ("<<p.first<<L":"<<p.second<<L")/"<<label<<L" to "<<to_state<<endl;

      if(alph.isTag(p.second) && next_trim_to->isFinal(trim_to.getAllFinals()))
      {
        //wcout << L"at a tag and isFinal, stop trimming this path"<<endl;
      }
      else
      {
        wstring r=L""; alph.getSymbol(r, p.second);
        if(next_trim_to->size() == 0) {
          //wcout<<L"\t"<<r<<L" ->trim"<<endl;
          edges.erase(label);
          transitions[current] = edges;
        }
        else
        {
          //wcout<<L"\t"<<r<<L" ->stepping"<<endl;
          untrimmed.push_front(untrPair(to_state, next_trim_to));
        }
      }
    }
    if(current_trim_to != trim_to.getInitial())
    {
      // TODO correct?
      delete current_trim_to;
    }
    //wcout <<untrimmed.size()<<endl;
  }
}
