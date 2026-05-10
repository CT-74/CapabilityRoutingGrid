// Copyright (c) 2026 Cyril Tissier. All rights reserved.
// =============================================================================
// CAPABILITY ROUTING GATEWAY (CRG) - THE ULTIMATE BENCHMARK EDITION
// =============================================================================

#include "raylib.h"
#include "raymath.h"
#include "imgui.h"
#include "rlImGui.h"
#include "json.hpp"         
#include <entt/entt.hpp>
#include <vector>
#include <tuple>
#include <typeinfo>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <chrono>
#include <type_traits>
#include <utility>
#include <atomic>           
#include <array>
#include <memory>
#include <cstring> 
#include <cfloat> 

const double CPU_WATT_ACTIVE = 35.0; 
const double CPU_WATT_IDLE   = 2.0;  

struct ProfileResult { 
    double physics_ms = 0; 
    double ai_ms = 0; 
    double struct_ms = 0; 
    double energy_microjoules = 0; 
};

enum class EngineMode { OOP, ECS, CRG };
using json = nlohmann::json;

// =============================================================================
// [ ENGINE CORE ] INFRASTRUCTURE & TENSOR ROUTER
// =============================================================================
template<class T> struct UniversalAnchor { static T& Get() { static T s_Value{}; return s_Value; } };
using ModelTypeID = std::size_t; 
template<class T> struct TypeIDOf { static ModelTypeID Get() { return typeid(T).hash_code(); } };
struct DenseModelID { std::size_t index; explicit DenseModelID(std::size_t i) : index(i) {} static constexpr std::size_t Invalid = static_cast<std::size_t>(-1); bool IsValid() const { return index != Invalid; } operator std::size_t() const { return index; } };
using ModelMap = std::unordered_map<ModelTypeID, std::size_t>; 
struct ModelHandle { DenseModelID denseID; explicit ModelHandle(ModelTypeID hash); template<class T> static ModelHandle FromType() { return ModelHandle(TypeIDOf<T>::Get()); } };
template<class TNode> using NodeListAnchor = UniversalAnchor<const TNode*>; 
template<class TNode, class TInterface> struct NodeList : public TInterface { const TNode* m_Next = nullptr; NodeList() { m_Next = NodeListAnchor<TNode>::Get(); NodeListAnchor<TNode>::Get() = static_cast<const TNode*>(this); } };
template<typename T> struct EnumTraits; struct GlobalState { constexpr operator std::size_t() const { return 0; } }; template<> struct EnumTraits<GlobalState> { static constexpr std::size_t Count = 1; };
template<class... TAxes> struct CapabilitySpace {
    using AxisTuple = std::tuple<TAxes...>; static constexpr std::size_t Dimensions = sizeof...(TAxes); static constexpr std::size_t Volume = (Dimensions == 0) ? 1 : (EnumTraits<TAxes>::Count * ... * 1);
    template<std::size_t DimIdx> static constexpr std::size_t GetStride() { if constexpr (Dimensions == 0) return 1; else { constexpr std::size_t dims[] = { EnumTraits<TAxes>::Count... }; std::size_t stride = 1; for (std::size_t i = DimIdx + 1; i < Dimensions; ++i) stride *= dims[i]; return stride; } }
    template<std::size_t DimIdx> static constexpr auto GetCoordAtIndex(std::size_t index) { if constexpr (Dimensions == 0) return 0; else { using AxisT = std::tuple_element_t<DimIdx, AxisTuple>; return static_cast<AxisT>((index / CapabilitySpace<TAxes...>::template GetStride<DimIdx>()) % EnumTraits<AxisT>::Count); } }
    template<typename... TArgs> static constexpr std::size_t ComputeOffset(const TArgs&... args) { if constexpr (Dimensions == 0) return 0; else return ComputeInternal(std::tie(args...), std::make_index_sequence<Dimensions>{}); }
private:
    template<typename TupleT, std::size_t... Is> static constexpr std::size_t ComputeInternal(const TupleT& t, std::index_sequence<Is...>) { const std::size_t coords[] = { static_cast<std::size_t>(std::get<const std::tuple_element_t<Is, AxisTuple>&>(t))... }; constexpr std::size_t dims[] = { EnumTraits<std::tuple_element_t<Is, AxisTuple>>::Count... }; std::size_t offset = 0; for (std::size_t i = 0; i < Dimensions; ++i) offset = offset * dims[i] + coords[i]; return offset; }
};
template<class TContract> struct CapabilityRoutingTraits { using SpaceType = CapabilitySpace<GlobalState>; }; template<auto... Values> struct At {};
template<class TSpace, std::size_t Index, class IdxSeq> struct MakeAt; template<class TSpace, std::size_t Index, std::size_t... DimIs> struct MakeAt<TSpace, Index, std::index_sequence<DimIs...>> { using Type = At<TSpace::template GetCoordAtIndex<DimIs>(Index)...>; };
template<class TSpace, std::size_t Index> using MakeAt_t = typename MakeAt<TSpace, Index, std::make_index_sequence<TSpace::Dimensions>>::Type;
struct NullContext { NullContext() = default; template<typename... Args> NullContext(const Args&...) {} }; template<typename T, typename = void> struct ContextSelector { using Type = NullContext; }; template<typename T> struct ContextSelector<T, std::void_t<typename CapabilityRoutingTraits<T>::RuleContext>> { using Type = typename CapabilityRoutingTraits<T>::RuleContext; }; template<typename T> using ContextTypeOf = typename ContextSelector<T>::Type;
template<typename T, typename = void> struct IsDODContract : std::false_type {}; template<typename T> struct IsDODContract<T, std::void_t<typename T::Params>> : std::bool_constant<!std::is_polymorphic_v<T>> {};
template<class TContract> struct DODDescriptor { void (*pfnExecute)(typename TContract::Params&); const char* debugName = "Unknown"; };
template<class TContract> struct Rule { using ContextT = ContextTypeOf<TContract>; using PredicatePtr = bool (*)(const void*, const ContextT&); DODDescriptor<TContract> descriptor; const void* configData; PredicatePtr predicate; int priority; bool Matches(const ContextT& ctx) const { return !predicate || predicate(configData, ctx); } };
template<class TContract> struct DispatchCell { std::vector<Rule<TContract>> dynamicRules; DODDescriptor<TContract> fallback; bool hasFallback = false; }; template<class TContract> using TensorArena = UniversalAnchor<std::vector<DispatchCell<TContract>>>; 
template<class TContract, class TConfig = void> struct Capability { using InterfaceType = TContract; using ConfigType = TConfig; TConfig config; }; template<class TContract> struct Capability<TContract, void> { using InterfaceType = TContract; using ConfigType = void; };
template<typename T, typename = void> struct HasConfigType : std::false_type {}; template<typename T> struct HasConfigType<T, std::void_t<typename T::ConfigType>> { static constexpr bool value = !std::is_same_v<typename T::ConfigType, void>; };
struct IAssembler { virtual void Bake() const = 0; }; struct IBindingNode : public NodeList<IBindingNode, IAssembler> {}; UniversalAnchor<const IBindingNode*> g_BindingAnchor;
template<class TModel, template<class, class> class Cap, class TIdxSeq> struct CapabilityNode; template<class TModel, template<class, class> class Cap, std::size_t... Is> struct CapabilityNode<TModel, Cap, std::index_sequence<Is...>> : public Cap<TModel, MakeAt_t<typename CapabilityRoutingTraits<typename Cap<TModel, At<>>::InterfaceType>::SpaceType, Is>>... { using ContractT = typename Cap<TModel, At<>>::InterfaceType; using TSpace = typename CapabilityRoutingTraits<ContractT>::SpaceType; using ContextT = ContextTypeOf<ContractT>; void FillArena(DenseModelID denseID) const { auto& arena = TensorArena<ContractT>::Get(); std::size_t baseIdx = denseID.index * TSpace::Volume; if (arena.size() < baseIdx + TSpace::Volume) arena.resize(baseIdx + TSpace::Volume); ([&]() { using Impl = Cap<TModel, MakeAt_t<TSpace, Is>>; auto& cell = arena[baseIdx + Is]; DODDescriptor<ContractT> desc { &Impl::Execute, typeid(Impl).name() }; if constexpr (HasConfigType<Impl>::value) { auto trampoline = [](const void* obj, const ContextT& ctx) -> bool { return static_cast<const Impl*>(obj)->config.Condition(ctx); }; cell.dynamicRules.push_back({ desc, &static_cast<const Impl*>(this)->config, trampoline, static_cast<const Impl*>(this)->config.priority }); std::sort(cell.dynamicRules.begin(), cell.dynamicRules.end(), [](auto& a, auto& b){ return a.priority > b.priority; }); } else { cell.fallback = desc; cell.hasFallback = true; } }(), ...); } };
template<class TModel, template<class, class> class... TCapabilities> struct CapabilityBinding : public IBindingNode { struct Unit : public CapabilityNode<TModel, TCapabilities, std::make_index_sequence<CapabilityRoutingTraits<typename TCapabilities<TModel, At<>>::InterfaceType>::SpaceType::Volume>>... { void Fill(DenseModelID slot) const { (CapabilityNode<TModel, TCapabilities, std::make_index_sequence<CapabilityRoutingTraits<typename TCapabilities<TModel, At<>>::InterfaceType>::SpaceType::Volume>>::FillArena(slot), ...); } } m_unit{}; void Bake() const override { ModelTypeID hash = TypeIDOf<TModel>::Get(); auto& map = UniversalAnchor<ModelMap>::Get(); if (map.find(hash) == map.end()) map[hash] = map.size(); m_unit.Fill(DenseModelID(map[hash])); } };
class CapabilityRouter { public: static void EnsureBaked() { static struct StaticGuard { StaticGuard() { for (const IBindingNode* b = UniversalAnchor<const IBindingNode*>::Get(); b; b = b->m_Next) b->Bake(); } } s_guard; } template<class TContract, class... Coords> static const DODDescriptor<TContract>* Find(ModelHandle handle, const ContextTypeOf<TContract>& ctx, Coords... coords) { EnsureBaked(); if (!handle.denseID.IsValid()) return nullptr; const auto& arena = TensorArena<TContract>::Get(); using TSpace = typename CapabilityRoutingTraits<TContract>::SpaceType; std::size_t idx = (handle.denseID.index * TSpace::Volume) + TSpace::ComputeOffset(coords...); if (idx >= arena.size()) return nullptr; const auto& cell = arena[idx]; for (const auto& rule : cell.dynamicRules) if (rule.Matches(ctx)) return &rule.descriptor; return cell.hasFallback ? &cell.fallback : nullptr; } };
template<class T, bool IsDOD = IsDODContract<T>::value> struct ActiveCapability; template<class T> struct ActiveCapability<T, true> { const DODDescriptor<T>* resolved = nullptr; std::size_t last_offset = 0; inline ActiveCapability& operator=(const DODDescriptor<T>* desc) { resolved = desc; return *this; } inline void operator()(typename T::Params& p) const { if (resolved && resolved->pfnExecute) resolved->pfnExecute(p); } inline explicit operator bool() const { return resolved != nullptr; } };
inline ModelHandle::ModelHandle(ModelTypeID hash) : denseID(DenseModelID::Invalid) { CapabilityRouter::EnsureBaked(); const auto& map = UniversalAnchor<ModelMap>::Get(); auto it = map.find(hash); if (it != map.end()) denseID = DenseModelID(it->second); }

