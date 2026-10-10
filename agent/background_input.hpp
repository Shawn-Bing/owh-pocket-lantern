#pragma once
#include "agent/platform.hpp"

namespace owh {
// Reproduces the sample's 0x140001774 background branch: WM_SETFOCUS,
// mapped scan code, paired WM_KEYDOWN/UP, with no global input or activation.
// API success means queued messages, never proof that the game consumed them.
class BackgroundInput {
  std::mutex mutex_;
  Process target_{};
  std::wstring executable_;
  unsigned epoch_{};
  bool enabled_{};
  WORD held_{};
  bool mouse_{};
  DWORD error_{};
  bool validLocked() const {
    DWORD owner{};
    return target_.window && IsWindow(target_.window) &&
      GetWindowThreadProcessId(target_.window,&owner) && owner==target_.pid &&
      sameProcess(target_,executable_);
  }
  bool postLocked(WORD vk,bool up,bool mouse) {
    if (!validLocked()) { error_=ERROR_INVALID_WINDOW_HANDLE;return false; }
    UINT message{};WPARAM param{};LPARAM bits{};
    if (mouse) {
      const bool right=vk==VK_RBUTTON;
      message=up ? (right ? WM_RBUTTONUP : WM_LBUTTONUP) : (right ? WM_RBUTTONDOWN : WM_LBUTTONDOWN);
      param=up ? 0 : (right ? MK_RBUTTON : MK_LBUTTON);
    } else {
      message=up ? WM_KEYUP : WM_KEYDOWN;param=vk;
      bits=LPARAM(1u | (MapVirtualKeyW(vk,MAPVK_VK_TO_VSC)<<16) | (up ? 0xc0000000u : 0u));
    }
    SetLastError(0);
    if (PostMessageW(target_.window,message,param,bits)) return true;
    error_=GetLastError();return false;
  }
  bool releaseLocked() {
    if (!held_) return true;
    const bool ok=postLocked(held_,true,mouse_);
    held_=0;mouse_=false;return ok;
  }
public:
  ~BackgroundInput() { stop(); }
  static bool allowedKey(WORD vk) {
    return vk=='W'||vk=='S'||vk=='A'||vk=='D'||vk=='V'||vk=='E'||vk=='Q'||vk==VK_LSHIFT;
  }
  bool begin(const Process &target,const std::wstring &executable,unsigned epoch) {
    std::lock_guard lock(mutex_);enabled_=false;releaseLocked();
    target_=target;executable_=executable;epoch_=epoch;error_=0;
    enabled_=validLocked() && !outranks(target_);
    if (!enabled_) error_=ERROR_ACCESS_DENIED;
    return enabled_;
  }
  void stop() { std::lock_guard lock(mutex_);enabled_=false;releaseLocked(); }
  void release() { std::lock_guard lock(mutex_);releaseLocked(); }
  Process target() { std::lock_guard lock(mutex_);return target_; }
  DWORD error() { std::lock_guard lock(mutex_);return error_; }
  bool tap(WORD vk,Millis duration,unsigned epoch,bool mouse=false) {
    {
      std::lock_guard lock(mutex_);
      if (!enabled_ || epoch!=epoch_ || held_ || duration<10 || duration>500 ||
          (!mouse && !allowedKey(vk)) || (mouse && vk!=VK_LBUTTON && vk!=VK_RBUTTON) ||
          !interactiveDesktop() || foreground(target_.window) || !validLocked()) return false;
      DWORD_PTR result{};SetLastError(0);
      // The original synchronous call can hang. Bound it and never SetFocus,
      // SetForegroundWindow, attach threads, or restore the target here.
      if (!SendMessageTimeoutW(target_.window,WM_SETFOCUS,0,0,
                              SMTO_ABORTIFHUNG|SMTO_BLOCK,150,&result)) {
        error_=GetLastError();if (!error_) error_=ERROR_TIMEOUT;return false;
      }
      if (foreground(target_.window) || !interactiveDesktop()) return false;
      if (!postLocked(vk,false,mouse)) return false;
      held_=vk;mouse_=mouse;
    }
    const auto until=GetTickCount64()+duration;
    while (GetTickCount64()<until) {
      Sleep(5);std::lock_guard lock(mutex_);
      if (!enabled_ || epoch!=epoch_ || held_!=vk) return false;
      if (!interactiveDesktop() || foreground(target_.window) || !validLocked()) {releaseLocked();return false;}
    }
    std::lock_guard lock(mutex_);
    if (!enabled_ || epoch!=epoch_ || held_!=vk) return false;
    return releaseLocked();
  }
};
} // namespace owh
