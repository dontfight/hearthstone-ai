#pragma once

#include <unordered_map>
#include "state/targetor/Targets.h"
#include "FlowControl/enchantment/TieredEnchantments.h"
#include "FlowControl/aura/Contexts.h"

namespace FlowControl
{
	class FlowContext;

	namespace aura
	{
		enum UpdatePolicy {
			kUpdateAlways,
			kUpdateWhenMinionChanges,
			kUpdateWhenEnrageChanges
		};

		enum EmitPolicy {
			kEmitInvalid,
			kEmitWhenAlive
		};

		class Handler
		{
		public:
			typedef bool FuncIsValid(contexts::AuraIsValid context);
			typedef void FuncGetTargets(contexts::AuraGetTargets context);
			typedef enchantment::TieredEnchantments::IdentifierType FuncApplyOn(contexts::AuraApplyOn context);

			Handler() :
				update_policy(kUpdateAlways), emit_policy(kEmitInvalid),
				get_targets(nullptr), apply_on(nullptr),
				first_time_update_(true),
				last_updated_change_id_first_player_minions_(-1), // ensure this is not the initial value of the actual change id
				last_updated_change_id_second_player_minions_(-1),
				last_updated_undamaged_(true)
			{}

		public:
			void SetUpdatePolicy(UpdatePolicy policy) { update_policy = policy; }
			void SetEmitPolicy(EmitPolicy policy) { emit_policy = policy; }
			void SetCallback_GetTargets(FuncGetTargets* callback) { get_targets = callback; }
			void SetCallback_ApplyOn(FuncApplyOn* callback) { apply_on = callback; }

			bool IsCallbackSet_GetTargets() const { return get_targets != nullptr; }
			bool IsCallbackSet_ApplyOn() const { return apply_on != nullptr; }

		public:
			bool NoAppliedEnchantment() const { return applied_enchantments.empty(); }
			bool Update(state::State & state, FlowControl::FlowContext & flow_context, state::CardRef card_ref);

			void AfterCopied() {
				first_time_update_ = true;
				applied_enchantments.clear();
			}

		private:
			bool NeedUpdate(state::State & state, FlowControl::FlowContext & flow_context, state::CardRef card_ref);
			void AfterUpdated(state::State & state, FlowControl::FlowContext & flow_context, state::CardRef card_ref);

			bool IsValid(state::State & state, FlowControl::FlowContext & flow_context, state::CardRef card_ref);

		public: // field for client code
			bool first_time_update_;
			int last_updated_change_id_first_player_minions_;
			int last_updated_change_id_second_player_minions_;
			bool last_updated_undamaged_;

		private:
			// TODO: update policy and emit policy can be shared with flag aura
			// TODO: the rest fields are the only difference between 'aura' and 'flag-aura'
			UpdatePolicy update_policy;
			EmitPolicy emit_policy;

			FuncGetTargets * get_targets;
			FuncApplyOn * apply_on;

			std::unordered_map<state::CardRef, enchantment::TieredEnchantments::IdentifierType> applied_enchantments;
		};
	}
}