// =============================================================================
// [ GAMEPLAY ] CUSTOM LIGHTWEIGHT PHYSICS & DATA
// =============================================================================
struct PhysicsTransform { Vector2 pos; Vector2 vel; float max_speed = 300.0f; float mass = 1.0f; };
struct BehaviorSettings { float perception_melee = 100.0f; float perception_ranged = 300.0f; };
struct Weapon { float cooldown; float fire_rate; };
struct Health { float current = 100.0f; bool is_dead = false; }; 
struct Renderable { Color color; float size; };
struct Projectile { float lifespan = 1.5f; };
struct AggressiveTag {}; 

enum class CombatState { Idle, Aggressive }; template<> struct EnumTraits<CombatState> { static constexpr std::size_t Count = 2; };
enum class PerceptionRange { Melee, Ranged, Safe }; template<> struct EnumTraits<PerceptionRange> { static constexpr std::size_t Count = 3; };
enum class GroupStrategy { Formation, Scatter }; template<> struct EnumTraits<GroupStrategy> { static constexpr std::size_t Count = 2; };

struct CRGIdentity { CombatState current_state = CombatState::Idle; PerceptionRange current_perception = PerceptionRange::Safe; GroupStrategy current_strategy = GroupStrategy::Formation; ModelHandle handle = ModelHandle(TypeIDOf<void>::Get()); };

