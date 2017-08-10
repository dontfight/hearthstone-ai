#pragma once

#include <random>
#include "MCTS/board/Board.h"

namespace mcts
{
	namespace policy
	{
		namespace simulation
		{
			class ChoiceGetter
			{
			public:
				ChoiceGetter(int choices) : choices_(choices) {}

				size_t Size() const { return (size_t)choices_; }

				int Get(size_t idx) const { return (int)idx; }

				template <typename Functor>
				void ForEachChoice(Functor&& functor) const {
					for (int i = 0; i < choices_; ++i) {
						if (!functor(i)) return;
					}
				}

			private:
				int choices_;
			};

			class RandomPolicy
			{
			public:
				static int GetChoice(ChoiceGetter const& choice_getter, board::Board const& board)
				{
					// TODO: use value network to enhance simulation
					size_t count = choice_getter.Size();
					assert(count > 0);
					size_t idx = 0;
					size_t rand_idx = (size_t)(std::rand() % count);
					int result = -1;
					choice_getter.ForEachChoice([&](int choice) {
						if (idx == rand_idx) {
							result = choice;
							return false;
						}
						++idx;
						return true;
					});
					assert([&]() {
						int result2 = choice_getter.Get(rand_idx);
						return result == result2;
					}());
					return result;
				}
			};

			class StateValueFunction
			{
			public:
				double GetStateValue(board::Board const& board) {
					board::BoardView view = board.CreateView(); // TODO: can we save this creation, just peak the board?

					double v = 0.0;

					for (auto const& minion : view.GetSelfMinions()) {
						v += 10.0 * GetMinionValue(minion.attack, minion.hp);
					}

					for (auto const& minion : view.GetOpponentMinions()) {
						v += -10.0 * GetMinionValue(minion.attack, minion.hp);
					}

					v += 3.0 * GetHeroValue(view.GetSelfHero().hp, view.GetSelfHero().armor);
					v += -3.0 * GetHeroValue(view.GetOpponentHero().hp, view.GetOpponentHero().armor);

					return v;
				}

			private:
				double GetMinionValue(int attack, int hp) {
					return 1.0 * attack + 1.5 * hp;
				}

				double GetHeroValue(int hp, int armor) {
					double v = hp + armor;
					if (v >= 15) v = 15;
					return v;
				}
			};

			class HeuristicPolicy
			{
			public:
				HeuristicPolicy(std::mt19937 & rand) : rand_(rand), decision_(), state_value_func_()
				{
				}

				int GetChoice(
					board::Board const& board,
					board::BoardActionAnalyzer & action_analyzer,
					ActionType action_type,
					ChoiceGetter const& choice_getter)
				{
					if (action_type == ActionType::kMainAction) {
						StartNewAction(board, action_analyzer);

						return GetChoiceForMainAction(
							board, action_analyzer, choice_getter);
					}

					// TODO: use value network to enhance simulation
					size_t count = choice_getter.Size();
					assert(count > 0);
					size_t idx = 0;
					size_t rand_idx = (size_t)(std::rand() % count);
					int result = -1;
					choice_getter.ForEachChoice([&](int choice) {
						if (idx == rand_idx) {
							result = choice;
							return false;
						}
						++idx;
						return true;
					});
					assert([&]() {
						int result2 = choice_getter.Get(rand_idx);
						return result == result2;
					}());
					return result;
				}

			private:
				void StartNewAction(
					board::Board const& board,
					board::BoardActionAnalyzer & action_analyzer)
				{
					DFSBestStateValue(board, action_analyzer);
					assert(!decision_.empty());
				}

