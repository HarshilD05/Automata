#include<iostream>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<stack>

using namespace std;

constexpr char EPSILON = 'E';

enum StateType { INIT, BRANCH, SOL, TERM };

typedef struct State {
    string _data;
    StateType _t;
    
    State(string data = "", StateType type = BRANCH) : _data(data), _t(type) {}
} State;

// NFA Transition: Multiple next states possible for same input
typedef unordered_map<char, unordered_set<int>> TransitionMap;

struct NFAFragment {
    int startState;
    int endState;
    
    NFAFragment(int start = -1, int end = -1) : startState(start), endState(end) {}
};

class NFA {
private:
    vector<State> states;
    vector<TransitionMap> transitions;
    int stateCounter;
    
    int CreateState(string data = "", StateType type = BRANCH) {
        states.push_back(State(data, type));
        transitions.push_back(TransitionMap());
        return stateCounter++;
    }
    
    void AddTransition(int from, char symbol, int to) {
        transitions[from][symbol].insert(to);
    }
    
    // Get epsilon closure of a set of states
    unordered_set<int> EpsilonClosure(unordered_set<int> stateSet) {
        stack<int> stk;
        unordered_set<int> closure = stateSet;
        
        for (int s : stateSet) {
            stk.push(s);
        }
        
        while (!stk.empty()) {
            int curr = stk.top();
            stk.pop();
            
            // Check if epsilon transition exists
            if (transitions[curr].find(EPSILON) != transitions[curr].end()) {
                for (int next : transitions[curr][EPSILON]) {
                    if (closure.find(next) == closure.end()) {
                        closure.insert(next);
                        stk.push(next);
                    }
                }
            }
        }
        
        return closure;
    }
    
    // Basic NFA for single character
    NFAFragment CreateBasicNFA(char c) {
        int start = CreateState(string(1, c), BRANCH);
        int end = CreateState(string(1, c), BRANCH);
        AddTransition(start, c, end);
        return NFAFragment(start, end);
    }
    
    // NFA for epsilon
    NFAFragment CreateEpsilonNFA() {
        int start = CreateState("E", BRANCH);
        int end = CreateState("E", BRANCH);
        AddTransition(start, EPSILON, end);
        return NFAFragment(start, end);
    }
    
    // Concatenation: NFA1 . NFA2
    NFAFragment Concatenate(NFAFragment nfa1, NFAFragment nfa2) {
        // Connect end of nfa1 to start of nfa2 with epsilon
        AddTransition(nfa1.endState, EPSILON, nfa2.startState);
        return NFAFragment(nfa1.startState, nfa2.endState);
    }
    
    // Union: NFA1 | NFA2
    NFAFragment Union(NFAFragment nfa1, NFAFragment nfa2) {
        int start = CreateState("|", BRANCH);
        int end = CreateState("|", BRANCH);
        
        // New start state with epsilon transitions to both NFAs
        AddTransition(start, EPSILON, nfa1.startState);
        AddTransition(start, EPSILON, nfa2.startState);
        
        // Both end states connect to new end state
        AddTransition(nfa1.endState, EPSILON, end);
        AddTransition(nfa2.endState, EPSILON, end);
        
        return NFAFragment(start, end);
    }
    
    // Kleene Star: NFA*
    NFAFragment KleeneStar(NFAFragment nfa) {
        int start = CreateState("*", BRANCH);
        int end = CreateState("*", BRANCH);
        
        // New start to NFA start
        AddTransition(start, EPSILON, nfa.startState);
        // NFA end back to NFA start (loop)
        AddTransition(nfa.endState, EPSILON, nfa.startState);
        // NFA end to new end
        AddTransition(nfa.endState, EPSILON, end);
        // New start to new end (zero occurrences)
        AddTransition(start, EPSILON, end);
        
        return NFAFragment(start, end);
    }
    
