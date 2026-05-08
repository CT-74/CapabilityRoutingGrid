=============================================================================
CPPCON TALK: STATELESS BEHAVIOR PROJECTION
FULL SCRIPT & SLIDE BREAKDOWN (V5.0 - IN PROGRESS)
=============================================================================

ACT I: THE FOUNDATION (Decoupling & Survival)

SLIDE 0: THE HOOK (Hardware-Software Symmetry)
[VISUAL]
Split-Screen comparison:
Left: A CPU Die diagram highlighting physical separation between L1 Instruction Cache and L1 Data Cache.
Right: CRG architecture split: "Stateless Behaviors" (Logic) vs "Pure Data Models" (State).

[EN]
[SPEAKER SCRIPT]
"If you look at the silicon of a modern CPU, the Level 1 cache is physically divided in two: the Data Cache (L1D) and the Instruction Cache (L1I). The hardware itself demands a strict separation between state and behavior to reach peak efficiency. Yet, for decades, we have forced them together in our objects. This coupling leads to the 'Virtual Deadlock' — a term I'll come back to, because it means more than it first appears. Stateless Behavior Projection is the answer. We stop fighting the hardware. We treat data and logic as two parallel dimensions — in any C++ system, at any scale, mirroring the silicon itself."
[FR]
[SPEAKER SCRIPT]
"Si vous regardez le silicium d'un processeur moderne, le cache de niveau 1 est
physiquement divisé en deux : le cache de données (L1D) et le cache d'instructions
(L1I). Le matériel lui-même exige une séparation stricte entre l'état et le
comportement pour atteindre son efficacité maximale. Pourtant, pendant des
décennies, nous les avons forcés ensemble dans nos objets. Ce couplage crée le
'Virtual Deadlock' — un terme sur lequel je reviendrai, parce qu'il signifie
plus qu'il n'y paraît au premier abord. Le CRG est la réponse à ce problème. On arrête de se
battre contre le matériel. On traite la donnée et la logique comme deux dimensions
parallèles — dans n'importe quel système C++, à n'importe quelle échelle — en
miroir du silicium lui-même."



SLIDE 1: THE COST OF COUPLING (The Build Wall)
[VISUAL]
A single central panel — "The Build Wall":
A dependency graph radiating from a central "ConfigManager.h" node.
12 header files branch out, each with a file count label.
The central node turns red. A countdown timer reads "38 min rebuild".
Caption: "217 files recompiled. For one bool."

Large quote below the graph:
"It doesn't feel like a technical problem. It feels like a people problem."

[CODE]
// You change this:
struct ConfigManager {
    bool b_EnableFeatureX = false; // ← one bool
};

// The compiler punishes everyone:
// config_manager.h → service_layer.h → data_pipeline.h → ui_controller.h → ...
// 217 files recompile. 38 minutes. Every programmer. Every day.

[EN]
[SPEAKER SCRIPT]
"I want to start with something every C++ developer in this room has felt.

You touch a header. One bool. You hit build. You go get a coffee.
You come back. Still building. You go to lunch. It finishes while you're eating.
38 minutes. For a bool.

That's Include Hell. And here's the thing — it doesn't feel like a
performance problem. It feels like a morale problem. A communication problem.
'Who touched ConfigManager.h again?' But it's not a people problem. It's an
architecture problem. Everything is entangled at compile time.

CRG cuts that knot. The goal of Act I is simple: how do you get behaviors
to discover each other at link time, with zero shared headers?"
[FR]
[SPEAKER SCRIPT]
"Je veux commencer par quelque chose que tout développeur C++ dans cette salle a
déjà ressenti.

Vous touchez un header. Un bool. Vous lancez le build. Vous allez chercher un café.
Vous revenez. Ça compile encore. Vous allez déjeuner. Ça finit pendant que vous
mangez. 38 minutes. Pour un bool.

C'est l'Include Hell. Et voilà le truc — ça ne ressemble pas à un problème de
performance. Ça ressemble à un problème de moral. Un problème de communication.
'Qui a encore touché ConfigManager.h ?' Mais ce n'est pas ça.
C'est un problème d'architecture. Tout est couplé au moment de la compilation.

Le CRG résout ce problème. L'objectif de l'Acte I est simple : comment faire en
sorte que les comportements se découvrent mutuellement au moment du link, sans
aucun header partagé ?"



SLIDE 2: THE INTRUSIVE BACKBONE (NodeList + RegistrySlot)
[VISUAL]
Two scenarios side by side:

Left — "Monolithic (Ship / static lib)":
  A single green block: RegistrySlot<const BehaviorNode*> with static inline.
  Three gameplay structs (DroneBehavior, TankBehavior, ScoutBehavior) pointing into it.
  Label: "OS wires everything before main(). Zero config. Default mode."

Right — "DLL mode (Dev / Tools / Plugins)":
  Engine Core (.exe) block: CRG_BIND_SLOT — one explicit definition.
  Three separate DLL blocks pointing to it via linker arrows.
  Label: "Linker resolves all NodeList constructors to one address."

Bottom caption: "Define a struct. Instantiate it. That’s the entire API."

[CODE]
// MONOLITHIC (default) — static inline, ODR safe, zero config
template<class T>
struct RegistrySlot { static inline T s_Value{}; };

// DLL MODE — one explicit definition anchored in engine core
#define CRG_BIND_SLOT(T) template<> T RegistrySlot<T>::s_Value{};
CRG_BIND_SLOT(const BehaviorNode*)

// MODULE A — knows nothing about Module B, nothing about core internals
struct DroneBehavior : NodeList<BehaviorNode, IBehavior> {
    void Execute() const override { /* ... */ }
};
static DroneBehavior g_drone; // ← OS loads DLL, constructor fires. Done.

// MODULE B — completely independent
struct TankBehavior : NodeList<BehaviorNode, IBehavior> {
    void Execute() const override { /* ... */ }
};
static TankBehavior g_tank;

[EN]
[SPEAKER SCRIPT]
"In production, you rarely have one binary. You have the core, a network module,
a tools module, a robotics module — and each one needs to register its capabilities
without the core ever knowing it exists.

The naive approach is static inline everywhere. And static inline works beautifully
in a monolithic build — one definition, ODR guaranteed, every NodeList constructor
writes to the same address at startup. That’s the default mode of RegistrySlot,
and it’s what you use for ship builds.

But in development you want hot-reload. In tools you want plugins. That’s where
DLL mode kicks in. CRG_BIND_SLOT moves the definition to the engine core executable
— one explicit template specialization that every DLL resolves to via the linker.
The developer never sees any of this. That macro lives in infrastructure code.

The discovery API is the same in both modes. Define a struct, instantiate it as a
static, and the OS fires the constructor on load. No Init() function, no central
registry, no include of anything from engine core. You drop a file in a DLL.
It registers itself. That’s it.

I want to be explicit about what just happened. What I just showed you is a
fully functional plugin system. Not a framework for building one — the system
itself, as a side effect of the architecture. You drop a .cpp file, you link
the binary, and the capability is in. The engine doesn’t know it exists until
it asks. Most teams spend weeks building plugin infrastructure. This is the
infrastructure — and you got it for free by writing zero extra code."
[FR]
[SPEAKER SCRIPT]
"En production, vous n'avez jamais un seul binaire. Vous avez le core, une DLL
réseau, une DLL outils, une DLL robotique — et chacune doit pouvoir enregistrer
ses capabilities sans que le core ait à la connaître.

L'approche naïve, c'est le static inline partout. Et ça marche très bien dans un
build monolithique — une seule définition, ODR garantie, chaque constructeur de
NodeList écrit à la même adresse au démarrage. C'est le mode par défaut de
RegistrySlot, et c'est ce qu'on utilise en ship.

Mais en développement, on veut du hot-reload. En tools, on veut des plugins.
C'est là que le mode DLL entre en jeu. CRG_BIND_SLOT déplace la définition vers
l'exécutable core — une spécialisation de template explicite que chaque DLL
résout via le linker. Le programmeur ne voit rien de tout ça. Cette macro vit
dans le code d'infrastructure.

