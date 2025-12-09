#include<iostream>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<stack>

using namespace std;


typedef unordered_map<int, unordered_map<char, int>> TransitionTable;

struct Operand {
    int state;
    string re;

    Operand(int s = 0, string re = ""): state(s), re(re)  {}
};


struct Eqn {
    int _state;
    vector<Operand> _val;

    Eqn(int s = 0, vector<Operand> v = vector<Operand>() ): _state(s), _val(v) {}
};


void DisplayTransitionTable(const TransitionTable& t) {
    cout << "Transition Table:\n";
    for (const auto& state : t) {
        cout << "State " << state.first << ":\n";
        for (const auto& link : state.second) {
            cout << "   - " << link.first << " -> " << link.second << "\n";
        }
    }
    cout << endl;
}

void DisplayEqns(const unordered_map<int, Eqn>& eqns) {
    cout << "Equations:\n";
    for (const auto& e : eqns) {
        cout << "Eqn for State " << e.first << ": ";
        for (size_t i = 0; i < e.second._val.size(); i++) {
            const Operand& op = e.second._val[i];
            cout << "(" << op.state << ", " << op.re << ")" << "+";
        }
        cout << endl;
    }
    cout << endl;
}


unordered_map<int, Eqn> GenerateEqns(TransitionTable t) {
    unordered_map<int, Eqn> eqns;

    // Insert all the States of Transition Table as Keys for Eqns
    for (auto p : t) {
        eqns.insert({p.first, Eqn(p.first)});
    }

    // Get Incoming Edges for Each State and Add to the Eqn
    for (auto p : t) {
        int curr = p.first;
        for (auto links : p.second) {
            int node = links.second;
            char c = links.first;
            eqns[node]._val.push_back(Operand(curr, string(1,c) ) );
        }
    }

    DisplayEqns(eqns);


    return eqns;
}

Eqn MergeSameStates(Eqn& eqn) {
    unordered_map<int, string> merged;
    
    // Collect all REs for each state
    for (auto op : eqn._val) {
        if (merged.find(op.state) != merged.end() ) {
            merged[op.state] += ("|" + op.re);
        }
        else {
            merged[op.state] = op.re;
        }
    }
    
    // Create new equation with merged operands
    Eqn newEq(eqn._state);
    for (auto m : merged) {
        newEq._val.push_back(Operand(m.first, m.second) );
    }

    return newEq;
}

Eqn _Solve(unordered_map<int, Eqn>& eqns, int idx, unordered_set<int>& _visitedStates) {
    // Add the Current State in Vistied State
    _visitedStates.insert(idx);
    
    // cout << "\n=== Solving State " << idx << " ===\n";
    // cout << "Initial operands: " << eqns[idx]._val.size() << endl;
        
    // Check all the Oparends of the State Eqn
    for (int i = 0; i < eqns[idx]._val.size(); i++ ) {
        Operand op = eqns[idx]._val[i];
        
        // cout << "   Evaluating Operand -> (State: " << op.state << ", RE: " << op.re << ")" << endl;
        // If Operand State Different from curr State and not previously visited goto that Eqn
        if (op.state != idx && _visitedStates.find(op.state) == _visitedStates.end() ) {
            // cout << "   --> Jumping to State " << op.state << endl;

            Eqn nEq = _Solve(eqns, op.state, _visitedStates);
            
            // cout << "   --> Returned from State " << op.state << " with " << nEq._val.size() << " operands\n";
            
            // Merge the Newly genrated Eqn into the current State
            // Distribute Curr State RE onto the RE of all operands of the Solved Eqn
            for (auto& o : nEq._val) {
                o.re = op.re + o.re;
                eqns[idx]._val.push_back(o);
            }

            // remove Previous Instance of Operand and add nEq values in currState Eqn
            eqns[idx]._val.erase(eqns[idx]._val.begin() + i);    
            i--;  // Decrementing i so that we donot skip a Operand after deletion
            
            // Simplify the States
            eqns[idx] = MergeSameStates(eqns[idx]);
            
            // cout << "   --> After merge, State " << idx << " has " << eqns[idx]._val.size() << " operands\n";
        }        
        
    }

    // Check if in Form Q + RP convert to QP*
    bool recurringCall = false;
    int i = 0;
    for (i = 0; i < eqns[idx]._val.size(); i++) {
        if (eqns[idx]._val[i].state == idx) {
            recurringCall = true;
            // cout << "   Found self-loop at operand " << i << " with RE: " << eqns[idx]._val[i].re << endl;
            break;
        }
    }

    if (recurringCall) {
        // Remove Old Operand and Store the RE
        string P = eqns[idx]._val[i].re;
        eqns[idx]._val.erase(eqns[idx]._val.begin() + i);
        
        // cout << "   Applying Arden's rule with P = " << P << endl;
        
        // If there are other operands (Q), apply QP*
        if (eqns[idx]._val.size() > 0) {
            for (auto& op : eqns[idx]._val) {
                op.re = op.re + "(" + P + ")*";
            }
        }
        // If no other operands, result is just P* (with epsilon)
        else {
            eqns[idx]._val.push_back(Operand(-1, "(" + P + ")*"));
        }
        
        // cout << "   After Arden's rule, State " << idx << " has " << eqns[idx]._val.size() << " operands\n";
    }

    // cout<<"Eqn Scanning Completed for State " << idx << "...\n";
    // Now Check if Recursive Property exists for curr State (R = Q + RP) -> R = QP*

    return eqns[idx];
}

string EvaluateRE(const TransitionTable t, int finalState) {
    // Generate Eqns
    unordered_map<int, Eqn> eqns = GenerateEqns(t);
    unordered_set<int> _visited;

    Eqn ans = _Solve(eqns, finalState, _visited);
    
    // Debug: Display the final equation
    // cout << "\nFinal Equation for State " << finalState << ":\n";
    // for (int i = 0; i < ans._val.size(); i++) {
    //     cout << "   Operand " << i << ": State=" << ans._val[i].state 
    //          << ", RE=\"" << ans._val[i].re << "\"\n";
    // }
    
    // Build the Final RE String from the Solved Eqn
    string result = "";
    for (int i = 0; i < ans._val.size(); i++) {
        result += ans._val[i].re;
        if (i < ans._val.size() - 1) {
            result += "|";
        }
    }

    return result;
}



int main () {
    // Creating the Transition table
    TransitionTable t;
    t.insert({1, unordered_map<char, int>() });
    t[1].insert({'0', 1});
    t[1].insert({'1', 2});

    t.insert({2, unordered_map<char, int>() });
    t[2].insert({'0', 3});
    t[2].insert({'1', 2});

    t.insert({3, unordered_map<char, int>() });
    t[3].insert({'0', 1});
    t[3].insert({'1', 2});

    int FinalState = 1;


    DisplayTransitionTable(t);
    string RE = EvaluateRE(t, FinalState);

    cout<<"\n\n========================================\n";
    cout<<"Final Regular Expression: "<<RE<<endl;
    cout<<"========================================\n";

    return 0;
}