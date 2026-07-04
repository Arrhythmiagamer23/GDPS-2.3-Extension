
using namespace geode::prelude;

class RPUtils {
public:

    static bool isPortalObject(int objectID) {
        return objectID == 31790;
    }

    static bool isBackObject(CCSize size) {
        return size == CCSize(49.f, 77.5f)
            || size == CCSize(48.75f, 77.5f)
            || size == CCSize(46.f, 74.5f)
            || size == CCSize(25.25f, 78.f);
    }

    static void updatePortalParticle(GameObject* obj, bool disableBlending) {
        if (!obj->m_particle) return;

        if (disableBlending) {
            obj->m_particle->setBlendFunc({ GL_ONE, GL_ZERO });
        }
        else {
            obj->m_particle->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        }

        switch (obj->m_objectID) { // Portal colors remade bc it the original color of the UFO blends in with the ball and the swing, lol
        case 31790:
            obj->m_particle->setStartColor({ 1.f, 0.85f, 0.f, 1.f });
            obj->m_particle->setEndColor({ 0.9f, 0.7f, 0.f, 1.f });
            break;
        }
    }

    static void updateBackSprite(GameObject* obj, CCSpriteFrameCache* frames, int texIndex) {
        if (!isBackObject(obj->getContentSize())) return;

        auto name = CCString::createWithFormat("back_%02d.png", texIndex)->getCString();
        auto frame = frames->spriteFrameByName(name);

        if (frame && isPortalObject(obj->m_objectID)) {
            obj->setDisplayFrame(frame);
            obj->setAnchorPoint({ 0.960f, 0.495f });
        }
    }

    static void updateFrontSprite(GameObject* obj, CCSpriteFrameCache* frames, int texIndex) {
        auto frontName = CCString::createWithFormat("front_%02d.png", texIndex)->getCString();
        auto frontFrame = frames->spriteFrameByName(frontName);

        if (!frontFrame) return;

        auto tabName = CCString::createWithFormat("tab_%02d.png", texIndex)->getCString();
        auto guideFrame = frames->spriteFrameByName(tabName);

        if (obj->getChildrenCount() >= 1) {
            auto child0 = typeinfo_cast<CCSprite*>(obj->getChildren()->objectAtIndex(0));

            if (child0) {
                if (obj->getChildrenCount() >= 2) {
                    auto child1 = typeinfo_cast<CCSprite*>(obj->getChildren()->objectAtIndex(1));

                    if (child1 && child1->getParent() == obj) {
                        auto worldPos = obj->convertToWorldSpace(child1->getPosition());
                        auto worldRotation = obj->getRotation() + child1->getRotation();

                        child1->retain();
                        obj->removeChild(child1, false);
                        child0->addChild(child1);
                        child1->release();

                        child1->setPosition(child0->convertToNodeSpace(worldPos));
                        child1->setRotation(worldRotation - child0->getRotation());
                        child1->setAnchorPoint({ 0.65f, 0.5f });

                        auto iconFrame = frames->spriteFrameByName("randomPortalIcon.png");
                        child1->setDisplayFrame(iconFrame);
                    }
                }

                child0->setAnchorPoint({ 1.25f, 0.5f });
                child0->setDisplayFrame(guideFrame);
            }
        }

        obj->setDisplayFrame(frontFrame);
        obj->setAnchorPoint({ 0.32f, 0.5f });
    }
};