L'API de découverte est identique dans les deux modes. Définissez une struct,
instanciez-la en statique, et l'OS déclenche le constructeur au chargement.
Aucun Init(), aucun registre central, aucun include du core. Vous déposez un
fichier dans une DLL. Il s'enregistre lui-même. C'est tout.

Je veux être explicite sur ce qui vient de se passer. Ce que je vous ai montré,
c'est un système de plugins complet. Pas un framework pour en construire un —
le système lui-même, comme effet de bord de l'architecture. La plupart des
équipes passent des semaines à construire cette infrastructure. Ici, vous l'avez
obtenue gratuitement, en n'écrivant aucune ligne supplémentaire."



SLIDE 3: THE BAKING (Taking Back Control)
[VISUAL]
Two memory states stacked vertically:

BEFORE — "The Discovery Phase":
  Linked list of scattered nodes in memory.
  Red zigzag arrows between each node.
  Label: "Pointer chasing. Cache miss on every step."
  Annotation: "O(N) traversal. 100k entities × 8 behaviors = death."

AFTER — "The Baked Matrix":
  A dense, contiguous blue block. No arrows.
  Label: "One flat array. Prefetcher loves this."
  Annotation: "The CPU doesn't search. It calculates."

A thick red arrow between both states: "First Find() → StaticGuard → Bake()"

[CODE]
// BEFORE: Discovery phase — linked list, built by static init.
// Beautiful for registration. Catastrophic for 100k lookups/frame.
for (auto* n = NodeListAnchor<BehaviorNode>::s_Value; n; n = n->m_Next) {
    if (n->GetTargetModelID() == modelID) return n; // cache miss on every hop
}

// AFTER: The first Find() triggers the StaticGuard — once, automatically.
// The list is flattened into a contiguous matrix and never walked again.
return s_CapabilityTensor[denseModelID][contextOffset]; // O(1). Done.

[EN]
[SPEAKER SCRIPT]
"Static initialization gave us free discovery. Every NodeList constructor fired,
each node wired itself into the chain. Elegant. Zero boilerplate.

But now I'm in the hot path. 100,000 entities, each needing to find its behavior
every frame. Walking a linked list means pointer chasing — every hop is a
potential cache miss. Do the math: 100k entities, 8 registered behaviors average,
60fps. That's 48 million pointer dereferences per second. We built a registration
system so clean it became a performance problem.

The Bake solves this. The first time Find() is called, a static guard triggers
it automatically — once, lazily, before the first result is returned. The linked
list is walked for the last time. It gets flattened into a dense, contiguous matrix.
From that point on, the list is never touched again. Lookup becomes a calculation,
not a search. The CPU doesn't chase pointers. It computes an offset and jumps
directly to the result.

We traded the flexibility of dynamic registration for the performance of static
layout. And we never had to ask for it."
[FR]
[SPEAKER SCRIPT]
"L'initialisation statique nous a offert la découverte gratuite. Chaque constructeur
de NodeList s'est déclenché, chaque nœud s'est câblé dans la chaîne. Élégant.
Zéro boilerplate.

Mais maintenant on est dans le hot path. Cent mille entités, chacune ayant besoin
de trouver son comportement à chaque frame. Parcourir une liste chaînée, c'est
sauter de pointeur en pointeur — chaque saut est un potentiel cache miss. Faites le calcul :
cent mille entités, huit comportements enregistrés en moyenne, soixante images par
seconde. Ça fait quarante-huit millions de déréférencements de pointeurs par
seconde. On a construit un système de registration tellement propre qu'il est
devenu un problème de performance.

Le Bake résout ça. La première fois que Find() est appelé, un StaticGuard le
déclenche automatiquement — une seule fois, en lazy, avant de retourner le premier
résultat. La liste est parcourue pour la dernière fois. Elle est aplatie en une
matrice dense et contiguë. À partir de ce moment, la liste n'est plus jamais
touchée. La recherche devient un calcul. Le CPU ne chasse plus les pointeurs.
Il calcule un offset et saute directement au résultat.

On a échangé la flexibilité de l'enregistrement dynamique contre la performance
d'un layout statique. Et on n'a jamais eu à le demander."



SLIDE 4: THE OPAQUE TRANSPORT (ModelShell)
[VISUAL]
Two columns separated by a "System Boundary" line:

Left — "Producer (Gameplay)":
  A Scout{ callsign, health, team } block.
  An arrow into a sealed ModelShell box.
  Label: "Type erased. Identity preserved."

Right — "ScoutCapability (Logic)":
  ModelShell received — opaque to everyone else.
  shell.Get<Scout>() opening it.
  Label: "Only the registered behavior opens the box."

Bottom: "No shared header. No base class. No coupling."
Small note: "natvis provided — GPP sees Scout{} in debugger, not bytes."

[CODE]
// THE SHELL — Klaus’s Type Erasure, stripped of all logic.
// Concept is minimal: identity only. No virtual Execute, no nothing.
class ModelShell {
    struct Concept {
        virtual ModelTypeID GetID() const = 0;
        // That’s it. No logic lives here.
    };
    template<class T> struct Model : Concept {
        T value;
        ModelTypeID GetID() const override { return typeid(T).hash_code(); }
    };
    // Production: SBO aligned buffer + placement new. Zero heap.
    // Demo: std::unique_ptr for readability.
    std::unique_ptr<Concept> m_ptr;
public:
    template<class T> void Set(T v) { m_ptr = std::make_unique<Model<T>>(std::move(v)); }
    ModelTypeID GetID() const { return m_ptr ? m_ptr->GetID() : 0; }
    template<class T> const T& Get() const {
        return static_cast<const Model<T>*>(m_ptr.get())->value;
    }
};

// THE BEHAVIOR — the only one who knows what’s inside
struct ScoutCapability : IDiagnostic {
    void Execute(const ModelShell& shell) const override {
        const Scout& s = shell.Get<Scout>(); // static_cast. Zero overhead.
        std::cout << s.callsign << "\n";
        // Scout.h is included HERE — in the behavior’s .cpp. Nowhere else.
    }
};

[EN]
[SPEAKER SCRIPT]
"If you’ve been to CppCon before, you may have seen Klaus’s talk on Type Erasure,
or read his Design Patterns book. I did both. I loved the talk, bought the book,
implemented every version of the pattern.

And then I realized two things. First: adding new behaviors was painful —
every new capability meant touching the Concept interface. Second, and more
importantly: I didn’t need the logic inside the shell at all. My behaviors
were registered externally. The shell’s only job was to carry data and
preserve identity across boundaries.

So I kept the coquille — the shell — and threw everything else away.
The Concept is minimal: one virtual method, GetID(). That’s it.
No Execute, no Process, no logic of any kind.

The only public API right now is Get<T>() — a static_cast to recover the
typed data. And crucially, that cast only happens inside the behavior that
was registered for that type. Scout.h is included in ScoutCapability.cpp
and nowhere else. The shell crosses every boundary opaque.

In production, the Concept and Model live in a fixed-size aligned buffer on
the stack — Small Buffer Optimization. Placement new, zero heap allocation.
If your model exceeds the buffer, it’s a static_assert at compile time.
That’s not a limitation — it’s an architectural contract. Keep your data
models lean. If you need to check: sizeof(ModelShell) fits in a cache line.

One more thing for the Gameplay Programmers in the room: there’s a natvis
provided. In the debugger you see Scout{ callsign=’Vanguard-01’ }, not
a void* and 48 bytes of hex.

And we’ll come back to this shell in Act II — because it has one more
trick. An Invoke() method that will make the virtual disappear entirely."
[FR]
[SPEAKER SCRIPT]
"Si vous connaissez les conférences C++, vous connaissez peut-être le talk de
Klaus sur le Type Erasure, ou son livre Design Patterns. J'ai suivi le talk,
acheté et dévoré le livre, implémenté chaque version du pattern.

Et j'ai réalisé deux choses. Premièrement : ajouter de nouveaux comportements
était pénible — chaque nouvelle capability nécessitait de toucher l'interface
Concept. Deuxièmement, et surtout : je n'avais pas besoin de la logique à
l'intérieur du shell. Mes comportements étaient enregistrés en externe. Le seul
rôle du shell était de transporter la donnée et de préserver l'identité à travers
les frontières.

