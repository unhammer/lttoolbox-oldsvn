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


Transducer
TrimTransducer::trim(Alphabet const &alph, FSTProcessor const &trim_to, int const epsilon_tag)
{
  bool DEBUG = true;

  Transducer trimmed;

  if(DEBUG) { wcout << L"full before min"<<endl; show(alph); }

  joinFinals(epsilon_tag);      // TODO necessary?

  Alphabet alph_trim_to = trim_to.getAlphabet();

  typedef std::pair< std::pair<int,int>, State*> untrPair;
  std::list<untrPair> untrimmed; // used last-in-first-out / depth-first to avoid memory blow-up
  std::map<int, int> seen;
  
  untrimmed.push_front(untrPair(std::pair<int,int>(getInitial(),
                                                   trimmed.getInitial()),
                                trim_to.getInitial()));
  while(untrimmed.size() > 0)
  {
    untrPair auxest = untrimmed.back();
    untrimmed.pop_back();

    std::pair<int,int> current = auxest.first;
    State* current_trim_to = auxest.second;
    
    if(DEBUG) wcout << L"from " << current.first << L"\ttrimmed.numberOfTransitions(): " << trimmed.numberOfTransitions() << L"\t";
    if(DEBUG) { if(current_trim_to == NULL) wcout <<L"copying all the way"<<endl; else wcout <<L"bidix analysed so far: " <<current_trim_to->getReadableString(trim_to.getAlphabet()) << endl; }

    std::map<int, multimap<int, int> >::const_iterator it = transitions.find(current.first);
    if(it == transitions.end())
    {
      wcerr << L"ERROR: Didn't find referenced state " << current.first << endl;
      exit(EXIT_FAILURE);
    }
    std::multimap<int, int> edges = it->second;
    for(std::multimap<int, int>::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
      int label = edge->first;
      std::pair<int, int> p = alph.decode(label);
      int to_state = edge->second;

      if(current_trim_to == NULL) 
      {
        int new_state = trimmed.insertSingleTransduction(label, current.second);
        seen[to_state] = new_state;
        untrimmed.push_front(untrPair(std::pair<int,int>(to_state,
                                                         new_state),
                                      NULL));
        if(isFinal(to_state)) 
        {
          trimmed.setFinal(new_state);
        }
        continue;
      }

      // There's no functional step method, copy manually :-/
      State* next_trim_to = new State(*current_trim_to);
      if(alph.isTag(p.second))
      {
        wstring r = L"";
        alph.getSymbol(r, p.second);
        next_trim_to->step(alph_trim_to(r));
      }
      else
      {
        next_trim_to->step(p.second);
      }

      if(DEBUG) wcout<<L"from "<<current.first<<L" ("<<p.first<<L":"<<p.second<<L")/"<<label<<L" to "<<to_state<<endl;
      if(DEBUG) wcout <<L"\tbidix analysed with this step: " <<next_trim_to->getReadableString(trim_to.getAlphabet()) << endl;
      if(DEBUG) { wstring r=L""; alph.getSymbol(r, p.second); wcout<<L"\t"<<r; }

      if(alph.isTag(p.second) && next_trim_to->isFinal(trim_to.getAllFinals()))
      {
        if(DEBUG) wcout << L"at a tag and isFinal, copy all the way"<<endl;
        int new_state = trimmed.insertSingleTransduction(label, current.second); // or insertNewSingleTransduction? TODO
        seen[to_state] = new_state;
        untrimmed.push_front(untrPair(std::pair<int,int>(to_state,
                                                         new_state),
                                      NULL));
        if(isFinal(to_state))
        {
          trimmed.setFinal(new_state);
        }
      }
      else if(next_trim_to->size() == 0)
      {
          if(DEBUG) wcout<<L" ->trim"<<endl;
      }
      else if(seen.count(to_state) == 0)
      {
        if(DEBUG) wcout<<L" ->stepping"<<endl;
        int new_state = trimmed.insertSingleTransduction(label, current.second);
        seen[to_state] = new_state;
        untrimmed.push_front(untrPair(std::pair<int,int>(to_state,
                                                         new_state),
                                      next_trim_to));
          
        if(isFinal(to_state)) 
        {
          trimmed.setFinal(new_state);
        }
      }
      else 
      {
        if(DEBUG) wcout<<L" ->linkStates "<<current.second<<L"→"<<seen[to_state]<<L" ("<<p.first<<L":"<<p.second<<L")"<<endl;
        trimmed.linkStates(current.second, seen[to_state], label);
      }
    }
    if(current_trim_to != trim_to.getInitial())
    {
      delete current_trim_to;   // TODO correct?
    }
  }

  if(DEBUG) { wcout << L"trimmed before min:"<<endl; trimmed.show(alph); }
  trimmed.minimize();
  if(DEBUG) { wcout << L"trimmed after min:"<<endl; trimmed.show(alph); }


  return trimmed;
}
