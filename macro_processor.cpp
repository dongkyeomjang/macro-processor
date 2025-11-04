#include <bits/stdc++.h>
using namespace std;

string trim(string s) {
    while (!s.empty() && isspace(s.front())) s.erase(s.begin());
    while (!s.empty() && isspace(s.back())) s.pop_back();
    return s;
}

string replaceAll(string s, const string &from, const string &to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

string formatAssemblyLine(const string &line) {
    string trimmed = trim(line);
    if (trimmed.empty()) return "";

    if (isspace(line[0])) return line;

    stringstream ss(trimmed);
    string label, rest;
    ss >> label;
    getline(ss, rest);
    rest = trim(rest);

    string result = label;
    int labelWidth = 9;
    if ((int)label.size() < labelWidth)
        result.append(labelWidth - label.size(), ' ');
    if (!rest.empty()) result += rest;
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> lines;
    string line;
    while (getline(cin, line)) lines.push_back(line);

    string macroName;
    vector<string> params, body;
    bool inMacro = false;
    vector<string> output;

    for (auto &l : lines) {
        string t = trim(l);
        if (!inMacro) {
            stringstream ss(t);
            string w1, w2;
            ss >> w1 >> w2;
            if (!w2.empty() && w2 == "MACRO") {
                macroName = w1;
                inMacro = true;
                string rest;
                getline(ss, rest);
                stringstream ps(rest);
                string p;
                while (getline(ps, p, ',')) {
                    p = trim(p);
                    if (p[0] != '&') p = '&' + p;
                    params.push_back(p);
                }
            } else {
                output.push_back(l);
            }
        } else {
            if (trim(t) == "MEND") inMacro = false;
            else body.push_back(l);
        }
    }

    vector<string> finalOut;
    for (auto &l : output) {
        string t = trim(l);
        if (!macroName.empty() && t.rfind(macroName + " ", 0) == 0) {
            string args = trim(t.substr(macroName.size()));
            vector<string> actuals;
            stringstream ss(args);
            string a;
            while (getline(ss, a, ',')) actuals.push_back(trim(a));

            for (auto &b : body) {
                string out = b;
                for (size_t i = 0; i < params.size(); i++)
                    out = replaceAll(out, params[i], actuals[i]);
                finalOut.push_back(formatAssemblyLine(out));
            }
        } else {
            finalOut.push_back(l);
        }
    }

    for (auto &l : finalOut)
        cout << l << "\n";
}