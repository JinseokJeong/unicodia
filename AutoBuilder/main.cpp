// C++
#include <iostream>
#include <fstream>
#include <charconv>

// PugiXML
#include "pugixml.hpp"

// Project-local
#include "data.h"

using namespace std::string_view_literals;

template <class T>
inline auto need(T&& val, const char* errmsg)
{
    if (!val) throw std::logic_error(errmsg);
    return std::forward<T>(val);
}

struct Fraction {
    long long num, denom;
};

long long fromChars(std::string_view x, std::string_view numType)
{
    long long r;
    auto beg = x.data();
    auto end = beg + x.length();
    auto res = std::from_chars(beg, end, r);
    if (res.ec != std::errc() || res.ptr != end) {
        std::string errm = "[fromChars] Cannot parse number ";
        errm.append(x);
        errm.append(", numType = ");
        errm.append(numType);
        throw std::invalid_argument(errm);
    }
    return r;
}

unsigned fromHex(std::string_view x)
{
    long long r;
    auto beg = x.data();
    auto end = beg + x.length();
    auto res = std::from_chars(beg, end, r, 16);
    if (res.ec != std::errc() || res.ptr != end) {
        std::string errm = "[fromChars] Cannot parse hex ";
        errm.append(x);
        throw std::invalid_argument(errm);
    }
    return r;
}

Fraction parseFraction(std::string_view numType, std::string_view x)
{
    auto pSlash = x.find('/');
    if (pSlash == std::string_view::npos) {
        // Integer
        return { fromChars(x, numType), 1 };
    } else {
        // Fraction
        auto num = x.substr(0, pSlash);
        auto den = x.substr(pSlash + 1);
        return { fromChars(num, numType), fromChars(den, numType) };
    }
}

std::string transformVersion(std::string s)
{
    for (auto& c : s) {
        if (c == '.')
            c = '_';
    }
    return s;
}

inline std::string transformVersion(std::string_view s) { return transformVersion(std::string{s}); }

struct StrMap {
    std::string_view source, image;
};

template <size_t N>
std::string_view transform(std::string_view x, StrMap (&map)[N])
{
    for (auto& v : map) {
        if (v.source == x) {
            return v.image;
        }
    }
    std::string errm = "[transform] Bad string ";
    errm.append(x);
    throw std::invalid_argument(std::move(errm));
}

StrMap smNumType[] {
    { "De"sv, "DIGIT" },
    { "Di"sv, "SPECIAL_DIGIT" },
    { "Nu"sv, "NUMBER" },
};
StrMap smCharCat[] {
    { "Cc"sv, "CONTROL"sv },
    { "Cf"sv, "FORMAT"sv },
    { "Ll"sv, "LETTER_LOWERCASE"sv },
    { "Lm"sv, "LETTER_MODIFIER"sv },
    { "Lo"sv, "LETTER_OTHER"sv },
    { "Lt"sv, "LETTER_TITLECASE"sv },
    { "Lu"sv, "LETTER_UPPERCASE"sv },
    { "Mc"sv, "MARK_SPACING"sv },
    { "Me"sv, "MARK_ENCLOSING"sv },
    { "Mn"sv, "MARK_NONSPACING"sv },
    { "Nd"sv, "NUMBER_DECIMAL"sv },
    { "Nl"sv, "NUMBER_LETTER"sv },
    { "No"sv, "NUMBER_OTHER"sv },
    { "Pc"sv, "PUNCTUATION_CONNECTOR"sv },
    { "Pd"sv, "PUNCTUATION_DASH"sv },
    { "Pe"sv, "PUNCTUATION_CLOSE"sv },
    { "Pf"sv, "PUNCTUATION_FINAL"sv },
    { "Pi"sv, "PUNCTUATION_INITIAL"sv },
    { "Po"sv, "PUNCTUATION_OTHER"sv },
    { "Ps"sv, "PUNCTUATION_OPEN"sv },
    { "Sc"sv, "SYMBOL_CURRENCY"sv },
    { "Sk"sv, "SYMBOL_MODIFIER"sv },
    { "Sm"sv, "SYMBOL_MATH"sv },
    { "So"sv, "SYMBOL_OTHER"sv },
    { "Zl"sv, "SEPARATOR_LINE"sv },
    { "Zp"sv, "SEPARATOR_PARAGRAPH"sv },
    { "Zs"sv, "SEPARATOR_SPACE"sv },
};

