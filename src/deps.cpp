#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <.hpp>

#include <Geode/modify/CCString.hpp>
class $modify(CCStringNilCallFix, CCString) {
	const char* getCString() {
		//log::debug("{}(int:{})->{}", this, (int)this, __func__);
		if (!this) log::error("{}->{}", this, __func__);
		return this ? CCString::getCString() : CCString::createWithFormat("")->getCString();
	}
};
#include <Geode/modify/CCObject.hpp>
class $modify(CCObjectCallFix, CCObject) {
	void release(void) {
		if (!this) log::error("{}->{}", this, __func__);
		return this ? CCObject::release() : void();
	};
	void retain(void) {
		if (!this) log::error("{}->{}", this, __func__);
		return this ? CCObject::retain() : void();
	};
	CCObject* autorelease(void) {
		if (!this) log::error("{}->{}", this, __func__);
		return this ? CCObject::autorelease() : new CCObject();
	};
};

#include <Geode/modify/CCSpriteFrameCache.hpp>
class $modify(CCSpriteFrameCache) {
	void addSpriteFramesWithFile(const char* plist) {
		if (getMod()->getSettingValue<bool>("past")) void();
		else if (string::contains(plist, Mod::get()->getID())) {
			auto path = CCFileUtils::get()->fullPathForFilename(plist, 0);
			if (!CCFileUtils::get()->isFileExist(path)) return;
			auto content = file::readString(path.c_str()).unwrapOrDefault();
			content = string::replace(content, "..", "/");
			content = string::replace(content, std::string(Mod::get()->getID()) + "/", "");
			file::writeStringSafe(path.c_str(), content).isOk();
			//fuuuck
			removeSpriteFramesFromFile(plist);
		}
		CCSpriteFrameCache::addSpriteFramesWithFile(plist);
	}
};

#include <Geode/modify/CCNode.hpp>
class $modify(NodeVisitController, CCNode) {
	auto replaceColors(float = 0.f) {
		if (Ref node = typeinfo_cast<GJListLayer*>(this)) {
			if (node->getOpacity() == 180) node->setOpacity(255);// list-bg
		}
		if (Ref<SetupTriggerPopup> a = typeinfo_cast<UIOptionsLayer*>(this); a = a ? a : typeinfo_cast<UIPOptionsLayer*>(this)) {
			a->setOpacity(160);
			a->setColor(ccBLACK);
		}
		/*Ref game = GameManager::get()->m_gameLayer;
		if (game and game->isRunning()) return;*/
		auto hexcfix = [] (std::string hex) { return std::string(hex.begin() + 1, hex.begin() + 7); };
#define repl(org, tar) if (node->getColor() == cc3bFromHexString(hexcfix(org)).unwrapOr(ccWHITE)) node->setColor(cc3bFromHexString(hexcfix(tar)).unwrapOr(ccWHITE));
		
		if (Ref node = typeinfo_cast<CCLabelBMFont*>(this)) {
			repl("#6C99D8FF", "#6C99D8FF");//inputplaceholder
			repl("#78AAF0FF", "#78AAF0FF");//inputplaceholder-LevelListLayer
		}
		else if (Ref node = typeinfo_cast<CCLayerColor*>(this)) {
			repl("#BF723EFF", "#BF723EFF");//GJListLayer
			repl("#A1582CFF", "#A1582CFF");//LevelCell1
			repl("#C2723EFF", "#C2723EFF");//LevelCell2
			//repl("", "");
		}
		else if (auto node = typeinfo_cast<CCRGBAProtocol*>(this)) {
			repl("#0066FFFF", "#0066FFFF");//background
			repl("#969696FF", "#969696FF");//background GJGarageLayer
			repl("#334499FF", "334499FF");//chest_glow_bg_001.png
			repl("#00388DFF", "#00388DFF");//level-search-bg
			repl("#002762FF", "#002762FF");//level-search-bar-bg
			repl("#002E75FF", "#002E75FF");//quick-search-bg
			repl("#00245BFF", "#00245BFF");//difficulty-filters-bg
			repl("#001F4FFF", "#001F4FFF");//length-filters-bg
			repl("#824021FF", "#824021FF");//desc-background
			repl("#203182FF", "#203182FF");//GJCommentListLayer outline
			repl("#824020FF", "#824020FF");//GJCommentListLayer outline
			repl("#203182FF", "#203182FF");//CommentCell CCScale9Sprite
			repl("#002E73FF", "#002E73FF");//play-button1
			repl("#0057DAFF", "#0057DAFF");//play-button11
			repl("#002E73FF", "#002E73FF");//play-button111
			repl("#BF723EFF", "#BF723EFF");//GJListLayer
			repl("#A1582CFF", "#A1582CFF");//LevelCell1
			repl("#C2723EFF", "#C2723EFF");//LevelCell2
			repl("#6C99D8FF", "#6C99D8FF");//inputplaceholder
		};
#undef repl
	}
	void addChild(CCNode * child, int zOrder, int tag) {
		if (!child) return;
		CCNode::addChild(child, zOrder, tag);
		if (Ref node = typeinfo_cast<CCLabelBMFont*>(this)) {
			node->schedule(schedule_selector(NodeVisitController::replaceColors), 0.f);
			if (Ref ch = typeinfo_cast<CCFontSprite*>(child)) queueInMainThread(
				[ch]
				{
					auto orgc = ch->getColor();
					auto lght = lighten3B(orgc, 80);
					if (ch->getColor() != ccWHITE) ch->runAction(CCRepeatForever::create(
#define roll_2893450 CCEaseSineInOut::create(CCTintTo::create(\
					1.f + CCRANDOM_0_1(),\
						lght.r, lght.g, lght.b\
						)),\
						CCEaseSineInOut::create(CCTintTo::create(\
							1.f + CCRANDOM_0_1(),\
							orgc.r, orgc.g, orgc.b\
						)),
						CCSequence::create(roll_2893450 roll_2893450 roll_2893450 nullptr)
					));
					if (ch->getColor() != ccWHITE) ch->setBlendFunc({GL_SRC_ALPHA, GL_ONE}); //additive
				}
			);
			if (getMod()->getSettingValue<bool>("past")) return;
			Ref((NodeVisitController*)this)->replaceColors();
			Ref((NodeVisitController*)child)->replaceColors();
		}
	}
	void visit() {
		Ref(this)->replaceColors();
		CCNode::visit();
	}
};

