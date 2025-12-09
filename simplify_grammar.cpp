#include<iostream>
#include<vector>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<set>
#include<queue>

using namespace std;

typedef unordered_map<char, set<string>> GRAMMAR_RULES;
constexpr char EPSILON = 'E';

void PrintGrammar(const GRAMMAR_RULES& gr, const string& label = "Grammar Rules") {
    cout<< label <<" : "<<endl;
    for (auto& p : gr) {
        cout<< p.first <<" : ";
        for (const string& s : p.second) {
            cout<< s <<"| ";
        }
        cout<<endl;
    }
    cout<<endl;
}

bool isVarChar(char c) {
    return (c >= 'A' && c <= 'Z' && c != EPSILON);
}

GRAMMAR_RULES _eliminateUnused(GRAMMAR_RULES gr, char StartVar = 'S') {
    // Find Non callable Grammar Rules
    unordered_set<char> CalledVars = { StartVar };
    queue<char> q;
    q.push(StartVar);

    while (!q.empty() ) {
        char currVar = q.front();
        q.pop();

        // Append all variables called by the currvar
        set<string>& terms = gr.at(currVar);
        for (string t : terms) {
            for (char& c : t) {
                // Check if is variable [Capital letter] and not already called
                if (isVarChar(c) && CalledVars.find(c) == CalledVars.end() ) {
                    // Add in Called Var set and Append in Que
                    q.push(c);
                    CalledVars.insert(c);
                }
            }
        }
    }

    // Gemove all Grammar variables which Cannot be Called from the Start variable
    unordered_set<char> unCalledVars;
    for (auto& p : gr) {
        if (CalledVars.find(p.first) == CalledVars.end() ) {
            unCalledVars.insert(p.first);
        }
    }

    // Remove all Uncalled Variables from the grammar
    for (char c : unCalledVars) {
        gr.erase(c);
    }

    return gr;
}

GRAMMAR_RULES _eliminateUnit(GRAMMAR_RULES gr) {
    // Find all unit pairs (A, B) where A =>* B through unit productions
    unordered_map<char, unordered_set<char>> unitPairs;
    
    // Initialize: each variable can reach itself
    for (auto& p : gr) {
        unitPairs[p.first].insert(p.first);
    }
    
    // Find all unit production pairs transitively
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (auto& p : gr) {
            char var = p.first;
            
            // Check all variables reachable from current variable
            unordered_set<char> reachable = unitPairs[var];
            
            for (char mid : reachable) {
                // Check if mid has unit productions
                for (const string& prod : gr[mid]) {
                    if (prod.size() == 1 && isVarChar(prod[0])) {
                        char target = prod[0];
                        
                        // If var cannot already reach target, add it
                        if (unitPairs[var].find(target) == unitPairs[var].end()) {
                            unitPairs[var].insert(target);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    
    // Build new grammar by replacing unit productions
    GRAMMAR_RULES newGr;
    
    for (auto& p : gr) {
        char var = p.first;
        newGr[var] = set<string>();
        
        // For each variable reachable through unit productions
        for (char reachableVar : unitPairs[var]) {
            // Add all non-unit productions of reachable variable
            for (const string& prod : gr[reachableVar]) {
                // Skip unit productions
                if (prod.size() == 1 && isVarChar(prod[0])) {
                    continue;
                }
                
                newGr[var].insert(prod);
            }
        }
    }
    
    return _eliminateUnused(newGr);
}

GRAMMAR_RULES _eliminateEpsilon(GRAMMAR_RULES gr) {
    // Find all Nullable Variables (variables that can produce epsilon)
    unordered_set<char> NullableVars;
    bool changed = true;
    
    // Keep iterating until no new nullable variables are found
    while (changed) {
        changed = false;
        
        for (auto& p : gr) {
            char var = p.first;
            
            // Skip if already marked as nullable
            if (NullableVars.find(var) != NullableVars.end() ) {
                continue;
            }
            
            // Check if Variable produces Epsilon directly
            for (const string& prod : p.second) {
                if (prod.size() == 1 && prod[0] == EPSILON) {
                    NullableVars.insert(var);
                    changed = true;
                    break;
                }
                
                // Check if all symbols in production are nullable
                bool allNullable = true;
                for (char c : prod) {
                    if (isVarChar(c) && NullableVars.find(c) == NullableVars.end() ) {
                        allNullable = false;
                        break;
                    }
                    else if (!isVarChar(c)) {
                        allNullable = false;
                        break;
                    }
                }
                
                if (allNullable && prod.size() > 0) {
                    NullableVars.insert(var);
                    changed = true;
                    break;
                }
            }
        }
    }
    
    // Generate new productions by eliminating epsilon
    GRAMMAR_RULES newGr;
    
    for (auto& p : gr) {
        char var = p.first;
        newGr[var] = set<string>();
        
        for (const string& prod : p.second) {
            // Skip epsilon productions
            if (prod.size() == 1 && prod[0] == EPSILON) {
                continue;
            }
            
            // Find positions of nullable variables in production
            vector<int> nullablePos;
            for (int i = 0; i < prod.size(); i++) {
                if (isVarChar(prod[i]) && NullableVars.find(prod[i]) != NullableVars.end() ) {
                    nullablePos.push_back(i);
                }
            }
            
            // Generate all combinations by removing nullable variables
            int numCombinations = 1 << nullablePos.size(); // 2^n combinations
            
            for (int mask = 0; mask < numCombinations; mask++) {
                string newProd = "";
                
                for (int i = 0; i < prod.size(); i++) {
                    bool skip = false;
                    
                    // Check if this position should be skipped
                    for (int j = 0; j < nullablePos.size(); j++) {
                        if (nullablePos[j] == i && (mask & (1 << j)) ) {
                            skip = true;
                            break;
                        }
                    }
                    
                    if (!skip) {
                        newProd += prod[i];
                    }
                }
                
                // Add non-empty productions
                if (newProd.size() > 0) {
                    newGr[var].insert(newProd);
                }
            }
        }
    }
    
    return newGr;
}

int main () {
    GRAMMAR_RULES gr = {
        {'S', {"ABC"}},
        {'A', {"aA", "E"}},
        {'B', {"bB", "Cb", "C", "E"}},
        {'C', {"c"}}
    };

    PrintGrammar(gr, "Original Grammar");
    
    GRAMMAR_RULES sgr = _eliminateUnused(gr);

    PrintGrammar(gr, "Removed Unused");

    GRAMMAR_RULES rugr = _eliminateUnit(gr);

    PrintGrammar(rugr, "Removed Unit");

    GRAMMAR_RULES regr = _eliminateEpsilon(rugr);

    PrintGrammar(regr, "Removed Epsilon");

    return 0;
}