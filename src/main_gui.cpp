#include "../resources/resource.h"
#include "modifier_key_fixer.h"
#include <Windows.h>
#include <shellapi.h>
#include <string>

// Application constants
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_APP_ICON 1001
#define ID_TRAY_EXIT 1002
#define ID_TRAY_PAUSE_RESUME 1003
#define ID_TRAY_SHOW_STATS 1004

// Global variables
HINSTANCE g_hInstance = nullptr;
NOTIFYICONDATA g_nid = {};
ModifierKeyFixer *g_pFixer = nullptr;
HWND g_hwnd = nullptr;
bool g_running = true;

// Function declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon();
void ShowContextMenu(HWND hwnd);
void ShowNotification(const char *title, const char *message);
void UpdateTrayTooltip();

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE:
    AddTrayIcon(hwnd);
    break;

  case WM_TRAYICON:
    if (lParam == WM_RBUTTONUP) {
      ShowContextMenu(hwnd);
    } else if (lParam == WM_LBUTTONDBLCLK) {
      // Double click - show statistics
      const auto &stats = g_pFixer->getStatistics();
      char buffer[256];
      sprintf_s(
          buffer, "Total Fixes: %d\nCtrl: %d | Shift: %d | Alt: %d | Win: %d",
          stats.totalFixes, stats.lctrlFixes + stats.rctrlFixes,
          stats.lshiftFixes + stats.rshiftFixes,
          stats.laltFixes + stats.raltFixes, stats.lwinFixes + stats.rwinFixes);
      MessageBoxA(nullptr, buffer, "Modifier Key Auto-Fix - Statistics",
                  MB_OK | MB_ICONINFORMATION);
    }
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case ID_TRAY_EXIT:
      g_running = false;
      PostQuitMessage(0);
      break;

    case ID_TRAY_PAUSE_RESUME:
      if (g_pFixer->isPaused()) {
        g_pFixer->resume();
        ShowNotification("Resumed", "Monitoring resumed");
      } else {
        g_pFixer->pause();
        ShowNotification("Paused", "Monitoring paused");
      }
      UpdateTrayTooltip();
      break;

    case ID_TRAY_SHOW_STATS: {
      const auto &stats = g_pFixer->getStatistics();
      char buffer[512];
      sprintf_s(buffer,
                "Total Fixes: %d\n\n"
                "Left Ctrl: %d\n"
                "Right Ctrl: %d\n"
                "Left Shift: %d\n"
                "Right Shift: %d\n"
                "Left Alt: %d\n"
                "Right Alt: %d\n"
                "Left Win: %d\n"
                "Right Win: %d",
                stats.totalFixes, stats.lctrlFixes, stats.rctrlFixes,
                stats.lshiftFixes, stats.rshiftFixes, stats.laltFixes,
                stats.raltFixes, stats.lwinFixes, stats.rwinFixes);
      MessageBoxA(nullptr, buffer, "Modifier Key Auto-Fix - Statistics",
                  MB_OK | MB_ICONINFORMATION);
      break;
    }
    }
    break;

  case WM_DESTROY:
    RemoveTrayIcon();
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

// Add tray icon
void AddTrayIcon(HWND hwnd) {
  memset(&g_nid, 0, sizeof(NOTIFYICONDATA));
  g_nid.cbSize = sizeof(NOTIFYICONDATA);
  g_nid.hWnd = hwnd;
  g_nid.uID = ID_TRAY_APP_ICON;
  g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  g_nid.uCallbackMessage = WM_TRAYICON;
  // Load custom icon from resources
  g_nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
  strcpy_s(g_nid.szTip, "Modifier Key Auto-Fix - Running");

  Shell_NotifyIcon(NIM_ADD, &g_nid);
}

// Remove tray icon
void RemoveTrayIcon() { Shell_NotifyIcon(NIM_DELETE, &g_nid); }

