# FULL TALK SCRIPT: THE CAPABILITY ROUTING GRID (CRG)
# From the Clean Code Crusade to Silicon Sympathy

=============================================================================
ACT I: THE ARCHITECT'S DREAM (0 - 25 min)
=============================================================================

# SLIDE: 00 - THE HOOK (The Build Wall)
## Visual

Un graphe de dépendances chaotique. Le nœud central `Config.h` devient rouge vif. Autour, 200 workers s'allument sur une grille réseau. Un chrono affiche : "⏱ 4 minutes".
## Code
```cpp
// You change this in a central header:
struct ConfigManager {
    bool b_EnableFeatureX = false; // ← one private bool
};

// The compiler punishes the entire network:
// 217 files recompile. 4 minutes. 200 workers.
```
## EN
Every C++ developer knows this pain. You touch a central header. You add a single private boolean. You hit build. Today, we don't wait 40 minutes anymore because our distributed build system wakes up 200 workers across the network. It takes 4 minutes. But distributed builds don't cure the disease; they just mask the symptom. We burn the carbon footprint of a small village to compile a boolean. The disease is tight coupling. Today, I am going to show you how to heal it at the root, so you never recompile the world for a boolean again.
## FR
Tout développeur C++ connaît cette douleur. Vous touchez un header central. Vous ajoutez un simple booléen privé. Vous lancez le build. Aujourd'hui, on n'attend plus 40 minutes car nos systèmes de build distribués réveillent 200 workers sur le réseau. Ça prend 4 minutes. Mais le build distribué ne guérit pas la maladie ; il ne fait que masquer le symptôme. On brûle le bilan carbone d'un petit village pour compiler un booléen. La maladie, c'est le couplage fort. Aujourd'hui, je vais vous montrer comment soigner l'architecture à la racine, pour ne plus jamais recompiler le monde pour un booléen.

---

# SLIDE: 01 - PILIER 1 (Mutual Anonymity & Zero-Exposure)
## Visual
Diagramme montrant un "Engine Core" statique et des fichiers `.cpp` isolés qui s'auto-enregistrent. Des flèches de "Linker Discovery" relient les plugins au Core sans aucun `#include`.
## Code
```cpp
// In an isolated .cpp file (Module A):
struct DroneBehavior : NodeList<BehaviorNode, IBehavior> {
    void Execute() const override { /* ... */ }
};
// ZERO EXPOSURE: No header. No public declaration. 
static const DroneBehavior g_Drone; // Registered at link-time.
```
## EN
My first pillar is "Mutual Anonymity." I wanted an architecture where adding a new capability was as simple as dropping a `.cpp` file into the project. No central registry. No `#include` of the core in the plugin. We use the Linker as our silent orchestrator. By inheriting from `NodeList`, your feature self-registers before `main()` begins. This is "Zero-Exposure": if it compiles, it's plugged in.
## FR
Mon premier pilier est l'"Anonymat Mutuel". Je voulais une architecture où ajouter une nouvelle capacité est aussi simple que de déposer un fichier `.cpp` dans le projet. Pas de registre central. Pas d'include du core dans le plugin. Nous utilisons le Linker comme orchestrateur silencieux. En héritant de `NodeList`, votre feature s'auto-enregistre avant même le début du `main()`. C'est le "Zero-Exposure" : si ça compile, c'est branché.

---

# SLIDE: 02 - PILIER 2.1 (The Interface Hell)
## Visual
Une explosion de fichiers : `IScout.h`, `ITank.h`, `IProjectile.h`. Des flèches de dépendances qui s'entrecroisent vers l'UI et le Moteur.
## EN
Traditionally, every new type needs a new interface. This creates physical dependency spikes. To show a "Scout" in the UI, the UI must know the `IScout` header. If you have 50 types, you have 50 headers. This is the death of decoupling and the birth of a monolithic build.
## FR
Traditionnellement, chaque nouveau type nécessite une nouvelle interface. Cela crée des pics de dépendances physiques. Pour afficher un "Scout" dans l'UI, l'UI doit connaître le header `IScout`. Si vous avez 50 types, vous avez 50 headers. C'est la mort du découplage et la naissance d'un build monolithique.

---

# SLIDE: 03 - PILIER 2.2 (The ModelShell & 64-byte Line)
## Visual

