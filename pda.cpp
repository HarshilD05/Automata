#include<iostream>
#include<vector>
#include<map>
#include<unordered_map>
#include<stack>
#include<string>

using namespace std;

enum StateType { INIT, BRANCH, SOL, TERM };

struct TransitionEntry {
    int nextStateIdx;
    string pops;   // Pop forward: "AB" means pop A then pop B
    string pushs;  // Push backward: "AB" means push B then push A
    
    TransitionEntry(int next = -1, string pop = "", string push = "") 
        : nextStateIdx(next), pops(pop), pushs(push) {}
};

// Tabular format: TransitionTable[inputSymbol][stackTop] = TransitionEntry
typedef unordered_map<char, unordered_map<char, TransitionEntry>> TransitionTable;

struct PDAState {
    StateType type;
    TransitionTable transitions;
    
    PDAState(StateType t = TERM) : type(t), transitions(TransitionTable()) {}
};

class PDA {
private:
    map<int, PDAState> states;
    stack<char> pdaStack;
    int currentState;
    constexpr static char STACK_BOTTOM = 'Z';
    constexpr static char EPSILON = 'E';
    
    // Helper function to validate and perform pop operations
    bool TryPop(const string& pops) {
        if (pops.empty()) return true;
        
        // Create temp stack to check if all pops are valid
        stack<char> temp = pdaStack;
        
        // Try to pop forward: "AB" means pop A then pop B
        for (char c : pops) {
            if (temp.empty() || temp.top() != c) {
                return false;
            }
            temp.pop();
        }
        
        // If validation succeeds, actually perform the pops
        for (char c : pops) {
            pdaStack.pop();
        }
        
        return true;
    }
    
    // Helper function to perform push operations
    void DoPush(const string& pushs) {
        if (pushs.empty()) return;
        
        // Push backward: "AB" means push B then push A
        for (int i = pushs.size() - 1; i >= 0; i--) {
            pdaStack.push(pushs[i]);
        }
    }
    
public:
    PDA() {
        currentState = 0;
        pdaStack.push(STACK_BOTTOM);
        InitializePDA();
    }
    
    void InitializePDA() {
        // State 0: INIT - Start state
        states[0] = PDAState(INIT);
        // Read 'a' with Z on stack -> Pop Z, Push aZ (effectively push 'a')
        states[0].transitions['a'][STACK_BOTTOM] = TransitionEntry(0, "Z", "aZ");
        // Read 'a' with 'a' on stack -> Pop a, Push aa (effectively push 'a')
        states[0].transitions['a']['a'] = TransitionEntry(0, "a", "aa");
        // Read 'b' with 'a' on stack -> Pop 'a', go to state 1
        states[0].transitions['b']['a'] = TransitionEntry(1, "a", "");
        // Read 'b' with Z on stack -> ERROR (b without any a), go to TERM
        states[0].transitions['b'][STACK_BOTTOM] = TransitionEntry(3, "", "");
        // Epsilon with Z on stack -> Accept (empty string)
        states[0].transitions[EPSILON][STACK_BOTTOM] = TransitionEntry(2, "", "");
        
        // State 1: BRANCH - Reading b's and popping a's
        states[1] = PDAState(BRANCH);
        // Read 'b' with 'a' on stack -> Pop 'a', stay in state 1
        states[1].transitions['b']['a'] = TransitionEntry(1, "a", "");
        // Epsilon with Z on stack -> Accept (equal a's and b's)
        states[1].transitions[EPSILON][STACK_BOTTOM] = TransitionEntry(2, "", "");
        // Read 'a' with 'a' on stack -> ERROR (a after b), go to TERM
        states[1].transitions['a']['a'] = TransitionEntry(3, "", "");
        // Read 'a' with Z on stack -> ERROR (a after b), go to TERM
        states[1].transitions['a'][STACK_BOTTOM] = TransitionEntry(3, "", "");
        // Read 'b' with Z on stack -> ERROR (more b's than a's), go to TERM
        states[1].transitions['b'][STACK_BOTTOM] = TransitionEntry(3, "", "");
        
        // State 2: SOL - Accept state
        states[2] = PDAState(SOL);
        
        // State 3: TERM - Reject/Error state
        states[3] = PDAState(TERM);
    }
    
    void DisplayStates() {
        cout << "\nPDA States and Transitions:\n";
        cout << "===========================\n";
        
        string typeNames[] = {"INIT", "BRANCH", "SOL", "TERM"};
        
        for (auto& p : states) {
            cout << "State " << p.first << " (" << typeNames[p.second.type] << "):\n";
            
            for (auto& inputEntry : p.second.transitions) {
                char inputSym = inputEntry.first;
                
                for (auto& stackEntry : inputEntry.second) {
                    char stackSym = stackEntry.first;
                    TransitionEntry entry = stackEntry.second;
                    
                    cout << "   Input[" << (inputSym == EPSILON ? "\u03B5" : string(1, inputSym)) << "] ";  // \u03B5 => ε
                    cout << "Stack[" << stackSym << "] ";
                    cout << "Pop[" << (entry.pops.empty() ? "\u03B5" : entry.pops) << "] ";  // \u03B5 => ε
                    cout << "Push[" << (entry.pushs.empty() ? "\u03B5" : entry.pushs) << "] ";  // \u03B5 => ε
                    cout << "-> State " << entry.nextStateIdx << "\n";
                }
            }
            cout << endl;
        }
    }
    
