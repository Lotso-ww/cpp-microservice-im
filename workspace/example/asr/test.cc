#include "../../common/asr.hpp"
#include <gflags/gflags.h>

DEFINE_bool(run_mode, false, "程序运行模式, false-调试 true-发布");
DEFINE_string(log_file, "", "发布模式下, 用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下, 用于指定日志输出等级");

DEFINE_string(app_id, "8012068", "语音识别平台应用 ID");
DEFINE_string(api_key, "pz31dgyMWksAIgFtlgY6mG1A", "语音识别平台应用 API Key");
DEFINE_string(secret_key, "fSna8Sbtc3jBYKPUSTfK6IMSAHJwX2ma", "语音识别平台应用加密密钥");


int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    ASRClient client(FLAGS_app_id, FLAGS_api_key, FLAGS_secret_key);

    std::string file_content;
    aip::get_file_content("16k.pcm", &file_content);

    std::string ret = client.recognize(file_content);
    if(ret.empty()) return -1;
    std::cout << ret << std::endl;
    return 0;
}