struct CommandBuffer {
    enum Type { SpawnProj, DestroyEnt }; struct Cmd { Type type; entt::entity target; Vector2 p; Vector2 v; };
    std::vector<Cmd> queue; 
    void PushSpawnProjectile(Vector2 pos, Vector2 vel) { queue.push_back({SpawnProj, entt::null, pos, vel}); }
    void PushDestroy(entt::entity e) { queue.push_back({DestroyEnt, e, {0,0}, {0,0}}); }
    void Flush(entt::registry& reg) {
        for(auto& c : queue) {
            if(c.type == SpawnProj) { 
                auto proj = reg.create(); reg.emplace<PhysicsTransform>(proj, PhysicsTransform{c.p, c.v, 800.0f, 0.1f}); reg.emplace<Projectile>(proj, 1.5f); reg.emplace<Renderable>(proj, YELLOW, 2.0f);
            } else if (c.type == DestroyEnt && reg.valid(c.target)) { reg.destroy(c.target); }
        } queue.clear();
    }
};

struct UnitAIContract {
    struct Params { CommandBuffer& cmd; entt::entity e; PhysicsTransform& t; Weapon& w; Renderable& r; float dt; Vector2 leaderPos; bool has_leader; };
    struct RuleContext { const BehaviorSettings& settings; }; 
};
template<> struct CapabilityRoutingTraits<UnitAIContract> { using SpaceType = CapabilitySpace<CombatState, PerceptionRange, GroupStrategy>; using RuleContext = UnitAIContract::RuleContext; };