J'ai donc gardé la coquille — le shell — et j'ai tout le reste jeté. Le Concept
est minimal : une seule méthode virtuelle, GetID(). C'est tout. Pas d'Execute,
pas de Process, pas de logique.

La seule API publique pour l'instant est Get<T>() — un static_cast pour récupérer
la donnée typée. Et ce cast n'existe qu'à l'intérieur du comportement enregistré
pour ce type. scout.h est inclus dans DiagnosticCapability.cpp et nulle part ailleurs. Le shell traverse toutes les frontières de manière opaque.

En production, le Concept et le Model vivent dans un buffer aligné de taille fixe
sur la stack — Small Buffer Optimization. Placement new, zéro allocation heap.
Si votre modèle dépasse la taille du buffer, c'est un static_assert à la
compilation. Ce n'est pas une limitation — c'est un contrat architectural.
Gardez vos modèles de données légers.

Une dernière chose : un natvis est fourni. Dans le debugger, vous voyez
Scout{ callsign='Vanguard-01' }, pas un void* et 48 octets en hexadécimal.

Et on reviendra sur ce shell en Acte II — parce qu'il a encore un tour dans son
sac. Une méthode Invoke() qui fera disparaître le virtual."



SLIDE 5: THE VIRTUAL DEADLOCK (Breaking the Lock)
[VISUAL]
Two columns separated by a red lock icon in the center:

Left — "What C++ forbids":
  struct ICollection {
      template<class T>
      virtual T Get() = 0; // ← ILLEGAL
  };
  Large red ❌ below.
  Label: "Virtual + Template = Compiler error. Always."

Right — "What ModelShell does":
  struct Concept {
      virtual ModelTypeID GetID() const = 0; // one virtual, non-template
  };
  template<class T>
  const T& Get() const { ... }              // template, non-virtual
  Large green ✓ below.
  Label: "One virtual for identity. One static_cast for data. Lock broken."

Bottom, centered:
"Cold path only. Not for hot loops."

[CODE]
// THE PROBLEM — C++ won't allow this:
struct IShape {
    template<class T>
    virtual T GetAs() = 0; // compiler error — virtual template forbidden
};

// THE LOCK — you have a heterogeneous collection:
std::vector<Object*> collection;
// You want to call GetAs<Scout>() polymorphically. C++ says no.

// THE BREAK — ModelShell splits the two concerns:
struct Concept {
    virtual ModelTypeID GetID() const = 0; // virtual: identity only
};
template<class T>
const T& Get() const {                     // template: non-virtual, on the shell
    return static_cast<Model<T>*>(m_ptr)->value; // direct static_cast
}

// One virtual jump to identify. One static_cast to recover.
// The collection stays heterogeneous. The cast stays type-safe.
// Cold path only — not for hot loops.

[EN]
[SPEAKER SCRIPT]
"I called it the Virtual Deadlock at the start. Let me show you what I actually mean.

You have a heterogeneous collection. Objects of different types, all behind
a pointer. You want to call a templatized method on each one — Get<Scout>(),
Get<Drone>(), whatever the concrete type is. So you write a virtual template
method on the interface.

The compiler says no. Virtual and template are mutually exclusive in C++.
A virtual method cannot be templated. This is fundamental — the vtable is
built at compile time, template instantiation is also compile time, but they
cannot be reconciled for the same method. You are literally deadlocked by
the language.

ModelShell breaks this lock by splitting the two concerns. The Concept
exposes one virtual method — GetID() — non-template, for identity only.
Get<T>() lives on the shell itself, non-virtual, as a template. One virtual
jump to identify the type, one static_cast to recover the data.

The collection stays heterogeneous. The cast stays type-safe. The lock
is broken.

Cold path only — for tools, queries, one-off access. Not for hot loops.
But in the cold path, this is one of the most powerful tricks C++ lets
you pull off."

[FR]
[SPEAKER SCRIPT]
"J'ai parlé du Virtual Deadlock au début. Voici ce que je voulais vraiment dire.

Vous avez une collection hétérogène. Des objets de types différents, tous
derrière un pointeur. Vous voulez appeler une méthode templatisée sur chacun
— Get<Scout>(), Get<Drone>(), quel que soit le type concret. Vous écrivez
donc une méthode virtuelle template sur l'interface.

Le compilateur dit non. Virtuel et template sont mutuellement exclusifs en
C++. Une méthode virtuelle ne peut pas être templatisée. C'est fondamental —
la vtable est construite à la compilation, l'instanciation des templates aussi,
mais les deux ne peuvent pas être réconciliés pour la même méthode. Vous êtes
littéralement bloqué par le langage.

Le ModelShell casse ce verrou en séparant les deux responsabilités. Le Concept
expose une seule méthode virtuelle — GetID() — non-template, pour l'identité
uniquement. Get<T>() vit sur le shell lui-même, non-virtuel, en tant que
template. Un seul saut virtuel pour identifier le type, un static_cast direct
pour récupérer la donnée.

La collection reste hétérogène. Le cast reste type-safe. Le verrou est cassé.

Cold path uniquement — pour les outils, les requêtes ponctuelles. Pas pour
les boucles hot path. Mais dans le cold path, c'est l'un des tricks les plus
puissants que C++ vous permet de faire."



SLIDE 6: IDENTITY DECOUPLING (The Capability Binding)
[VISUAL]
Three distinct columns, no arrows between them — a central label: "Zero shared headers"

Column 1 — "Data (scout.h)":
  struct Scout { string callsign; int health; };
  Label: "Pure state. No logic. No base class."

Column 2 — "Logic (diagnostic_capability.cpp)":
  struct DiagnosticCapability : IDiagnostic { ... }
  Label: "Pure logic. Knows IDiagnostic. Knows Scout."

Column 3 — "Binding (scout_bindings.cpp)":
  static CapabilityBinding<Scout, DiagnosticCapability> g_binding;
  Label: "The wire. One line. Self-registers on load."

Bottom — a green line crossing all 3 columns:
"The Gateway resolves them at runtime. They never include each other."

[CODE]
// DATA — pure state, zero dependencies (scout.h)
struct Scout { std::string callsign; int health; };

// LOGIC — knows IDiagnostic and Scout, nothing else (diagnostic_capability.cpp)
struct DiagnosticCapability : IDiagnostic {
    void Execute(const ModelShell& shell) const override {
        std::cout << shell.Get<Scout>().callsign << "\n";
    }
};

// THE BINDING — the wire between data and logic (scout_bindings.cpp)
static const CapabilityBinding<Scout, DiagnosticCapability> g_ScoutDiag;
// ↑ One static. NodeList constructor fires. Baker picks it up. Done.

// GATEWAY — resolves at runtime, zero coupling between the three
if (auto* diag = FindCapability<IDiagnostic>(shell))
    diag->Execute(shell);

[EN]
[SPEAKER SCRIPT]
"Now we have all three pieces. Data lives in scout.h — pure state,
no base class, no dependencies. Logic lives in DiagnosticCapability —
it knows IDiagnostic and Scout, and nothing else. And the binding
is a single static that wires the two together.

These three files never include each other. The Gateway resolves
them at runtime through the baked matrix. You can add a new capability
for Scout by dropping a new .cpp file with a new static binding.
No central registry to update. No manager to touch. No recompile
of anything that doesn't need to change.

This is what kills the Build Wall. scout.h doesn't know DiagnosticCapability
exists. DiagnosticCapability doesn't know TelemetryCapability exists.
They're all isolated. The only things they share are the interface header —
idiagnostic.h — and the data model — scout.h. Both are stable and rarely
touched. And that's exactly the coupling we choose to keep.

That's the Identity Decoupling. Data, Logic, and Binding are three
orthogonal dimensions. You compose them at link time, not at compile time."
[FR]
[SPEAKER SCRIPT]
"Maintenant on a les trois pièces. La donnée vit dans scout.h — état pur, aucune
dépendance. La logique vit dans DiagnosticCapability — elle connaît IDiagnostic
et Scout, rien d'autre. Et le binding est un simple statique qui relie les deux.

Ces trois fichiers ne s'incluent jamais mutuellement. Le Gateway effectue le
routage au runtime à travers la matrice baked. On peut ajouter une nouvelle
capability pour Scout en déposant un nouveau .cpp avec un nouveau binding statique.
Aucun registre central à mettre à jour. Aucun manager à toucher. Aucune
recompilation de ce qui n'a pas changé.

