#ifndef BITCOIN_QUANTUM_MINING_QUANTUM_POW_H
#define BITCOIN_QUANTUM_MINING_QUANTUM_POW_H

#include <vector>
#include <uint256.h>
#include <primitives/block.h>
#include <chain.h>

// Forward declarations
class CBlockIndex;

/**
 * Quantum-Resistant Proof of Work Implementation
 * Protects against Grover's algorithm and quantum speedup attacks
 */

/**
 * Quantum PoW challenge structure
 * Uses lattice-based problems for quantum resistance
 */
struct CQuantumChallenge {
    std::vector<uint8_t> lattice_matrix;     // Lattice basis for the challenge
    std::vector<uint8_t> syndrome;           // Target syndrome to find
    uint32_t dimension;                      // Lattice dimension
    uint32_t modulus;                        // Arithmetic modulus
    
    SERIALIZE_METHODS(CQuantumChallenge, obj) {
        READWRITE(obj.lattice_matrix, obj.syndrome, obj.dimension, obj.modulus);
    }
    
    bool IsValid() const;
    size_t GetSerializeSize() const;
};

/**
 * Quantum PoW solution
 * Lattice-based solution that proves computational work
 */
struct CQuantumProof {
    std::vector<int32_t> solution_vector;    // Short vector solution
    uint256 classical_nonce;                 // Classical nonce for hybrid mining
    uint64_t quantum_iterations;             // Quantum-resistant work counter
    
    SERIALIZE_METHODS(CQuantumProof, obj) {
        READWRITE(obj.solution_vector, obj.classical_nonce, obj.quantum_iterations);
    }
    
    bool IsValid() const;
    uint256 GetHash() const;
};

/**
 * Mining transition configuration
 * Manages the transition from classical to quantum-resistant mining
 */
struct CQuantumMiningConfig {
    bool fEnableQuantumMining;               // Enable quantum mining
    bool fHybridMining;                      // Allow both classical and quantum
    double quantum_weight_factor;            // Weight for quantum vs classical PoW
    uint32_t transition_start_height;        // Block height to start transition
    uint32_t full_quantum_height;            // Block height for full quantum mining
    
    // Difficulty adjustment parameters
    uint32_t quantum_target_spacing;         // Target time between quantum blocks
    uint32_t quantum_retarget_interval;      // Blocks between difficulty adjustments
    
    CQuantumMiningConfig() : 
        fEnableQuantumMining(false),
        fHybridMining(true),
        quantum_weight_factor(0.5),
        transition_start_height(0),
        full_quantum_height(0),
        quantum_target_spacing(600), // 10 minutes
        quantum_retarget_interval(2016) {}
};

/**
 * Quantum-Resistant Proof of Work Engine
 */
class CQuantumPoW {
public:
    /**
     * Generate quantum challenge for a block
     * Creates lattice-based challenge using block header data
     */
    static CQuantumChallenge GenerateChallenge(const CBlockHeader& header);
    
    /**
     * Solve quantum challenge
     * Find short vector solution to the lattice problem
     */
    static bool SolveChallenge(
        const CQuantumChallenge& challenge,
        CQuantumProof& proof,
        const std::function<bool()>& interrupt_check = nullptr
    );
    
    /**
     * Verify quantum proof of work
     * Validate that the solution is correct and meets difficulty target
     */
    static bool VerifyQuantumProof(
        const CBlockHeader& header,
        const CQuantumChallenge& challenge,
        const CQuantumProof& proof,
        const uint256& target
    );
    
    /**
     * Calculate quantum difficulty target
     * Adjust difficulty based on quantum mining performance
     */
    static uint256 GetNextQuantumWorkRequired(
        const CBlockIndex* pindexLast,
        const CBlockHeader* pblock,
        const CQuantumMiningConfig& config
    );
    