    // Kleene Plus: NFA+
    NFAFragment KleenePlus(NFAFragment nfa) {
        int start = CreateState("+", BRANCH);
        int end = CreateState("+", BRANCH);
        
        // New start to NFA start
        AddTransition(start, EPSILON, nfa.startState);
        // NFA end back to NFA start (loop)
        AddTransition(nfa.endState, EPSILON, nfa.startState);
        // NFA end to new end
        AddTransition(nfa.endState, EPSILON, end);
        
        return NFAFragment(start, end);
    }
    
    bool IsOperator(char c) {
        return (c == '|' || c == '*' || c == '+' || c == '.' || c == '(' || c == ')');
    }
    
    int Precedence(char op) {
        if (op == '|') return 1;
        if (op == '.') return 2;
        if (op == '*' || op == '+') return 3;
        return 0;
    }
    
    // Add explicit concatenation operator
    string AddConcatOperator(string re) {
        string result = "";
        
        for (int i = 0; i < re.size(); i++) {
            result += re[i];
            
            if (i + 1 < re.size()) {
                char curr = re[i];
                char next = re[i + 1];
                
                // Add '.' between: char-char, char-(, )-char, )-(, *-char, *-(, +-char, +-(
                if ((curr != '(' && curr != '|' && next != ')' && next != '|' && next != '*' && next != '+')) {
                    result += '.';
                }
            }
        }
        
        return result;
    }
    
    // Convert infix to postfix
    string InfixToPostfix(string re) {
        stack<char> ops;
        string postfix = "";
        
        for (char c : re) {
            if (c == '(') {
                ops.push(c);
            }
            else if (c == ')') {
                while (!ops.empty() && ops.top() != '(') {
                    postfix += ops.top();
                    ops.pop();
                }
                if (!ops.empty()) ops.pop(); // Remove '('
            }
            else if (IsOperator(c)) {
                while (!ops.empty() && ops.top() != '(' && Precedence(ops.top()) >= Precedence(c)) {
                    postfix += ops.top();
                    ops.pop();
                }
                ops.push(c);
            }
            else {
                // Operand
                postfix += c;
            }
        }
        
        while (!ops.empty()) {
            postfix += ops.top();
            ops.pop();
        }
        
        return postfix;
    }

public:
    NFA() : stateCounter(0) {}
    
    void BuildFromRE(string re) {
        // Add explicit concatenation
        string withConcat = AddConcatOperator(re);
        cout << "With concat operator: " << withConcat << endl;
        
        // Convert to postfix
        string postfix = InfixToPostfix(withConcat);
        cout << "Postfix: " << postfix << endl << endl;
        
        // Build NFA using stack
        stack<NFAFragment> nfaStack;
        
        for (char c : postfix) {
            if (c == '.') {
                // Concatenation
                NFAFragment nfa2 = nfaStack.top(); nfaStack.pop();
                NFAFragment nfa1 = nfaStack.top(); nfaStack.pop();
                nfaStack.push(Concatenate(nfa1, nfa2));
            }
            else if (c == '|') {
                // Union
                NFAFragment nfa2 = nfaStack.top(); nfaStack.pop();
                NFAFragment nfa1 = nfaStack.top(); nfaStack.pop();
                nfaStack.push(Union(nfa1, nfa2));
            }
            else if (c == '*') {
                // Kleene Star
                NFAFragment nfa = nfaStack.top(); nfaStack.pop();
                nfaStack.push(KleeneStar(nfa));
            }
            else if (c == '+') {
                // Kleene Plus
                NFAFragment nfa = nfaStack.top(); nfaStack.pop();
                nfaStack.push(KleenePlus(nfa));
            }
            else {
                // Basic character or epsilon
                if (c == EPSILON) {
                    nfaStack.push(CreateEpsilonNFA());
                }
                else {
                    nfaStack.push(CreateBasicNFA(c));
                }
            }
        }
        
        // Final NFA
        if (!nfaStack.empty()) {
            NFAFragment finalNFA = nfaStack.top();
            states[0]._t = INIT;
            states[finalNFA.endState]._t = SOL;
        }
    }
    
