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
constexpr char END_MARKER = '$';

struct FirstFollow {
    set<char> first;
    set<char> follow;
    
    FirstFollow() : first(set<char>()), follow(set<char>()) {}
};

void PrintFirstFollow(const unordered_map<char, FirstFollow>& ff) {
    cout << "\nFirst and Follow Sets:\n";
    cout << "=====================\n";
    for (auto& p : ff) {
        cout << p.first << ":\n";
        
        cout << "  First  = { ";
        for (char c : p.second.first) {
            cout << c << " ";
        }
        cout << "}\n";
        
        cout << "  Follow = { ";
        for (char c : p.second.follow) {
            cout << c << " ";
        }
        cout << "}\n\n";
    }
}

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

unordered_map<char, FirstFollow> ComputeFirstFollow(GRAMMAR_RULES gr, char StartVar = 'S') {
    // Step 1: Eliminate Epsilon productions for simplified computation
    GRAMMAR_RULES epsilonReduced = _eliminateEpsilon(gr);
    
    unordered_map<char, FirstFollow> result;
    
    // Initialize FirstFollow for all variables
    for (auto& p : epsilonReduced) {
        result[p.first] = FirstFollow();
    }
    
    // Step 2: Compute FIRST sets
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (auto& p : epsilonReduced) {
            char var = p.first;
            
            for (const string& prod : p.second) {
                if (prod.size() == 0) continue;
                
                char firstSym = prod[0];
                
                // If first symbol is terminal, add to FIRST set
                if (!isVarChar(firstSym)) {
                    if (result[var].first.find(firstSym) == result[var].first.end()) {
                        result[var].first.insert(firstSym);
                        changed = true;
                    }
                }
                // If first symbol is variable, add its FIRST to current variable's FIRST
                else {
                    for (char c : result[firstSym].first) {
                        if (result[var].first.find(c) == result[var].first.end()) {
                            result[var].first.insert(c);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    
    // Step 3: Compute FOLLOW sets
    // Add $ to start variable's follow
    result[StartVar].follow.insert(END_MARKER);
    
    changed = true;
    while (changed) {
        changed = false;
        
        for (auto& p : epsilonReduced) {
            char var = p.first;
            
            for (const string& prod : p.second) {
                // Check each symbol in production
                for (int i = 0; i < prod.size(); i++) {
                    char currSym = prod[i];
                    
                    // Only process variables
                    if (!isVarChar(currSym)) continue;
                    
                    // Check if there's a symbol after current symbol
                    if (i + 1 < prod.size()) {
                        char nextSym = prod[i + 1];
                        
                        // If next symbol is terminal, add to FOLLOW
                        if (!isVarChar(nextSym)) {
                            if (result[currSym].follow.find(nextSym) == result[currSym].follow.end()) {
                                result[currSym].follow.insert(nextSym);
                                changed = true;
                            }
                        }
                        // If next symbol is variable, add its FIRST to current's FOLLOW
                        else {
                            for (char c : result[nextSym].first) {
                                if (result[currSym].follow.find(c) == result[currSym].follow.end()) {
                                    result[currSym].follow.insert(c);
                                    changed = true;
                                }
                            }
                        }
                    }
                    
                    // If variable is at the end of production, add producer's FOLLOW to its FOLLOW
                    if (i == prod.size() - 1) {
                        for (char c : result[var].follow) {
                            if (result[currSym].follow.find(c) == result[currSym].follow.end()) {
                                result[currSym].follow.insert(c);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

int main () {
    GRAMMAR_RULES gr = {
        {'S', {"ABC"}},
        {'A', {"aA", "E"}},
        {'B', {"bB", "Cb", "C", "E"}},
        {'C', {"c"}}
    };

    PrintGrammar(gr, "Original Grammar");

    GRAMMAR_RULES regr = _eliminateEpsilon(gr);

    PrintGrammar(regr, "Epsilon Reduced Grammar");

    unordered_map<char, FirstFollow> ff = ComputeFirstFollow(gr, 'S');

    PrintFirstFollow(ff);

    return 0;
}