    bool ProcessString(const string& input) {
        // Reset PDA state
        currentState = 0;
        while (!pdaStack.empty()) pdaStack.pop();
        pdaStack.push(STACK_BOTTOM);
        
        cout << "\nProcessing string: \"" << input << "\"\n";
        cout << "================================\n";
        
        int inputIdx = 0;
        
        while (true) {
            char stackTopChar = pdaStack.empty() ? '\0' : pdaStack.top();
            char inputChar = (inputIdx < input.size()) ? input[inputIdx] : EPSILON;
            
            cout << "State: " << currentState 
                 << " | Stack Top: " << stackTopChar 
                 << " | Input: " << (inputChar == EPSILON ? "\u03B5" : string(1, inputChar))  // \u03B5 => ε
                 << " | Stack Size: " << pdaStack.size() << endl;
            
            // Check if we're in accept state
            if (states[currentState].type == SOL) {
                cout << "\n\u2713 String ACCEPTED!\n";  // \u2713 => ✓
                return true;
            }
            
            // Check if we're in reject state
            if (states[currentState].type == TERM) {
                cout << "\n\u2717 String REJECTED!\n";  // \u2717 => ✗
                return false;
            }
            
            // Find matching transition using tabular format
            bool transitionFound = false;
            TransitionEntry entry;
            
            // First try epsilon transition
            if (states[currentState].transitions.find(EPSILON) != states[currentState].transitions.end()) {
                auto& epsilonMap = states[currentState].transitions[EPSILON];
                if (epsilonMap.find(stackTopChar) != epsilonMap.end()) {
                    entry = epsilonMap[stackTopChar];
                    
                    // Try to perform pops
                    if (TryPop(entry.pops)) {
                        cout << "   -> Taking epsilon transition: ";
                        cout << "Pop[" << (entry.pops.empty() ? "\u03B5" : entry.pops) << "] ";  // \u03B5 => ε
                        
                        // Perform pushes
                        DoPush(entry.pushs);
                        cout << "Push[" << (entry.pushs.empty() ? "\u03B5" : entry.pushs) << "] ";  // \u03B5 => ε
                        
                        cout << "-> State " << entry.nextStateIdx << endl;
                        currentState = entry.nextStateIdx;
                        transitionFound = true;
                    }
                }
            }
            
            // If no epsilon transition, try regular input transition
            if (!transitionFound && inputChar != EPSILON) {
                if (states[currentState].transitions.find(inputChar) != states[currentState].transitions.end()) {
                    auto& inputMap = states[currentState].transitions[inputChar];
                    if (inputMap.find(stackTopChar) != inputMap.end()) {
                        entry = inputMap[stackTopChar];
                        
                        // Try to perform pops
                        if (TryPop(entry.pops)) {
                            cout << "   -> Transition found: ";
                            cout << "Pop[" << (entry.pops.empty() ? "\u03B5" : entry.pops) << "] ";  // \u03B5 => ε
                            
                            // Perform pushes
                            DoPush(entry.pushs);
                            cout << "Push[" << (entry.pushs.empty() ? "\u03B5" : entry.pushs) << "] ";  // \u03B5 => ε
                            
                            cout << "-> State " << entry.nextStateIdx << endl;
                            currentState = entry.nextStateIdx;
                            inputIdx++;
                            transitionFound = true;
                        }
                        else {
                            cout << "   -> Pop validation failed!\n";
                        }
                    }
                }
            }
            
            // No transition found
            if (!transitionFound) {
                // Check for end of input
                if (inputIdx >= input.size()) {
                    // Try epsilon transition one more time for accept
                    if (states[currentState].transitions.find(EPSILON) != states[currentState].transitions.end()) {
                        auto& epsilonMap = states[currentState].transitions[EPSILON];
                        if (epsilonMap.find(stackTopChar) != epsilonMap.end()) {
                            entry = epsilonMap[stackTopChar];
                            if (TryPop(entry.pops)) {
                                DoPush(entry.pushs);
                                currentState = entry.nextStateIdx;
                                
                                if (states[currentState].type == SOL) {
                                    cout << "\n\u2713 String ACCEPTED!\n";  // \u2713 => ✓
                                    return true;
                                }
                            }
                        }
                    }
                }
                
                cout << "   -> No valid transition found!\n";
                cout << "\n\u2717 String REJECTED!\n";  // \u2717 => ✗
                return false;
            }
        }
        
        return false;
    }
    
    void DisplayStack() {
        cout << "Stack Contents (top to bottom): ";
        stack<char> temp = pdaStack;
        while (!temp.empty()) {
            cout << temp.top() << " ";
            temp.pop();
        }
        cout << endl;
    }
};

int main() {
    PDA pda;
    
    cout << "PDA for L = {a^n b^n | n >= 0}\n";
    cout << "================================\n";
    
    pda.DisplayStates();
    
    // Test cases
    vector<string> testStrings = {
        "",           // n=0, should accept
        "ab",         // n=1, should accept
        "aabb",       // n=2, should accept
        "aaabbb",     // n=3, should accept
        "aaaabbbb",   // n=4, should accept
        "a",          // unequal, should reject
        "b",          // unequal (b with empty stack), should reject
        "aab",        // unequal, should reject
        "abb",        // unequal, should reject
        "aba",        // invalid order, should reject
        "abab"        // invalid pattern, should reject
    };
    
    cout << "\n\nTesting PDA:\n";
    cout << "============\n";
    
    for (const string& str : testStrings) {
        pda.ProcessString(str);
        cout << endl;
    }
    
    return 0;
}
