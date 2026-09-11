#pragma once

#include <vector>

#include "GameConstants.h"
#include "Vector/Vectors.h"
#include "game/Ball.h"

// =====================================================================
// HumanScan — Human-style Auto Play AI (8-Ball, Direct Shots Only)
//
// Completely separate from the wild scanner (AutoPlay::ScanWild). Pipeline:
//   AnalyzeTable -> ChooseGroup -> CreateCandidates -> EasyFilter
//   -> RankCandidates -> SimulateCandidate (Phase A: pot pool,
//   Phase R: multi-shot run-out planner to the 8-ball, Phase B:
//   spin/power refinement for position play)
//   -> CommitShot -> ExecuteShot
// (BreakPhase handles the opening break separately.)
//
// Performance contract (v1.8):
//   * Normal turn: bounded candidate validation plus a bounded run/position
//     search. The entry point is called once per settled turn, not per frame.
//   * Break:       up to 492 bounded simulations (41 angles x 3 spins x 4
//     power levels), with scratch rejection before committing the shot.
// =====================================================================

namespace HumanScan {

    struct BallEntry {
        int index;
        Point2D pos;
        Ball::Classification cls;
    };

    // Live snapshot of the table, read once per decision cycle.
    struct TableInfo {
        bool isBreak = false;
        bool cueOnTable = false;
        int  solidsLeft = 0;
        int  stripesLeft = 0;
        int  solidsPotted = 0;
        int  stripesPotted = 0;
        bool only8BallLeft = false;
        bool onLastGroupBall = false;
        Ball::Classification gameGroup = Ball::Classification::ANY;
        Ball::Classification myGroup = Ball::Classification::ANY;
        int  nominatedPocket = 6; // < 6 => a pocket is nominated
        int  nominationMode = 0;   // 0=none, 1=8ball, 2=all
        bool eightBallMustBank = false;
        Point2D cuePos;
        Point2D pockets[TABLE_POCKETS_COUNT];
        std::vector<BallEntry> allBalls; // cached once per decision cycle
    };

    // A single geometric candidate (no simulation performed yet).
    struct HumanCandidate {
        int   idx = -1;
        int   pocket = -1;
        double angle = 0.0;
        double score = 1e18;      // geometric rank score (lower = easier)
        double dist = 0.0;        // target ball -> pocket distance
        double power = 0.0;
        double cutDot = 1.0;      // 1.0 = perfectly straight
        double cueDist = 0.0;     // cue -> ghost ball distance
        Point2D ballPos;
        Point2D ghost;
        Point2D predictedCueRest;
        Vec2d  spin;              // raw english (mEnglish) chosen for this shot
        int    spinPreset = -1;   // AutoPlay::SpinPreset (or -1 = don't touch)
        const char* spinTag = "-";// short label for logs
        double shapeScore = 1e18; // ease of the NEXT shot from the cue resting spot (lower = better; 0 = none needed)
        int    followTarget = -1; // ball planned as the next shot (index)
        bool   simulated = false;
        bool   valid = false;
        bool   potted = false;
        bool   hasFollowUp = false;
        double finalScore = 1e18;
    };

    struct CommittedShot {
        bool   locked = false;
        int    idx = -1;
        int    pocket = -1;
        double angle = 0.0;
        double power = 0.0;
        Vec2d  spin;
        int    spinPreset = -1;
        const char* spinTag = "-";
        int    followTarget = -1;
        double shapeScore = 1e18;
        bool   hasFollowUp = false;
    };

    enum class SimStatus { INVALID, SAFE, POTTED };

    struct Context {
        TableInfo table;
        std::vector<HumanCandidate> candidates;
        HumanCandidate best;
        HumanCandidate safeShot;
        CommittedShot shot;
        bool   turnAnalyzed = false;
        bool   awaitingSettlement = false;
        bool   breakDone = false;
        bool   breakShotActive = false;
        bool   postBreakShot = false;
        std::vector<BallEntry> breakBeforeBalls;
        double lastFailTime = -1000.0;
        Point2D lastDecisionCuePos;
        int    simsUsed = 0;
        int    potCands = 0;      // diagnostics: sims that potted a legal ball
        int    safeCands = 0;     // diagnostics: legal contact without a pot
        int    invalidCands = 0;  // diagnostics: fouls / scratches / bad contact
    };

    // --- Stage functions (implemented in HumanScan.impl.h) ---
    bool AnalyzeTable();                    // live read, zero sim
    void ChooseGroup();                     // zero sim
    void CreateCandidates();                // direct shots only, zero sim
    void CreateBankCandidates();             // 1-rail bank candidates for rule-required 8-ball shots
    void AppendIndirectCandidates();        // combos + general 1-rail banks, zero sim
    void EasyFilter();                      // reject unsuitable candidates, zero sim
    void RankCandidates();                  // keep top MAX_CANDIDATES, zero sim
    SimStatus SimulateCandidate();          // up to 4 full sims, sets ctx.best/safeShot
    void LookAheadOneShot(HumanCandidate& c); // geometric only, zero extra sim
    void CommitShot(const HumanCandidate& c);
    void ExecuteShot();
    bool BreakPhase();                      // ~41-sim radial sweep at full power
    bool RunIfReady();                      // guarded entry from AutoPlay::Update
    void ResetTurn();

    inline Context ctx;
}