    void PrintNFA() {
        cout << "NFA States:\n";
        cout << "===========\n";
        
        string typeNames[] = {"INIT", "BRANCH", "SOL", "TERM"};
        
        for (int i = 0; i < states.size(); i++) {
            cout << "State " << i << " (" << typeNames[states[i]._t] << ")";
            cout << " [" << states[i]._data << "]\n";
        }
        cout << endl;
        
        cout << "NFA Transitions:\n";
        cout << "================\n";
        
        for (int i = 0; i < transitions.size(); i++) {
            if (transitions[i].empty()) continue;
            
            cout << "State " << i << ":\n";
            for (auto& p : transitions[i]) {
                char symbol = p.first;
                cout << "   On '" << (symbol == EPSILON ? "\u03B5" : string(1, symbol)) << "' -> {";  // \u03B5 => ε
                
                bool first = true;
                for (int nextState : p.second) {
                    if (!first) cout << ", ";
                    cout << nextState;
                    first = false;
                }
                cout << "}\n";
            }
        }
        cout << endl;
    }
    
    bool Run(string input) {
        // Start with epsilon closure of initial state
        unordered_set<int> currentStates = {0};
        currentStates = EpsilonClosure(currentStates);
        
        cout << "Processing string: \"" << input << "\"\n";
        cout << "================================\n";
        
        // Print initial states
        cout << "Initial states (with \u03B5-closure): {";  // \u03B5 => ε
        bool first = true;
        for (int s : currentStates) {
            if (!first) cout << ", ";
            cout << s;
            first = false;
        }
        cout << "}\n\n";
        
        // Process each character
        for (char c : input) {
            unordered_set<int> nextStates;
            
            cout << "Reading '" << c << "':\n";
            
            // For each current state, follow transitions on 'c'
            for (int state : currentStates) {
                if (transitions[state].find(c) != transitions[state].end()) {
                    for (int next : transitions[state][c]) {
                        nextStates.insert(next);
                    }
                }
            }
            
            // Get epsilon closure of next states
            nextStates = EpsilonClosure(nextStates);
            
            cout << "   Next states (with \u03B5-closure): {";  // \u03B5 => ε
            first = true;
            for (int s : nextStates) {
                if (!first) cout << ", ";
                cout << s;
                first = false;
            }
            cout << "}\n\n";
            
            if (nextStates.empty()) {
                cout << "\u2717 String REJECTED! (No valid transitions)\n";  // \u2717 => ✗
                return false;
            }
            
            currentStates = nextStates;
        }
        
        // Check if any current state is an accepting state
        for (int state : currentStates) {
            if (states[state]._t == SOL) {
                cout << "\u2713 String ACCEPTED! (Reached accepting state " << state << ")\n";  // \u2713 => ✓
                return true;
            }
        }
        
        cout << "\u2717 String REJECTED! (No accepting state reached)\n";  // \u2717 => ✗
        return false;
    }
};

int main() {
    // Test RE to NFA conversion
    vector<string> regularExpressions = {
        "(a|b)*abb"     // Pattern matching
    };
    
    for (string re : regularExpressions) {
        cout << "\n========================================\n";
        cout << "Regular Expression: " << re << "\n";
        cout << "========================================\n\n";
        
        NFA nfa;
        nfa.BuildFromRE(re);
        nfa.PrintNFA();
        
        // Test strings
        vector<string> testStrings = {"abb", "aabb", "babb", "abababb", "ab", "abba"};
        
        cout << "\nTesting strings:\n";
        cout << "================\n";
        for (string str : testStrings) {
            bool result = nfa.Run(str);
            cout << endl;
        }
        
        cout << "\n";
    }
    
    return 0;
}
