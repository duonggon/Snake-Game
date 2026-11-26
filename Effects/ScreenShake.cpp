#include "Effects/ScreenShake.h"
#include "Effects/VfxStyleConfig.h"
#include <algorithm>
#include <cmath>

ScreenShake::ScreenShake()
    : m_enabled(VfxStyle::ENABLE_SCREEN_SHAKE),
      m_stackCount(0),
      m_currentLevel(0.0f),
      m_remainingTime(0.0f),
      m_totalAccumulatedDuration(0.0f),
      m_elapsedWaveTime(0.0f),
      m_currentOffset{0.0f, 0.0f},
      m_rng(1337u) {}

void ScreenShake::trigger() {
    if (!m_enabled) {
        return;
    }

    if (m_remainingTime <= 0.0f) {
        // Bắt đầu một chuỗi rung mới: số hạng đầu u1 = 1.5 giây, mức rung nhẹ ban đầu
        m_stackCount = 1;
        m_currentLevel = std::min(VfxStyle::SHAKE_MAX_LEVEL_CAP, VfxStyle::SHAKE_INITIAL_LEVEL);
        m_remainingTime = VfxStyle::SHAKE_FIRST_TERM_SECONDS;
        m_totalAccumulatedDuration = VfxStyle::SHAKE_FIRST_TERM_SECONDS;
        m_elapsedWaveTime = 0.0f;
    } else {
        // Đang trong chuỗi rung -> cộng dồn lần thứ k (k >= 2)
        m_stackCount += 1;

        // Số hạng thứ k của cấp số nhân: u_k = u1 * q^(k - 1), với u1 = 1.5s, q = 0.30
        const float addedDuration =
            VfxStyle::SHAKE_FIRST_TERM_SECONDS *
            std::pow(VfxStyle::SHAKE_GEOMETRIC_RATIO, static_cast<float>(m_stackCount - 1));

        m_remainingTime += addedDuration;
        m_totalAccumulatedDuration += addedDuration;

        // Tăng dần cường độ rung nhưng khóa trần tuyệt đối ở mức 6/10
        m_currentLevel = std::min(
            VfxStyle::SHAKE_MAX_LEVEL_CAP,
            m_currentLevel + VfxStyle::SHAKE_STACK_ADD_LEVEL
        );
    }
}

void ScreenShake::reset() {
    m_stackCount = 0;
    m_currentLevel = 0.0f;
    m_remainingTime = 0.0f;
    m_totalAccumulatedDuration = 0.0f;
    m_elapsedWaveTime = 0.0f;
    m_currentOffset = {0.0f, 0.0f};
}

void ScreenShake::update(float unscaledDeltaTime) {
    if (!m_enabled || m_remainingTime <= 0.0f) {
        if (m_remainingTime <= 0.0f && m_stackCount != 0) {
            reset();
        }
        return;
    }

    m_remainingTime -= unscaledDeltaTime;
    m_elapsedWaveTime += unscaledDeltaTime;

    if (m_remainingTime <= 0.0f) {
        reset();
        return;
    }

    // Hệ số giảm chấn mượt từ 1.0 -> 0.0 khi thời gian rung trôi dần về cuối
    const float safeTotal = std::max(0.001f, m_totalAccumulatedDuration);
    const float dampingRatio = std::clamp(m_remainingTime / safeTotal, 0.0f, 1.0f);

    // Độ lệch tối đa (pixel) = mức rung hiện tại (tối đa 6/10) * hệ số quy đổi pixel * giảm chấn
    const float maxAmplitudePx =
        m_currentLevel * VfxStyle::SHAKE_PIXELS_PER_LEVEL * dampingRatio;

    // Kết hợp dao động sin êm với nhiễu nhỏ để màn hình rung tự nhiên, không bị giật gắt
    std::uniform_real_distribution<float> jitterDist(-0.35f, 0.35f);
    const float smoothX = std::sin(m_elapsedWaveTime * 34.0f) * 0.65f + jitterDist(m_rng);
    const float smoothY = std::cos(m_elapsedWaveTime * 29.0f) * 0.65f + jitterDist(m_rng);

    m_currentOffset = {
        smoothX * maxAmplitudePx,
        smoothY * maxAmplitudePx
    };
}

sf::View ScreenShake::applyToView(const sf::View& baseView) const {
    if (!m_enabled || m_remainingTime <= 0.0f) {
        return baseView;
    }
    sf::View shakenView = baseView;
    shakenView.move(m_currentOffset);
    return shakenView;
}

void ScreenShake::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (!m_enabled) {
        reset();
    }
}

bool ScreenShake::isEnabled() const {
    return m_enabled;
}

bool ScreenShake::isActive() const {
    return m_enabled && m_remainingTime > 0.0f;
}

int ScreenShake::getStackCount() const {
    return m_stackCount;
}

float ScreenShake::getCurrentLevel() const {
    return m_currentLevel;
}

float ScreenShake::getRemainingTime() const {
    return m_remainingTime;
}

float ScreenShake::getTotalAccumulatedDuration() const {
    return m_totalAccumulatedDuration;
}

sf::Vector2f ScreenShake::getCurrentOffset() const {
    return m_currentOffset;
}
