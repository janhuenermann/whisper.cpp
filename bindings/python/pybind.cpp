#include <iostream>
#include <thread>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "whisper.h"

namespace py = pybind11;

using namespace pybind11::literals;

struct whisper_context * g_context = nullptr;

PYBIND11_MODULE(pywhisper, m) {
    m.doc() = "Python bindings for whisper.cpp";

    py::enum_<whisper_sampling_strategy>(m, "SamplingStrategy")
        .value("BEAM_SEARCH", whisper_sampling_strategy::WHISPER_SAMPLING_BEAM_SEARCH)
        .value("GREEDY", whisper_sampling_strategy::WHISPER_SAMPLING_GREEDY)
        .export_values()
    ;

    m.def(
        "init",
        [](const std::string& model_path) {
            if (g_context != nullptr) {
                return false;
            }
            g_context = whisper_init_from_file(model_path.c_str());
            return g_context != nullptr;
        },
        "model_path"_a,
        "Loads a Whisper model from the given path and returns True on success."
    );

    m.def(
        "transcribe",
        [](const py::array_t<float, py::array::c_style>& audio, const std::string& lang, bool translate, whisper_sampling_strategy sampling_strategy) -> py::list {
            if (g_context == nullptr) {
                throw std::runtime_error("pywhisper is not initialized. please call pywhisper.init(model_path) first.");
            }

            struct whisper_full_params params = whisper_full_default_params(sampling_strategy);
            params.print_realtime   = false;
            params.print_progress   = false;
            params.print_timestamps = false;
            params.print_special    = false;
            params.translate        = translate;
            params.language         = whisper_is_multilingual(g_context) ? lang.c_str() : "en";
            params.n_threads        = std::min(8, (int) std::thread::hardware_concurrency());
            params.offset_ms        = 0;

            // run whisper
            whisper_reset_timings(g_context);
            whisper_full(g_context, params, audio.data(), audio.size());

            py::list transcript;
            for (int i = 0, n = whisper_full_n_segments(g_context); i < n; i++) {
                int64_t t0 = whisper_full_get_segment_t0(g_context, i);
                int64_t t1 = whisper_full_get_segment_t1(g_context, i);
                transcript.append(
                    py::make_tuple(
                        double(t0) / 100.0,
                        double(t1) / 100.0,
                        std::string(whisper_full_get_segment_text(g_context, i))
                    )
                );
            }

            return transcript;
        },
        "audio"_a,
        "lang"_a = "en",
        "translate"_a = false,
        "sampling_strategy"_a = whisper_sampling_strategy::WHISPER_SAMPLING_GREEDY,
        "Transcribes the given audio data and returns a list of "
        "(start [seconds], end [seconds], text chunk) tuples. "
        "The audio must be represented as a 16kHz PCM signal."
    );
}
