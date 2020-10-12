#include "SystemClock.h"

#include <thread>

namespace Netcode {

	Duration SystemClock::theta;
	Duration SystemClock::delta;
	Duration SystemClock::elapsedTimeSinceLastFrame;
	Timestamp SystemClock::currentFrameTimestamp;
	Timestamp SystemClock::processStartedAt;
	
	void SystemClock::SynchronizeClocks(Duration delta, Duration theta) {
		SystemClock::theta = theta;
		SystemClock::delta = delta;
	}

	void SystemClock::Tick() {
		auto newFrameTimestamp = ClockType::now();

		elapsedTimeSinceLastFrame = newFrameTimestamp - currentFrameTimestamp;
		currentFrameTimestamp = newFrameTimestamp;
	}

	[[nodiscard]]
	Timestamp SystemClock::LocalNow() {
		return Timestamp{ ClockType::now() - processStartedAt };
	}

	[[nodiscard]]
	Timestamp SystemClock::GlobalNow() {
		return Timestamp{ (ClockType::now() - processStartedAt) + theta + (delta / 2) };
	}

	[[nodiscard]]
	float SystemClock::GetDeltaTimeInSeconds() {
		return std::chrono::duration<float>(elapsedTimeSinceLastFrame).count();
	}

	[[nodiscard]]
	double SystemClock::GetTotalTimeInSeconds() {
		return std::chrono::duration<double>((currentFrameTimestamp - processStartedAt) + theta + (delta / 2)).count();
	}

	Timestamp ConvertUInt64ToTimestamp(uint64_t v) {
		return Timestamp{ Duration{ v } };
	}

	uint64_t ConvertTimestampToUInt64(const Timestamp & timestamp) {
		return timestamp.time_since_epoch().count();
	}

}