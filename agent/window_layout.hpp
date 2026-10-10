#pragma once
#include <windows.h>
#include <commctrl.h>
#include <algorithm>
#include <vector>

namespace owh::ui {
// Presentation-only layout. Stored bounds use 96-DPI units; configuration and
// worker threads continue to use the existing control IDs and event handlers.
class WindowLayout {
public:
  static constexpr int BaseWidth=844, MinimumHeight=460, ContentTop=153,
                       ContentBottom=711, FooterHeight=112;
  enum class Band { Header, Content, Footer };
  enum class WidthRule { Fixed, Stretch, Right, ThirdFirst, ThirdMiddle, ThirdLast };
  struct Item { HWND window; int id,x,y,width,height; Band band; WidthRule rule; bool list; };
private:
  HWND root_{},pane_{};
  int dpi_=96,scroll_=0,maximum_=0,wheel_=0;
  std::vector<Item> items_;
  int scale(int value) const { return MulDiv(value,dpi_,96); }
public:
  void attach(HWND root,HWND pane,int dpi) { root_=root;pane_=pane;dpi_=dpi;scroll_=wheel_=0;items_.clear(); }
  void setDpi(int value) { dpi_=value;scroll_=0; }
  HWND pane() const { return pane_; }
  HWND find(int id) const {
    for (const auto &item:items_) if (item.id==id && id) return item.window;
    return nullptr;
  }
  const std::vector<Item> &items() const { return items_; }
  int position() const { return scroll_; }
  int maximum() const { return maximum_; }
  void remember(HWND window,int id,int x,int y,int width,int height,Band band,WidthRule rule,bool list=false) {
    items_.push_back({window,id,x,y,width,height,band,rule,list});
  }
  void arrangeTabOrder() {
    HWND previous=HWND_TOP;
    for (const auto &item:items_) if (item.band==Band::Header) {
      SetWindowPos(item.window,previous,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);previous=item.window;
    }
    SetWindowPos(pane_,previous,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);previous=pane_;
    for (const auto &item:items_) if (item.band==Band::Footer) {
      SetWindowPos(item.window,previous,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);previous=item.window;
    }
  }
  void resize() {
    if (!root_ || !pane_ || IsIconic(root_)) return;
    RECT client{};GetClientRect(root_,&client);
    const int paneHeight=std::max(1,int(client.bottom)-scale(ContentTop+FooterHeight));
    SetWindowPos(pane_,nullptr,0,scale(ContentTop),client.right,paneHeight,SWP_NOZORDER|SWP_NOACTIVATE);
    const int extra=std::max(0,paneHeight-scale(ContentBottom-ContentTop));
    const int contentHeight=scale(ContentBottom-ContentTop)+extra;
    maximum_=std::max(0,contentHeight-paneHeight);scroll_=std::clamp(scroll_,0,maximum_);
    SCROLLINFO info{sizeof(info),SIF_RANGE|SIF_PAGE|SIF_POS};
    info.nMax=contentHeight-1;info.nPage=UINT(paneHeight);info.nPos=scroll_;
    SetScrollInfo(pane_,SB_VERT,&info,TRUE);
    const int dx=client.right-scale(BaseWidth),dy=client.bottom-scale(826);
    struct Position { HWND window;int x,y,width,height; };
    std::vector<Position> positions;positions.reserve(items_.size());
    for (const auto &item:items_) {
      int x=scale(item.x),y=scale(item.y),width=scale(item.width),height=scale(item.height);
      switch (item.rule) {
      case WidthRule::Stretch: width+=dx;break;
      case WidthRule::Right: x+=dx;break;
      case WidthRule::ThirdFirst: width+=dx/3;break;
      case WidthRule::ThirdMiddle: x+=dx/3;width+=dx/3;break;
      case WidthRule::ThirdLast: x+=2*(dx/3);width+=dx-2*(dx/3);break;
      default:break;
      }
      if (item.band==Band::Content) {
        y-=scale(ContentTop)+scroll_;
        if (item.list) height+=extra;
        else if (item.y>=680) y+=extra;
      } else if (item.band==Band::Footer) y+=dy;
      positions.push_back({item.window,x,y,std::max(1,width),height});
    }
    // Win32 batches must share a parent. The fixed chrome and scrolling fields
    // therefore move in separate batches; fall back safely if allocation fails.
    for (const auto parent:{root_,pane_}) {
      auto batch=BeginDeferWindowPos(int(positions.size()));
      bool failed=!batch;
      for (const auto &position:positions) if (GetParent(position.window)==parent && batch) {
        batch=DeferWindowPos(batch,position.window,nullptr,position.x,position.y,position.width,position.height,
                             SWP_NOZORDER|SWP_NOACTIVATE);
        if (!batch) failed=true;
      }
      if (batch && !EndDeferWindowPos(batch)) failed=true;
      if (failed) for (const auto &position:positions) if (GetParent(position.window)==parent)
        SetWindowPos(position.window,nullptr,position.x,position.y,position.width,position.height,
                     SWP_NOZORDER|SWP_NOACTIVATE);
    }
    InvalidateRect(pane_,nullptr,TRUE);InvalidateRect(root_,nullptr,TRUE);
  }
  void scrollTo(int value) { scroll_=std::clamp(value,0,maximum_);resize(); }
  void scrollCommand(int command) {
    RECT client{};GetClientRect(pane_,&client);
    SCROLLINFO info{sizeof(info),SIF_TRACKPOS};GetScrollInfo(pane_,SB_VERT,&info);
    const int page=std::max(scale(28),int(client.bottom)-scale(28));
    switch (command) {
    case SB_TOP:scrollTo(0);break;
    case SB_BOTTOM:scrollTo(maximum_);break;
    case SB_LINEUP:scrollTo(scroll_-scale(28));break;
    case SB_LINEDOWN:scrollTo(scroll_+scale(28));break;
    case SB_PAGEUP:scrollTo(scroll_-page);break;
    case SB_PAGEDOWN:scrollTo(scroll_+page);break;
    case SB_THUMBTRACK:case SB_THUMBPOSITION:scrollTo(info.nTrackPos);break;
    }
  }
  void mouseWheel(int delta) {
    wheel_+=delta;const int ticks=wheel_/WHEEL_DELTA;wheel_%=WHEEL_DELTA;
    if (!ticks) return;
    UINT lines=3;SystemParametersInfoW(SPI_GETWHEELSCROLLLINES,0,&lines,0);
    RECT client{};GetClientRect(pane_,&client);
    const int distance=lines==WHEEL_PAGESCROLL ? int(client.bottom)-scale(28) : scale(18)*int(lines);
    scrollTo(scroll_-ticks*distance);
  }
  void reveal(HWND child) {
    if (!child || GetParent(child)!=pane_) return;
    RECT bounds{},client{};GetWindowRect(child,&bounds);MapWindowPoints(nullptr,pane_,reinterpret_cast<POINT*>(&bounds),2);
    GetClientRect(pane_,&client);
    if (bounds.top<0) scrollTo(scroll_+bounds.top-scale(4));
    else if (bounds.bottom>client.bottom) scrollTo(scroll_+bounds.bottom-client.bottom+scale(4));
  }
};
inline constexpr wchar_t PaneClass[]=L"OverwatchHelper.SettingsPane";
inline LRESULT CALLBACK focusSubclass(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data) {
  auto *layout=reinterpret_cast<WindowLayout*>(data);
  if (message==WM_SETFOCUS) layout->reveal(window);
  if (message==WM_NCDESTROY) RemoveWindowSubclass(window,focusSubclass,id);
  return DefSubclassProc(window,message,wp,lp);
}
inline LRESULT CALLBACK paneProc(HWND window,UINT message,WPARAM wp,LPARAM lp) {
  auto *layout=reinterpret_cast<WindowLayout*>(GetWindowLongPtrW(window,GWLP_USERDATA));
  if (message==WM_NCCREATE) {
    layout=static_cast<WindowLayout*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
    SetWindowLongPtrW(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(layout));
  }
  if (message==WM_COMMAND || message==WM_CTLCOLORSTATIC) return SendMessageW(GetParent(window),message,wp,lp);
  if (layout && message==WM_VSCROLL) { layout->scrollCommand(LOWORD(wp));return 0; }
  if (layout && message==WM_MOUSEWHEEL) { layout->mouseWheel(GET_WHEEL_DELTA_WPARAM(wp));return 0; }
  return DefWindowProcW(window,message,wp,lp);
}
} // namespace owh::ui
