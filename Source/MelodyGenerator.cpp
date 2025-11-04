// MelodyGenerator.cpp
#include "MelodyGenerator.h"
#include <sstream>
#include <numeric>
#include <algorithm>

MelodyGenerator::MelodyGenerator()
    : env(ORT_LOGGING_LEVEL_WARNING, "MelodyGenerator"),
    memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
{}

MelodyGenerator::~MelodyGenerator() {}

bool MelodyGenerator::loadVocabulary()
{
    try
    {
        // Load note_to_int from JSON in BinaryData
        juce::String noteToIntJson(BinaryData::note_to_int_json, BinaryData::note_to_int_jsonSize);
        auto noteToIntVar = juce::JSON::parse(noteToIntJson);

        if (!noteToIntVar.isObject())
        {
            lastError = "Failed to parse note_to_int.json";
            return false;
        }

        auto noteToIntObj = noteToIntVar.getDynamicObject();

        for (auto& prop : noteToIntObj->getProperties())
        {
            juce::String key = prop.name.toString();
            int value = static_cast<int>(prop.value);
            note_to_int[key.toStdString()] = value;
        }

        // Load int_to_note from JSON in BinaryData
        juce::String intToNoteJson(BinaryData::int_to_note_json, BinaryData::int_to_note_jsonSize);
        auto intToNoteVar = juce::JSON::parse(intToNoteJson);

        if (!intToNoteVar.isObject())
        {
            lastError = "Failed to parse int_to_note.json";
            return false;
        }

        auto intToNoteObj = intToNoteVar.getDynamicObject();
        for (auto& prop : intToNoteObj->getProperties())
        {
            int key = prop.name.toString().getIntValue();
            juce::String value = prop.value.toString();
            int_to_note[key] = value.toStdString();
        }

        return true;
    }
    catch (const std::exception& e)
    {
        lastError = "Error loading vocabulary: " + std::string(e.what());
        DBG(lastError);
        return false;
    }
}

bool MelodyGenerator::initialize()
{
    try
    {
        // STEP 1: Load vocabulary mappings FIRST
        if (!loadVocabulary())
        {
            return false;
        }

        // Concatenate the split parts
        juce::MemoryBlock modelBlock;
        modelBlock.append(BinaryData::floralv2_part1_bin, BinaryData::floralv2_part1_binSize);
        modelBlock.append(BinaryData::floralv2_part2_bin, BinaryData::floralv2_part2_binSize);
        modelBlock.append(BinaryData::floralv2_part3_bin, BinaryData::floralv2_part3_binSize);
        modelBlock.append(BinaryData::floralv2_part4_bin, BinaryData::floralv2_part4_binSize);
        modelBlock.append(BinaryData::floralv2_part5_bin, BinaryData::floralv2_part5_binSize);

        // Create session options
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

        // Create session from model data
        session = std::make_unique<Ort::Session>(env, modelBlock.getData(), modelBlock.getSize(), sessionOptions);

        // Get input and output names for verification
        auto inputName = session->GetInputNameAllocated(0, allocator);
        auto outputName = session->GetOutputNameAllocated(0, allocator);

        // Verify input and output counts
        if (session->GetInputCount() != 1 || session->GetOutputCount() != 1)
        {
            lastError = "Model must have exactly 1 input and 1 output";
            DBG(lastError);
            return false;
        }

        // Verify input shape: dynamic batch, fixed seq=8, features=1
        auto inputInfo = session->GetInputTypeInfo(0);
        auto inputTensorInfo = inputInfo.GetTensorTypeAndShapeInfo();
        auto inputShape = inputTensorInfo.GetShape();

        if (inputShape.size() != 3 || inputShape[0] != -1 || inputShape[1] != 8 || inputShape[2] != 1)
        {
            lastError = "Invalid input shape; expected {-1, 8, 1}";
            DBG(lastError);
            return false;
        }

        // Verify output shape: dynamic batch, fixed vocab
        auto outputInfo = session->GetOutputTypeInfo(0);
        auto outputTensorInfo = outputInfo.GetTensorTypeAndShapeInfo();
        auto outputShape = outputTensorInfo.GetShape();

        if (outputShape.size() != 2 || outputShape[0] != -1 || outputShape[1] <= 0)
        {
            lastError = "Invalid output shape; expected {-1, N}";
            DBG(lastError);
            return false;
        }

        vocab_size = static_cast<int>(outputShape[1]);

        // Verify vocab_size matches our loaded vocabulary
        if (vocab_size != static_cast<int>(note_to_int.size()))
        {
            lastError = "Vocabulary size mismatch: model has " + std::to_string(vocab_size) + " but loaded vocabulary has " + std::to_string(note_to_int.size());
            DBG(lastError);
            return false;
        }

        DBG("Model loaded and verified successfully.");

        return true;
    }
    catch (const Ort::Exception& e)
    {
        lastError = "ONNX Runtime error: " + std::string(e.what());
        DBG(lastError);
        return false;
    }
    catch (const std::exception& e)
    {
        lastError = "Initialization error: " + std::string(e.what());
        DBG(lastError);
        return false;
    }

}

