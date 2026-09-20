#include "agent/platform.hpp"
#include "core/cn_recovery.hpp"
#include <gdiplus.h>
#include <iostream>

using namespace owh;
void require(bool ok, const char *message) {if(!ok) throw std::runtime_error(message);}
Frame load(const std::wstring &path, int cropTop) {
  Gdiplus::Bitmap image(path.c_str());
  require(image.GetLastStatus()==Gdiplus::Ok,"Cannot read replay image");
  Frame frame;frame.width=int(image.GetWidth());frame.height=int(image.GetHeight())-cropTop;
  require(frame.width>0 && frame.height>0,"Invalid replay image size");
  Gdiplus::Rect rect(0,cropTop,frame.width,frame.height);
  Gdiplus::BitmapData data{};
  require(image.LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&data)==Gdiplus::Ok,"Cannot lock replay pixels");
  frame.pixels.resize(std::size_t(frame.width)*frame.height);
  for(int y=0;y<frame.height;++y)
    std::memcpy(frame.pixels.data()+std::size_t(y)*frame.width,
                static_cast<const char *>(data.Scan0)+std::ptrdiff_t(y)*data.Stride,std::size_t(frame.width)*4);
  image.UnlockBits(&data);
  return frame;
}
int main(int argc,char **argv) {
  try {
    Frame empty, blank{960,540,std::vector<std::uint32_t>(960*540,0xff182348)};
    require(!recognizeCNGame(empty) && !recognizeCNGame(blank),"Unknown or blue-only screen must not restart");
    require(!recognizeCNLauncher(blank,1),"Blue-only launcher must not launch");
    require(!recognizeCNLauncher(blank,0),"Invalid DPI must not launch");
    if(argc==1) {std::cout<<"CN recovery negative cases passed\n";return 0;}
    require(argc==2,"Usage: cn-replay [manifest.tsv]");
    Gdiplus::GdiplusStartupInput input;ULONG_PTR token{};
    require(Gdiplus::GdiplusStartup(&token,&input,nullptr)==Gdiplus::Ok,"GDI+ startup");
    std::ifstream manifest(argv[1]);require(bool(manifest),"Cannot read replay manifest");
    unsigned rows{}, errors{}, mutations{};
    std::string line;
    while(std::getline(manifest,line)) {
      if(line.empty())continue;
      std::istringstream in(line);std::string expected,scope,top,scale,path;
      std::getline(in,expected,'\t');std::getline(in,scope,'\t');std::getline(in,top,'\t');
      std::getline(in,scale,'\t');std::getline(in,path);
      if(!path.empty() && path.back()=='\r')path.pop_back();
      auto frame=load(wide(path),std::stoi(top));
      auto result=scope=="launcher" ? recognizeCNLauncher(frame,std::stod(scale)) : recognizeCNGame(frame);
      bool accepted=false;
      std::istringstream allowed(expected);std::string page;
      while(std::getline(allowed,page,'|')) accepted=accepted || int(result.page)==std::stoi(page);
      if(!accepted) {
        ++errors;std::cout<<"Mismatch row="<<rows<<" expected="<<expected<<" got="<<int(result.page)<<" score="<<result.score<<"\n";
      }
      if(result.page==CNPage::LoginError) {
        // Keep the logo/blue background/button but erase the error message.
        for(int y=int(frame.height*.63);y<int(frame.height*.715);++y)
          for(int x=int(frame.width*.41);x<int(frame.width*.59);++x)
            frame.pixels[std::size_t(y)*frame.width+x]=0xff151828;
        require(recognizeCNGame(frame).page!=CNPage::LoginError,"Logo and login button without error text must not restart");
        ++mutations;
      }
      ++rows;
    }
    Gdiplus::GdiplusShutdown(token);
    std::cout<<"Replay rows="<<rows<<" mismatches="<<errors<<" error-text-negative-cases="<<mutations<<"\n";
    return errors?1:0;
  } catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
}
