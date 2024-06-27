#include <vector>
#include <optional>

#include "logic_core/card.h"

namespace logic_core
{
    class Block
    {
    public:
        void put(const Card& card);

    private:
        bool is_empty = true;
        Card card;
    };

    class Board
    {
    public:

    private:
        std::vector<std::vector<Block>> blocks;
        std::vector<Card> hand_cards;
        std::vector<Card> rival_cards;
        std::vector<Card> card_deck;
    };
}