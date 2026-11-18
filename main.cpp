#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

class $modify(FastWavePlayer, PlayerObject) {
	struct Fields {
		float m_visualAngle = 0.f;
		bool m_active = false;
	};

	// Rotates only the icon sprites. The PlayerObject itself (and its hitbox)
	// is never touched.
	void setIconOffset(float offset) {
		if (m_iconSprite) m_iconSprite->setRotation(offset);
		if (m_iconGlow) m_iconGlow->setRotation(offset);
	}

	void update(float dt) {
		PlayerObject::update(dt);

		auto f = m_fields.self();
		auto mod = Mod::get();

		// Leaving wave mode or mod disabled: restore the icon
		if (!m_isDart || !mod->getSettingValue<bool>("enabled")) {
			if (f->m_active) {
				setIconOffset(0.f);
				f->m_active = false;
			}
			return;
		}

		// Direction from vertical velocity
		bool goingUp = m_yVelocity > 0.0;
		if (m_isUpsideDown) goingUp = !goingUp;

		float maxAngle = (m_vehicleSize < 1.f) ? 60.f : 45.f;
		float target = goingUp ? -maxAngle : maxAngle;
		if (m_isUpsideDown) target = -target;

		// Start from the angle the game currently shows
		if (!f->m_active) {
			f->m_visualAngle = this->getRotation();
			f->m_active = true;
		}

		float speed = static_cast<float>(mod->getSettingValue<double>("rotation-speed"));

		if (speed >= 10.f) {
			// instant turn
			f->m_visualAngle = target;
		} else {
			// speed 1.0 = 360 degrees per second
			float maxStep = speed * 360.f * dt;
			float diff = target - f->m_visualAngle;
			if (std::fabs(diff) <= maxStep) {
				f->m_visualAngle = target;
			} else {
				f->m_visualAngle += (diff > 0.f ? maxStep : -maxStep);
			}
		}

		// Offset relative to the object's own rotation, so the sprite
		// ends up showing exactly m_visualAngle
		setIconOffset(f->m_visualAngle - this->getRotation());
	}
};
