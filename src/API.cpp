#define GEODE_DEFINE_EVENT_EXPORTS
#include <cosmic_clones/API.hpp>

#include "hooks/Gameplay.hpp"
#include "internal/CosmicClonesController.hpp"

void cosmic_clones::setClonesVisible(bool visible) {
    auto bgl = static_cast<CosmicClonesGJBGL*>(GJBaseGameLayer::get());
    if (!bgl) return;
    auto fields = bgl->m_fields.self();
    for (auto clone : fields->m_controller->getClones()) {
        clone->getP1()->setVisible(visible);
        clone->getP2()->setVisible(visible);
    }
    for (auto [id, controller] : fields->m_triggerControllers) {
        for (auto clone : controller->getClones()) {
            clone->getP1()->setVisible(visible);
            clone->getP2()->setVisible(visible);
        }
    }
}