Schéma montrant le stockage SBO (64 bytes) et l'effacement de type (Type Erasure).
## Code
```cpp
class ModelShell {
    alignas(64) std::byte m_SBO[64]; // Zero heap, fixed size
public:
    template<typename T> T& Get() { 
        return *reinterpret_cast<T*>(m_SBO); 
    }
};
```
## EN
We use a single, opaque container: the `ModelShell`. It carries data across boundaries without ever revealing its type. No heap, no allocation, just a raw 64-byte buffer. This buffer size is intentional: it matches a CPU cache line. We are already planting the seeds for hardware affinity while seeking architectural purity.
## FR
Nous utilisons un conteneur unique et opaque : le `ModelShell`. Il transporte la donnée à travers les frontières sans jamais révéler son type. Pas de heap, pas d'allocation, juste un buffer brut de 64 octets. Cette taille est intentionnelle : elle correspond à une ligne de cache CPU. Nous plantons déjà les graines de l'affinité matérielle tout en cherchant la pureté architecturale.

---

# SLIDE: 04 - PILIER 2.3 (The Interface Killer: Definitions)
## Visual
Un schéma montrant comment le Template génère la plomberie automatiquement. Le framework reste au centre, immuable.
## Code
```cpp
// The Universal Contract
struct IRenderContract { virtual void Draw(ModelShell& s) = 0; };

// The Automated Definition (The Glue)
template<typename T>
struct RenderDefinition : IRenderContract {
    void Draw(ModelShell& s) override {
        auto& data = s.Get<T>(); // Back to concrete type
        Display(data); // Call logic
    }
};
```
## EN
This is the "Interface Killer." We define a single `IRenderContract`. We don't implement it manually 50 times. We use a **Template Definition**. It’s the glue: it inherits from the interface and uses `shell.Get<T>()` to recover the concrete data. The compiler writes the boilerplate for us. Adding a "Dragon" model doesn't require a new interface; you just provide the data. The framework is **Immutable**.
## FR
C'est l'"Interface Killer". Nous définissons un seul `IRenderContract`. Nous ne l'implémentons pas manuellement 50 fois. Nous utilisons une **Définition Template**. C'est la glu : elle hérite de l'interface et utilise `shell.Get<T>()` pour récupérer la donnée concrète. Le compilateur écrit la plomberie pour nous. Ajouter un modèle "Dragon" ne nécessite pas de nouvelle interface ; vous fournissez juste la donnée. Le framework est **Immuable**.

---

=============================================================================
ACT II: THE HARDWARE REALITY CHECK (25 - 45 min)
=============================================================================

# SLIDE: 05 - THE PIVOT (Mechanical Sympathy by Accident)
## Visual
Split-screen : À gauche, "Clean Code Crusade" (Pillars 1 & 2). À droite, un écran de profiler montrant des barres de performance saturées.
## EN
I was proud of my Zero-Header plugins. Then a friend asked: *"Does it work with an ECS?"* I said yes, because my logic was already decoupled. But when I fired up the profiler to check the throughput, I got a shock. By pushing Clean Code to its extreme, I had accidentally eliminated the flaws that starve modern processors. I sought decoupling; I achieved Mechanical Sympathy by accident.
## FR
J'étais fier de mes plugins "Zero-Header". Puis un ami m'a demandé : *"Est-ce que ça marche avec un ECS ?"*. J'ai dit oui, car ma logique était déjà découplée. Mais quand j'ai lancé le profiler pour vérifier le débit, j'ai eu un choc. En poussant le Clean Code à l'extrême, j'avais accidentellement éliminé les défauts qui affament les processeurs. J'ai cherché le découplage ; j'ai atteint l'Affinité Matérielle par accident.

---

# SLIDE: 06 - HARDWARE 101 (The Latency Cliff)
## Visual

Un diagramme montrant les cycles d'attente : L1 (4 cycles) vs RAM (300 cycles). 
## EN
To understand why traditional architectures fail, we must stop thinking in "Objects" and start thinking in "Cache Lines." A CPU reads memory in 64-byte blocks. If your data is in L1, it's 4 cycles. If it's in RAM, it's 300 cycles. While waiting, the CPU sits completely idle. This is the "Abyss." Most of our "Clean" OOP code is just a series of 300-cycle pauses.
## FR
Pour comprendre pourquoi les architectures traditionnelles échouent, nous devons arrêter de penser en "Objets" et commencer à penser en "Lignes de Cache". Un CPU lit la mémoire par blocs de 64 octets. Si la donnée est en L1, c'est 4 cycles. Si elle est en RAM, c'est 300 cycles. En attendant, le CPU reste totalement inactif. C'est l'"Abysse". La plupart de notre code OOP "propre" n'est qu'une série de pauses de 300 cycles.

