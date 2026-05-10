#include "../external/catch.hpp"
#include "crg/routing/domain.hpp"

// On définit des types locaux pour le test
namespace {
    struct TestPolicy {};
    struct StandardDomain { using Policy = TestPolicy; };
    struct CustomDomain {
        using Policy = TestPolicy;
        static constexpr crg::u64 GetCustomHash() { return 0xABC; }
    };
}

TEST_CASE("Domain Identity Governance", "[routing][domain]") {
    
    SECTION("Stable Hashing (Default)") {
        crg::u64 hash1 = crg::DomainTraits<StandardDomain>::GetHash();
        crg::u64 hash2 = crg::DomainTraits<StandardDomain>::GetHash();
        
        REQUIRE(hash1 != 0);
        REQUIRE(hash1 == hash2); // Vérifie le déterminisme
    }

    SECTION("SFINAE Custom Hash Discovery") {
        crg::u64 hash = crg::DomainTraits<CustomDomain>::GetHash();
        REQUIRE(hash == 0xABC);
    }
}