    /**
     * Hybrid mining verification
     * Verify both classical and quantum proofs during transition
     */
    static bool VerifyHybridProof(
        const CBlockHeader& header,
        const CQuantumProof& quantum_proof,
        const CQuantumMiningConfig& config
    );
    
    /**
     * Estimate quantum work value
     * Convert quantum proof difficulty to equivalent classical work
     */
    static double EstimateQuantumWork(const CQuantumProof& proof);
    
    /**
     * Check if quantum mining is required at height
     */
    static bool IsQuantumMiningRequired(
        uint32_t height,
        const CQuantumMiningConfig& config
    );
};

/**
 * Quantum Mining Pool Support
 */
class CQuantumMiningPool {
public:
    /**
     * Create mining template with quantum challenge
     */
    static bool CreateQuantumBlockTemplate(
        CBlock& block,
        CQuantumChallenge& challenge,
        const CScript& coinbase_script
    );
    
    /**
     * Submit quantum proof solution
     */
    static bool SubmitQuantumSolution(
        CBlock& block,
        const CQuantumProof& proof
    );
    
    /**
     * Distribute quantum mining work
     * Split lattice problem for parallel mining
     */
    static std::vector<CQuantumChallenge> DistributeWork(
        const CQuantumChallenge& master_challenge,
        size_t num_workers
    );
};

/**
 * Lattice-based algorithms for quantum resistance
 */
namespace QuantumResistantAlgorithms {
    /**
     * SWIFFT-based hash function for quantum PoW
     */
    uint256 SwifftHash(const std::vector<uint8_t>& input);
    
    /**
     * Learning With Errors (LWE) based challenge
     */
    CQuantumChallenge GenerateLWEChallenge(const uint256& seed, uint32_t difficulty);
    
    /**
     * NTRU lattice-based proof of work
     */
    bool SolveNTRUChallenge(
        const CQuantumChallenge& challenge,
        CQuantumProof& proof
    );
    
    /**
     * Ring-LWE based mining algorithm
     */
    bool SolveRingLWEChallenge(
        const CQuantumChallenge& challenge,
        CQuantumProof& proof
    );
}

/**
 * Quantum attack resistance analysis
 */
namespace QuantumSecurity {
    /**
     * Estimate quantum speedup resistance
     * Returns factor by which quantum computers are slowed down
     */
    double EstimateQuantumSpeedupResistance(const CQuantumChallenge& challenge);
    
    /**
     * Validate post-quantum security level
     */
    bool ValidatePostQuantumSecurity(
        const CQuantumChallenge& challenge,
        uint32_t required_security_bits
    );
    
    /**
     * Check resistance to known quantum algorithms
     */
    struct QuantumAttackResistance {
        bool grover_resistant;          // Resistant to Grover's algorithm
        bool shor_resistant;            // Resistant to Shor's algorithm
        bool quantum_period_finding;    // Resistant to period finding
        uint32_t security_level;        // Bits of quantum security
    };
    
    QuantumAttackResistance AnalyzeQuantumResistance(
        const CQuantumChallenge& challenge
    );
}

/**
 * Performance optimization for quantum mining
 */
namespace QuantumMiningOptimization {
    /**
     * SIMD-optimized lattice operations
     */
    void OptimizeLatticeOperations();
    
    /**
     * GPU acceleration for quantum mining
     */
    bool InitializeGPUMining();
    
    /**
     * Parallel quantum proof search
     */
    class ParallelQuantumMiner {
    public:
        bool StartMining(
            const CQuantumChallenge& challenge,
            size_t num_threads = 0
        );
        
        bool GetSolution(CQuantumProof& proof);
        void StopMining();
        
    private:
        std::atomic<bool> mining_active{false};
        std::vector<std::thread> mining_threads;
        std::mutex solution_mutex;
        std::optional<CQuantumProof> found_solution;
    };
}

extern CQuantumMiningConfig g_quantum_mining_config;

#endif // BITCOIN_QUANTUM_MINING_QUANTUM_POW_H 