// =============================================================================
// [ THE BOTTLENECK ] L'IA LOURDE & FLOCKING
// =============================================================================
inline float SimulateHeavySensors(Vector2 pos, Vector2 leaderPos) {
    float tactical_weight = 0.0f;
    for (int i = 1; i <= 25; ++i) { tactical_weight += sinf(pos.x * i * 0.01f) * cosf(leaderPos.y * i * 0.01f); }
    return tactical_weight;
}

inline void ApplySteeringForce(PhysicsTransform& t, Vector2 center_pos, float dt, entt::entity e, bool has_leader) {
    uint32_t id = static_cast<uint32_t>(e);
    Vector2 desired_vel = {0, 0};

    if (has_leader) {
        // SPIRALE DE FIBONACCI (Nuage Organique O(1))
        float angle = id * 2.39996f; 
        float radius = sqrtf(id % 2000) * 5.0f; 
        Vector2 personal_target = { center_pos.x + cosf(angle) * radius, center_pos.y + sinf(angle) * radius };
        desired_vel = Vector2Subtract(personal_target, t.pos);
        float dist = Vector2Length(desired_vel);
        if (dist > 0) {
            float speed = (dist < 50.0f) ? (t.max_speed * (dist / 50.0f)) : t.max_speed; 
            desired_vel = Vector2Scale(desired_vel, speed / dist);
        }
    } else {
        // WANDER CHAOTIQUE (S'éparpillent sans leader)
        float time_sec = GetTime();
        float noise_angle = id * 0.1f + sinf(time_sec * 0.5f + id * 0.01f) * PI * 2.0f;
        desired_vel = { cosf(noise_angle) * t.max_speed * 0.4f, sinf(noise_angle) * t.max_speed * 0.4f };
    }
    Vector2 steering = Vector2Subtract(desired_vel, t.vel);
    t.vel = Vector2Add(t.vel, Vector2Scale(steering, 3.0f * dt));
}

