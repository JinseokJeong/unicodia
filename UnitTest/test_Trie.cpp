// What we are testing
#include "Search/trie.h"

// Google test
#include "gtest/gtest.h"

#include "UcCp.h"


enum class Emoji {
    CUBA,
    PUERTO_RICO,
    SPAIN,
    WOMAN_WHITE,
    MAN_BLACK,
    HEART_RED,
    KISS_INTERRACIAL,
};


class Trie1 : public srh::TrieRoot<Emoji>
{
public:
    Trie1();
};

Trie1::Trie1()
{
    addMulti(Emoji::CUBA,        cp::FLAG_C, cp::FLAG_U);
    addMulti(Emoji::PUERTO_RICO, cp::FLAG_P, cp::FLAG_R);
    addMulti(Emoji::SPAIN,       cp::FLAG_E, cp::FLAG_S);
    addMulti(Emoji::WOMAN_WHITE, cp::WOMAN,  cp::SKIN1);
    addMulti(Emoji::MAN_BLACK,   cp::MAN,    cp::SKIN5);
    addMulti(Emoji::HEART_RED,   cp::EMOJI_RED_HEART, cp::VS16);
    addMulti(Emoji::KISS_INTERRACIAL,
               cp::WOMAN, cp::SKIN1, cp::ZWJ,
               cp::EMOJI_RED_HEART, cp::VS16, cp::ZWJ,
               cp::KISS_MARK, cp::ZWJ,
               cp::MAN, cp::SKIN5);
};

///
///  This was a trouble for #371
///
TEST (DecodeTrie, ThreeFlags)
{
    Trie1 tr;
    const char32_t data[] { cp::FLAG_P, cp::FLAG_R, cp::FLAG_C, cp::FLAG_U, cp::FLAG_E, cp::FLAG_S, 0 };
    auto res = tr.decode(data);

    EXPECT_EQ(3u, res.size());

    auto& r0 = res[0];
    EXPECT_EQ(0u, r0.index);
    EXPECT_EQ(Emoji::PUERTO_RICO, r0.result);

    auto& r1 = res[1];
    EXPECT_EQ(2u, r1.index);
    EXPECT_EQ(Emoji::CUBA, r1.result);

    auto& r2 = res[2];
    EXPECT_EQ(4u, r2.index);
    EXPECT_EQ(Emoji::SPAIN, r2.result);
}


///
///  Prerequisite: decode interracial kiss
///
TEST (DecodeTrie, InterracialKiss)
{
    Trie1 tr;
    const char32_t data[] { 'A',
                cp::WOMAN, cp::SKIN1, cp::ZWJ,
                cp::EMOJI_RED_HEART, cp::VS16, cp::ZWJ,
                cp::KISS_MARK, cp::ZWJ,
                cp::MAN, cp::SKIN5, 0 };
    auto res = tr.decode(data);

    EXPECT_EQ(1u, res.size());

    auto& r0 = res[0];
    EXPECT_EQ(1u, r0.index);
    EXPECT_EQ(Emoji::KISS_INTERRACIAL, r0.result);
}


///
///  Prerequisite: decode interracial kiss, then man
///
TEST (DecodeTrie, InterracialKissMan)
{
    Trie1 tr;
    const char32_t data[] { 'A',
                cp::WOMAN, cp::SKIN1, cp::ZWJ,
                cp::EMOJI_RED_HEART, cp::VS16, cp::ZWJ,
                cp::KISS_MARK, cp::ZWJ,
                cp::MAN, cp::SKIN5,
                'B',
                cp::MAN, cp::SKIN5, 0 };
    auto res = tr.decode(data);

    EXPECT_EQ(2u, res.size());

    auto& r0 = res[0];
    EXPECT_EQ(1u, r0.index);
    EXPECT_EQ(Emoji::KISS_INTERRACIAL, r0.result);

    auto& r1 = res[1];
    EXPECT_EQ(12u, r1.index);
    EXPECT_EQ(Emoji::MAN_BLACK, r1.result);
}


///
///  Incomplete interracial kiss with some bad char
///
TEST (DecodeTrie, KissBadChar)
{
    Trie1 tr;
    const char32_t data[] {
                cp::WOMAN, cp::SKIN1, cp::ZWJ,
                cp::EMOJI_RED_HEART, cp::VS16, cp::ZWJ,
                cp::KISS_MARK, cp::ZWJ,
                cp::MAN, 'A', 0 };
    auto res = tr.decode(data);

    EXPECT_EQ(2u, res.size());

    auto& r0 = res[0];
    EXPECT_EQ(0u, r0.index);
    EXPECT_EQ(Emoji::WOMAN_WHITE, r0.result);

    auto& r1 = res[1];
    EXPECT_EQ(3u, r1.index);
    EXPECT_EQ(Emoji::HEART_RED, r1.result);
}


///
///  Incomplete interracial kiss that ends abruptly
///
TEST (DecodeTrie, KissAbrupt)
{
    Trie1 tr;
    const char32_t data[] {
                cp::WOMAN, cp::SKIN1, cp::ZWJ,
                cp::EMOJI_RED_HEART, cp::VS16, cp::ZWJ,
                cp::KISS_MARK, cp::ZWJ,
                cp::MAN, 0 };
    auto res = tr.decode(data);

    EXPECT_EQ(2u, res.size());

    auto& r0 = res[0];
    EXPECT_EQ(0u, r0.index);
    EXPECT_EQ(Emoji::WOMAN_WHITE, r0.result);

    auto& r1 = res[1];
    EXPECT_EQ(3u, r1.index);
    EXPECT_EQ(Emoji::HEART_RED, r1.result);
}


///
///  Incomplete interracial kiss followed with more emoji
///
TEST (DecodeTrie, KissMoreEmoji)
{
    Trie1 tr;
    const char32_t data[] {
                cp::WOMAN, cp::SKIN1, cp::ZWJ,
                cp::EMOJI_RED_HEART, cp::VS16, cp::ZWJ,
                cp::KISS_MARK, cp::ZWJ,
                cp::MAN,
                cp::FLAG_E, cp::FLAG_S, 0 };
    auto res = tr.decode(data);

    EXPECT_EQ(3u, res.size());

    auto& r0 = res[0];
    EXPECT_EQ(0u, r0.index);
    EXPECT_EQ(Emoji::WOMAN_WHITE, r0.result);

    auto& r1 = res[1];
    EXPECT_EQ(3u, r1.index);
    EXPECT_EQ(Emoji::HEART_RED, r1.result);

    auto& r2 = res[2];
    EXPECT_EQ(9u, r2.index);
    EXPECT_EQ(Emoji::SPAIN, r2.result);
}
