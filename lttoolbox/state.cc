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
#include <lttoolbox/state.h>

#include <cstring>
#include <cwctype>
#include <climits>

//debug//
//#include <iostream>
//using namespace std;
//debug//

State::State(Pool<vector<int> > *p)
{
  pool = p;
}
 
State::~State()
{
  destroy();
}

State::State(State const &s)
{
  copy(s);
}

State &
State::operator =(State const &s)
{
  if(this != &s)
  {
    destroy();
    copy(s);
  }

  return *this;
}

void 
State::destroy()
{
  // release references
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    pool->release(state[i].sequence);
  }

  state.clear();
}

void
State::copy(State const &s)
{
  // release references
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    pool->release(state[i].sequence);
  }

  state = s.state;
  pool = s.pool;

  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    vector<int> *tmp = pool->get();
    *tmp = *(state[i].sequence);
    state[i].sequence = tmp;
  }
}

int 
State::size() const
{
  return state.size();
}

void
State::init(Node *initial)
{
  state.clear();
  state.push_back(TNodeState(initial,pool->get(),false));
  state[0].sequence->clear();
  epsilonClosure();  
}  

/*
void
State::apply(wstring const input, map<int, MatchExe> &t, Alphabet &a, FILE *err)
{
  vector<TNodeState> new_state;
  fwprintf(err, L" apply: %S\n", input.c_str());
  if(input == L"")
  {
    state = new_state;
    return;
  }
  
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    map<int, Dest>::const_iterator it;

    //it = state[i].where->transitions.find(input);
    for(map<int, Dest>::const_iterator it = state[i].where->transitions.begin(); 
        it != state[i].where->transitions.end(); it++)
    { 
      MatchExe me = t[it->first];
      MatchState ms;
      fwprintf(err, L"    it->first: %d\n", it->first);
      bool found = false;
      ms.clear();
      ms.init(me.getInitial());
     
      for(wstring::const_iterator it9 = input.begin(); it9 != input.end(); it9++) 
      {
        fwprintf(err, L"    %C:%C = %d (%d)\n", *it9, *it9, a(*it9, *it9), ms.size());
        if(ms.size() == 0)
        { 
          break;
        }
        ms.step(a(*it9, *it9));
      } 
      int val = ms.classifyFinals(me.getFinals());
      if(val != -1) 
      { 
        found = true;
      }

      //found = t[it->first].recognise(input, a, err); 
      wstring sym = L"";
      a.getSymbol(sym, it->first, false);
      fwprintf(err, L"  state: %d, transition: %d\n", i, it->first);
      fwprintf(err, L"  recognise(%S, %S) = %d ", sym.c_str(), input.c_str(), found);
    
      // if recognised
      // if(it != state[i].where->transitions.end())
      if(found)
      {
        for(int j = 0; j != it->second.size; j++)
        {
          vector<int> *new_v = pool->get();
          *new_v = *(state[i].sequence);
          if(it->first != 0)
          {
            new_v->push_back(it->second.out_tag[j]);
          }
          new_state.push_back(TNodeState(it->second.dest[j], new_v, state[i].dirty||false));
        }
        fwprintf(err, L"recognised.\n");
      }
      else
      {
        fwprintf(err, L"not recognised.\n");
      }
    }
    pool->release(state[i].sequence);
  }
  
  state = new_state;
}
*/


void
State::apply(wstring const input, map<int, Transducer> &t, Alphabet &a, FILE *err)
{
  vector<TNodeState> new_state;
  //fwprintf(err, L" apply: %S\n", input.c_str());
  if(input == L"")
  {
    state = new_state;
    return;
  }
  
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    map<int, Dest>::const_iterator it;

    //it = state[i].where->transitions.find(input);
    for(map<int, Dest>::const_iterator it = state[i].where->transitions.begin(); 
        it != state[i].where->transitions.end(); it++)
    { 
      bool found = false;
      found = t[it->first].recognise(input, a, err); 
      wstring sym = L"";
      a.getSymbol(sym, it->first, false);
      //fwprintf(err, L"  state: %d, transition: %d, tsize: %d\n", i, it->first, t[it->first].size());
      //fwprintf(err, L"  recognise(%S, %S) = %d ", sym.c_str(), input.c_str(), found);
    
      // if recognised
      // if(it != state[i].where->transitions.end())
      if(found)
      {
        for(int j = 0; j != it->second.size; j++)
        {
          vector<int> *new_v = pool->get();
          *new_v = *(state[i].sequence);
          if(it->first != 0)
          {
            new_v->push_back(it->second.out_tag[j]);
          }
          new_state.push_back(TNodeState(it->second.dest[j], new_v, state[i].dirty||false));
        }
        //fwprintf(err, L"recognised.\n");
      }
      else
      {
        //fwprintf(err, L"not recognised.\n");
      }
  
    }
      pool->release(state[i].sequence);
  }
  
  state = new_state;
}