template<class T, class TAt = At<>> struct DroneLogic : Capability<UnitAIContract> {
    static void Execute(UnitAIContract::Params& p) { 
        p.r.color = SKYBLUE; 
        ApplySteeringForce(p.t, p.leaderPos, p.dt, p.e, p.has_leader);
        if (p.w.cooldown > 0) p.w.cooldown -= p.dt; 
    }
};

template<class T, auto... R> 
struct DroneLogic<T, At<CombatState::Aggressive, R...>> : Capability<UnitAIContract> {
    static void Execute(UnitAIContract::Params& p) { 
        p.r.color = WHITE; 
        if (p.w.cooldown <= 0) { 
            p.w.cooldown = p.w.fire_rate; 
            Vector2 dir = p.has_leader ? Vector2Subtract(p.t.pos, p.leaderPos) : Vector2{ cosf((float)static_cast<uint32_t>(p.e)), sinf((float)static_cast<uint32_t>(p.e)) };
            if (Vector2LengthSqr(dir) > 0) dir = Vector2Normalize(dir); else dir = {1.0f, 0.0f};
            p.cmd.PushSpawnProjectile(p.t.pos, Vector2Scale(dir, 800.0f)); 
        }
        if (p.w.cooldown > 0) p.w.cooldown -= p.dt; 
        ApplySteeringForce(p.t, p.leaderPos, p.dt, p.e, p.has_leader); 
    }
};

struct Drone {}; static const CapabilityBinding<Drone, DroneLogic> g_DroneBinding{};

// =============================================================================
// [ ENGINE CORE ] OOP INTEGRATION (VRAI POINTER CHASING)
// =============================================================================
class IOOPUnit { public: virtual ~IOOPUnit() = default; virtual float GetHeapPenalty() = 0; virtual void ExecuteLogic(UnitAIContract::Params& p, CombatState state) = 0; };
struct OOPComponent { std::shared_ptr<IOOPUnit> ptr; CombatState state = CombatState::Idle; };

class OOPDrone : public IOOPUnit {
    char payload[1024]; // Payload 1KB sur le Heap
public:
    OOPDrone() { payload[GetRandomValue(0, 1023)] = 1; }
    float GetHeapPenalty() override { return (float)payload[128] * 0.00001f; }
    void ExecuteLogic(UnitAIContract::Params& p, CombatState state) override {
        if (state == CombatState::Aggressive) DroneLogic<Drone, At<CombatState::Aggressive, PerceptionRange::Safe, GroupStrategy::Formation>>::Execute(p);
        else DroneLogic<Drone>::Execute(p);
    }
};

// =============================================================================
// [ GAMEPLAY ] BATTLEFIELD ENGINE
// =============================================================================
class BattlefieldEngine {
    entt::registry reg;
    CommandBuffer cmdBuffer;
    float mutation_rate = 5.0f;
    size_t frame_count = 0; 
    GroupStrategy global_strategy = GroupStrategy::Formation;
    Vector2 leaderPos = {640.0f, 360.0f};
    Vector2 leaderTarget = {640.0f, 360.0f};
    std::vector<float> history_ai, history_mut, history_phys, history_energy;
    std::vector<char> archetype_ram; 

public:
    void Init() { archetype_ram.resize(1024 * 1024 * 64); AddWave(25000); } 
    void AddWave(size_t count) {
        for(size_t i = 0; i < count; i++) {
            auto e = reg.create();
            PhysicsTransform t { {(float)GetRandomValue(50, 1230), (float)GetRandomValue(50, 670)}, {(float)GetRandomValue(-150, 150), (float)GetRandomValue(-150, 150)}, 300.0f, 1.0f };
            reg.emplace<PhysicsTransform>(e, t); reg.emplace<Weapon>(e, 0.0f, 0.5f); reg.emplace<Health>(e, 100.0f); reg.emplace<Renderable>(e, SKYBLUE, 3.0f); 
            reg.emplace<CRGIdentity>(e, CombatState::Idle, PerceptionRange::Safe, global_strategy, ModelHandle::FromType<Drone>());
            reg.emplace<ActiveCapability<UnitAIContract>>(e); reg.emplace<OOPComponent>(e, std::make_shared<OOPDrone>(), CombatState::Idle);
        }
    }
    void RemoveWave(size_t count) {
        auto view = reg.view<CRGIdentity>(); size_t removed = 0;
        for (auto entity : view) { if (removed >= count) break; cmdBuffer.PushDestroy(entity); removed++; }
    }