C'est ce qui tue le Build Wall. scout.h ne sait pas que DiagnosticCapability
existe. DiagnosticCapability ne sait pas que TelemetryCapability existe. Tout est
isolé. La seule chose qu'ils partagent, c'est le header d'interface — idiagnostic.h —
et le modèle de données — scout.h. Les deux sont stables et rarement touchés.
Et c'est précisément le couplage que l'on choisit de garder.

C'est l'Identity Decoupling. Donnée, Logique et Binding sont trois dimensions
orthogonales. On les compose au moment du link, pas à la compilation."



SLIDE 7: THE OOP ILLUSION (ModelRouter + Invoke/TryInvoke)
[VISUAL]
Two columns, separated by a central label "One virtual jump":

Left — "Cold Path (tools, UI, one-off)":
  shell.Invoke<&IMovement::Move>(ctx)
  A downward arrow:
    → ModelShellMethodTraits<funcPtr>::Interface = IMovement
    → FindInterface<IMovement>() [one virtual]
    → ModelRouter<Scout>::FindInterface(interfaceID) [O(N) at this stage]
    → IMovement::Move(shell, ctx)
  Green label: "Clean OOP syntax. Zero boilerplate."

Right — "Hot Path (NEVER here)":
  for (auto [shell, data] : ecs.View<...>())
      shell.Invoke<&IMovement::Move>(ctx); // ← RED CROSS
  Red label: "One virtual per call × 100k entities = wrong tool."

Bottom caption: "Invoke is your query API. Not your loop API."

[CODE]
// THE ROUTER — at this stage, O(N) linear search
// Same CapabilityBindings as stage 04, just accessed differently
template<class ImplClassT>
struct ModelRouter {
    static const void* FindInterface(InterfaceTypeID interfaceID) {
        for (auto* n = NodeListAnchor<ICapabilityNode<void>>::s_Value; n; n = n->m_Next) {
            if (n->GetTargetModelID() == TypeIDOf<ImplClassT>::Get())
                if (auto* ptr = n->Resolve(interfaceID)) return ptr;
        }
        return nullptr;
    }
};

// THE ILLUSION — method pointer carries the interface in its type
// GPP never names InterfaceT explicitly
shell.Invoke<&IMovement::Move>(ctx);
// ModelShellMethodTraits extracts at compile time:
//   Interface = IMovement  (from the method pointer type)
//   Validates: first arg must be const ModelShell& — compile error otherwise
//   ptr = FindInterface<IMovement>()  // one virtual to recover concrete type
//   ptr->Move(shell, ctx)             // direct call

// TryInvoke — returns result if found, Optional{} only on failure
auto result = shell.TryInvoke<&ILogger::GetAsString>();
// → returns the string directly if registered
// → returns Optional{} if shell is invalid or interface not found

[EN]
[SPEAKER SCRIPT]
"The ModelShell has one more API: Invoke and TryInvoke.

The method pointer you pass is analyzed by ModelShellMethodTraits at compile
time. It extracts the interface — you never name it explicitly. And it
validates that the method signature starts with const ModelShell& — if it
doesn't, it's a compile error. That's a strong guarantee: you cannot call
a method that wasn't designed for this system.

Invoke assumes success — two asserts, no Optional, direct call. If the shell
is empty or the interface not registered, it fails fast and loudly. Use it
when you know the capability exists.

TryInvoke is the defensive version. If the shell is invalid or the interface
not found, it returns an empty Optional — silently. If the capability is
found, it returns the result directly.

One practical example: a fmt formatter on the shell delegates to the concrete
type's own formatter via TryInvoke. If the type has a formatter registered,
you get a readable print. If not, silent fallback. Optional feature — no
obligation to implement it for every type in production.

One convention: the first argument of every interface method is always the
shell itself — const ModelShell& self. Invoke passes *this automatically.
Your behavior always has access to the full model if it needs it.

Now — I need to say this clearly. Invoke and TryInvoke are your cold path API.
Tools. UI. One-off queries. Debug code. The moment you put Invoke inside a
loop over 10,000 entities you've paid a virtual call per iteration. That's
not what this is for.

The hot path comes later. And when it does, you won't use Invoke at all."
[FR]
[SPEAKER SCRIPT]
"Le ModelShell a encore une API : Invoke et TryInvoke.

Le pointeur de méthode que vous passez est analysé par ModelShellMethodTraits
à la compilation. Il en extrait l'interface — vous ne la nommez jamais
explicitement. Et il vérifie que la signature commence bien par
const ModelShell& — si ce n'est pas le cas, erreur de compilation. C'est
une garantie forte : vous ne pouvez pas appeler une méthode qui n'est pas
conçue pour ce système.

Invoke assume le succès — deux asserts, pas d'Optional, appel direct. Si le
shell est vide ou l'interface non enregistrée, ça crashe vite et clairement.
Utilisez-le quand vous êtes certain que la capability existe et que le shell n'est pas vide.

TryInvoke est la version défensive. Si le shell est invalide ou l'interface
non trouvée, il retourne un Optional vide — silencieusement. Si la capability
est trouvée, il retourne le résultat directement.

Un exemple concret : un fmt formatter sur le shell délègue au formatter du type
concret via TryInvoke. Si le type a un formatter enregistré, vous obtenez un
affichage lisible. Sinon, fallback silencieux. C'est une feature optionnelle —
pas d'obligation de l'implémenter pour tous les types en production.

Une convention : le premier argument de chaque méthode d'interface est toujours
le shell lui-même — const ModelShell& self. Invoke passe *this automatiquement.
Votre comportement a toujours accès au modèle complet si besoin.

Cold path uniquement. Outils, UI, requêtes ponctuelles, code de debug. Le
moment où vous mettez Invoke dans une boucle sur dix mille entités, vous payez
un appel virtuel par itération. Ce n'est pas l'usage prévu.

Le hot path vient plus tard. Et quand il arrive, vous n'utiliserez plus
Invoke du tout."



SLIDE 8: THE BAKER (One Line, N Models)
[VISUAL]
An inverted funnel in three steps:

Top — three separate blocks: Scout, Drone, HeavyLifter
Middle — DiagCapability<T> + TeleCapability<T> (templates)
Bottom — CapabilitySpace baked into the Router for each model

One line highlighted in green:
  static CapabilityBaker<AirModels, DiagCapability, TeleCapability> g_AirBaker;
Label: "One static. Six entries in the matrix. Zero boilerplate."

Right side, greyed out: stage 05 manual CapabilityBindings.
Red label: "What we had before."

[CODE]
// Stage 05 — what we had to write manually:
static CapabilityBinding<Scout, DiagCapability<Scout>>   g_s1;
static CapabilityBinding<Scout, TeleCapability<Scout>>   g_s2;
static CapabilityBinding<Drone, DiagCapability<Drone>>   g_d1;
static CapabilityBinding<Drone, TeleCapability<Drone>>   g_d2;
// ... × every model × every capability. Doesn't scale.

// Stage 06 — the Baker:
template<class T> struct DiagCapability : Capability<IDiagnostic> {
    void Execute(const ModelShell& self) const override {
        std::cout << "[Diag] " << self.Get<T>().name << "\n";
    }
};

using AirModels = TypeList<Scout, Drone, HeavyLifter>;

// One line. All models. All capabilities. Self-registered. Baked on first Find().
static const CapabilityBaker<AirModels, DiagCapability, TeleCapability> g_AirBaker;

// Adding a new system for the same models: one more line, separate .cpp file.
static const CapabilityBaker<AirModels, TeleCapability> g_TeleBaker;

[EN]
[SPEAKER SCRIPT]
"Stage 05 showed the ModelRouter and the Invoke API. But the registration
was still manual — one CapabilityBinding per model per capability.
Three models, two capabilities: six statics to write and maintain.
Add a model: two more lines. Add a capability: three more lines.
It doesn't scale.

The Baker fixes this. DiagCapability becomes a template on TModel.
Inside Execute, instead of Get<Scout>() hardcoded, we call Get<T>().
The type is known at compile time — the template carries it.