#include <Geode/modify/FMODAudioEngine.hpp>
class $modify(FMODAudioEngineResourcesExt, FMODAudioEngine) {
	int playEffectAdvanced(gd::string strPath, float speed, float p2, float volume, float pitch, bool fastFourierTransform, bool reverb, int startMillis, int endMillis, int fadeIn, int fadeOut, bool loopEnabled, int p12, bool override, bool p14, int p15, int uniqueID, float minInterval, int sfxGroup) {
		return FMODAudioEngine::playEffectAdvanced(
			CCFileUtils::get()->fullPathForFilename(strPath.c_str(), 0).c_str(),
			speed, p2, volume, pitch, fastFourierTransform, reverb, startMillis,
			endMillis, fadeIn, fadeOut, loopEnabled, p12, override, p14, p15, uniqueID, minInterval, sfxGroup
		);
	}
};

#include <Geode/modify/CCFileUtils.hpp>
class $modify(CCFileUtilsResourcesExt, CCFileUtils) {
	inline static std::unordered_set<std::string> s_checkedDirs;
	inline static std::unordered_map<std::string, std::vector<std::string>> s_randomDirs;

	virtual gd::string fullPathForFilename(const char* pszFileName, bool skipSuffix) {

		std::string_view fileName(pszFileName);

		auto lastDot = fileName.find_last_of('.');
		if (lastDot != std::string_view::npos && lastDot > 2) {
			std::string noExtName(fileName.substr(0, lastDot));

			auto it = s_randomDirs.find(noExtName);
			if (it != s_randomDirs.end()) {
				if (!it->second.empty()) {
					size_t idx = std::rand() % it->second.size();
					return CCFileUtils::fullPathForFilename(it->second[idx].c_str(), skipSuffix);
				}
			}
			else if (s_checkedDirs.find(noExtName) == s_checkedDirs.end()) {
				s_checkedDirs.insert(noExtName);

				auto& searchPaths = getSearchPaths();
				for (auto& p : searchPaths) {
					std::string dirPath = p.c_str() + noExtName + "/";
					auto varList = file::readDirectory(dirPath.c_str()).unwrapOrDefault();

					if (!varList.empty()) {
						auto& cached = s_randomDirs[noExtName];
						cached.reserve(varList.size());
						for (auto& entry : varList) {
							cached.push_back(entry.string());
						}

						log::debug("{} -> {} files", noExtName, cached.size());
						size_t idx = std::rand() % cached.size();
						return CCFileUtils::fullPathForFilename(cached[idx].c_str(), skipSuffix);
					}
				}
			}
		}

		if (fileName.find('/') != std::string_view::npos) {
			std::string noSub;
			noSub.reserve(fileName.size() * 2);

			for (char ch : fileName) {
				if (ch == '/') noSub += "..";
				else noSub += ch;
			}

			if (fileExistsInSearchPaths(noSub.c_str())) {
				auto* texCache = CCTextureCache::get();
				if (auto* tex = texCache->textureForKey(noSub.c_str())) {
					NOT_APPLE(texCache->reloadTexture(noSub.c_str()));
				}
				return CCFileUtils::fullPathForFilename(noSub.c_str(), skipSuffix);
			}
		}

		return CCFileUtils::fullPathForFilename(pszFileName, skipSuffix);
	}
};

