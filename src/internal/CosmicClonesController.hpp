#pragma once

#include "CosmicClone.hpp"
#include "CosmicClonesTrigger.hpp"

struct CosmicClonesGJBGL;

class CosmicClonesController {
protected:
    CosmicClonesGJBGL* m_bgl = nullptr;

    std::vector<std::shared_ptr<CosmicClone>> m_clones;
    std::map<unsigned int, Snapshot> m_snapshots;
    std::vector<int> m_sfxIds;

    unsigned int m_startOffset = 0;

    void cleanup();

    bool m_stopped = true;

    CosmicClonesTrigger* m_trigger = nullptr;
public:
    CosmicClonesController(CosmicClonesGJBGL* bgl);
    CosmicClonesController(CosmicClonesGJBGL* bgl, CosmicClonesTrigger* trigger);
    ~CosmicClonesController();

    // i don't really wanna make getters/setters for the config options
    int m_count = 3;
    float m_initialDelay = 1.75;
    float m_delay = 1;
    bool m_damage = true;
    std::vector<Style> m_styles = {{"Cosmic Mario\n(SMG 1)"}};
    bool m_sfx = false;

    bool start();
    void stop(bool immediate = false);

    void loadConfigFromSettings();
    void loadConfigFromTrigger(const CosmicClonesTrigger* trigger);
    void loadConfigFromTrigger();

    unsigned int getOffset() const;
    bool isStopped() const;
    std::vector<std::shared_ptr<CosmicClone>> getClones() const;

    void tick(int prog);
    void softReset(int prog);

    static std::shared_ptr<CosmicClonesController> createWithSettings(CosmicClonesGJBGL* bgl);
    static std::shared_ptr<CosmicClonesController> createFromTrigger(CosmicClonesGJBGL* bgl, CosmicClonesTrigger* trigger);
};