Then a single CapabilityBaker line expands the TypeList. Three models,
two capabilities: one static, six entries in the matrix. The Baker
inherits from CapabilitySpace<Scout, ...>, CapabilitySpace<Drone, ...>,
CapabilitySpace<HeavyLifter, ...> — all assembled into the Router on
the first Find() call via the StaticGuard we saw in slide 3.

And decentralization is preserved. You can group all definitions of a domain
into a single Baker — a TypeList of models, a list of capabilities. Add a model
to the TypeList? Every capability propagates. Add a capability to the list?
Every model gets it. The matrix builds itself. Or split them into separate
Bakers for independent teams — both approaches coexist.
The Router holds them all.

This is the GPP API in full: define a templatized capability, declare
one Baker static, and you're done. No registration, no Init(),
no central file to touch."
[FR]
[SPEAKER SCRIPT]
"Le slide précédent a montré l'Invoke. Mais l'enregistrement était encore manuel
— un CapabilityBinding par modèle par capability. Trois modèles, deux capabilities :
six statiques à écrire et maintenir. Ajoutez un modèle : deux lignes de plus.
Ajoutez une capability : trois lignes de plus. Ça ne scale pas.

Le Baker résout ça. DiagCapability devient un template sur TModel. À l'intérieur
d'Execute, au lieu de Get<Scout>() en dur, on appelle Get<T>(). Le type est connu
à la compilation — le template le porte.

Ensuite, une seule ligne CapabilityBaker expand la TypeList. Trois modèles, deux
capabilities : un seul statique, six entrées dans la matrice. Le Baker hérite de
CapabilitySpace<Scout, ...>, CapabilitySpace<Drone, ...>,
CapabilitySpace<HeavyLifter, ...> — tous assemblés dans le Router au premier
appel de Find() via le StaticGuard qu'on a vu au slide 3.

Et la décentralisation est préservée. On peut regrouper toutes les définitions
d'un domaine dans un seul Baker — une TypeList de modèles, une liste de
capabilities. Ajoutez un modèle dans la TypeList ? Toutes les capabilities se
propagent. Ajoutez une capability ? Tous les modèles en bénéficient. La matrice
se construit seule. Ou séparez-les en Bakers distincts pour des équipes
indépendantes — les deux approches coexistent. Le Router les tient tous.

C'est l'API GPP complète : définissez une capability templatisée, déclarez un
Baker statique, et c'est terminé. Aucun enregistrement, aucun Init(), aucun
fichier central à toucher."



=============================================================================
ACT II: THE TENSOR & THE ILLUSION
=============================================================================