#include <Geode/modify/CCSprite.hpp>
class $modify(CCSpriteExt, CCSprite) {
	static void onModify(auto& self) {
		auto names = {
			"cocos2d::CCSprite::initWithSpriteFrameName",
			"cocos2d::CCSprite::initWithFile",
			"cocos2d::CCSprite::create",
			"cocos2d::CCSprite::createWithSpriteFrameName",
		};
		for (auto name : names) if (!self.setHookPriorityPost(name, Priority::Last)) {
			log::error("Failed to set hook priority for {}.", name);
		}
	}
	void updateShader(float deltaTime) {
		static float time = 0.0f;
		time += deltaTime;
		if (auto* glprogram = this->getShaderProgram()) {
			glprogram->use();
			auto p = glprogram->getProgram();
			glUniform1f(glGetUniformLocation(p, "u_time"), time);
			glUniform1f(glGetUniformLocation(p, "u_deltaTime"), deltaTime);
			auto mouse = cocos::getMousePos();
			glUniform1f(glGetUniformLocation(p, "u_mouseX"), mouse.x);
			glUniform1f(glGetUniformLocation(p, "u_mouseY"), mouse.y);
			glUniform2f(glGetUniformLocation(p, "u_mouse"), mouse.x, mouse.y);
			auto fmod = FMODAudioEngine::sharedEngine();
			if (!fmod->m_metering) fmod->enableMetering();
			glUniform1f(glGetUniformLocation(p, "u_pulse1"), fmod->m_pulse1);
			glUniform1f(glGetUniformLocation(p, "u_pulse2"), fmod->m_pulse2);
			glUniform1f(glGetUniformLocation(p, "u_pulse3"), fmod->m_pulse3);
			glUniform3f(glGetUniformLocation(p, "u_pulse"), fmod->m_pulse1, fmod->m_pulse2, fmod->m_pulse3);
		}
	}

	static auto tryApplyShader(CCSprite * spr, std::string pszName) {
		if (!spr) {
			//log::warn("sprite from {} is {}", pszName, spr);
			return spr;
		}
		if (getMod()->getSettingValue<bool>("past")) return spr;

		bool isAvailabilityCheck = ((uintptr_t)spr == "isAvailable"_h);

		std::string path = CCFileUtils::get()->fullPathForFilename(pszName.c_str(), 0);
		if (!string::contains(path, ".png")) {
			path = CCFileUtils::get()->fullPathForFilename((pszName + ".fsh").c_str(), 0);
		}
		else {
			path += ".fsh";
		}

		bool shaderExists = CCFileUtils::get()->isFileExist(path);

		if (isAvailabilityCheck) return (CCSprite*)shaderExists;

		if (shaderExists) {
			Ref<CCGLProgram> program = new CCGLProgram();
			if (program->initWithVertexShaderFilename(
				(pszName + ".vsh").c_str(),
				(pszName + ".fsh").c_str()
			)) {
				program->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
				program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
				program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
				program->link();
				program->updateUniforms();
				spr->setShaderProgram(program);
				spr->schedule(schedule_selector(CCSpriteExt::updateShader));
			}
		}

		return spr;
	}

