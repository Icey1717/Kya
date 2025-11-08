#include <cstdint>

static uint64_t m_seed = 0;

// Constants matching the PS2 LCG algorithm
static const uint64_t MULTIPLIER = 0x5851f42d4c957f2dULL;
static const uint64_t INCREMENT = 1ULL;
static const int SHIFT_AMOUNT = 32;
static const uint32_t SIGN_MASK = 0x7fffffffU;

int randImpl()
{
	// Perform 64-bit multiplication: seed = seed * MULTIPLIER
	m_seed = (m_seed * MULTIPLIER) + INCREMENT;

	// Extract upper 32 bits by right-shifting 32 positions
	uint32_t result = static_cast<uint32_t>(m_seed >> SHIFT_AMOUNT);

	// Mask to ensure only positive values (sign bit cleared)
	return static_cast<int>(result & SIGN_MASK);
}