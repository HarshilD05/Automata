#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <deque>

using namespace std;

enum StateType { INIT, BRANCH, SOL, TERM };

typedef struct State {
  string _data;
  StateType _t;

  State(string data, StateType type = BRANCH) : _data(data), _t(type) {}
} State;

void DisplayState(const State s) {
  cout << "Data : " << s._data << endl;
  cout << "Type : " << s._t << endl;
}

void PrintMap(unordered_map<char, int> map) {
  cout << "[";
  for (auto p : map) {
    cout << "{" << p.first << " : " << p.second << "}, ";
  }
  cout << "]" << endl;
}

class Automaton {
  unordered_set<char> _lang;
  vector<State> states;
  string patternChars;
  unordered_map<char, int> patternCharsIdx;

  vector<vector<int>> _transitions;

  bool checkChar(char c) {
    auto it = _lang.find(c);
    return it != _lang.end();
  }

 public:
  Automaton(string language, string endPattern);

  void printStates() {
    for (State& s : this->states) {
      DisplayState(s);
    }
    cout << endl;
  }

  void printTransitions() {
    for (int i = 0; i < _transitions.size(); i++) {
      cout << i << " | ";
      for (int x : _transitions[i]) {
        cout << x << ",";
      }
      cout << endl;
    }
    cout << endl;
  }

  void printPatternCharIdx() { PrintMap(this->patternCharsIdx); }

  bool run(string input);
};

Automaton::Automaton(string language, string endPattern) {
  _lang = unordered_set<char>(language.begin(), language.end());

  // Create Initial State
  states.push_back(State("", INIT));

  // Add States Based on the Pattern
  string pat = "";
  for (int i = 0; i < endPattern.size(); i++) {
    pat.push_back(endPattern[i]);
    states.push_back(pat);
  }

  states.back()._t = SOL;

  // Add terminate State for sys Faults
  states.push_back(State("", TERM) );

  // get the Unique Characters Required in the end Pattern
  for (char c : endPattern) {
    auto it = patternCharsIdx.find(c);
    if (it == patternCharsIdx.end()) {
      patternCharsIdx.insert({c, patternCharsIdx.size()});
      patternChars.push_back(c);
    }
  }
  patternCharsIdx = patternCharsIdx;

  // Allocate Transition table
  _transitions =
      vector<vector<int>>(states.size(), vector<int>(patternCharsIdx.size()));
  


  // Setup Main Transition Links
  for (int i = 0; i < endPattern.size(); ++i) {
    // Get the Uniq Chars Index for the Input character in the Finding Pattern
    int idx = patternCharsIdx.find(endPattern[i])->second;
    
    _transitions[i][idx] = i + 1;

    cout << endl;
  }

  // Other Transition Links
  // Skipping First Row Since all other chars apart from first Pattern input
  // char will result in Q0 only
  string inStr = "";
  bool matchFound = false;
  for (int i = 1; i <= endPattern.length(); i++) {
    // Put for All Unallocated States
    for (int col = 0; col < patternChars.size(); col++) {
      
      if (_transitions[i][col] == 0) {
        inStr = states[i]._data + patternChars[col];
      
        // Check if Substrings [1:n], ... exist as states
        // Donot consider Final Terminating Character
        deque<char> q(inStr.begin(), inStr.end() );
      
        // Removing the first char since we know curr inStr doesnt exist as a State
        q.pop_front();
        matchFound = false;

        while (!matchFound && !q.empty() ) {
          inStr = string(q.begin(), q.end() );
    
          // Check in Existing states
          for (int j = 0; j < states.size(); j++) {
            if (inStr == states[j]._data) {
              _transitions[i][col] = j;

              matchFound = true;
              break;
            }
          }

          q.pop_front();
        }

      }
    }
  }


}

bool Automaton::run(string input) {
  // Always Start at Initial State
  int stateNo = 0;
  int charIdx;

  for (char c : input) {
    // Check if Valid Character else send to trap State
    if (_lang.find(c) == _lang.end() ) {
      cout<<"Char '"<< c <<"'  not in Automatons Language..."<<endl;
      cout<<"Recahed TRAP state Terminating...."<<endl;
      return false;
    }

    // Check if Character is in the Pattern
    auto it = patternCharsIdx.find(c);
    if (it != patternCharsIdx.end() ) {
      charIdx = it->second;
      // Update State
      stateNo = _transitions[stateNo][charIdx];
    }
    
  }

  // Check if SOL state reached
  if (states[stateNo]._t == SOL) {
    return true;
  }

  return false;
}

int main() {
  Automaton a("ab", "bab");
  vector<string> testStrings = {
    "aaabbaaabb",
    "aabbabbab"
  };

  for (string s : testStrings) {
    bool ans = a.run(s);
    cout<<"String : "<< s <<endl<<"Ans : "<< ans <<endl<<endl;
  }
  

  return 0;
}