#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <algorithm>

using namespace geode::prelude;

class $modify(FastWavePlayer, PlayerObject) {
	struct Fields {
		float m_currentAngle = 0.f;
		bool m_wasDart = false;
	};

	void update(float dt) {
		PlayerObject::update(dt);

		auto mod = Mod::get();
		if (!mod->getSettingValue<bool>("enabled")) return;

		auto f = m_fields.self();

		// только в режиме вейва
		if (!m_isDart) {
			f->m_wasDart = false;
			return;
		}

		// направление определяем по вертикальной скорости
		bool goingUp = m_yVelocity > 0.0;
		if (m_isUpsideDown) goingUp = !goingUp;

		// в cocos2d положительный угол = по часовой стрелке
		// вверх = -45°, вниз = +45° (для мини-вейва угол круче)
		float maxAngle = (m_vehicleSize < 1.f) ? 60.f : 45.f;
		float target = goingUp ? -maxAngle : maxAngle;
		if (m_isUpsideDown) target = -target;

		if (!f->m_wasDart) {
			f->m_currentAngle = this->getRotation();
			f->m_wasDart = true;
		}

		float speed = static_cast<float>(mod->getSettingValue<double>("rotation-speed"));

		if (speed >= 10.f) {
			f->m_currentAngle = target; // мгновенно
		} else {
			// чем выше speed, тем быстрее приближаемся к целевому углу
			float t = std::min(1.f, dt * speed * 20.f);
			f->m_currentAngle += (target - f->m_currentAngle) * t;
		}

		this->setRotation(f->m_currentAngle);
	}
};
