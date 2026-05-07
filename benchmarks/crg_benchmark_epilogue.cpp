// Copyright (c) 2026 Cyril Tissier. All rights reserved.
// =============================================================================
// CRG BENCHMARK SUITE - FAST VERSION (No Warmup)
// =============================================================================

#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <memory>
#include <numeric>
#include <algorithm>
#include <random>
#include <cmath>
#include <cstring>

const double CPU_WATT_BASE = 35.0;       
const double MEMORY_STRESS_WATT = 25.0;  

struct Vector3 { float x, y, z; };
struct IPhysics { 
    Vector3 pos, vel, target; 
    float hp; int ammo, state; 
    char pad[464]; // 512 bytes total
};

class IOOPUnit {
public:
    virtual ~IOOPUnit() = default;
    virtual void Update(int frame, int id, int mut_threshold, int freq_mod) = 0;
};

struct PhysicsContract { struct Params { IPhysics* d; }; };
using PFN_Logic = void (*)(PhysicsContract::Params&);

struct IdleLogic   { static void Execute(PhysicsContract::Params& p) { p.d->vel.x *= 0.9f; } };
struct PatrolLogic { static void Execute(PhysicsContract::Params& p) { p.d->pos.x += p.d->vel.x; p.d->pos.y += p.d->vel.y; } };
struct CombatLogic { static void Execute(PhysicsContract::Params& p) { p.d->pos.x += p.d->vel.x * 2.0f; p.d->target.x = p.d->pos.x; } };
struct FleeLogic   { static void Execute(PhysicsContract::Params& p) { float d = std::sqrt(p.d->pos.x*p.d->pos.x + p.d->pos.y*p.d->pos.y); p.d->pos.x -= (p.d->vel.x*3.0f)/(d+0.1f); } };

inline int EvaluateBehaviorTree(const IPhysics& e) {
    float dsq = (e.target.x-e.pos.x)*(e.target.x-e.pos.x) + (e.target.y-e.pos.y)*(e.target.y-e.pos.y);
    if (e.hp < 25.0f) return 3; 
    if (dsq < 10000.0f) return (e.ammo > 0) ? 2 : 0; 
    return 1; 
}

inline PFN_Logic StateToPointer(int state) {
    switch(state) { case 0: return &IdleLogic::Execute; case 1: return &PatrolLogic::Execute; case 2: return &CombatLogic::Execute; case 3: return &FleeLogic::Execute; default: return &IdleLogic::Execute; }
}

class OOPDrone : public IOOPUnit {
    IPhysics m_Data;
public:
    OOPDrone(std::mt19937& rng, std::uniform_real_distribution<float>& dist) { 
        m_Data.hp = 100; m_Data.ammo = 30; m_Data.state = 1; m_Data.vel = {1,1,0};
        m_Data.pos = {dist(rng), dist(rng), 0}; m_Data.target = {dist(rng), dist(rng), 0};
    }
    void Update(int frame, int id, int mut_threshold, int freq_mod) override {
        if ((id + frame) % 100 < mut_threshold) { m_Data.hp = (m_Data.hp > 50.0f) ? 10.0f : 100.0f; }
        PhysicsContract::Params par{ &m_Data }; 
        StateToPointer(m_Data.state)(par);
        if ((id % freq_mod) == (frame % freq_mod)) { m_Data.state = EvaluateBehaviorTree(m_Data); }
    }
};