void
State::apply(int const input)
{
  vector<TNodeState> new_state;
  if(input == 0)
  {
    state = new_state;
    return;
  }
  
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    map<int, Dest>::const_iterator it;
    it = state[i].where->transitions.find(input);
    if(it != state[i].where->transitions.end())
    {
      for(int j = 0; j != it->second.size; j++)
      {
        vector<int> *new_v = pool->get();
        *new_v = *(state[i].sequence);
        if(it->first != 0)
        {
          new_v->push_back(it->second.out_tag[j]);
        }
        new_state.push_back(TNodeState(it->second.dest[j], new_v, state[i].dirty||false));
      }
    }
    pool->release(state[i].sequence);
  }
  
  state = new_state;
}

void 
State::apply(int const input, int const alt)
{
  vector<TNodeState> new_state;
  if(input == 0 || alt == 0)
  {
    state = new_state;
    return;
  }

  
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    map<int, Dest>::const_iterator it;
    it = state[i].where->transitions.find(input);
    if(it != state[i].where->transitions.end())
    {
      for(int j = 0; j != it->second.size; j++)
      {
        vector<int> *new_v = pool->get();
        *new_v = *(state[i].sequence);
        if(it->first != 0)
        {
          new_v->push_back(it->second.out_tag[j]);
        }
        new_state.push_back(TNodeState(it->second.dest[j], new_v, state[i].dirty||false));
      }
    }
    it = state[i].where->transitions.find(alt);
    if(it != state[i].where->transitions.end())
    {
      for(int j = 0; j != it->second.size; j++)
      {
        vector<int> *new_v = pool->get();
        *new_v = *(state[i].sequence);
        if(it->first != 0)
        {
          new_v->push_back(it->second.out_tag[j]);
        }
        new_state.push_back(TNodeState(it->second.dest[j], new_v, true));
      }
    }
    pool->release(state[i].sequence);
  }

  state = new_state;
}

void
State::epsilonClosure()
{
  for(size_t i = 0; i != state.size(); i++)
  {
    map<int, Dest>::iterator it2;
    it2 = state[i].where->transitions.find(0);
    if(it2 != state[i].where->transitions.end())
    {
      for(int j = 0 ; j != it2->second.size; j++)
      {
        vector<int> *tmp = pool->get();
        *tmp = *(state[i].sequence);
        if(it2->second.out_tag[j] != 0)
        {
	  tmp->push_back(it2->second.out_tag[j]);
        }
        state.push_back(TNodeState(it2->second.dest[j], tmp, state[i].dirty));
      }          
    }
  }
}

/*
void
State::step(wstring const input, map<int, MatchExe> &transducers, Alphabet &a, FILE *err)
{
  apply(input, transducers, a, err);
  epsilonClosure();
}
*/

void
State::step(wstring const input, map<int, Transducer> &transducers, Alphabet &a, FILE *err)
{
  apply(input, transducers, a, err);
  epsilonClosure();
}

void
State::step(int const input)
{
  apply(input);
  epsilonClosure();
}

void
State::step(int const input, int const alt)
{
  apply(input, alt);
  epsilonClosure();
}


void 
State::step_case(wchar_t val, bool caseSensitive) 
{
  if (!iswupper(val) || caseSensitive) {
    step(val);
  } else {
    step(val, towlower(val));
  }
}


bool
State::isFinal(set<Node *> const &finals) const
{
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    if(finals.find(state[i].where) != finals.end())
    {
      return true;
    }
  }
  
  return false;
}

wstring
State::filterFinals(set<Node *> const &finals, 
		    Alphabet const &alphabet,
		    set<wchar_t> const &escaped_chars,
		    bool uppercase, bool firstupper, int firstchar) const
{
  wstring result = L"";

  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    if(finals.find(state[i].where) != finals.end())
    {
      if(state[i].dirty)
      {
        result += L'/';
        unsigned int const first_char = result.size() + firstchar;
        for(size_t j = 0, limit2 = state[i].sequence->size(); j != limit2; j++)
        {
          if(escaped_chars.find((*(state[i].sequence))[j]) != escaped_chars.end())
          {
            result += L'\\';
          }
          alphabet.getSymbol(result, (*(state[i].sequence))[j], uppercase);
        }
        if(firstupper)
        {
  	  if(result[first_char] == L'~')
	  {
	    // skip post-generation mark
	    result[first_char+1] = towupper(result[first_char+1]);
	  }
	  else
	  {
            result[first_char] = towupper(result[first_char]);
	  }
        }
      }
      else
      {
        result += L'/';
        for(size_t j = 0, limit2 = state[i].sequence->size(); j != limit2; j++)
        {
          if(escaped_chars.find((*(state[i].sequence))[j]) != escaped_chars.end())
          {
            result += L'\\';
          }
          alphabet.getSymbol(result, (*(state[i].sequence))[j]);
        }
      }
    }
  }
  
  return result;
}

