#include "whisper.h"

#include <cstdio>
#include <string>
#include <thread>

// command-line parameters
struct whisper_params {
    int32_t n_threads   = std::min(4, (int32_t) std::thread::hardware_concurrency());

    std::string model     = "models/ggml-base.en.bin";
};

void whisper_print_usage(int argc, char ** argv, const whisper_params & params);

bool whisper_params_parse(int argc, char ** argv, whisper_params & params) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-t" || arg == "--threads") {
            params.n_threads = std::stoi(argv[++i]);
        } else if (arg == "-m" || arg == "--model") {
            params.model = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            whisper_print_usage(argc, argv, params);
            exit(0);
        } else {
            fprintf(stderr, "error: unknown argument: %s\n", arg.c_str());
            whisper_print_usage(argc, argv, params);
            exit(0);
        }
    }

    return true;
}

void whisper_print_usage(int argc, char ** argv, const whisper_params & params) {
    fprintf(stderr, "\n");
    fprintf(stderr, "usage: %s [options]\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  -h,       --help           show this help message and exit\n");
    fprintf(stderr, "  -t N,     --threads N      number of threads to use during computation (default: %d)\n", params.n_threads);
    fprintf(stderr, "  -m FNAME, --model FNAME    model path (default: %s)\n", params.model.c_str());
    fprintf(stderr, "\n");
}

int main(int argc, char ** argv) {
    whisper_params params;

    if (whisper_params_parse(argc, argv, params) == false) {
        return 1;
    }

    // whisper init

    struct whisper_context * ctx = whisper_init(params.model.c_str());

    {
        fprintf(stderr, "\n");
        fprintf(stderr, "system_info: n_threads = %d / %d | %s\n", params.n_threads, std::thread::hardware_concurrency(), whisper_print_system_info());
    }

    if (ctx == nullptr) {
        fprintf(stderr, "error: failed to initialize whisper context\n");
        return 2;
    }

    if (int ret = whisper_set_mel(ctx, nullptr, 0, WHISPER_N_MEL)) {
        fprintf(stderr, "error: failed to set mel: %d\n", ret);
        return 3;
    }

    if (int ret = whisper_encode(ctx, 0, params.n_threads) != 0) {
        fprintf(stderr, "error: failed to encode model: %d\n", ret);
        return 4;
    }

    whisper_print_timings(ctx);
    whisper_free(ctx);

    fprintf(stderr, "\n");
    fprintf(stderr, "If you wish, you can submit these results here:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  https://github.com/ggerganov/whisper.cpp/issues/89\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Please include the following information:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  - CPU model\n");
    fprintf(stderr, "  - Operating system\n");
    fprintf(stderr, "  - Compiler\n");
    fprintf(stderr, "\n");

    return 0;
}