---

# SLIDE: 07 - THE ASM TRUTH (Simple Jump vs Treasure Hunt)
## Visual
Comparaison d'assembleur. Gauche (OOP): `mov rax, [rcx]` puis `call qword ptr [rax]`. Droite (CRG): `call [address]`.
## EN
Look at the assembly. Traditional OOP is a "Treasure Hunt" in RAM. To call a virtual function, the CPU must fetch the pointer, then fetch the v-table, then jump. It's a double memory dependency. The CRG reduces this to the simplest possible instruction: a **Simple Jump**. No search, no indirection, just a direct `call [address]`. 
## FR
Regardez l'assembleur. L'OOP traditionnelle est une "Chasse au Trésor" en RAM. Pour appeler une fonction virtuelle, le CPU doit chercher le pointeur, puis chercher la v-table, puis sauter. C'est une double dépendance mémoire. Le CRG réduit cela à l'instruction la plus simple possible : un **Simple Jump**. Pas de recherche, pas d'indirection, juste un `call [address]` direct.

---

# SLIDE: 08 - DOD & THE MUTATION TRAP (Archetype Swap)
## Visual

Animation montrant une entité déplacée physiquement d'un tableau à un autre (Archetype Swap) pour changer d'état.
## EN
Data-Oriented Design (DOD) uses contiguous arrays to feed the prefetcher. But classical ECS has a sin: **Structural Mutation**. To change behavior (Patrol -> Combat), you physically move data between buffers. You waste bandwidth moving static bytes just to change a logical intention. It's a hardware disaster that forces cache misses and synchronization stalls.
## FR
Le Data-Oriented Design (DOD) utilise des tableaux contigus pour nourrir le prefetcher. Mais l'ECS classique a un péché : la **Mutation Structurelle**. Pour changer de comportement (Patrouille -> Combat), vous déplacez physiquement les données entre les buffers. Vous gâchez de la bande passante à déplacer des octets statiques juste pour changer une intention logique. C'est un désastre matériel qui force des cache-miss et des blocages de synchronisation.

---

=============================================================================
ACT III: THE MATHEMATICAL FUSION (45 - 55 min)
=============================================================================

# SLIDE: 09 - PILIER 3.1 (Authoring & Horner's Routing)
## Visual

Axes: Model, Biome. L'intersection `[Drone][Tundra]` s'allume.
## Code
```cpp
// 1. AUTHORING (Zero-Boilerplate Specialization via Binding)
// Default behavior for the DroneFleet
static const CapabilityBinding<DroneFleet, DefaultDrain> g_Base;

// Specialized behavior constrained to the Tundra axis (No 'if' needed)
static const CapabilityBinding<DroneFleet, TundraDrain, Biome::Tundra> g_Cold;

// 2. ROUTING (Branchless O(1) inside the Framework)
// CapabilityRouter collapses N-Dims into 1 offset using Horner's Method:
// offset = (biomeID * count) + handle.id;
auto logic = CapabilityRouter::Find<EnergyContract>(modelHandle, biome);
```
## EN
How do we write the logic, and how do we route it without branching? 
First, the Authoring. You don't write `if (biome == Tundra)`. You declare a specialized `CapabilityBinding` and pass the Axis (`Biome::Tundra`) as a template parameter. The compiler registers it.
Second, the Routing. The framework projects this into a spatial grid: the **N-Dimensional Tensor**. Using Horner’s method, it collapses the context into a single integer via simple arithmetic. Finding the behavior is no longer a search; it's a math coordinate.
## FR
Comment écrit-on la logique, et comment la route-t-on sans branchement ? 
Premièrement, l'Écriture. Vous n'écrivez pas de `if (biome == Tundra)`. Vous déclarez un `CapabilityBinding` spécialisé en passant l'Axe (`Biome::Tundra`) comme paramètre template. Le compilateur l'enregistre.
Deuxièmement, le Routage. Le framework projette cela dans une grille spatiale : le **Tenseur N-Dimensionnel**. En utilisant la méthode de Horner, il réduit le contexte en un seul entier via une simple arithmétique. Trouver le comportement n'est plus une recherche ; c'est une coordonnée mathématique.