wstring
State::filterFinalsSAO(set<Node *> const &finals, 
		       Alphabet const &alphabet,
		       set<wchar_t> const &escaped_chars,
		       bool uppercase, bool firstupper, int firstchar) const
{
  wstring result = L"";
  wstring annot = L"";
  
  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    if(finals.find(state[i].where) != finals.end())
    {
      result += L'/';
      unsigned int const first_char = result.size() + firstchar;
      for(size_t j = 0, limit2 = state[i].sequence->size(); j != limit2; j++)
      {
        if(escaped_chars.find((*(state[i].sequence))[j]) != escaped_chars.end())
        {
          result += L'\\';
        }
        if(alphabet.isTag((*(state[i].sequence))[j]))
        {
          annot = L"";
          alphabet.getSymbol(annot, (*(state[i].sequence))[j]);
          result += L'&'+annot.substr(1,annot.length()-2)+L';';
        }
        else
        {
          alphabet.getSymbol(result, (*(state[i].sequence))[j], uppercase);
        }
      }
      if(firstupper)
      {
	if(result[first_char] == L'~')
	{
	  // skip post-generation mark
	  result[first_char+1] = towupper(result[first_char+1]);
	}
	else
	{
          result[first_char] = towupper(result[first_char]);
	}
      }
    }
  }
  
  return result;
}

wstring
State::filterFinalsTM(set<Node *> const &finals, 
		      Alphabet const &alphabet,
                      set<wchar_t> const &escaped_chars,
                      queue<wstring> &blankqueue, vector<wstring> &numbers) const
{
  wstring result = L"";

  for(size_t i = 0, limit = state.size(); i != limit; i++)
  {
    if(finals.find(state[i].where) != finals.end())
    {
      result += L'/';
      for(size_t j = 0, limit2 = state[i].sequence->size(); j != limit2; j++)
      {
        if(escaped_chars.find((*(state[i].sequence))[j]) != escaped_chars.end())
        {
          result += L'\\';
        }
        alphabet.getSymbol(result, (*(state[i].sequence))[j]);
      }
    }
  }


  wstring result2 = L"";
  vector<wstring> fragmentos;
  fragmentos.push_back(L"");
 
  for(unsigned int i = 0, limit = result.size(); i != limit ; i++)
  {
    if(result[i] == L')')
    {
      fragmentos.push_back(L"");
    }
    else
    {
      fragmentos[fragmentos.size()-1] += result[i];
    }
  }
  
  for(unsigned int i = 0, limit = fragmentos.size(); i != limit; i++)
  {
    if(i != limit -1)
    {
      if(fragmentos[i].size() >=2 && fragmentos[i].substr(fragmentos[i].size()-2) == L"(#")
      {
        wstring whitespace = L" ";
        if(blankqueue.size() != 0)
	{
          whitespace = blankqueue.front().substr(1);
	  blankqueue.pop();
	  whitespace = whitespace.substr(0, whitespace.size() - 1);
        }  
        fragmentos[i] = fragmentos[i].substr(0, fragmentos[i].size()-2) +
	                whitespace;
      }
      else
      {
        bool sustituido = false;
	for(int j = fragmentos[i].size() - 1; j >= 0; j--)
	{
	  if(fragmentos[i].size()-j > 3 && fragmentos[i][j] == L'\\' && 
	     fragmentos[i][j+1] == L'@' && fragmentos[i][j+2] == L'(')
	  {
	    int num = 0;
	    bool correcto = true;
	    for(unsigned int k = (unsigned int) j+3, limit2 = fragmentos[i].size();
		k != limit2; k++)
	    {
	      if(iswdigit(fragmentos[i][k]))
	      {
		num = num * 10;
		num += (int) fragmentos[i][k] - 48;	
	      }
	      else
	      {
		correcto = false;
		break;
	      }
	    }
	    if(correcto)
	    {
	      fragmentos[i] = fragmentos[i].substr(0, j) + numbers[num - 1];
	      sustituido = true;
	      break;
	    }
	  }
	}
	if(sustituido == false)
	{
	  fragmentos[i] += L')';
	}
      }
    }    
  }
  
  result = L"";

  for(unsigned int i = 0, limit = fragmentos.size(); i != limit; i++)
  {
    result += fragmentos[i];
  }
  
  return result;
}



