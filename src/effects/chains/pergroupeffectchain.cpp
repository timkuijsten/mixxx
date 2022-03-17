#include "effects/chains/pergroupeffectchain.h"

#include "effects/effectsmanager.h"

PerGroupEffectChain::PerGroupEffectChain(
        const ChannelHandleAndGroup& handleAndGroup,
        const QString& chainSlotGroup,
        SignalProcessingStage stage,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(chainSlotGroup,
                  pEffectsManager,
                  pEffectsMessenger,
                  stage) {
    // Set the chain to be fully wet.
    m_pControlChainMix->set(1.0);
    sendParameterUpdate();

    // Register this channel alone with the chain slot.
    registerInputChannel(handleAndGroup);
    enableForInputChannel(handleAndGroup);
}
