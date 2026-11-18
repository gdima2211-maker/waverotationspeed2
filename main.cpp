#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

static float wrapAngle(float a) {
	a = std::fmod(a, 360.f);
	if (a > 180.f) a -= 360.f;
	else if (a < -180.f) a += 360.f;
	return a;
}

class $modify(FastWavePlayer, PlayerObject) {
	struct Fields {
		float m_visualAngle = 0.f;   // angle we show (degrees)
		float m_targetAngle = 0.f;   // angle the wave "wants" to have
		float m_lastX = 0.f;
		float m_lastY = 0.f;
		bool m_hasLast = false;
		bool m_active = false;
	};

	// Rotates only the icon sprites. The PlayerObject itself (and its hitbox)
	// is never touched.
	void setIconOffset(float offset) {
		if (m_iconSprite) m_iconSprite->setRotation(offset);
		if (m_iconGlow) m_iconGlow->setRotation(offset);
	}

	void update(float dt) {
		// Position at the start of the step = result of the previous step
		// including collisions (floor, ceiling, blocks).
		float curX = this->getPositionX();
		float curY = this->getPositionY();

		PlayerObject::update(dt);

		auto f = m_fields.self();
		auto mod = Mod::get();
		float speed = static_cast<float>(mod->getSettingValue<double>("rotation-speed"));
		bool run = m_isDart && mod->getSettingValue<bool>("enabled") && speed > 0.01f;

		// Not in wave mode / disabled / speed 0 (= vanilla): restore the icon
		if (!run) {
			if (f->m_active) {
				setIconOffset(0.f);
				f->m_active = false;
			}
			f->m_hasLast = false;
			return;
		}

		float vanilla = wrapAngle(this->getRotation());

		if (!f->m_active) {
			f->m_active = true;
			f->m_hasLast = false;
			f->m_visualAngle = vanilla;
			f->m_targetAngle = vanilla;
		}

		// Target angle from the REAL movement of the wave.
		// Free flight -> slope of the path, sliding on floor/ceiling/block -> flat (0).
		if (f->m_hasLast) {
			float dx = std::fabs(curX - f->m_lastX);
			float dy = curY - f->m_lastY;
			if (dx > 0.01f) {
				float ratio = std::fabs(dy) / dx;
				// ratio > 3 = teleport / respawn jump, ignore it
				if (ratio <= 3.f) {
					float ang = 0.f;
					if (ratio >= 0.2f) { // below 0.2 = blocked, stay flat
						ang = -std::atan2(dy, dx) * 57.29578f;
					}
					f->m_targetAngle = ang;
				}
			}
		}
		f->m_lastX = curX;
		f->m_lastY = curY;
		f->m_hasLast = true;

		// GD may pass dt in 1/60 s units; real steps are never above 0.02 s
		float dtReal = (dt > 0.02f) ? dt / 60.f : dt;

		float target = f->m_targetAngle;
		float mine = f->m_visualAngle;

		if (speed >= 9.99f) {
			mine = target; // instant
		} else {
			// speed 0 = vanilla, grows exponentially up to instant at 10
			float rate = 400.f * std::pow(1.6f, speed); // degrees per second
			float maxStep = rate * dtReal;
			float diff = target - mine;
			if (std::fabs(diff) <= maxStep) mine = target;
			else mine += (diff > 0.f ? maxStep : -maxStep);
		}

		// Never be slower than the vanilla rotation
		if (std::fabs(target - vanilla) < std::fabs(target - mine)) {
			mine = vanilla;
		}

		f->m_visualAngle = mine;
		setIconOffset(wrapAngle(mine - vanilla));
	}
};
