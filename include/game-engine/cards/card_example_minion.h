#pragma once

#error "This file should not be included"

DEFINE_CARD_CLASS_START(CS2_999999)

static void GetRequiredTargets(Player const& player, SlotIndexBitmap &targets, bool & meet_requirements)
{
}

static void BattleCry(Player & player, Move::PlayMinionData const& play_minion_data)
{
}

static void AfterSummoned(MinionIterator summoned_minion)
{
	// Do NOT attach deathrattle to minion; the framework will automatically attach it.
}

static void Deathrattle(MinionIterator & triggering_minion)
{

}

DEFINE_CARD_CLASS_END()