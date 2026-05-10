// Copyright (c) 2026 Cyril Tissier. All rights reserved.
#pragma once

// =============================================================================
// CAPABILITY ROUTING GRID (CRG) - COMPILER & ENVIRONMENT CONFIG
// =============================================================================

// DLL / Shared Library Support
#ifndef CRG_DLL_ENABLED
    #define CRG_DLL_ENABLED 0
#endif

// Architecture detection (Optional but handy)
#if defined(_MSC_VER)
    #define CRG_COMPILER_MSVC 1
#elif defined(__clang__)
    #define CRG_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define CRG_COMPILER_GCC 1
#endif