#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
using namespace geode::prelude;

#include <regex>

#include <user95401.game-objects-factory/include/main.hpp>

void createDialogTrigger();
$execute{createDialogTrigger();}
inline void createDialogTrigger() {

    class DialogTriggerDelegate : public DialogDelegate {
    public:
        DialogLayer* m_dialogLayer = nullptr;
        GJBaseGameLayer* m_game = nullptr;
        virtual void dialogClosed(DialogLayer* p0) {
            m_dialogLayer = nullptr;
            if (m_game) m_game->resumeSchedulerAndActions();
        };
    };
    static auto sharedDialogTriggerDelegate = new DialogTriggerDelegate();

    GameObjectsFactory::registerGameObject(GameObjectsFactory::createTriggerConfig(
        (fabs("dialog_trigger"_h) * 0.00001),
        "dialog-trigger.png"_spr,
        [](EffectGameObject* trigger, GJBaseGameLayer* game, int p1, gd::vector<int> const* p2)
        {
            if (auto DialogTriggerDataNode = typeinfo_cast<CCNode*>(trigger->getUserObject("data"_spr))) {
                sharedDialogTriggerDelegate->m_game = game;
                auto raw_data = "[" + DialogTriggerDataNode->getID() + "]";
                auto parse = matjson::parse(raw_data);
                matjson::Value data = parse.err()
                    ? matjson::parse("[ \"<cr>err: " + parse.err().value().message + "</c>\" ]").unwrapOrDefault()
                    : parse.unwrapOrDefault();

                /*
                    example text:
                    ```
                    1,"char:the noone","<cr>hi!</c> why ar u here ever?","!","its feels bad you here"
                    ```
                */

                DialogChatPlacement placement = DialogChatPlacement::Center;

                auto not_skippable = true;
                auto character = std::string("");
                auto characterFrame = 0;

                auto dialogObjectsArr = CCArrayExt<DialogObject>();

                for (auto& val : data) {
                    if (val.isNumber()) {
                        characterFrame = val.asInt().unwrapOrDefault();
                    }
                    if (val.isString()) {
                        auto text = val.asString().unwrapOrDefault();
                        if (string::startsWith(text, "!char:")) {
                            character = string::replace(text, "!char:", "");
                            continue;
                        }
                        if (string::startsWith(text, "!place:")) {
                            auto place = string::replace(text, "!place:", "");
                            if (place == "t") placement = DialogChatPlacement::Top;
                            if (place == "c") placement = DialogChatPlacement::Center;
                            if (place == "b") placement = DialogChatPlacement::Bottom;
                            continue;
                        }
                        if (string::startsWith(text, "!")) {
                            not_skippable = false;
                            continue;
                        }
                        dialogObjectsArr.push_back(DialogObject::create(
                            character, text, characterFrame, 1.f, not not_skippable, ccWHITE
                        ));
                    }
                }

                if (false) log::debug("placement {}", static_cast<int>(placement));

                auto& dialog = sharedDialogTriggerDelegate->m_dialogLayer;
                if (dialog) dialog->removeFromParent();
                dialog = DialogLayer::createDialogLayer(
                    dialogObjectsArr[0], dialogObjectsArr.inner(), 1
                );
                dialog->m_delegate = sharedDialogTriggerDelegate;
                dialog->updateChatPlacement(placement);
                dialog->animateInRandomSide();
                dialog->addToMainScene();

                if (game) game->pauseSchedulerAndActions();
            }
        },
        [](EditTriggersPopup* popup, EffectGameObject* trigger, CCArray* objects)
        {
            if (auto data = typeinfo_cast<CCNode*>(trigger->getUserObject("data"_spr))) {
                if (auto title = popup->getChildByType<CCLabelBMFont*>(0)) {
                    title->setString("Edit Dialog Trigger");
                    title->setAnchorPoint(CCPointMake(0.5f, 0.3f));
                }
                if (auto inf = popup->m_buttonMenu->getChildByType<InfoAlertButton*>(0)) {
                    inf->setVisible(false);
                }

                auto input = TextInput::create(312.f, "Dialog data string...");
                input->setFilter(" !\"#$ % &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
                input->getInputNode()->m_allowedChars = " !\"#$ % &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
                input->setString(data->getID());
                input->setPositionY(55.000f);
                input->setCallback(
                    [data = Ref(data)](const std::string& p0) {
                        data->setID(p0);
                    }
                );
                popup->m_buttonMenu->addChild(input);

                auto editor = CCMenuItemExt::createSpriteExtra(
                    ButtonSprite::create("String Guide"), [input = Ref(input)](CCMenuItem*) {
                        openInfoPopup(getMod());
                    }
                );
                editor->setPosition(CCPointMake(100.f, 28.f));
                editor->setScale(0.55f);
                editor->m_baseScale = (0.55f);
                editor->m_scaleMultiplier = (1.0f + (0.61f - 0.55f));
                popup->m_buttonMenu->addChild(editor);
            }
        }
    )->customSetup(
        [](GameObject* object)
        {
            auto data = CCNode::create();
            object->setUserObject("data"_spr, data);
            return object;
        }
    )->saveString(
        [](std::string str, GameObject* object, GJBaseGameLayer* level)
        {
            if (auto data = typeinfo_cast<CCNode*>(object->getUserObject("data"_spr))) {
                str += ",228,";
                str += ZipUtils::base64URLEncode(data->getID().c_str()).c_str();
            }
            return gd::string(str.c_str());
        }
    )->objectFromVector(
        [](GameObject* object, gd::vector<gd::string>& p0, gd::vector<void*>&, void*, bool) 
        {
            if (!object) return object;

            auto data = typeinfo_cast<CCNode*>(object->getUserObject("data"_spr));
            if (data) {
                data->setID(ZipUtils::base64URLDecode(p0[228].c_str()).c_str());
            };

            return object;
        }
    ));

}