struct StringPayload
{
    char32_t subj = 0;
    int offset = -1;
};

class StringLib
{
public:
    /// @return offset
    int remember(const std::string& s, char32_t subj);
    auto& inOrder() const { return fInOrder; }
private:
    using M = std::unordered_map<std::string, StringPayload>;
    M fData;
    std::vector<const M::value_type*> fInOrder;
    size_t fLength = 0;
};

int StringLib::remember(const std::string& s, char32_t subj)
{
    auto [it, wasIns] = fData.try_emplace(s, StringPayload());
    if (wasIns) {   // Was inserted
        it->second.subj = subj;
        it->second.offset = fLength;
        fLength += (s.length() + 1);
        fInOrder.push_back(&*it);
    } else {}    // was found — do nothing
    return it->second.offset;
}


bool hasSubstr(std::string_view haystack, std::string_view needle)
{
    auto pos = haystack.find(needle);
    return (pos != std::string_view::npos);
}


int main()
{
    std::ofstream os("UcAuto.cpp");
    os << "// Automatically generated, do not edit!" << '\n';
    os << '\n';
    os << R"(#include "UcData.h")" << '\n';
    os << '\n';
    os << R"(using namespace std::string_view_literals;)" << '\n';
    os << '\n';

    pugi::xml_document doc;

    std::cout << "Loading Unicode base..." << std::flush;
    doc.load_file("ucd.all.flat.xml");
    std::cout << "OK" << std::endl;

    ///// CpInfo ///////////////////////////////////////////////////////////////

    unsigned nChars = 0, nSpecialRanges = 0;

    auto elRoot = need(doc.root().child("ucd"), "Need <ucd>");
    auto elRepertoire = need(elRoot.child("repertoire"), "Need <repertoire>");
    std::cout << "Found repertoire, generating character info..." << std::flush;
    os << '\n';
    os << R"(uc::Cp uc::cpInfo[N_CPS] {)" << '\n';

    StringLib strings;
    for (pugi::xml_node elChar : elRepertoire.children("char")) {
        std::string_view sCp = elChar.attribute("cp").as_string();
        if (sCp.empty()) {
            ++nSpecialRanges;
            continue;
        }
        // Get CP
        auto cp = fromHex(sCp);

        // Aliases:
        // • Abbreviation: Implement later
        // • Alternate: Prefer na1
        // • Control: Prefer na1
        // • Figment: Implement

        /// @todo [future] Sometimes we have fixups, what to take?
        /// @todo [future] Sometimes we have abbreviations, take them
        std::string_view sName = elChar.attribute("na").as_string();
        if (sName.empty())
            sName = elChar.attribute("na1").as_string();

        // Aliases?
        for (auto elAlias : elChar.children("name-alias")) {
            std::string_view sType = elAlias.attribute("type").as_string();
            if (sType == "alternate"sv || sType == "control"sv || sType == "figment") {
                if (sName.empty())
                    sName = elAlias.attribute("alias").as_string();
            } else if (sType == "correction") {
                // Checked known chars, and corrections ARE BETTER than originals
                sName = elAlias.attribute("alias").as_string();
            }
        }

        std::string sLowerName = decapitalize(sName);
        auto iTech = strings.remember(sLowerName, cp);
        os << "{ "
           << "0x" << std::hex << cp << ", "  // subj
           << "{ "                      // name
                << std::dec << iTech << ", "  // name.tech,
                << "-1 "                      // name.loc
           << " }, ";                   // /name

        // Char’s type
        std::string_view sCharCat = elChar.attribute("gc").as_string();
        os << "EcCategory::" << transform(sCharCat, smCharCat) << ", ";

        // Char’s version
        std::string_view sVersion = elChar.attribute("age").as_string();
        os << "EcVersion::V_" << transformVersion(sVersion) << ", ";

        // Char’s bidirectional data
        std::string_view sBidiClass = elChar.attribute("bc").as_string();
        bool isMirrored = elChar.attribute("Bidi_M").as_bool();
        if (isMirrored) {
            if (sBidiClass != "ON"sv)
                throw std::logic_error("Got strange bidi class for mirrored char");
            sBidiClass = "MIR";
        }
        os << "EcBidiClass::z_" << sBidiClass << ", ";

        // Char’s script
        std::string_view sScript = elChar.attribute("sc").as_string();
        //std::string_view sScriptX = elChar.attribute("scx").as_string();
        //if (sScript != sScriptX) {
        //    std::cout << sCp << " has sc=" << sScript << ", scx=" << sScriptX << std::endl;
        //}
        os << "EcScript::" << sScript << ", ";

        // Char’s numeric values
        // nt = …
        //    • None — no numeric value
        //    • De — decimal digit
        //    • Di — special digit
        //    • Nu — number
        // nv = Nan / whole number / vulgar fraction
        std::string_view sNumType = elChar.attribute("nt").as_string();
        os << '{';
        if (sNumType != "None"sv) {
            std::string_view sNumValue = elChar.attribute("nv").as_string();
            auto frac = parseFraction(sNumType, sNumValue);
            os << frac.num << ", " << frac.denom << ", "
               << " EcNumType::" << transform(sNumType, smNumType) << ' ';
        }
        os << "} ";

        os << "}," << '\n';
        ++nChars;
    }

    os << "};" << '\n';

    std::cout << "OK" << std::endl;
    std::cout << "Found " << nChars << " chars, " << nSpecialRanges << " special ranges " << std::endl;

    os << "const char8_t uc::allStrings[] = \n";
    for (auto& v : strings.inOrder()) {
        os << R"(u8")" << v->first << R"("   "\0"   // )" << std::hex << static_cast<int>(v->second.subj) << '\n';
    }
    os << ";\n";

    ///// Blocks ///////////////////////////////////////////////////////////////

    os << R"(const uc::Block uc::blocks[N_BLOCKS] {)" << '\n';
    std::cout << "Found blocks, generating block info..." << std::flush;

    size_t nBlocks = 0;
    auto elBlocks = need(elRoot.child("blocks"), "Need <blocks>");

    for (pugi::xml_node elBlock : elBlocks.children("block")) {
        auto sFirst = elBlock.attribute("first-cp").as_string();
        auto sLast = elBlock.attribute("last-cp").as_string();
        std::string_view name = elBlock.attribute("name").as_string();

        if (hasSubstr(name, "Private Use") || hasSubstr(name, "Surrogate"))
            continue;

        os << "{ 0x" << sFirst << ", 0x" << sLast << ", \""
           << name << "\"sv },\n";
        ++nBlocks;
    }
    os << "};" << '\n';

    std::cout << "OK" << std::endl;
    std::cout << "Found " << nBlocks << " blocks" << std::endl;

    os.close();

    os.open("UcAutoCount.h");
    os << "#pragma once\n";
    os << '\n';
    os << "// Automatically generated, do not edit!\n";
    os << '\n';
    os << "namespace uc {\n";
    os << "constexpr int N_CPS = " << std::dec << nChars << ";\n";
    os << "constexpr int N_BLOCKS = " << nBlocks << ";\n";
    os << "}\n";

    std::cout << "Successfully finished!" << std::endl << std::endl;

    return 0;
}