// Update tray tooltip
void UpdateTrayTooltip() {
  if (g_pFixer->isPaused()) {
    strcpy_s(g_nid.szTip, "Modifier Key Auto-Fix - Paused");
  } else {
    char buffer[128];
    sprintf_s(buffer, "Modifier Key Auto-Fix - Running (Fixes: %d)",
              g_pFixer->getStatistics().totalFixes);
    strcpy_s(g_nid.szTip, buffer);
  }
  Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

// Show context menu
void ShowContextMenu(HWND hwnd) {
  POINT pt;
  GetCursorPos(&pt);

  HMENU hMenu = CreatePopupMenu();

  if (g_pFixer->isPaused()) {
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_PAUSE_RESUME, "Resume Monitoring");
  } else {
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_PAUSE_RESUME, "Pause Monitoring");
  }

  AppendMenuA(hMenu, MF_STRING, ID_TRAY_SHOW_STATS, "Show Statistics");
  AppendMenuA(hMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit");

  SetForegroundWindow(hwnd);
  TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd,
                 nullptr);
  DestroyMenu(hMenu);
}

// Show notification
void ShowNotification(const char *title, const char *message) {
  g_nid.uFlags = NIF_INFO;
  strcpy_s(g_nid.szInfoTitle, title);
  strcpy_s(g_nid.szInfo, message);
  g_nid.dwInfoFlags = NIIF_INFO;
  g_nid.uTimeout = 3000;

  Shell_NotifyIcon(NIM_MODIFY, &g_nid);

  g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
}

// Worker thread for processing events
DWORD WINAPI WorkerThread(LPVOID lpParam) {
  int prevFixCount = 0;

  while (g_running) {
    g_pFixer->processEvents(50);

    // Check if a fix occurred
    int currentFixCount = g_pFixer->getStatistics().totalFixes;
    if (currentFixCount > prevFixCount) {
      int fixedCount = currentFixCount - prevFixCount;
      prevFixCount = currentFixCount;

      char message[128];
      sprintf_s(message, "Fixed %d stuck key(s)", fixedCount);
      ShowNotification("Auto-Fix", message);
      UpdateTrayTooltip();
    }
  }

  return 0;
}

// WinMain entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  // Check for single instance using Mutex
  HANDLE hMutex =
      CreateMutexA(nullptr, TRUE, "Global\\ModifierKeyAutoFix_SingleInstance");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    // Another instance is already running, exit silently
    if (hMutex) {
      CloseHandle(hMutex);
    }
    return 0;
  }

  g_hInstance = hInstance;

  // Create and initialize fixer
  ModifierKeyFixer fixer;
  g_pFixer = &fixer;

  if (!fixer.initialize()) {
    MessageBoxA(nullptr,
                "Failed to initialize Modifier Key Fixer.\n\n"
                "Please ensure:\n"
                "1. Interception driver is installed\n"
                "2. Running with administrator privileges",
                "Initialization Error", MB_OK | MB_ICONERROR);
    if (hMutex) {
      CloseHandle(hMutex);
    }
    return 1;
  }

  // Disable console messages for GUI mode
  fixer.setShowMessages(false);

  // Register window class
  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "ModifierKeyFixerClass";

  if (!RegisterClassEx(&wc)) {
    MessageBoxA(nullptr, "Failed to register window class", "Error",
                MB_OK | MB_ICONERROR);
    if (hMutex) {
      CloseHandle(hMutex);
    }
    return 1;
  }

  // Create hidden window
  g_hwnd = CreateWindowEx(0, "ModifierKeyFixerClass", "Modifier Key Auto-Fix",
                          0, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

  if (!g_hwnd) {
    MessageBoxA(nullptr, "Failed to create window", "Error",
                MB_OK | MB_ICONERROR);
    if (hMutex) {
      CloseHandle(hMutex);
    }
    return 1;
  }

  // Show startup notification
  ShowNotification("Started", "Modifier Key Auto-Fix is now running");

  // Create worker thread
  HANDLE hThread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);

  // Message loop
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Cleanup
  g_running = false;
  if (hThread) {
    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
  }

  // Release mutex
  if (hMutex) {
    CloseHandle(hMutex);
  }

  return (int)msg.wParam;
}
