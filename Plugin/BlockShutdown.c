#include <windows.h>

#include <nsis/pluginapi.h>  // nsis plugin

HINSTANCE g_hInstance = 0;
HWND g_hwnd = 0;
HANDLE g_thread = INVALID_HANDLE_VALUE;
LPTSTR g_name = 0;
LPTSTR g_reason = 0;

UINT_PTR PluginCallback(enum NSPIM msg) {
  if (msg == NSPIM_UNLOAD) {
    PostMessage(g_hwnd, WM_USER, 0, 0);
    WaitForSingleObject(g_thread, 5 * 1000 /* ms */);
    CloseHandle(g_thread);
  }
  return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
  switch (uiMsg) {
    case WM_QUERYENDSESSION:
      return FALSE;
    case WM_USER: {
      DestroyWindow(hwnd);
      PostQuitMessage(0);
      return 0;
    }
  }
  return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

DWORD WINAPI ThreadProc(void* lParam) {
  LPCTSTR className = TEXT("NSISBlockShutdown");

  WNDCLASS wc = {.style = 0,
                 .lpfnWndProc = WndProc,
                 .hInstance = g_hInstance,
                 .lpszClassName = className};
  if (!RegisterClass(&wc)) {
    return 0;
  }

  g_hwnd = CreateWindow(className, g_name, WS_POPUP, 0, 0, 0, 0, 0, 0,
                        g_hInstance, 0);
  if (!g_hwnd) {
    return 0;
  }

  if (!ShutdownBlockReasonCreate(g_hwnd, g_reason)) {
    return 0;
  }

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnregisterClass(className, g_hInstance);

  LocalFree(g_name);
  LocalFree(g_reason);

  return 0;
}

void __declspec(dllexport) BlockShutdown(HWND hwndParent,
                                         int string_size,
                                         LPTSTR variables,
                                         stack_t** stacktop,
                                         extra_parameters* extra,
                                         ...) {
  EXDLL_INIT();

  if (g_thread != INVALID_HANDLE_VALUE) {
    return;
  }

  g_name = (LPTSTR)LocalAlloc(LPTR, (3 + string_size + 1) * sizeof(*g_name));
  g_reason =
      (LPTSTR)LocalAlloc(LPTR, (3 + string_size + 1) * sizeof(*g_reason));
  if (popstring(g_name) || popstring(g_reason)) {
    return;
  }

  extra->RegisterPluginCallback(g_hInstance, PluginCallback);

  g_thread = (HANDLE)CreateThread(NULL, 0, ThreadProc, 0, 0, 0);
}

BOOL WINAPI DllMain(HINSTANCE hInst,
                    ULONG ul_reason_for_call,
                    LPVOID lpReserved) {
  g_hInstance = hInst;
  return TRUE;
}
