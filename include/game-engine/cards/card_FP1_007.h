#ifndef GAME_ENGINE_CARDS_CARD_FP1_007
#define GAME_ENGINE_CARDS_CARD_FP1_007

#include "game-engine/board.h"
#include "game-engine/card-id-map.h"
#include "game-engine/stages/helper.h"

namespace GameEngine {
namespace Cards {

class Card_FP1_007
{
public:
	static constexpr int card_id = CARD_ID_FP1_007;

	// Nerubian Egg

	static void Deathrattle(GameEngine::Board & board, GameEngine::BoardObjects::MinionsIteratorWithIndex triggering_minion)
	{
		// summon Nerubian (AT_036t) when death at [pos]
		Card card = CardDatabase::GetInstance().GetCard(CARD_ID_AT_036t);
		
		StageHelper::SummonMinion(board, card, triggering_minion);
	}

	static void AfterSummoned(GameEngine::Board &, GameEngine::BoardObjects::Minion & summoned_minion)
	{
		// summon Nerubian (AT_036t) when death
		summoned_minion.AddOnDeathTrigger(Deathrattle);
	}
};

} // namespace Cards
} // namespace GameEngine

#endif
