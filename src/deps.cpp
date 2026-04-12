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
