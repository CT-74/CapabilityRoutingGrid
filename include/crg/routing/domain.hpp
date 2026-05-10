// Copyright (c) 2026 Cyril Tissier. All rights reserved.
// Licensed under the Apache License, Version 2.0.
//
// =============================================================================
// CAPABILITY ROUTING GRID (CRG) - DOMAIN & IDENTITY GOVERNANCE
// =============================================================================
//
// @intent:
// Provide a robust, C++17 compliant identity system that isolates behaviors 
// into 'Domains'. This ensures deterministic IDs across module boundaries 
// (DLL/WASM) and network streams without manual registration.
//
// @architecture:
// - Stable Hashing: Uses a constexpr FNV-1a algorithm on compiler-specific 
//   type signatures to guarantee cross-platform ID stability.
// - DomainTraits: A SFINAE-powered detector that discovers user-defined 
//   custom hashes without forcing specialization inside the 'crg' namespace.
// - Policy Enforcement: Static contracts ensuring every Domain declares its 
//   Routing Policy, preventing architectural "silent failures".
// =============================================================================

#pragma once
#include "crg/core/types.hpp"
#include <type_traits>

namespace crg::internal {

    // =========================================================================
    // 1. STABLE HASHING (FNV-1a 64-bit)
    // =========================================================================
    // Standard typeid(T).hash_code() is non-deterministic across compilers.
    // We use a compile-time string hash on the function signature instead.
    
    constexpr uint64_t FNV1a_Baseline = 14695981039346656037ull;
    constexpr uint64_t FNV1a_Prime    = 1099511628211ull;

    constexpr uint64_t HashString(const char* str, uint64_t hash = FNV1a_Baseline) {
        return (*str == '\0') ? hash : HashString(str + 1, (hash ^ static_cast<uint64_t>(*str)) * FNV1a_Prime);
    }

    template <typename T>
    constexpr uint64_t GetStableTypeHash() {
#if defined(_MSC_VER)
        return HashString(__FUNCSIG__);
#else
        return HashString(__PRETTY_FUNCTION__);
#endif
    }

    // =========================================================================
    // 2. SFINAE DETECTION (C++17)
    // =========================================================================
    
    // Check if T defines a 'Policy' alias/type
    template <typename T, typename = void>
    struct has_policy : std::false_type {};

    template <typename T>
    struct has_policy<T, std::void_t<typename T::Policy>> : std::true_type {};

    // Check if T defines a 'static constexpr uint64_t GetCustomHash()' method
    template <typename T, typename = void>
    struct has_custom_hash : std::false_type {};

    template <typename T>
    struct has_custom_hash<T, std::void_t<decltype(T::GetCustomHash())>> : std::true_type {};

} // namespace internal

namespace crg {
    // =========================================================================
    // 3. DOMAIN TRAITS (The Bridge)
    // =========================================================================
    /**
     * @brief DomainTraits resolves the unique identity of a Domain.
     * * Architecture: It prioritizes TDomain::GetCustomHash() if present, 
     * otherwise it falls back to the stable compiler-generated hash.
     * It enforces the presence of a 'Policy' through static_assert.
     */
    template <typename TDomain>
    struct DomainTraits {
        // FAIL-FAST: A Domain without a Policy is architecturally invalid.
        static_assert(internal::has_policy<TDomain>::value, 
            "\n[CRG ERROR] Domain Contract Violation: Your domain must define a 'Policy'.\n"
            "Example: struct MyDomain { using Policy = crg::StableNetworkPolicy; };\n");

        /**
         * @return A deterministic 64-bit ID for the Domain.
         */
        static constexpr uint64_t GetHash() {
            // C++17 'if constexpr' ensures zero runtime overhead.
            if constexpr (internal::has_custom_hash<TDomain>::value) {
                return TDomain::GetCustomHash();
            } else {
                return internal::GetStableTypeHash<TDomain>();
            }
        }

        // Convenience alias for internal routing
        using Policy = typename TDomain::Policy;
    };

} // namespace crg