SLIDE 9: THE PRICE OF FREEDOM (Why a Linked List Isn't Enough)
[VISUAL]
Two columns, same frame:

Left — "What the Baker gave us" (green):
  Linked list → baked matrix. One static. Zero boilerplate.
  Label: "Act I: solved. Registration is free."

Right — "What the hot path costs" (red):
  The baked registry as a vector of N model nodes.
  One hundred thousand entities each iterating through it.
  Each iteration: a modelID comparison + potential cache miss.
  Counter: "100,000 entities × N models = O(N) per frame. Still a search."

Bottom: "We solved coupling. We created a new bottleneck."

[CODE]
// What the Baker gave us — free, elegant, zero boilerplate:
static const CapabilityBaker<AirModels, DiagCapability> g_Baker;

// What CapabilityRouter::Find() actually does right now:
for (const auto* node : RouterSlot::s_Value) {        // iterate N model nodes
    if (node->GetTargetModelID() == modelID) {         // comparison per node
        if (const void* ptr = node->Resolve(iid))
            return static_cast<const InterfaceT*>(ptr);
    }
}
// Fine for tools and cold path (Invoke/TryInvoke).
// Catastrophic at one hundred thousand entities × 60fps.

// What we need:
return s_CapabilityTensor[denseModelID][contextOffset]; // zero search. pure math.
// But modelID is a hash — random 32-bit value. Can't use as array index.
// That's the next problem.

[EN]
[SPEAKER SCRIPT]
"The Baker is elegant. One static, N models, zero boilerplate. The Router
assembles everything lazily on the first Find(). For tools, for the cold
path, for one-off queries — it's perfect.

But let's look at what Find() actually does. It iterates over the registered
model nodes, compares modelIDs, resolves the interface. That's O(N) over
the number of registered models. For three models it's invisible. For 300
models at one hundred thousand entities per frame at 60fps — it's a
bottleneck we just moved, not eliminated.

The goal is this: one calculation, zero search. Given a modelID and a
context, compute a memory offset and jump directly. No loop. No comparison.
Pure math.

But there's a problem. Our modelID right now is a hash — a random 32-bit
value. You can't use a random 32-bit value as an array index. That's the
next slide."
[FR]
[SPEAKER SCRIPT]
"Le Baker est élégant. Un statique, N modèles, zéro boilerplate. Le Router
assemble tout lazily au premier Find(). Pour les outils, pour le cold path,
pour les requêtes ponctuelles — c'est parfait.

Mais regardons ce que Find() fait réellement. Il itère sur les nœuds de modèles
enregistrés, compare les IDs, résout l'interface. C'est O(N) sur le nombre de
modèles enregistrés. Pour trois modèles c'est invisible. Pour trois cents modèles
à cent mille entités par frame à soixante images par seconde — c'est un goulot
d'étranglement qu'on a juste déplacé, pas éliminé.

L'objectif c'est ça : un calcul, zéro recherche. Donné un modelID et un contexte,
calculer un offset mémoire et sauter directement. Pas de boucle. Pas de
comparaison. Du pur calcul.

Mais il y a un problème. Notre modelID en ce moment c'est un hash — une valeur
size_t aléatoire. Ça ne peut pas servir d'index de tableau. C'est le slide suivant."



SLIDE 10: THE ID EXPLOSION (The Sparse Universe)
[VISUAL]
Two memory layouts stacked vertically:

Top — "Your ModelTypeIDs (typeid hashes)":
  A horizontal number line from 0 to 4,294,967,295.
  Three labeled dots scattered far apart:
    Scout  → 0x4A1F2C8E (1,243,990,158)
    Drone  → 0x9B3D7F21 (2,604,859,169)
    Heavy  → 0xC7E30F44 (3,353,071,428)
  Huge empty gaps. Below: "Array needs 4 billion slots. 16 GB for pointers alone."

Bottom — "What we need (Dense IDs)":
  A tight block: [0][1][2]. Labels: Scout=0, Drone=1, Heavy=2.
  Below: "3 slots. Direct index. O(1)."
  A large red arrow from Top to Bottom: "The Collapse."

[CODE]
// ModelTypeIDs are typeid hashes — random size_t values
ModelTypeID scoutID = typeid(Scout).hash_code(); // e.g. 0x4A1F2C8E
ModelTypeID droneID = typeid(Drone).hash_code(); // e.g. 0x9B3D7F21

// Direct array indexing? Impossible.
s_CapabilityTensor[scoutID]; // → billions of slots. No.

// Hash map? Pointer chase to bucket + collision handling. Cache-unfriendly.
// Sorted array + binary search? O(log N). Still a search.

// We need this:
scoutID → dense index 0
droneID → dense index 1
s_CapabilityTensor[0]; // Scout's capability. One instruction.

// DenseTypeID — assigns a contiguous index (0, 1, 2...) at startup.
template<class T>
struct DenseTypeID {
    static DenseID Get() {
        static DenseID s_id = TypeIDGenerator::GetNext(); // 0, 1, 2...
        return s_id;
    }
};

[EN]
[SPEAKER SCRIPT]
"Here's the problem. Our ModelTypeIDs come from typeid().hash_code().
Scout hashes to one point two billion. Drone hashes to two point six billion.
These are arbitrary size_t values scattered across a four-billion-slot space.

If I try to use them directly as array indices, my behavior table needs
four billion slots. Sixteen gigabytes just for pointers. That's not an option.

A hash map avoids the memory cost but reintroduces the problem we're trying
to solve — pointer chasing, collision handling, unpredictable access patterns.
A sorted array with binary search is O(log N). Still a search.

The only path to a true direct array access is to collapse this sparse
universe into consecutive integers. Zero, one, two, three. That's DenseTypeID.
Each type gets a static local that increments a global counter once, the first
time it's touched. Scout gets zero. Drone gets one. HeavyLifter gets two.

Now s_CapabilityTensor[0] is Scout's capability. One array access. One instruction.
That's the foundation of the tensor.

One important note: DenseID is non-deterministic across processes and machines.
Never send it over the network. For serialization or networking, use the hash
— the receiver converts hash to DenseID on its own side. If you need a fully
deterministic catalog, register your types against a stable key: a string,
an enum, or a compile-time hash from a code generation step."
[FR]
[SPEAKER SCRIPT]
"Voilà le problème. Nos ModelTypeIDs viennent de typeid().hash_code(). Scout hash
à un virgule deux milliards. Drone hash à deux virgule six milliards. Ce sont des
valeurs size_t arbitraires éparpillées sur un spectre de quatre milliards de slots.

Si j'essaie de les utiliser directement comme index de tableau, ma table de
capabilities a besoin de quatre milliards de slots. Seize gigaoctets juste pour
des pointeurs. Hors de question.

Une hash map évite le coût mémoire mais réintroduit le problème qu'on essaie de
résoudre — pointer chasing, gestion des collisions, accès mémoire imprévisible.
Un tableau trié avec recherche dichotomique, c'est O(log N). C'est encore une
recherche.

Le seul chemin vers un accès direct O(1) est de réduire cet espace épars en
entiers consécutifs. Zéro, un, deux, trois. C'est DenseTypeID. Chaque type
obtient un local statique qui incrémente un compteur global une seule fois —
la première fois qu'on le touche. Scout obtient zéro. Drone obtient un.
HeavyLifter obtient deux.

Maintenant s_CapabilityTensor[0] c'est la capability de Scout. Un accès tableau.
Une instruction.

Note importante : DenseID n'est pas déterministe entre processus et machines.
Ne l'envoyez jamais sur le réseau. Pour la sérialisation, utilisez le hash —
le récepteur convertit hash vers DenseID de son côté."



SLIDE 11: THE DENSE INDEXER (The Collapse in Action)
[VISUAL]
Animation in two steps:

Step 1 — "Static init fires":
  Three blocks lighting up in sequence:
  DenseTypeID<Scout>::Get()       → 0
  DenseTypeID<Drone>::Get()       → 1
  DenseTypeID<HeavyLifter>::Get() → 2
  Label: "Happens once. On first touch. C++11 static local — thread safe."

Step 2 — "The flat tensor":
  A contiguous 2D table:
  Rows    : [Scout=0] [Drone=1] [HeavyLifter=2]
  Columns : [context offset 0] [context offset 1] ... [context offset N]
  One cell highlighted in green: s_CapabilityTensor[1][3]
  Label: "Drone. Context offset 3. One array access. Done."

[CODE]
// THE GENERATOR — one global counter, increments on first touch per type
struct TypeIDGenerator {
    static DenseID GetNext() {
        static DenseID s_id = 0;
        return s_id++;
    }
};

// THE DENSE ID — assigned once, cached forever in a static local
template<class T>
struct DenseTypeID {
    static DenseID Get() {
        static DenseID s_id = TypeIDGenerator::GetNext();
        return s_id;
    }
};

// THE FLAT TENSOR — outer index = DenseID, inner = context offset
// No search. No comparison. Pure array arithmetic.
static std::vector<std::vector<RouteNode*>> s_CapabilityTensor;

// Registration: auto-expand on first encounter
if (denseID >= s_CapabilityTensor.size())
    s_CapabilityTensor.resize(denseID + 1, /* context volume slots */);

// Lookup: two array accesses. That's it.
return s_CapabilityTensor[denseID][contextOffset];

[EN]
[SPEAKER SCRIPT]
"DenseTypeID is beautifully simple. Each type has a static local that calls
GetNext() exactly once — the first time that type is touched. Scout gets zero,
Drone gets one, HeavyLifter gets two. The C++11 standard guarantees this is
thread-safe. No mutex, no atomic, no ceremony.

Now we can build the flat tensor. It's a vector of vectors — outer index is
the DenseID, inner index is the context offset we'll compute in a moment.
Registration auto-expands the outer vector when a new DenseID arrives.
Lookup is two array accesses. No loop, no comparison, no hash collision.

This is the moment the search disappears. From here on, finding a capability
is not a question of 'where is it?' — it's a question of 'what is the offset?'
And that's pure math."
[FR]
[SPEAKER SCRIPT]
"DenseTypeID est d'une simplicité désarmante. Chaque type a un local statique qui
appelle GetNext() exactement une fois — la première fois qu'on le touche. Scout
obtient zéro, Drone obtient un, HeavyLifter obtient deux. Le standard C++11
garantit que c'est thread-safe. Pas de mutex, pas d'atomique, zéro cérémonie.

Maintenant on peut construire le CapabilityTensor. C'est un vecteur de vecteurs —
l'index extérieur c'est le DenseID, l'index intérieur c'est l'offset de contexte
qu'on calculera dans un moment. L'enregistrement expand automatiquement le vecteur
extérieur quand un nouveau DenseID arrive. Le lookup c'est deux accès tableau.
Pas de boucle, pas de comparaison, pas de collision.

C'est le moment où la recherche disparaît. À partir d'ici, trouver une capability
n'est plus une question de 'où est-elle ?' — c'est une question de 'quel est
l'offset ?' Et ça, c'est du pur calcul."



SLIDE 12: THE HYPERCUBE (N-Dimensional Space)
[VISUAL]
A 3D cube with three labelled axes:
  Y — DenseID  (Scout=0, Drone=1, HeavyLifter=2)
  X — Zone          (Desert=0, Tundra=1)
  Z — State         (Init=0, Combat=1)

One cell highlighted in green: [Drone=1][Combat=1][Desert=0]
Label: "One intersection. One behavior. Computed, not searched."

Below, the Horner formula in large text:
  offset = (State * Zone_count) + Zone
         = (1     * 2         ) + 0    = 2
  index  = DenseID * Volume + offset
         = 1       * 4      + 2        = 6
Caption: "Pure arithmetic. Resolved at compile time. Zero branches."

[CODE]
// THE SPACE — compile-time stride math via Horner's method
template<class... TAxes>
struct Space {
    static constexpr std::size_t Volume = (EnumTraits<TAxes>::Count * ... * 1);

    template<typename... TArgs>
    static constexpr std::size_t ComputeOffset(const TArgs&... args) {
        // Horner's method: offset = (...((a0 * n1 + a1) * n2 + a2)...)
        // One fold, no branches, fully resolved at compile time.
        std::size_t offset = 0;
        const std::size_t coords[] = { static_cast<std::size_t>(args)... };
        const std::size_t dims[]   = { EnumTraits<TAxes>::Count... };
        for (std::size_t i = 0; i < sizeof...(TAxes); ++i)
            offset = offset * dims[i] + coords[i];
        return offset;
    }
};

// INTERFACE DECLARATION — GPP defines the axes, nothing else
struct IMovement {
    using InterfaceType = IMovement;
    virtual void Move(const ModelShell& self) const = 0;
};

// ROUTING TRAITS — binds the Space to the interface (separate from the interface)
template<> struct CapabilityRoutingTraits<IMovement> {
    using SpaceType = CapabilitySpace<State, Zone>; // 2D: 2×2 = 4 cells
};

// LOOKUP — two numbers, one jump. No search.
std::size_t offset = CapabilityRoutingTraits<IMovement>::SpaceType
                         ::ComputeOffset(State::Combat, Zone::Desert); // = 2
return s_CapabilityTensor[DenseTypeID<Drone>::Get()][offset];               // [1][2]

[EN]
[SPEAKER SCRIPT]
"We have the rows — DenseID gives us the model dimension.
Now we need the columns — the context dimension.

A behavior doesn't just depend on what the entity is. It depends on
where it is, when it is, who controls it. State. Zone. Authority.
These are your context axes. You declare them on CapabilityRoutingTraits —
separate from the interface, so the interface stays clean.

Space encodes the axes at compile time. IMovement has two: State and Zone.
Two values each — a 2×2 grid, four cells. Volume equals four.

ComputeOffset uses Horner's method. It's the same algorithm used to evaluate
polynomials efficiently — one accumulation loop, no branches, no divisions.
State times Zone_count plus Zone. Done. One number.

Now lookup is two numbers. Two array accesses. The CPU computes both in the
same cycle. There is no search, no comparison, no branch.

And here's the beautiful part: every axis you add multiplies the Volume —
but the lookup cost stays absolutely flat. One dimension or ten,
it's always the same two array accesses. Complexity is free."
[FR]
[SPEAKER SCRIPT]
"On a les lignes — DenseID nous donne la dimension modèle. Maintenant on a
besoin des colonnes — la dimension contexte.

Un comportement ne dépend pas seulement de ce qu'est l'entité. Il dépend de
où elle est, quand elle est, qui la contrôle. State. Zone. Authority. Ce sont
vos axes de contexte. On les déclare dans CapabilityRoutingTraits — séparé de
l'interface, pour que l'interface reste propre.

CapabilitySpace encode les axes à la compilation. IMovement a deux axes : State
et Zone. Deux valeurs chacun — une grille 2×2, quatre cellules. Volume égal quatre.

ComputeOffset utilise la méthode de Horner. C'est le même algorithme qu'on
utilise pour évaluer des polynômes efficacement — une boucle d'accumulation,
pas de branches, pas de divisions. State multiplié par Zone_count plus Zone.
Terminé. Un seul nombre.

Maintenant le lookup c'est deux nombres. Deux accès tableau. Le CPU calcule
les deux dans le même cycle. Pas de recherche, pas de comparaison, pas de branche.

Et le plus beau dans tout ça ? Chaque axe que vous ajoutez multiplie le Volume
— mais le coût du lookup reste absolument identique. Une dimension ou dix, ce sont
toujours les mêmes deux accès tableau. La complexité est gratuite."



=============================================================================
ACT III: SYMBIOSIS & THE HARDWARE LIMIT
=============================================================================

SLIDE 13: THE CRG CURE (Immutable Topology)
[VISUAL]
Two columns side by side:

Left — "Mutable Topology (The Classic Approach)":
  A behavior graph rewiring itself every frame.
  Red arrows: Add, Remove, Rewire.
  A green prefetcher line turning red.
  Label: "Topology mutates. Prefetcher loses the stream. Every time."

Right — "Immutable Topology (CRG)":
  The same matrix. Frozen. No mutation arrows.
  One green arrow: "context coordinate → tensor cell"
  Label: "Matrix never changes. Only the coordinate changes."

Caption: "Immutability is not a constraint. It's a performance guarantee."

[CODE]
// MUTABLE TOPOLOGY — behavior change = structural rewire
behaviors.remove(entity, patrol);   // topology mutates
behaviors.add(entity, combat);      // prefetcher loses the stream

// CRG — behavior change = coordinate update. Topology: untouched.
world_state = State::Combat;        // one write. matrix: unchanged.

// Same two array accesses. Same memory addresses. Prefetcher never blinked.
auto* b = s_CapabilityTensor[denseID][CapabilitySpace::ComputeOffset(world_state, zone)];

[EN]
[SPEAKER SCRIPT]
"Act II gave us a tensor. A flat, contiguous matrix of behavior pointers,
indexed by model and context. Built once, never touched again.

That immutability is not an accident — it's the core guarantee.

In a mutable topology, every behavior change is a structural operation.
You add, remove, rewire. The memory layout shifts. The prefetcher had
a prediction. Now it's wrong. You pay a cache penalty on every transition
— not for any logical reason, purely structural.

In CRG, the matrix never changes. A behavior transition is a coordinate
update. world_state flips to Combat. On the next lookup, the tensor
returns a different pointer. The matrix didn't move. The memory addresses
didn't change. The prefetcher kept its stream.

One extension worth mentioning: for continuous conditions — health below 30,
distance under 10 meters — each tensor cell holds a small sorted list of
predicates. The Narrow Phase. O(1) to the right cell, O(K) through K rules.
K is always small. Happy to take questions on that one.

Immutability is not a constraint. It's a performance guarantee.
And it's what allows us to reach the hardware limit — which is the
next slide."
[FR]
[SPEAKER SCRIPT]
"TODO"



SLIDE 14: ECS SYMBIOSIS (The Cherry on the Cake)
[VISUAL]
Two horizontal lines stacked:

Top — "The Brain (5 Hz)":
  A tick every 200ms.
  CapabilityRouter::Find() → assigns result into ActiveCapability.
  Label: "Expensive. Runs rarely. Owns the decision."

Bottom — "The Muscle (60 Hz)":
  A tick every 16ms.
  Loop over contiguous arrays. cap(params). Direct call.
  Label: "Free. Runs always. Owns the execution."

Arrow between the two: "ActiveCapability — the handoff."
Caption: "CRG owns Logic Projection. ECS owns the Data Pipeline."

[CODE]
// THE BRAIN — 5 Hz. Resolves context, caches the result.
void System_UpdateLogic(Registry& ecs, WorldState state, Zone zone) {
    for (auto& [handle, cap] : ecs.View<ModelShell, ActiveCapability<EnergyContract>>()) {
        cap = CapabilityRouter::Find<EnergyContract>(handle, state, zone);
    }
}

// THE MUSCLE — 60 Hz. No decision. No lookup. No virtual.
void System_Execute(Registry& ecs) {
    for (auto& [battery, cap] : ecs.View<Battery, ActiveCapability<EnergyContract>>()) {
        EnergyContract::Params params { battery.charge };
        cap(params);
    }
}

[EN]
[SPEAKER SCRIPT]
"This is why I called it a symbiosis — not a replacement.

The ECS owns the data pipeline. Entities are pure structs packed in
contiguous arrays. Nothing moves. The prefetcher is happy.

CRG owns the logic projection. It knows which behavior applies given
the current context — state, zone, whatever axes you've defined.

The handoff is an ActiveCapability component. The Brain system ticks
at 5Hz. It calls CapabilityRouter::Find() — O(1) tensor lookup —
and assigns the result with a simple operator=. That's the decision.

The Muscle system ticks at 60fps. It iterates packed arrays and calls
the ActiveCapability like a function — operator(). No virtual dispatch,
no lookup, no decision of any kind. A direct function pointer call.
Fetch, read, execute.

The expensive part runs rarely. The cheap part runs always.
And the entity never moved. The prefetcher never blinked.

We're not bottlenecked by logic anymore. We're bottlenecked by RAM.
That's exactly where we want to be."
[FR]
[SPEAKER SCRIPT]
"TODO"



SLIDE 15: THREE LEVELS (The Performance Ladder)
[VISUAL]
Three stacked rows, left-aligned code, cost-per-entity on the right:
Row 1 (grey)  — "Level 1: Classic OOP"              → ~20 ns/entity
Row 2 (blue)  — "Level 2: CRG Shell (cold path)"    →  ~7 ns/entity
Row 3 (green) — "Level 3: ActiveCapability (DOD)"   → 1.5 ns/entity
Each row lights up in sequence as you speak.
Small note: "Measured at 1M+ entities, memory-bound zone. Apple M-Series."

[CODE]
// Level 1 — classic vtable on every call
entity.GetBehavior()->Update(data);                    // ~20 ns

// Level 2 — CRG Shell cold path: one virtual, then O(1) tensor
shell.Invoke<&IMovement::Move>(ctx);                   //  ~7 ns
// Use for tools, UI, one-off queries. Never in a hot loop.

// Level 3 — ActiveCapability: Brain cached it. Muscle just calls.
cap(params);                                           // 1.5 ns
// Zero virtual. Zero lookup. Direct function pointer.

[EN]
[SPEAKER SCRIPT]
"Three levels. Each one trades a bit of flexibility for a lot of speed.

Level one: classic OOP. A vtable dispatch on every call. Around twenty
nanoseconds per entity in the memory-bound zone. Familiar, comfortable,
but expensive at scale.

Level two: the CRG Shell. Invoke pays one virtual call to recover the
concrete type — that's the price of type erasure. Then it jumps straight
into the O(1) tensor. Seven nanoseconds. Three times faster than OOP.
But this is your cold path API — tools, UI, one-off queries. Not your loop.

Level three: ActiveCapability. The Brain already ran at 5Hz. The winning
result is cached in the ECS component. The Muscle just calls operator().
No virtual, no lookup, no decision. 1.5 nanoseconds per entity.

That last number is important. 1.5 nanoseconds is below the cost of a
cache-warm ECS loop. We're not bottlenecked by logic anymore.
We're bottlenecked by RAM. That's exactly where we want to be."
[FR]
[SPEAKER SCRIPT]
"TODO"



SLIDE 16: REACHING THE SILICON LIMIT (Benchmarks)
[VISUAL]
Two charts side by side:

Left — bar chart with three groups (1%, 5%, 10% mutation rate), 1M entities:
  ECS bars in red, CRG bars in green.
  At 1%:  ECS ≈ CRG — gap negligible.
  At 10%: ECS 4.18ms vs CRG 3.18ms — gap grows with mutation rate.
  Label: "The higher the mutation rate, the more ECS pays."
  Footnote: "Windows/MSVC /O2. Apple M-Series (clang -O3) shows wider gap."

Right — "Per-Entity Cost at Scale":
  ECS line (dashed): spikes hard past L3 cache boundary (~1M entities).
  CRG line (solid green): flat.
  Annotation: "CRG Flatline — cost never moves."

[EN]
[SPEAKER SCRIPT]
"Two different questions answered by two charts.

The left chart: what does state transition cost at different mutation rates?
At 1%, both systems are essentially identical — the structural migration is
rare enough to not matter. You can see it right there on the chart.
At 10%, ECS is 30% slower. The gap grows with mutation rate.
This is on Windows with MSVC. On Apple M-Series with clang, the gap is
historically wider. The honest number: the advantage is real, it scales
with mutation rate, and the exact ratio depends on your hardware.

The right chart is the one I'd hang on a wall. It shows cost per entity
as your dataset grows past the L3 cache. The ECS line spikes.
The CRG line doesn't move. The data never migrated.
The prefetcher always knew where to look.

That flatness is the whole argument. CRG doesn't make your logic faster
in isolation — it makes your data immobile, and immobile data is
predictable data, and predictable data is what the hardware was built for.

Now — I want to be honest with you. If your game has few entities, or
your mutation rate is low — say 1% per frame — a classic ECS will match
or beat CRG. The left chart shows this clearly: at 1%, they're identical.

What CRG gives you is not raw speed. It's stability. Predictable
performance regardless of mutation rate. The cost per entity doesn't
spike when your world state changes. It doesn't spike when you add a
new behavior dimension. It stays flat.

If you're already happy with your ECS performance, CRG might not be
for you. If you're hitting throughput walls under high mutation rates,
or if your include graph is killing your build times — that's exactly
the problem CRG was built for."
[FR]
[SPEAKER SCRIPT]
"TODO"



SLIDE 17: CONCLUSION
[VISUAL]
Three lines appearing one by one, centered:

"Zero coupling.   →  Your modules discover each other. The linker does the work."
"Zero search.     →  Your context is a coordinate. The math does the work."
"Zero migration.  →  Your topology is immutable. The hardware does the work."

Below, after a pause:
"CRG is not a game engine framework.
 It's what happens when you stop fighting C++."

[EN]
[SPEAKER SCRIPT]
"Let me close with what CRG actually is — and what it isn't.

It isn't a game engine framework. It isn't an ECS replacement.
It isn't even specific to AAA production. The ECS symbiosis you saw
in Act III is the best example of application — but not the definition.

CRG is a linker-driven, zero-allocation capability dispatch framework
for any C++ system that needs decoupled module discovery and
high-performance behavior projection. Plugins, simulations, rule engines,
embedded systems — anywhere behaviors need to find each other without
knowing each other exist.

Three guarantees. Zero coupling: your modules self-register. The linker
resolves them. You never write an Init() function again. Zero search:
your context is a coordinate in a CapabilitySpace. Horner computes the
offset. You never walk a list in a hot path again. Zero migration: your
topology is immutable. The CapabilityTensor never moves. The prefetcher
never loses its stream.

We gave up none of the developer experience. The GPP still writes
one static and gets automatic discovery. The system still reads like OOP
in the cold path. And in the hot path, we reach the hardware limit.

Stop fighting C++. Stop fighting the hardware.
Give them what they were built for — and they'll give you everything back.

Thank you."
[FR]
[SPEAKER SCRIPT]
"TODO"



=============================================================================
Q&A PREP — ANTICIPATED QUESTIONS
=============================================================================

Q: "Isn't static inline ODR-unsafe across DLLs?"
A: Exactly right — that's precisely why RegistrySlot exists. In monolithic builds,
   static inline is perfectly safe: one binary, one definition, ODR guaranteed.
   The moment you cross a DLL boundary, each module gets its own copy of the static
   and your list fragments. CRG_BIND_SLOT solves this by forcing one explicit template
   specialization in the engine core, which the linker resolves as a single address
   across all modules.

Q: "Does CRG work with Live++?"
A: Yes — and it's friendly by design. Two cases:
   1. Logic changes (the common case — modifying what Execute/Move does):
      Live++ patches function bodies in place. The flat tensor never moves.
      The static objects are the same, their vtables just got patched.
      Works transparently, zero CRG involvement.
   2. Topology changes (adding/removing a capability or Baker):
      Hook Live++'s post-patch callback to call Bake(). Since NodeList is
      append-only, new statics register themselves on top of the existing
      list. Bake() rebuilds the flat tensor from the full list — including
      the new entries. The Baker's Register() already deduplicates by
      modelID, so no stale duplicates in the tensor.
      For pure logic Live++ patches: no Bake() needed at all.

Q: "Can you send DenseIDs over the network?"
A: No — DenseID is non-deterministic. It depends on the order types are touched
   at startup, which varies per process and per machine. Never serialize or send
   a DenseID. For networking, send the hash (typeid().hash_code() or a stable
   compile-time CRC). The receiver converts hash → DenseID on its own side.
   If you need a fully deterministic catalog across machines, register types
   against a stable key (string, enum, or codegen hash) instead of relying
   on typeid().

Q: "What about DLL hot-reload? The lazy StaticGuard fires only once."
A: In DLL mode you call Bake() manually at your controlled sync point after
   LoadLibrary(). The lazy init is the default for monolithic builds where all
   statics are wired by the OS before the first Find(). For plugins and hot-reload,
   the sync point is yours to own — which is intentional. You decide when the
   matrix rebuilds.

Q: "Why a manual sync point? Why not a dirty flag that triggers a re-bake automatically?"
A: Two reasons. First, in DLL mode the lazy StaticGuard has already fired before
   the new module's statics are wired — lazy re-bake simply can't work. Second,
   and more importantly: a dirty flag would itself be a thread-safety problem.
   Two threads could read the dirty flag simultaneously, trigger two concurrent
   Bakes, and corrupt the tensor. By forcing Bake() into a controlled sync point,
   we guarantee that tensor construction is always single-threaded and deterministic.
   The sync point is not a limitation — it's the contract.

Q: "Is the StaticGuard thread-safe? Two threads could call Find() simultaneously."
A: Yes — thread-safe by the C++11 standard. Static local initialization is
   guaranteed to execute exactly once, even under concurrent calls. The compiler
   generates an internal guard. No mutex needed, no race condition possible.

Q: "What is ActiveCapability exactly? How does operator() work without virtual?"
A: ActiveCapability is an ECS component that wraps a resolved capability result.
   It supports operator= to cache the result from CapabilityRouter::Find(), and
   operator() to invoke it directly. Internally it holds either a raw function
   pointer (DOD path) or a virtual interface pointer (OOP path) — a template
   specialization selects automatically based on whether your contract defines
   a Params type. The GPP never sees that distinction. Full implementation is
   in the GitHub repo, stage 12.

Q: "Can I use CRG without ECS?"
A: Absolutely — ECS is just the most compelling example. CRG works for any
   C++ system that needs decoupled module discovery: plugin systems, rule engines,
   tool pipelines, embedded systems. The ECS symbiosis in Act III is a consequence
   of CRG's properties — zero-allocation, linker-driven, immutable topology —
   not a design requirement. The framework is domain-agnostic.