#include <stdio.h>
#include <string.h>

#include "X11/Xlib.h"
#include "MvCameraControl.h"

int main()
{
    // Инициализируем XOpenDisplay
    Display* display = XOpenDisplay(0);
    if (display == NULL) {
        printf("Ошибка при выполнении XOpenDisplay, инициализация SDK вернет предупреждение\n");
    }
    printf("XOpenDisplay инициализирована успешно\n");

    // Инициализируем HikRobot SDK
    int nRet = MV_OK;

    nRet = MV_CC_Initialize();
    if (MV_OK != nRet) {
        printf("Ошибка при инициализации HikRobot SDK! Код ошибки: [0x%x]\n", nRet);
        return -1;
    }
    printf("Инициализация HikRobot SDK выполнена успешно\n");
    
    // Перечисляем устройства, подключенные по USB
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != nRet) {
        printf("Ошибка при выполнении MV_CC_EnumDevices! Код ошибки: [%x]\n", nRet);
        return -1;
    }

    if (stDeviceList.nDeviceNum > 0) {
        printf("Найдено %d устройств:\n", stDeviceList.nDeviceNum);

        for (int i = 0; i < stDeviceList.nDeviceNum; i++) {
            MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
            if (NULL == pDeviceInfo)
                break;
            
            printf("\tУстройство №%d\n", i);
            printf("\tМодель камеры: %s\n", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName);
            printf("\tСерийный номер: %s\n", pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            printf("\tАппаратная версия: %s\n", pDeviceInfo->SpecialInfo.stUsb3VInfo.chDeviceVersion);
            printf("\tПроизводитель: %s\n", pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName);

            // Пытаемся открыть канал взаимодействия с камерой
            void* handle = NULL;
            nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[i]);
            if (MV_OK == nRet) {
                nRet = MV_CC_OpenDevice(handle);
                if (MV_OK == nRet) {
                    
                    // Тут делаем какую-либо полезную работу с камерой
                    // Например, можно вызвать MV_CC_StartGrabbing для захвата изображений
                    printf("\t=>Канал взаимодействия с камерой %s успешно проверен\n", pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);

                    // Закрываем канал взаимодействия с камерой
                    nRet = MV_CC_CloseDevice(handle);
                    if (MV_OK != nRet)
                    {
                        printf("\tОшибка при выполнении MV_CC_CloseDevice! Код ошибки: [%x]\n", nRet);
                    }
                }
                else {
                    // В ряде случаев MV_CC_OpenDevice возвращает код ошибки 80000301, при этом проблем с подключением не наблюдается
                    // Возможно, камера находится в состоянии BUSY
                    printf("\tОшибка при выполнении MV_CC_OpenDevice! Код ошибки: [%x]\n", nRet);
                }
            }
            else {
                printf("\tОшибка при выполнении MV_CC_CreateHandle! Код ошибки: [%x]\n", nRet);
            }

            // Освобождаем ссылку на ресурсы
            if (handle != NULL) {
                nRet = MV_CC_DestroyHandle(handle);
                if (MV_OK != nRet) {
                    printf("\tОшибка при выполнении MV_CC_DestroyHandle! Код ошибки: [%x]\n", nRet);
                }
                handle = NULL;
            }
            printf("\n");
        }
    }
    else {
        printf("Не найдено ни одного устройства HikRobot Camera\n");
    }
    
    // Высвобождаем ресурсы SDK
    nRet = MV_CC_Finalize();
    if (MV_OK != nRet) {
        printf("Ошибка при выполнении MV_CC_Finalize! Код ошибки: [%x]\n", nRet);
    }

    return 0;
}