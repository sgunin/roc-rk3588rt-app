#include <cstdio>
#include <string.h>
#include <pthread.h>

#include "X11/Xlib.h"
#include "MvCameraControl.h"

Display* display = NULL;
Window window = NULL;
void* handle = NULL;
int nRet = MV_OK;
bool stopProcess;

void ReleaseResources()
{
	// Освобождаем ссылку на ресурсы
	if (handle != NULL) {
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet) {
			printf("\tОшибка при выполнении MV_CC_DestroyHandle! Код ошибки: [%x]\n", nRet);
		}
		handle = NULL;
	}

	// Высвобождаем ресурсы SDK
	nRet = MV_CC_Finalize();
	if (MV_OK != nRet) {
		printf("Ошибка при выполнении MV_CC_Finalize! Код ошибки: [%x]\n", nRet);
	}

	if (display != NULL)
		XCloseDisplay(display);
}

static void* WorkThread(void* pUser) {
	int nRet = MV_OK;
	MV_FRAME_OUT stImageInfo = { 0 };
	MV_DISPLAY_FRAME_INFO stDisplayInfo = { 0 };
	int i = 0;
	while (!stopProcess) {
		// Получаем буффер изображения
		nRet = MV_CC_GetImageBuffer(pUser, &stImageInfo, 1000);
		if (MV_OK == nRet) {
			if (window) {
				stDisplayInfo.hWnd = (void*)window;
				stDisplayInfo.pData = stImageInfo.pBufAddr;
				stDisplayInfo.nDataLen = stImageInfo.stFrameInfo.nFrameLen;
				stDisplayInfo.nWidth = stImageInfo.stFrameInfo.nWidth;
				stDisplayInfo.nHeight = stImageInfo.stFrameInfo.nHeight;
				stDisplayInfo.enPixelType = stImageInfo.stFrameInfo.enPixelType;

				nRet = MV_CC_DisplayOneFrame(pUser, &stDisplayInfo);
				if (MV_OK != nRet)
					printf("Ошибка отображения фрейма изображения! Код ошибки: [0x%x]\n", nRet);
			}

			// Освобождаем буффер изображения
			nRet = MV_CC_FreeImageBuffer(pUser, &stImageInfo);
			if (nRet != MV_OK)
				printf("Ошибка освобождения буффера изображения! Код ошибки: [0x%x]\n", nRet);
		}
		else {
			printf("Ошибка получения буффера изображения! Код ошибки: [0x%x]\n", nRet);
		}
	}

	return 0;
}

int main()
{
	// Инициализируем XOpenDisplay
	display = XOpenDisplay(0);
	if (display == NULL) {
		printf("Ошибка при обращении к XOpenDisplay, инициализация SDK вернет предупреждение\n");
	}
	else {
		printf("XOpenDisplay инициализирован успешно\n");
	}

	// Инициализируем HikRobot SDK
	nRet = MV_CC_Initialize();
	if (MV_OK != nRet) {
		printf("Ошибка при инициализации HikRobot SDK! Код ошибки: [0x%x]\n", nRet);
		ReleaseResources();
		return -1;
	}
	printf("Инициализация HikRobot SDK выполнена успешно\n");

	// Перечисляем устройства, подключенные по USB
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

	nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet) {
		printf("Ошибка при выполнении MV_CC_EnumDevices! Код ошибки: [%x]\n", nRet);
		ReleaseResources();
		return -1;
	}

	if (stDeviceList.nDeviceNum == 0) {
		printf("Не найдено ни одной подключенной по USB камеры\n");
		ReleaseResources();
		return -1;
	}

	MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[0];
	if (NULL == pDeviceInfo) {
		printf("Ошибка при получении информации от устройства\n", nRet);
		ReleaseResources();
		return -1;
	}

	// Пытаемся открыть канал взаимодействия с камерой
	nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[0]);
	if (MV_OK != nRet) {
		printf("\tОшибка при выполнении MV_CC_CreateHandle! Код ошибки: [%x]\n", nRet);
		ReleaseResources();
		return -1;
	}

	// Открываем устройство
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK == nRet) {
		printf("\tУспешное соединение с камерой %s модели %s\n", 
			pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber,
			pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName);
	}
	else {
		// В ряде случаев MV_CC_OpenDevice возвращает код ошибки 80000301, при этом проблем с подключением не наблюдается
		// Возможно, камера находится в состоянии BUSY
		printf("\tОшибка при выполнении MV_CC_OpenDevice! Код ошибки: [%x]\n", nRet);
		ReleaseResources();
		return -1;
	}

	// Запускаем захват изображения
	nRet = MV_CC_StartGrabbing(handle);
	if (MV_OK == nRet) {
		printf("\tИнициирован захват изображения с камеры %d\n", nRet);
	}
	else {
		printf("MV_CC_StartGrabbing fail! nRet [%x]\n", nRet);
		MV_CC_CloseDevice(handle);
		ReleaseResources();
		return -1;
	}

	// Получение номера экрана по умолчанию
	int screen = DefaultScreen(display);

	// Получаем ссылку на родительское окно
	Window parent_window = DefaultRootWindow(display);

	unsigned int border_color = BlackPixel(display, screen);
	unsigned int background_color = WhitePixel(display, screen);

	// Создаем окно
	window = XCreateSimpleWindow(display, parent_window, 0, 0, 752, 480, 1, border_color, background_color);
	if (!window) {
		printf("Ошибка при выполнении XCreateSimpleWindow\n");
		ReleaseResources();
		return 1;
	}

	long event_mask = ExposureMask
		| KeyPressMask
		| KeyReleaseMask
		| ButtonPressMask
		| ButtonReleaseMask
		| FocusChangeMask;

	// Select window events
	XSelectInput(display, window, event_mask);

	// Make window visible
	XMapWindow(display, window);

	// Set window title
	XStoreName(display, window, "XServer window");

	// Get WM_DELETE_WINDOW atom
	Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", True);

	// Subscribe WM_DELETE_WINDOW message
	XSetWMProtocols(display, window, &wm_delete, 1);

	// Создаем Graphics context
	GC gc = DefaultGC(display, screen);

	// Tell the GC we draw using the white color
	XSetForeground(display, gc, background_color);

	pthread_t nThreadID;
	nRet = pthread_create(&nThreadID, NULL, WorkThread, handle);
	if (nRet != 0)
	{
		printf("В процессе создания потока зазникла ошибка: %d\n", nRet);
		ReleaseResources();
		return -1;
	}

	// Event loop
	for (;;)
	{
		// Get events from event loop
		XEvent event;
		XNextEvent(display, &event);

		// Close button
		if (event.type == ClientMessage) {
			if (event.xclient.data.l[0] == wm_delete) {
				stopProcess = true;
				break;
			}
		}
	}

	// Ждем завершения и освобождаем дескриптор потока
	void* ret_val;
	if (pthread_join(nThreadID, &ret_val) != 0) {
		printf("Ошибка завершения потока\n");
	}

	// Останавливаем захват изображения
	//handle
	nRet = MV_CC_StopGrabbing(handle);
	if (MV_OK != nRet)
		printf("MV_CC_StopGrabbing fail! nRet [%x]\n", nRet);

	// Закраваем канал взаимодействия с камерой
	nRet = MV_CC_CloseDevice(handle);
	if (MV_OK != nRet)
		printf("MV_CC_CloseDevice fail! nRet [%x]\n", nRet);

	ReleaseResources();

	return 0;
}