				void DFSBestStateValue(
					board::Board const& board,
					board::BoardActionAnalyzer & action_analyzer)
				{
					class RandomPolicy : public mcts::board::IRandomGenerator {
					public:
						RandomPolicy(std::mt19937 & rand) : rand_(rand) {}
						int Get(int exclusive_max) final { return rand_() % exclusive_max; }

					private:
						std::mt19937 rand_;
					};

					struct DFSItem {
						size_t choice_;
						size_t total_;

						DFSItem(size_t choice, size_t total) : choice_(choice), total_(total) {}
					};

					class UserChoicePolicy : public mcts::board::IActionParameterGetter {
					public:
						UserChoicePolicy(std::vector<DFSItem> & dfs,
							std::vector<DFSItem>::iterator & dfs_it,
							std::mt19937 & rand) :
							dfs_(dfs), dfs_it_(dfs_it), rand_(rand)
						{}

						int GetNumber(ActionType::Types action_type, board::ActionChoices const& action_choices) {

							int total = action_choices.Size();

							assert(action_type != ActionType::kRandom);
							
							if (action_type == ActionType::kChooseMinionPutLocation) {
								assert(total >= 1);
								int idx = rand_() % total;
								return action_choices.Get(idx);
							}

							if (dfs_it_ == dfs_.end()) {
								assert(total >= 1);
								dfs_.emplace_back(0, (size_t)total);
								dfs_it_ = dfs_.end();
								return action_choices.Get(0);
							}
							
							assert(dfs_it_->total_ == (size_t)total);
							size_t idx = dfs_it_->choice_;
							assert((int)idx < total);
							++dfs_it_;
							return action_choices.Get(idx);
						}

					private:
						std::vector<DFSItem> & dfs_;
						std::vector<DFSItem>::iterator & dfs_it_;
						std::mt19937 & rand_;
					};

					std::vector<DFSItem> dfs;
					std::vector<DFSItem>::iterator dfs_it = dfs.begin();

					auto step_next_dfs = [&]() {
						while (!dfs.empty()) {
							if ((dfs.back().choice_ + 1) < dfs.back().total_) {
								break;
							}
							dfs.pop_back();
						}

						if (dfs.empty()) return false; // all done

						++dfs.back().choice_;
						return true;
					};

					// Need to fix a random sequence for a particular run
					// Since, some callbacks might depend on a random
					// For example, choose one card from randomly-chosen three cards
					std::mt19937 rand1(rand_());
					RandomPolicy cb_random(rand1);

					std::mt19937 rand2(rand_());
					UserChoicePolicy cb_user_choice(dfs, dfs_it, rand2);

					double best_value = -std::numeric_limits<double>::infinity();
					action_analyzer.ForEachMainOp([&](size_t idx, board::BoardActionAnalyzer::OpType main_op) {
						while (true) {
							board::CopiedBoard copy_board(board);

							dfs_it = dfs.begin();
							auto result = copy_board.GetBoard().ApplyAction(
								(int)idx,
								action_analyzer,
								cb_random,
								cb_user_choice);

							if (result != Result::kResultInvalid) {
								double value = -std::numeric_limits<double>::infinity();
								int self_win = copy_board.GetBoard().IsSelfWin(result);
								if (self_win == 1) {
									value = std::numeric_limits<double>::infinity();
								}
								else if (self_win == -1) {
									value = -std::numeric_limits<double>::infinity();
								}
								else {
									state_value_func_.GetStateValue(copy_board.GetBoard());
								}

								if (decision_.empty() || value > best_value) {
									best_value = value;

									decision_.clear();
									decision_.push_back((int)idx);
									for (auto const& item : dfs) {
										decision_.push_back((int)item.choice_);
									}
								}
							}

							if (!step_next_dfs()) break; // all done
						}
						return true;
					});
				}

				int GetChoiceForMainAction(
					board::Board const& board,
					board::BoardActionAnalyzer const& action_analyzer,
					ChoiceGetter const& choice_getter)
				{
					// TODO: maybe we don't need to rule out end-turn manually
					// it will be naturally ruled out, since its state-value should be lower

					// choose end-turn only when it is the only option
					size_t count = choice_getter.Size();
					assert(count > 0);
					if (count == 1) {
						// should be end-turn
						assert(action_analyzer.GetMainOpType((size_t)0) == board::BoardActionAnalyzer::kEndTurn);
						return 0;
					}

					// rule out the end-turn action
					assert(action_analyzer.GetMainOpType(count - 1) == board::BoardActionAnalyzer::kEndTurn);
					--count;
					assert(count > 0);

					// otherwise, choose randomly
					size_t rand_idx = (size_t)(std::rand() % count);
					return choice_getter.Get(rand_idx);
				}

			private:
				std::mt19937 & rand_;

				std::vector<int> decision_;
				StateValueFunction state_value_func_;
			};
		}
	}
}