	static CCSprite* create(const char* pszName) {
		auto* spr = CCSprite::create(pszName);

		if (!spr) {
			auto lastDot = std::string(pszName).find_last_of('.');
			if (lastDot != std::string::npos) {
				std::string name = std::string(pszName).substr(0, lastDot);
				std::string ext = std::string(pszName).substr(lastDot + 1);
				spr = CCSprite::create(fmt::format("{}-FALLBACK.{}", name, ext).c_str());
			}
		}
		if (getMod()->getSettingValue<bool>("past")) return spr;

		spr = tryApplyShader(spr, pszName);

		return spr;
	}
	static auto createAsSizeFixedSpr(const char* pszName, float h = 0.f) {
		auto* spr = CCSprite::create(pszName);
		if (h) queueInMainThread([spr = Ref(spr), h] {
			if (spr) limitNodeHeight(spr, h, 99.f, 0.1f);
			});
		return spr;
	}
	static CCSprite* createWithSpriteFrameName(const char* pszName) {

		static auto bruh = std::unordered_map<std::string, float>();
		if (bruh.empty()) for (auto a : file::readDirectory(
			Mod::get()->getResourcesDir().parent_path(), 1).unwrapOrDefault()
			) {
			bruh[string::pathToString(a.filename())] = utils::numFromString<float>(file::readString(
				a.parent_path() / (string::pathToString(a.filename()) + ".sz")
			).unwrapOrDefault()).unwrapOrDefault();
		}

		if (bruh.contains(pszName)) //44.750
			return createAsSizeFixedSpr(pszName, bruh[pszName]);

		auto* spr = CCSprite::createWithSpriteFrameName(pszName);

		/*if (auto xd = AlphaUtils::Cocos::getSprite(pszName).value_or(nullptr)) {
			spr = xd;
		}*/

		if (!spr) {
			auto lastDot = std::string(pszName).find_last_of('.');
			if (lastDot != std::string::npos) {
				std::string name = std::string(pszName).substr(0, lastDot);
				std::string ext = std::string(pszName).substr(lastDot + 1);
				spr = CCSprite::createWithSpriteFrameName(
					fmt::format("{}-FALLBACK.{}", name, ext).c_str()
				);
			}
		}

		//spr = tryApplyShader(spr, pszName);

		return spr;
	}
};

#include <Geode/modify/CCLayer.hpp>
class $modify(CCLayerExt, CCLayer) {
	virtual void onEnter() {
		CCLayer::onEnter();
		if (getMod()->getSettingValue<bool>("past")) return;

		if (!CCSpriteExt::tryApplyShader(
			(CCSprite*)"isAvailable"_h,
			cocos::getClassName(this)
		)) {
			return;
		}

		auto contentSize = this->getContentSize();
		Ref<CCRenderTexture> render = CCRenderTexture::create(
			contentSize.width,
			contentSize.height
		);

		if (!render) return;

		render->beginWithClear(0, 0, 0, 0);
		this->visit();
		render->end();

		auto* sprite = render->getSprite();
		sprite->setPosition(contentSize / 2);
		this->addChild(sprite, INT_MAX, 0);

		CCSpriteExt::tryApplyShader(sprite, cocos::getClassName(this));

		this->runAction(CCRepeatForever::create(
			CCSequence::create(
				CallFuncExt::create([render = Ref(render), layer = Ref(this)] {
					auto* spr = render->getSprite();
					spr->setVisible(false);
					render->beginWithClear(0, 0, 0, 0);
					layer->visit();
					render->end();
					spr->setVisible(true);
					}),
				nullptr
			)
		));
	}
};

//deps and dodeps
#include <Geode/modify/MenuLayer.hpp>
class $modify(MenuLayerExt, MenuLayer) {
	static auto onModify(auto) {
		//texture pack
		CCTexturePack xd;
		xd.m_id = std::string(Mod::get()->getID());
		xd.m_paths.push_back(R"(C:\Users\user95401\source\repos\REMINA\resources\)");
		xd.m_paths.push_back(string::pathToString(Mod::get()->getResourcesDir().parent_path()).c_str());
		xd.m_paths.push_back(string::pathToString(Mod::get()->getResourcesDir()).c_str());
		log::debug("Adding texture pack: \"{}\"", xd.m_paths[0]);
		CCFileUtils::get()->addTexturePack(xd);
		//add sub files
		for (auto path : file::readDirectory(getMod()->getResourcesDir()).unwrapOrDefault()) {
			auto str = string::pathToString(path);
			auto name = string::pathToString(path.filename());
			auto nsub = string::replace(name, "..", "/");
			if (string::contains(str, "..")) CCFileUtils::get()->m_fullPathCache[nsub] = CCFileUtils::get()->fullPathForFilename(
				name.c_str(), 0
			);
		}
	};
};