    ProfileResult Update(float dt, EngineMode mode, bool immortal, bool auto_leader) {
        ProfileResult p; frame_count++;
        bool has_leader = false;

        if (auto_leader) {
            has_leader = true; float dist = Vector2Distance(leaderPos, leaderTarget);
            if (dist < 20.0f) leaderTarget = { (float)GetRandomValue(150, 1130), (float)GetRandomValue(150, 570) };
            if (dist > 0) leaderPos = Vector2Add(leaderPos, Vector2Scale(Vector2Normalize(Vector2Subtract(leaderTarget, leaderPos)), 250.0f * dt));
        } else { has_leader = IsCursorOnScreen(); if (has_leader) leaderPos = GetMousePosition(); }

        auto t_phys = std::chrono::high_resolution_clock::now();
        reg.view<PhysicsTransform>().each([&](auto entity, auto& t) {
            t.pos.x += t.vel.x * dt; t.pos.y += t.vel.y * dt;
            if (t.pos.x < 10.0f) { t.pos.x = 10.0f; t.vel.x = std::abs(t.vel.x); }
            if (t.pos.x > 1270.0f) { t.pos.x = 1270.0f; t.vel.x = -std::abs(t.vel.x); }
            if (t.pos.y < 10.0f) { t.pos.y = 10.0f; t.vel.y = std::abs(t.vel.y); }
            if (t.pos.y > 710.0f) { t.pos.y = 710.0f; t.vel.y = -std::abs(t.vel.y); }
        });
        reg.view<Projectile>().each([&](auto entity, auto& proj) { proj.lifespan -= dt; if (proj.lifespan <= 0) cmdBuffer.PushDestroy(entity); });
        p.physics_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t_phys).count();

