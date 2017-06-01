#pragma once

#include "state/Types.h"
#include "state/targetor/TargetsGenerator.h"

namespace state
{
	class State;
	namespace Cards { class Card; }
}

namespace FlowControl
{
	class FlowContext;

	namespace onplay
	{
		namespace context
		{
			struct GetSpecifiedTarget {
			public:
				GetSpecifiedTarget(
					Manipulate const& manipulate,
					state::PlayerIdentifier player,
					state::CardRef card_ref) :
					manipulate_(manipulate),
					player_(player),
					card_ref_(card_ref),
					need_to_prepare_target_(false),
					allow_no_target_(false)
				{
				}

			public:
				Manipulate const& manipulate_;
				state::PlayerIdentifier player_;
				state::CardRef card_ref_;

				template <typename... Args>
				auto & SetRequiredBattlecryTargets(Args&&... args) {
					return SetRequiredTargets(std::forward<Args>(args)...).Targetable();
				}
				template <typename... Args>
				auto & SetOptionalBattlecryTargets(Args&&... args) {
					return SetOptionalTargets(std::forward<Args>(args)...).Targetable();
				}

				template <typename... Args>
				auto & SetRequiredSpellTargets(Args&&... args) {
					return SetRequiredTargets(std::forward<Args>(args)...).SpellTargetable();
				}
				template <typename... Args>
				auto & SetOptionalSpellTargets(Args&&... args) {
					return SetOptionalTargets(std::forward<Args>(args)...).SpellTargetable();
				}

				state::targetor::Targets GetTargets() {
					return targets_generator_.GetInfo();
				}

				bool NeedToPrepareTarget() { return need_to_prepare_target_; }
				bool IsAllowedNoTarget() { return allow_no_target_; }

			private:
				template <typename... Args>
				state::targetor::TargetsGenerator & SetRequiredTargets(Args&&... args) {
					need_to_prepare_target_ = true;
					allow_no_target_ = false;
					targets_generator_.Initialize(std::forward<Args>(args)...);
					return targets_generator_;
				}
				template <typename... Args>
				state::targetor::TargetsGenerator & SetOptionalTargets(Args&&... args) {
					need_to_prepare_target_ = true;
					allow_no_target_ = true;
					targets_generator_.Initialize(std::forward<Args>(args)...);
					return targets_generator_;
				}

			private:
				state::targetor::TargetsGenerator targets_generator_;
				bool need_to_prepare_target_;
				bool allow_no_target_;
			};

			struct OnPlay
			{
				Manipulate const& manipulate_;
				state::PlayerIdentifier player_;
				state::CardRef card_ref_;
				state::CardRef * new_card_ref;

				state::CardRef GetTarget() const;
			};
		}
	}
}