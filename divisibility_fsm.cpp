#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <deque>
#include<exception>
#include<algorithm>

using namespace std;

enum StateType { INIT, BRANCH, SOL, TERM };

typedef struct State {
  int _data;
  StateType _t;

  State(int data, StateType type = BRANCH) : _data(data), _t(type) {}
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

const string NUMBER_LANG = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

class DivisibilityAutomaton {
  int _langBase;
  unordered_set<char> _lang;
  vector<State> states;
  unordered_map<char, int> inCharRemSetIdx;
  

  vector<vector<int>> _transitions;

  bool checkChar(char c) {
    auto it = _lang.find(c);
    return it != _lang.end();
  }

 public:
  DivisibilityAutomaton(int inputLangBase, int divisor);

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

  void printPatternCharIdx() { PrintMap(this->inCharRemSetIdx); }

  bool run(string input);
};

// Updated the Transition table to Include routes to self at solState for any input char
DivisibilityAutomaton::DivisibilityAutomaton(int inputLangBase, int divisor) {
  if (inputLangBase > 35) {
    throw range_error("Please Enter Values 1 <= x <= 35");
  }
  _langBase = inputLangBase;
  _lang = unordered_set<char>(NUMBER_LANG.begin(), NUMBER_LANG.begin() + _langBase);

  // Create Initial State
  states.push_back(State(0, INIT));

  // Add States Based on the Pattern
  for (int i = 0; i < divisor; i++) {
    states.push_back(State(i) );
  }

  // Remainder 0 must be Solution
  states[1]._t = SOL;

  // Add terminate State for sys Faults
  states.push_back(State(INT32_MAX, TERM) );

  // Create Inputs Set of Chars which will lead to the Same Divsible State
  for (int i = 0; i < _lang.size(); i++) {
    inCharRemSetIdx.insert(pair<char, int>(NUMBER_LANG[i], i % divisor) );
  }

  // Allocate Transition table
  int nCols = min((int) _lang.size(), divisor);
  _transitions =
      vector<vector<int>>(states.size(), vector<int>( nCols) );
  


  // Setup Main Transition Links
  // For Initial State Input Set maps to Direct Divisor
  for (int c = 0; c < nCols; c++) {
    _transitions[0][c] = c + 1;
  }
  // For Remaining Use Formula (BASE * Remainder + InputDigit) mod Divisor
  for (int i = 1; i <= divisor; ++i) {
    for (int j = 0; j < nCols; j++) {
      _transitions[i][j] = ( (_langBase*(i-1) + j) % divisor) + 1;
    }    

  }

}

bool DivisibilityAutomaton::run(string input) {
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
    auto it = inCharRemSetIdx.find(c);
    if (it != inCharRemSetIdx.end() ) {
      charIdx = it->second;
      // Update State
      stateNo = _transitions[stateNo][charIdx];
    }
    
  }

  cout<<"Final State : "<<endl;
  DisplayState(states[stateNo]);
  
  // If reached Solution State Exit
  if (states[stateNo]._t == SOL) {
    return true;
  }

  return false;
}

int main() {
  DivisibilityAutomaton a(16, 3);
  vector<string> testStrings = {
    "1F",
    "4FB"
  };

  // a.printStates();
  // a.printPatternCharIdx();
  // a.printTransitions();

  for (string s : testStrings) {
    cout<<"String : "<< s <<endl;
    bool ans = a.run(s);
    cout<<"Ans : "<< ans <<endl<<endl;
  }
  

  return 0;
}
