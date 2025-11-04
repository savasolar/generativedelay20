// MelodyGenerator.h
#pragma once
#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <map>
#include <string>
#include <random>
#include <cmath>

class MelodyGenerator {
public:
    MelodyGenerator();
    ~MelodyGenerator();
    bool initialize();
    std::vector<int> generateMelody(const std::vector<int>& inputEvents, float temperature, int steps);
    std::string getLastError() const { return lastError; }
    bool isInitialized() const { return session != nullptr; }

private:
    bool loadVocabulary();

    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::MemoryInfo memoryInfo;
    std::string lastError;
    int vocab_size = 0;

    // vocabulary mappings
    std::map<std::string, int> note_to_int;
    std::map<int, std::string> int_to_note;

    // post-process formatting
    void magnetize(std::vector<int>& melody, float probability) const;
};