        auto t_mut = std::chrono::high_resolution_clock::now();
        int mut_threshold = (int)(mutation_rate * 100.0f);
        reg.view<CRGIdentity, OOPComponent, PhysicsTransform, Weapon>().each([&](auto entity, auto& id, auto& oop, auto& t, auto& w) {
            if (GetRandomValue(0, 10000) < mut_threshold) {
                CombatState next = (id.current_state == CombatState::Idle) ? CombatState::Aggressive : CombatState::Idle;
                id.current_state = next; oop.state = next; 
                if (mode == EngineMode::ECS) { 
                    if (next == CombatState::Aggressive) reg.emplace_or_replace<AggressiveTag>(entity); else reg.remove<AggressiveTag>(entity);
                    size_t chunk_offset = GetRandomValue(0, (int)archetype_ram.size() - 1024);
                    std::memcpy(&archetype_ram[chunk_offset], &t, sizeof(PhysicsTransform));
                    std::memset(&archetype_ram[chunk_offset + sizeof(PhysicsTransform)], 0xAA, 1024 - sizeof(PhysicsTransform));
                }
            }
        });
        p.struct_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t_mut).count();

        auto t_ai = std::chrono::high_resolution_clock::now();
        if (mode == EngineMode::OOP) {
            reg.view<OOPComponent, PhysicsTransform, Weapon, Renderable>().each([&](auto entity, auto& oop, auto& t, auto& w, auto& r) {
                t.vel.x += oop.ptr->GetHeapPenalty(); // Pointer Chasing
                float dummy = SimulateHeavySensors(t.pos, leaderPos); t.vel.x += dummy * 0.001f; 
                UnitAIContract::Params params { cmdBuffer, entity, t, w, r, dt, leaderPos, has_leader };
                oop.ptr->ExecuteLogic(params, oop.state);
            });
        }
        else if (mode == EngineMode::ECS) {
            reg.view<AggressiveTag, PhysicsTransform, Weapon, Renderable>().each([&](auto entity, auto& t, auto& w, auto& r) {
                float dummy = SimulateHeavySensors(t.pos, leaderPos); t.vel.x += dummy * 0.001f;
                UnitAIContract::Params params { cmdBuffer, entity, t, w, r, dt, leaderPos, has_leader };
                DroneLogic<Drone, At<CombatState::Aggressive, PerceptionRange::Safe, GroupStrategy::Formation>>::Execute(params);
            });
            reg.view<PhysicsTransform, Weapon, Renderable>(entt::exclude<AggressiveTag>).each([&](auto entity, auto& t, auto& w, auto& r) {
                float dummy = SimulateHeavySensors(t.pos, leaderPos); t.vel.x += dummy * 0.001f;
                UnitAIContract::Params params { cmdBuffer, entity, t, w, r, dt, leaderPos, has_leader };
                DroneLogic<Drone>::Execute(params);
            });
        }
        else if (mode == EngineMode::CRG) {
            auto route_view = reg.view<CRGIdentity, PhysicsTransform, ActiveCapability<UnitAIContract>>();
            route_view.each([&](auto entity, auto& id, auto& t, auto& cap) {
                if ((static_cast<uint32_t>(entity) % 6) == (frame_count % 6)) {
                    float dummy = SimulateHeavySensors(t.pos, leaderPos); t.vel.x += dummy * 0.001f;
                    float d2 = Vector2DistanceSqr(t.pos, leaderPos);
                    id.current_perception = (d2 < 10000.0f) ? PerceptionRange::Melee : (d2 < 90000.0f) ? PerceptionRange::Ranged : PerceptionRange::Safe;
                    cap = CapabilityRouter::Find<UnitAIContract>(id.handle, {BehaviorSettings{}}, id.current_state, id.current_perception, id.current_strategy);
                    cap.last_offset = CapabilityRoutingTraits<UnitAIContract>::SpaceType::ComputeOffset(id.current_state, id.current_perception, id.current_strategy);
                }
            });
            reg.view<ActiveCapability<UnitAIContract>, PhysicsTransform, Weapon, Renderable>().each([&](auto entity, auto& cap, auto& t, auto& w, auto& r) { 
                UnitAIContract::Params params { cmdBuffer, entity, t, w, r, dt, leaderPos, has_leader }; cap(params); 
            });
        }
        p.ai_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t_ai).count();
        p.energy_microjoules = (p.ai_ms / 1000.0) * CPU_WATT_ACTIVE * 1000000.0;
        reg.view<Health>().each([&](auto entity, auto& h) { h.current -= 10.0f * dt; if (h.current <= 0) { if (immortal) h.current = 100.0f; else cmdBuffer.PushDestroy(entity); } }); 
        cmdBuffer.Flush(reg);
        history_phys.push_back((float)p.physics_ms); if(history_phys.size() > 100) history_phys.erase(history_phys.begin());
        history_ai.push_back((float)p.ai_ms); if(history_ai.size() > 100) history_ai.erase(history_ai.begin());
        history_mut.push_back((float)p.struct_ms); if(history_mut.size() > 100) history_mut.erase(history_mut.begin());
        history_energy.push_back((float)p.energy_microjoules); if(history_energy.size() > 100) history_energy.erase(history_energy.begin());
        return p;
    }

    void Render(ProfileResult p, EngineMode& mode, bool& immortal, bool& auto_leader) {
        BeginDrawing(); ClearBackground(Color{10, 10, 15, 255});
        reg.view<PhysicsTransform, Renderable, Health>().each([&](auto entity, const auto& t, const auto& r, const auto& h) {
            Color c = r.color; if (h.current < 40.0f) c.a = 80; DrawRectangle((int)t.pos.x, (int)t.pos.y, 3, 3, c);
        });
        if (auto_leader || IsCursorOnScreen()) { DrawCircleLines(leaderPos.x, leaderPos.y, 100, Fade(RED, 0.5f)); DrawCircleLines(leaderPos.x, leaderPos.y, 300, Fade(YELLOW, 0.5f)); }
        if (auto_leader) { DrawLine(leaderPos.x - 10, leaderPos.y, leaderPos.x + 10, leaderPos.y, Fade(WHITE, 0.3f)); DrawLine(leaderPos.x, leaderPos.y - 10, leaderPos.x, leaderPos.y + 10, Fade(WHITE, 0.3f)); DrawCircleV(leaderTarget, 4, RED); }
        rlImGuiBegin();
        ImGui::Begin("CRG Command Center"); ImGui::Separator(); 
        int cidx = (int)mode; if (ImGui::Combo("Engine Paradigm", &cidx, "OOP\0ECS\0CRG\0\0")) mode = (EngineMode)cidx;
        ImGui::Checkbox("Immortal Swarm", &immortal); ImGui::SameLine(); ImGui::Checkbox("Auto Roaming", &auto_leader); 
        ImGui::SliderFloat("Mutation Rate", &mutation_rate, 0, 100); 
        if (ImGui::Button("+ 10k Drones")) AddWave(10000); ImGui::SameLine(); if (ImGui::Button("- 10k Drones")) RemoveWave(10000); ImGui::SameLine(); if (ImGui::Button("Clear All")) RemoveWave(reg.view<CRGIdentity>().size());
        ImGui::End();
        ImGui::Begin("Telemetry & Energy Bench");
        int fps = GetFPS(); ImVec4 fclr = (fps >= 60) ? ImVec4(0,1,0,1) : (fps >= 45) ? ImVec4(1,0.6f,0,1) : ImVec4(1,0.2f,0.2f,1);
        ImGui::TextColored(fclr, "FPS: %d", fps); ImGui::SameLine(); ImGui::TextColored(ImVec4(1, 1, 0, 1), "  |  Entities: %zu", reg.view<CRGIdentity>().size()); ImGui::Separator();
        char buf[64]; snprintf(buf, sizeof(buf), "%.4f ms", p.physics_ms); ImGui::Text("Phys (Custom DOD) : %s", buf); if (!history_phys.empty()) ImGui::PlotLines("##p", history_phys.data(), (int)history_phys.size(), 0, buf, 0, FLT_MAX, ImVec2(-1, 40));
        snprintf(buf, sizeof(buf), "%.4f ms", p.struct_ms); ImGui::Text("Churn (Mutation 1KB) : %s", buf); if (!history_mut.empty()) ImGui::PlotLines("##m", history_mut.data(), (int)history_mut.size(), 0, buf, 0, FLT_MAX, ImVec2(-1, 40));
        snprintf(buf, sizeof(buf), "%.4f ms", p.ai_ms); ImGui::TextColored(ImVec4(0, 1, 0, 1), "AI Logic : %s", buf); if (!history_ai.empty()) ImGui::PlotLines("##a", history_ai.data(), (int)history_ai.size(), 0, buf, 0, FLT_MAX, ImVec2(-1, 40));
        ImGui::Separator(); ImGui::Text("IMPACT ECOLOGIQUE (MicroJoules/Frame)"); if (!history_energy.empty()) ImGui::PlotLines("##e", history_energy.data(), (int)history_energy.size(), 0, nullptr, 0, FLT_MAX, ImVec2(-1, 60));
        ImGui::End(); rlImGuiEnd(); EndDrawing();
    }
};

int main() { 
    InitWindow(1280, 720, "CRG BATTLEFIELD - ULTIMATE BENCHMARK"); rlImGuiSetup(true); 
    BattlefieldEngine engine; engine.Init(); EngineMode mode = EngineMode::CRG; bool immortal = true, auto_leader = true; 
    while (!WindowShouldClose()) { engine.Render(engine.Update(GetFrameTime(), mode, immortal, auto_leader), mode, immortal, auto_leader); }
    rlImGuiShutdown(); CloseWindow(); return 0; 
}