---

# SLIDE: 10 - PILIER 3.2 (Mass Binding: Scaling Clean Code)
## Visual
Une grille 10x10. Une seule ligne de code apparaît. Instantanément, toutes les cases se remplissent de vert.
## Code
```cpp
using AirFleet = TypeList<Scout, Drone, HeavyLifter>;

// One line in an isolated .cpp to bind logic to the entire fleet.
static const CapabilityBinding<AirFleet, DiagnosticLogic> g_FleetDiag;
```
## EN
I didn't want to write 500 manual bindings. This is industrial-scale Clean Code. You provide a `TypeList` of models and a capability. In one isolated `.cpp` file, you write one line. The compiler expands the grid for you. Adding a model to the list propagates all behaviors automatically. The complexity of maintenance stays linear, while the capability matrix grows exponentially.
## FR
Je ne voulais pas écrire 500 bindings manuels. C'est du Clean Code à l'échelle industrielle. Vous fournissez une `TypeList` de modèles et une capability. Dans un fichier `.cpp` isolé, vous écrivez une ligne. Le compilateur génère la grille pour vous. Ajouter un modèle à la liste propage tous les comportements automatiquement. La complexité de maintenance reste linéaire, tandis que la matrice de capacités croît de façon exponentielle.

---

=============================================================================
ACT IV: PRAGMATISM & CONCLUSION (55 - 60 min)
=============================================================================

# SLIDE: 11 - PILIER 4 (The 1.5ns Reveal & Symbiosis)
## Visual
Un graphique de performance montrant l'ActiveCapability vs ECS Classique. Saturation à 30 GB/s.
## EN
By replacing the final jump with a cached raw function pointer (`ActiveCapability`), the logical tax falls to **1.5 nanoseconds** per entity. Total cost: 2.2ns. We have achieved **Structural Immunity**: the data NEVER moves, only the routing pointer changes. We hit the physical speed of the motherboard: 30 GB/s.
## FR
En remplaçant le dernier saut par un pointeur de fonction brut mis en cache (`ActiveCapability`), la taxe logique tombe à **1,5 nanoseconde** par entité. Coût total : 2,2ns. Nous avons atteint l'**Immunité Structurelle** : la donnée ne bouge JAMAIS, seul le pointeur de routage change. On sature la vitesse physique de la carte mère : 30 Go/s.

---

# SLIDE: 12 - WHEN NOT TO USE CRG (The Break-Even)
## Visual

Graphique montrant la zone bleue (ECS Supremacy) et la zone verte (CRG Supremacy).
## EN
CRG isn't a silver bullet. Look at the break-even curve. If your entities are tiny (under 64 bytes) or your world is static with 0% mutation, classic ECS is faster. But as soon as your entities cross the 64-byte cache-line threshold or your mutation rate grows, CRG wins. It’s a tool for high-complexity, volatile logic.
## FR
Le CRG n'est pas une solution miracle. Regardez la courbe d'équilibre. Si vos entités sont minuscules (moins de 64 octets) ou si votre monde est statique avec 0% de mutation, l'ECS classique est plus rapide. Mais dès que vos entités dépassent le seuil des 64 octets ou que votre taux de mutation grimpe, le CRG gagne. C'est un outil pour la logique complexe et volatile.

---

# SLIDE: 13 - CONCLUSION (Mechanical Sympathy by Accident)
## Visual
"Zero Coupling -> Zero Search -> Zero Migration"
## EN
I sought Clean Code; I achieved Mechanical Sympathy by accident. The CRG is a "Pay-as-you-go" architecture. Use Pillar 1 for your tools Monday morning, push to Pillar 4 for your massive simulations. Stop fighting the compiler. Stop fighting the hardware. Let the data be still, and the logic be fluid. Thank you.
## FR
J'ai cherché le Clean Code ; j'ai atteint l'Affinité Matérielle par accident. Le CRG est une architecture "Pay-as-you-go". Utilisez le Pilier 1 pour vos outils dès lundi matin, poussez jusqu'au Pilier 4 pour vos simulations massives. Arrêtez de vous battre contre le compilateur. Arrêtez de vous battre contre le matériel. Laissez la donnée immobile, et la logique fluide. Merci.