void RunCoreTest(size_t N, int mut_pct, int freq_mod, std::ofstream& out) {
    const int FRAMES = 300; // Faster but still statistically relevant
    std::mt19937 rng(42); std::uniform_real_distribution<float> dist(0.0f, 200.0f);
    
    std::vector<std::unique_ptr<IOOPUnit>> oop(N); 
    std::vector<IPhysics> data(N); 
    std::vector<PFN_Logic> crg(N);
    
    for(size_t i=0; i<N; ++i) {
        oop[i] = std::make_unique<OOPDrone>(rng, dist);
        data[i].hp = 100; data[i].ammo = 30; data[i].state = 1; data[i].vel = {1,1,0};
        data[i].pos = {dist(rng), dist(rng), 0}; data[i].target = {dist(rng), dist(rng), 0};
        crg[i] = &PatrolLogic::Execute;
    }

    auto bench = [&](const std::string& name, bool ecs_penalty, auto logic) {
        std::vector<double> samples;
        double pen_ms = 0, tot_ms = 0;
        for (int f=0; f<FRAMES; ++f) {
            auto t0 = std::chrono::high_resolution_clock::now();
            double p = logic(f);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            tot_ms += ms; pen_ms += (ecs_penalty ? p : 0);
            samples.push_back(ms);
        }
        
        std::sort(samples.begin(), samples.end());
        double stable_jitter = samples[samples.size()*0.99 - 1] - samples[samples.size()*0.01];
        double avg = tot_ms / FRAMES;
        double nrg = ((avg/1000.0)*CPU_WATT_BASE + (pen_ms/(1000.0*FRAMES))*MEMORY_STRESS_WATT) * 1000000.0;
        
        out << name << "," << (freq_mod==1?100:25) << "," << mut_pct << "," << N << "," << avg << "," << stable_jitter << "," << nrg << "\n";
    };

    bench("OOP", false, [&](int f) { for(size_t i=0; i<N; ++i) oop[i]->Update(f, i, mut_pct, freq_mod); return 0.0; });
    
    bench("ECS", true, [&](int f) {
        double p = 0;
        for(size_t i=0; i<N; ++i) {
            if ((i + f) % 100 < mut_pct) data[i].hp = (data[i].hp > 50.0f) ? 10.0f : 100.0f;
            PhysicsContract::Params par{&data[i]}; StateToPointer(data[i].state)(par);
            if ((i % freq_mod) == (f % freq_mod)) {
                int nxt = EvaluateBehaviorTree(data[i]);
                if (nxt != data[i].state) {
                    auto t0 = std::chrono::high_resolution_clock::now();
                    data[i].state = nxt; std::memcpy(&data[(i+N/2)%N], &data[i], sizeof(IPhysics));
                    p += std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
                }
            }
        } return p;
    });

    bench("CRG", false, [&](int f) {
        for(size_t i=0; i<N; ++i) {
            if ((i + f) % 100 < mut_pct) data[i].hp = (data[i].hp > 50.0f) ? 10.0f : 100.0f;
            PhysicsContract::Params par{&data[i]}; crg[i](par);
            if ((i % freq_mod) == (f % freq_mod)) {
                int nxt = EvaluateBehaviorTree(data[i]);
                if (nxt != data[i].state) { data[i].state = nxt; crg[i] = StateToPointer(nxt); }
            }
        } return 0.0;
    });
}

int main() {
    std::system("mkdir -p data");
    
    std::cout << "Starting Suite A (Time-Slicing)..." << std::endl;
    std::ofstream outA("data/crg_bench_timeslice.csv");
    outA << "Paradigm,FreqPct,MutPct,N,Avg_ms,Jit_ms,Nrg_uJ\n";
    for(size_t n : {100000, 500000, 1000000}) { RunCoreTest(n, 1, 4, outA); RunCoreTest(n, 1, 1, outA); }
    outA.close();

    std::cout << "Starting Suite B (Mutation Stress)..." << std::endl;
    std::ofstream outB("data/crg_bench_mutation.csv");
    outB << "Paradigm,FreqPct,MutPct,N,Avg_ms,Jit_ms,Nrg_uJ\n";
    for(int m : {1, 5, 10}) { for(size_t n : {100000, 500000, 1000000}) { RunCoreTest(n, m, 1, outB); } }
    outB.close();

    std::cout << "Done." << std::endl;
    return 0;
}