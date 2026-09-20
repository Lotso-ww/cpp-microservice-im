#include "../../common/aip-cpp-sdk-4.16.7/speech.h"

void asr(aip::Speech &client)
{
    // 无可选参数调用接口
    std::string file_content;
    if (aip::get_file_content("16k.pcm", &file_content) != 0) {
        std::cout << "read 16k.pcm failed" << std::endl;
        return;
    }

    // 如果需要覆盖或者加入参数
    std::map<std::string, std::string> options;
    options["dev_pid"] = "1537";
    Json::Value result = client.recognize(file_content, "pcm", 16000, options);
    if (result["err_no"].asInt() != 0) {
        std::cout << result["err_msg"].asString() << std::endl;
        return;
    }

    std::cout << result["result"][0].asString() << std::endl;
}

int main()
{
    // 设置APPID/AK/SK
    std::string app_id = "8012068";
    std::string api_key = "pz31dgyMWksAIgFtlgY6mG1A";
    std::string secret_key = "fSna8Sbtc3jBYKPUSTfK6IMSAHJwX2ma";

    aip::Speech client(app_id, api_key, secret_key);

    asr(client);
    return 0;
}
