#define CATCH_CONFIG_MAIN
#include "external/catch.hpp"

// On inclut ici les fichiers .test.cpp (oui, on inclut le .cpp pour la co-location)
// C'est une technique courante pour compiler tous les tests éparpillés
#include "routing/domain.test.cpp"
// #include "crg/discovery/node_list.test.cpp" // futur