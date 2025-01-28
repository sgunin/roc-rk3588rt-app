#include <cstdio>
#include <rknn_api.h>

int main()
{
    rknn_context ctx = 0;
    char* model = "mobilenet_v1.rknn";

    int ret = rknn_init(&ctx, model, 0, 0, NULL);
    if (ret < 0) {
        printf("Ошибка инициализации RKNN: %d", ret);
        return 1;
    }

    rknn_sdk_version ver;
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &ver, sizeof(ver));
    if (RKNN_SUCC != ret) {
        printf("Ошибка выполнения KNN_QUERY_SDK_VERSION: %d", ret);
        return 1;
    }
    printf("Версия SDK: %s, версия драйвера: %s", ver.api_version, ver.drv_version);

    rknn_destroy(ctx);
    return 0;
}