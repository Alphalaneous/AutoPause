#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(push, 0)
#include <cocos2d.h>
#pragma warning(pop)
#include <MinHook.h>
#include <gd.h>
#include <chrono>
#include <thread>
#include <random>

LONG_PTR oWindowProc;
bool newWindowProcSet = false;

LRESULT CALLBACK nWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_KILLFOCUS:
        
        gd::PlayLayer* playLayer = gd::PlayLayer::get();

        if (playLayer != nullptr) {
            playLayer->pauseGame(true);
        }

        break;
    }

    return CallWindowProc((WNDPROC)oWindowProc, hwnd, msg, wparam, lparam);
}

bool(__thiscall* MenuLayer_init)(gd::MenuLayer* self);

bool __fastcall MenuLayer_init_H(gd::MenuLayer* self, void*) {

    if (!MenuLayer_init(self)) return false;

    if (!newWindowProcSet) oWindowProc = SetWindowLongPtrA(GetForegroundWindow(), GWL_WNDPROC, (LONG_PTR)nWindowProc);
    newWindowProcSet = true;

    return true;
}


bool(__thiscall* AppDelegate_applicationDidEnterBackground)(cocos2d::CCSceneDelegate*);

bool __fastcall AppDelegate_applicationDidEnterBackground_H(cocos2d::CCSceneDelegate* self, void*) {

    if (!AppDelegate_applicationDidEnterBackground(self)) return false;

    return true;
}


DWORD WINAPI thread_func(void* hModule) {
    MH_Initialize();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(100, 2000);

    int random = distr(gen);

    std::this_thread::sleep_for(std::chrono::milliseconds(random));

    auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
    auto addr = GetProcAddress(GetModuleHandleA("libcocos2d.dll"), "?end@CCDirector@cocos2d@@QAEXXZ");
   
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x3CF40),
        AppDelegate_applicationDidEnterBackground_H,
        reinterpret_cast<void**>(&AppDelegate_applicationDidEnterBackground)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1907B0),
        MenuLayer_init_H,
        reinterpret_cast<void**>(&MenuLayer_init)
    );


    MH_EnableHook(MH_ALL_HOOKS);
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {

    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x100, thread_func, handle, 0, 0);
    }
    return TRUE;
}