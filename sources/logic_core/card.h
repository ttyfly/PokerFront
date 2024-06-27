#include <vector>

namespace logic_core
{
    enum class CardRank
    {
        A = 1,
        _2 = 2,
        _3 = 3,
        _4 = 4,
        _5 = 5,
        _6 = 6,
        _7 = 7,
        _8 = 8,
        _9 = 9,
        _10 = 10,
        J = 11,
        Q = 12,
        K = 13,
        BlackJoker = -1,
        RedJoker = -2,
    };

    enum class CardSuit
    {
        Spade = 1,
        Club = 2,
        Diamond = 3,
        Heart = 4,
    };

    class Card
    {
        CardRank rank;
        CardSuit suit;
    };
}