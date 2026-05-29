#include <algorithm>
#include <cstring>
#include <exception>
#include <string>

#include <script/miniscript.h>

#include "compiler.h"

namespace {

using miniscript::operator"" _mst;

void Output(const std::string& str, char* out, int outlen) {
    int maxlen = std::min<int>(outlen - 1, str.size());
    memcpy(out, str.c_str(), maxlen);
    out[maxlen] = 0;
}

std::string TypeString(const miniscript::NodeRef<std::string>& node) {
    if (node->GetType() == ""_mst) return "[invalid]";

    std::string ret;
    if (node->GetType() << "B"_mst) ret += 'B';
    if (node->GetType() << "V"_mst) ret += 'V';
    if (node->GetType() << "W"_mst) ret += 'W';
    if (node->GetType() << "K"_mst) ret += 'K';
    if (node->GetType() << "z"_mst) ret += 'z';
    if (node->GetType() << "o"_mst) ret += 'o';
    if (node->GetType() << "n"_mst) ret += 'n';
    if (node->GetType() << "d"_mst) ret += 'd';
    if (node->GetType() << "f"_mst) ret += 'f';
    if (node->GetType() << "e"_mst) ret += 'e';
    if (node->GetType() << "m"_mst) ret += 'm';
    if (node->GetType() << "u"_mst) ret += 'u';
    if (node->GetType() << "s"_mst) ret += 's';
    if (node->GetType() << "k"_mst) ret += 'k';
    return ret;
}

std::string Props(const miniscript::NodeRef<std::string>& node, const std::string& name) {
    return name + " (type: " + TypeString(node) +
           ", scriptlen: " + std::to_string(node->ScriptSize()) +
           ", max ops: " + std::to_string(node->GetOps()) +
           ", max stack size: " + std::to_string(node->GetStackSize()) + ")";
}

void Analyze(const miniscript::NodeRef<std::string>& node, std::string& out, int indent) {
    const std::string pad(indent, ' ');
    auto append_node = [&](const std::string& name) {
        out += pad + Props(node, name) + "\n";
    };
    auto append_children = [&]() {
        for (const auto& sub : node->subs) {
            Analyze(sub, out, indent + 2);
        }
    };

    switch (node->fragment) {
        case miniscript::Fragment::PK_K:
            append_node("pk_k(" + (*COMPILER_CTX.ToString(node->keys[0])) + ")");
            break;
        case miniscript::Fragment::PK_H:
            append_node("pk_h(" + (*COMPILER_CTX.ToString(node->keys[0])) + ")");
            break;
        case miniscript::Fragment::MULTI:
            append_node("multi(" + std::to_string(node->k) + " of " + std::to_string(node->keys.size()) + ")");
            break;
        case miniscript::Fragment::AFTER:
            append_node("after(" + std::to_string(node->k) + ")");
            break;
        case miniscript::Fragment::OLDER:
            append_node("older(" + std::to_string(node->k) + ")");
            break;
        case miniscript::Fragment::SHA256:
            append_node("sha256()");
            break;
        case miniscript::Fragment::RIPEMD160:
            append_node("ripemd160()");
            break;
        case miniscript::Fragment::HASH256:
            append_node("hash256()");
            break;
        case miniscript::Fragment::HASH160:
            append_node("hash160()");
            break;
        case miniscript::Fragment::JUST_0:
            append_node("false");
            break;
        case miniscript::Fragment::JUST_1:
            append_node("true");
            break;
        case miniscript::Fragment::WRAP_A:
            append_node("a:");
            Analyze(node->subs[0], out, indent + 2);
            break;
        case miniscript::Fragment::WRAP_S:
            append_node("s:");
            Analyze(node->subs[0], out, indent + 2);
            break;
        case miniscript::Fragment::WRAP_C:
            append_node("c:");
            Analyze(node->subs[0], out, indent + 2);
            break;
        case miniscript::Fragment::WRAP_D:
            append_node("d:");
            Analyze(node->subs[0], out, indent + 2);
            break;
        case miniscript::Fragment::WRAP_V:
            append_node("v:");
            Analyze(node->subs[0], out, indent + 2);
            break;
        case miniscript::Fragment::WRAP_N:
            append_node("n:");
            Analyze(node->subs[0], out, indent + 2);
            break;
        case miniscript::Fragment::WRAP_J:
            append_node("j:");
            Analyze(node->subs[0], out, indent + 2);
            break;
        case miniscript::Fragment::AND_V:
            append_node("and_v");
            append_children();
            break;
        case miniscript::Fragment::AND_B:
            append_node("and_b");
            append_children();
            break;
        case miniscript::Fragment::OR_B:
            append_node("or_b");
            append_children();
            break;
        case miniscript::Fragment::OR_C:
            append_node("or_c");
            append_children();
            break;
        case miniscript::Fragment::OR_D:
            append_node("or_d");
            append_children();
            break;
        case miniscript::Fragment::OR_I:
            append_node("or_i");
            append_children();
            break;
        case miniscript::Fragment::ANDOR:
            append_node("andor");
            append_children();
            break;
        case miniscript::Fragment::THRESH:
            append_node("thresh(" + std::to_string(node->k) + " of " + std::to_string(node->subs.size()) + ")");
            append_children();
            break;
    }
}

}

extern "C" {

void miniscript_compile(const char* desc, char* msout, int msoutlen, char* costout, int costoutlen, char* asmout, int asmoutlen) {
    try {
        std::string str(desc);
        str.erase(str.find_last_not_of(" \n\r\t") + 1);
        miniscript::NodeRef<std::string> ret;
        double avgcost;
        if (!Compile(Expand(str), ret, avgcost)) {
            Output("[compile error]", msout, msoutlen);
            Output("[compile error]", costout, costoutlen);
            Output("[compile error]", asmout, asmoutlen);
            return;
        }
        Output(Abbreviate(*(ret->ToString(COMPILER_CTX))), msout, msoutlen);
        std::string coststr = "Script: " + std::to_string(ret->ScriptSize()) +
                              " WU\nInput: " + std::to_string(avgcost) +
                              " WU\nTotal: " + std::to_string(ret->ScriptSize() + avgcost) + " WU";
        Output(coststr, costout, costoutlen);
        Output(Disassemble(ret->ToScript(COMPILER_CTX)), asmout, asmoutlen);
    } catch (const std::exception& e) {
        Output("[exception: " + std::string(e.what()) + "]", msout, msoutlen);
        Output("", costout, costoutlen);
        Output("", asmout, asmoutlen);
    }
}

void miniscript_analyze(const char* ms, char* costout, int costoutlen, char* asmout, int asmoutlen) {
    try {
        std::string str(ms);
        str.erase(str.find_last_not_of(" \n\r\t") + 1);
        miniscript::NodeRef<std::string> ret;
        ret = miniscript::FromString(Expand(str), COMPILER_CTX);
        if (!ret || !ret->IsValidTopLevel()) {
            Output("[analysis error]", costout, costoutlen);
            Output("[analysis error]", asmout, asmoutlen);
            return;
        }
        std::string analysis = "Size: " + std::to_string(ret->ScriptSize()) + " bytes script\n";
        Analyze(ret, analysis, 0);
        Output(analysis, costout, costoutlen);
        Output(Disassemble(ret->ToScript(COMPILER_CTX)), asmout, asmoutlen);
    } catch (const std::exception& e) {
        Output("[exception: " + std::string(e.what()) + "]", costout, costoutlen);
        Output("", asmout, asmoutlen);
    }
}

}
