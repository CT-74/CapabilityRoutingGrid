// Copyright (c) 2026 Cyril Tissier. All rights reserved.
// Licensed under the Apache License, Version 2.0.
//
// =============================================================================
// CAPABILITY ROUTING GRID (CRG) - MASTER HEADER
// =============================================================================
//
// @intent:
// The entry point for the CRG framework. This header orchestrates the 
// high-performance routing grid, bridging decoupled domains and hardware-bound 
// logic at near-zero cost.
//
// @architecture:
// - C++17 Strict: Leveraging std::void_t and 'if constexpr' for zero-cost abstraction.
// - Header-Only: Designed for rapid integration and ODR-safety.
// - Pillars: Organized into orthogonal modules (Discovery, Routing, Dispatch).
//
// @performance:
// Uses #pragma once for de facto cross-platform build performance. CRG is built
// to minimize compiler pressure while hitting the 1.5ns dispatch floor.
// =============================================================================

#pragma once

// --- PILLAR II: IDENTITY & GOVERNANCE ---
#include "routing/domain.hpp"

// Note: Future pillars (Discovery, Dispatch, Tensors) will be included here 
// as the "Core" implementation progresses.

namespace crg {
    /**
     * CRG Versioning
     */
    inline constexpr int VERSION_MAJOR = 1;
    inline constexpr int VERSION_MINOR = 0;
    inline constexpr int VERSION_PATCH = 0;
}