std::vector<int> MelodyGenerator::generateMelody(const std::vector<int>& inputEvents, float temperature, int steps)
{
    if (!session)
    {
        lastError = "Model not initialized";
        DBG(lastError);
        return {};
    }

    // Preprocess input vector to 8 MIDI note numbers
    std::vector<int> processed_input;

    // Extract positive note numbers
    for (const int& event : inputEvents)
    {
        if (event >= 0)
        {
            processed_input.push_back(event);
        }
    }

    // If more than 8, keep only the last 8
    if (processed_input.size() > 8)
    {
        processed_input.erase(processed_input.begin(), processed_input.begin() + (processed_input.size() - 8));
    }

    // If less than 8 and not empty, repeat the sequence to fill up to 8
    if (processed_input.size() < 8 && !processed_input.empty())
    {
        std::vector<int> original = processed_input;  // Copy the original sequence
        while (processed_input.size() < 8)
        {
            for (const int& note : original)
            {
                if (processed_input.size() < 8)
                {
                    processed_input.push_back(note);
                }
                else
                {
                    break;
                }
            }
        }
    }
    else if (processed_input.empty())
    {
        // Handle case with no positive notes; perhaps fill with a default or error
        lastError = "No positive MIDI note numbers found in inputEvents";
        DBG(lastError);
        return {};
    }

    // Now processed_input has exactly 8 positive MIDI note numbers

    // Shuffle the 8 numbers 'cause the model's non-stochastic
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(processed_input.begin(), processed_input.end(), g);

    // Clamp input note numbers to a safe range
    for (auto& note : processed_input)
    {
        while (note < 60) note += 12;
        while (note > 78) note -= 12;
    }

    // Debug print processed input
    juce::String debugStringProcessed = "processed input events: ";
    for (const int& note : processed_input)
    {
        debugStringProcessed += juce::String(note) + " ";
    }
    DBG(debugStringProcessed);

    const int SEQUENCE_LENGTH = 8;

    // Verify input sequence length
    if (processed_input.size() != SEQUENCE_LENGTH)
    {
        lastError = "Input sequence must have length " + std::to_string(SEQUENCE_LENGTH) + ", got " + std::to_string(processed_input.size());
        DBG(lastError);
        return {};
    }

    try
    {
        // Get input and output names
        auto inputNameAlloc = session->GetInputNameAllocated(0, allocator);
        auto outputNameAlloc = session->GetOutputNameAllocated(0, allocator);
        const char* inputNamePtr = inputNameAlloc.get();
        const char* outputNamePtr = outputNameAlloc.get();

        // Python: pattern = [note_to_int[token] for token in input_sequence_tokens]
        std::vector<int> pattern;
        for (const auto& event : processed_input)
        {
            std::string token = std::to_string(event);
            auto it = note_to_int.find(token);
            if (it == note_to_int.end())
            {
                lastError = "Token '" + token + "' not found in vocabulary";
                DBG(lastError);
                return {};
            }
            pattern.push_back(it->second);
        }

        std::vector<int> prediction_output;
        int number_of_notes_to_generate = steps;

        for (int step = 0; step < number_of_notes_to_generate; ++step)
        {
            // Python: input_sequence = np.reshape(pattern, (1, len(pattern), 1))
            std::vector<int64_t> inputShape = { 1, SEQUENCE_LENGTH, 1 };
            size_t inputTensorSize = SEQUENCE_LENGTH;
            std::vector<float> inputTensorValues(inputTensorSize);

            // Python: input_sequence = input_sequence / float(unique_notes_count)
            for (int i = 0; i < SEQUENCE_LENGTH; ++i)
            {
                inputTensorValues[i] = static_cast<float>(pattern[i]) / static_cast<float>(vocab_size);
            }

            // Create input tensor
            auto inputTensor = Ort::Value::CreateTensor<float>(
                memoryInfo,
                inputTensorValues.data(),
                inputTensorSize,
                inputShape.data(),
                inputShape.size()
            );

            // Python: prediction = ort_session.run([output_name], {input_name: input_sequence})[0]
            const char* inputNames[] = { inputNamePtr };
            const char* outputNames[] = { outputNamePtr };
            auto outputTensors = session->Run(
                Ort::RunOptions{ nullptr },
                inputNames,
                &inputTensor,
                1,
                outputNames,
                1
            );

            // Get prediction output
            float* outputData = outputTensors[0].GetTensorMutableData<float>();
            auto outputShapeInfo = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
            int64_t vocab_size_output = outputShapeInfo[1];

            // original python argmax with no temperature control

            // Python: index = np.argmax(prediction)
            int index = 0;
            float maxValue = outputData[0];
            for (int64_t i = 1; i < vocab_size_output; ++i)
            {
                if (outputData[i] > maxValue)
                {
                    maxValue = outputData[i];
                    index = static_cast<int>(i);
                }
            }

            //            DBG("Step " + juce::String(step) + ": Predicted index = " + juce::String(index));

            if (index == 55)
            {
                index = 1;
                DBG("Step " + juce::String(step) + ": Replaced predicted index 55 (SONG_END) with 1 (-2)");
            }

            // Python: result = int_to_note[index]
            auto it = int_to_note.find(index);
            if (it == int_to_note.end())
            {
                lastError = "Index " + std::to_string(index) + " not found in int_to_note mapping";
                DBG(lastError);
                return {};
            }
            std::string resultStr = it->second;

            // Convert result string back to int
            int result = std::stoi(resultStr);

            // Print progress (matching Python format)
            prediction_output.push_back(result);

            // Python: pattern.append(index); pattern = pattern[1:]
            pattern.push_back(index);
            pattern.erase(pattern.begin());
        }

        //        DBG("Generated sequence complete. Generated " + juce::String(prediction_output.size()) + " notes.");

                // DBG print the generated melody here

                // Debug print
        juce::String debugString = "prediction_output: ";
        for (const int& note : prediction_output)
        {
            debugString += juce::String(note) + " ";
        }
        DBG(debugString);

        //        return prediction_output;

        std::vector<int> post_processed;

        // PLACE LENGTH DISTRIBUTION LOGIC HERE

        // Generate random lengths
        if (steps < 1 || steps > 32)
        {
            lastError = "Steps must be between 1 and 32";
            DBG(lastError);
            return {};
        }
        std::vector<int> lengths(steps, 1);
        int extras = 32 - steps;
        std::uniform_int_distribution<> dis(0, steps - 1);
        for (int i = 0; i < extras; ++i)
        {
            int idx = dis(g);
            lengths[idx]++;
        }

        // Build post_processed
        for (size_t i = 0; i < static_cast<size_t>(steps); ++i)
        {
            post_processed.push_back(prediction_output[i]);
            for (int j = 1; j < lengths[i]; ++j)
            {
                post_processed.push_back(-2);
            }
        }

        // Magnetize to on-beats 100% of the time
        magnetize(post_processed, 1.0);

        // Debug print post-processed output
        juce::String debugPostProcessed = "post-processed output: ";
        for (const int& event : post_processed)
        {
            debugPostProcessed += juce::String(event) + " ";
        }
        DBG(debugPostProcessed);

        return post_processed;
    }
    catch (const Ort::Exception& e)
    {
        lastError = "ONNX Runtime error during generation: " + std::string(e.what());
        DBG(lastError);
        return {};
    }
    catch (const std::exception& e)
    {
        lastError = "Generation error: " + std::string(e.what());
        DBG(lastError);
        return {};
    }
}

void MelodyGenerator::magnetize(std::vector<int>& melody, float probability) const
{
    if (probability <= 0.0f) return;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    size_t size = melody.size();
    for (size_t i = 1; i < size; i += 2)
    {
        if (melody[i] >= 0)
        {
            if (dis(gen) >= probability) continue;

            ssize_t lower = static_cast<ssize_t>(i) - 1;
            ssize_t higher = static_cast<ssize_t>(i) + 1;

            bool lower_free = (lower >= 0 && melody[lower] < 0);
            bool higher_free = (higher < static_cast<ssize_t>(size) && melody[higher] < 0);

            if (!lower_free && !higher_free)
            {
                continue;
            }

            ssize_t target = -1;
            if (lower_free && higher_free)
            {
                if (dis(gen) < 0.5f)
                {
                    target = lower;
                }
                else
                {
                    target = higher;
                }
            }
            else if (lower_free)
            {
                target = lower;
            }
            else
            {
                target = higher;
            }

            melody[target] = melody[i];
            melody[i] = -2;
        }
    }
}