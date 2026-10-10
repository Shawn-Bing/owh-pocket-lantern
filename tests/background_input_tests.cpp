#include "agent/background_input.hpp"
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace owh;
struct Event {UINT msg;WPARAM key;LPARAM bits;};
std::vector<Event> events;
HWND fixture{};
LRESULT CALLBACK receive(HWND w,UINT msg,WPARAM wp,LPARAM lp) {
  if (w==fixture && (msg==WM_SETFOCUS || msg==WM_KEYDOWN || msg==WM_KEYUP ||
      msg==WM_LBUTTONDOWN || msg==WM_LBUTTONUP || msg==WM_RBUTTONDOWN || msg==WM_RBUTTONUP)) {
    events.push_back({msg,wp,lp});return 0;
  }
  return DefWindowProcW(w,msg,wp,lp);
}
void require(bool ok,const char *why) {if(!ok) throw std::runtime_error(why);}
void pump() {MSG msg;while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
int main() {
 try {
  WNDCLASSW wc{};wc.lpfnWndProc=receive;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"OWH.BackgroundFixture";
  require(RegisterClassW(&wc)!=0,"Register isolated fixture");
  fixture=CreateWindowW(wc.lpszClassName,L"Hidden input fixture",WS_OVERLAPPEDWINDOW,-30000,-30000,100,100,nullptr,nullptr,wc.hInstance,nullptr);
  require(fixture!=nullptr,"Create fixture");
  auto originalFocus=GetForegroundWindow();
  Process own{GetCurrentProcessId(),processCreation(GetCurrentProcess()),fixture};
  auto name=processName(own.pid);BackgroundInput backend;
  require(backend.begin(own,name,7),"Bind PID, creation time, executable and window");events.clear();
  ShowWindow(fixture,SW_SHOWMINNOACTIVE);pump();events.clear();
  require(IsIconic(fixture) && GetForegroundWindow()==originalFocus,"Fixture is truly minimized without activation");
  require(!backend.tap(VK_LSHIFT,20,6),"Old generation sends nothing");
  require(!backend.tap(VK_RETURN,20,7),"Navigation keys are never background gameplay");
  require(!backend.tap('W',501,7),"Reject unbounded holds");pump();require(events.empty(),"Rejected calls send no messages");
  require(backend.tap(VK_LSHIFT,20,7),"Targeted Shift pair queues on hidden window");pump();
  require(events.size()==3 && events[0].msg==WM_SETFOCUS && events[1].msg==WM_KEYDOWN &&
      events[1].key==VK_LSHIFT && events[2].msg==WM_KEYUP,"Sample focus/down/up order and left Shift preserved");
  require((unsigned(events[1].bits)&0xffff)==1 && (unsigned(events[2].bits)&0xc0000000u)==0xc0000000u &&
      ((unsigned(events[1].bits)>>16)&255)==MapVirtualKeyW(VK_LSHIFT,MAPVK_VK_TO_VSC),"Scan code and release flags match sample");
  require(GetForegroundWindow()==originalFocus,"Synthetic focus message never steals OS foreground");
  require(IsIconic(fixture),"Background key messages never restore minimized window");
  events.clear();require(backend.tap(VK_RBUTTON,20,7,true),"Mouse pair targets same window");pump();
  require(events.size()==3 && events[1].msg==WM_RBUTTONDOWN && events[1].key==MK_RBUTTON &&
      events[2].msg==WM_RBUTTONUP && events[2].key==0,"Right mouse pair is complete");
  events.clear();bool result=true;
  std::thread tapping([&]{result=backend.tap('W',500,7);});
  const auto deadline=GetTickCount64()+1000;
  while(GetTickCount64()<deadline) {pump();if(events.size()>=2) break;Sleep(1);}
  backend.stop();tapping.join();pump();
  require(!result && events.size()==3 && events.back().msg==WM_KEYUP,"Stop cancels hold promptly and releases only target");
  events.clear();require(!backend.tap('Q',20,7),"Stopped backend cannot send");
  auto stale=own;++stale.created;
  require(!backend.begin(stale,name,8),"Creation-time mismatch rejects a reused process");
  stale=own;stale.pid+=100000;
  require(!backend.begin(stale,name,8),"Window owner mismatch rejected");pump();require(events.empty(),"Identity failures send nothing");
  require(backend.begin(own,name,9),"Original identity can be rebound");DestroyWindow(fixture);
  require(!backend.tap('A',20,9),"Destroyed target rejected");
  Input global;global.enable(true,10);global.blockForeground(true);
  require(!global.press(originalFocus,VK_RETURN,10) && !global.pulse(originalFocus,10) &&
      !global.click(originalFocus,{.5,.5},10) && !global.text(originalFocus,L"never",10),
      "Foreground-use mode closes global input even on actual foreground window");
  global.enable(false,11);
  std::cout<<"Background messages, identity, paired release, cancellation and foreground isolation passed; no game or global input sent\n";return 0;
 } catch(const std::exception &e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