void
State::pruneCompounds(int requiredSymbol, int separationSymbol, int compound_max_elements) 
{
  int minNoOfCompoundElements = compound_max_elements;
  int *noOfCompoundElements = new int[state.size()];

  //wcerr << L"pruneCompounds..." << endl;

  for (unsigned int i = 0;  i<state.size(); i++) {
    vector<int> seq = *state.at(i).sequence;

    if (lastPartHasRequiredSymbol(seq, requiredSymbol, separationSymbol)) {
      int this_noOfCompoundElements = 0;
      for (int j = seq.size()-2; j>0; j--) if (seq.at(j)==separationSymbol) this_noOfCompoundElements++;
      noOfCompoundElements[i] = this_noOfCompoundElements;
      minNoOfCompoundElements = (minNoOfCompoundElements < this_noOfCompoundElements) ? 
                        minNoOfCompoundElements : this_noOfCompoundElements;
    }
    else {
      noOfCompoundElements[i] = INT_MAX;
		  //wcerr << L"Prune - No requiered symbol in state number " << i << endl;
    }
  }

  // remove states with more than minimum number of compounds (or without the requiered symbol in the last part)
  vector<TNodeState>::iterator it = state.begin();
  int i=0;
  while(it != state.end()) {
    if (noOfCompoundElements[i] > minNoOfCompoundElements) {
      it = state.erase(it);
      //wcerr << L"Prune - State number " << i << L" removed!" << endl;
    }
    else it++;
    i++;
  }

 delete[] noOfCompoundElements;
}



void
State::pruneStatesWithForbiddenSymbol(int forbiddenSymbol) 
{
  vector<TNodeState>::iterator it = state.begin();
  while(it != state.end()) {
    vector<int> *seq = (*it).sequence;
    bool found = false;
    for(int i = seq->size()-1; i>=0; i--) {
      if(seq->at(i) == forbiddenSymbol) {
        i=-1;
        it = state.erase(it);
        found = true;
      }
    }
    if (!found) it++;
  }
}



bool
State::lastPartHasRequiredSymbol(const vector<int> &seq, int requiredSymbol, int separationSymbol) 
{
  // state is final - it should be restarted it with all elements in stateset restart_state, with old symbols conserved
  bool restart=false;
  for (int n=seq.size()-1; n>=0; n--) {
    int symbol=seq.at(n);
    if (symbol==requiredSymbol) {
      restart=true;
      break;
    }
    if (symbol==separationSymbol) {
      break;
    }
  }
  return restart;
}


void
State::restartFinals(const set<Node *> &finals, int requiredSymbol, State *restart_state, int separationSymbol) 
{

  for (unsigned int i=0;  i<state.size(); i++) {
    TNodeState state_i = state.at(i);
    // A state can be a possible final state and still have transitions

    if (finals.count(state_i.where) > 0) {
      bool restart = lastPartHasRequiredSymbol(*(state_i.sequence), requiredSymbol, separationSymbol);
      if (restart) {
        if (restart_state != NULL) {
          for (unsigned int j=0; j<restart_state->state.size(); j++) {
            TNodeState initst = restart_state->state.at(j);
            vector<int> *tnvec = new vector<int>;

            for(unsigned int k=0; k < state_i.sequence->size(); k++) tnvec->push_back(state_i.sequence->at(k));
            TNodeState tn(initst.where, tnvec, state_i.dirty);
            tn.sequence->push_back(separationSymbol);
            state.push_back(tn);
            }
          }
        }
      }
    }
}



wstring
State::getReadableString(const Alphabet &a) 
{
  wstring retval = L"[";

  for(unsigned int i=0; i<state.size(); i++) {
    vector<int>* seq = state.at(i).sequence;
    if(seq != NULL) for (unsigned int j=0; j<seq->size(); j++) {
      wstring ws = L"";
      a.getSymbol(ws, seq->at(j));
      //if(ws == L"") ws = L"?";
      retval.append(ws);
    }

    /*Node *where = state.at(i).where;
    if(where == NULL) retval.append(L"→@null");
    else {
      retval.append(L"→");
      map<int, Dest>::iterator it;
      wstring ws;
      for (it = where->transitions.begin(); it != where->transitions.end(); it++) {
        int symbol = (*it).first;
        a.getSymbol(ws, symbol);
        retval.append(ws);
      }
    }*/
    if (i+1 < state.size()) retval.append(L", ");
  }
  retval.append(L"]");
  return retval;
}

