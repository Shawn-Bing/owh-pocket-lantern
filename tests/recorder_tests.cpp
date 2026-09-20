#include "recorder/collectors.hpp"
#include <iostream>
void require(bool value,const char *message){if(!value)throw std::runtime_error(message);}
std::string read(const std::filesystem::path &p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
  using namespace owh;using namespace owh::rec;
  try {
    auto o=parse({L"recorder",L"--headless",L"--seconds",L"3",L"--no-screenshots",L"--no-source-logs",L"--output",L"test recording"});
    require(o.headless&&!o.screenshots&&!o.sources&&o.seconds==3&&o.output.is_absolute(),"CLI options");
    for(auto args:{std::vector<std::wstring>{L"r",L"--seconds",L"-1"},{L"r",L"--seconds",L"999999"},{L"r",L"--seconds",L"1abc"},{L"r",L"--output"},{L"r",L"--unknown"}}){
      bool failed=false;try{parse(args);}catch(...){failed=true;}require(failed,"Malformed options rejected");}
    auto root=sessionPath(std::filesystem::current_path()/L"recorder-test-data");
    Store limited(root/L"limited",5);
    require(limited.text("bounded.log","12345"),"Within byte limit");
    require(!limited.text("bounded.log","6")&&limited.capped,"Stop at capacity");
    require(read(root/L"limited"/L"bounded.log")=="12345","Capacity preserves earlier evidence");
    bool rejected=false;try{limited.text("../outside","x");}catch(...){rejected=true;}
    require(rejected,"Output cannot escape session");
    auto first=planTail(0,100000,false);
    require(first.length==65536&&first.offset==34464&&first.skipped==34464&&first.reset,"Initial tail is bounded and reports omission");
    auto append=planTail(100,150,true);
    require(append.offset==100&&append.length==50&&!append.reset,"Incremental append");
    require(planTail(100,20,true).reset,"Truncation restarts source");
    require(planTail(100,150,false).reset,"Rotation changes source identity");
    auto source=root/L"fixture.log";std::filesystem::create_directories(root);
    {std::ofstream f(source,std::ios::binary);f<<"alpha\n";}
    Store store(root/L"tail");Tailer tailer;tailer.read(store,source);
    {std::ofstream f(source,std::ios::binary|std::ios::app);f<<"beta\n";}
    tailer.read(store,source);tailer.read(store,source);
    require(read(store.directory/L"sources"/L"1.log")=="alpha\nbeta\n","Preserve tail without duplicating unchanged bytes");
    {std::ofstream f(source,std::ios::binary|std::ios::trunc);f<<"x\n";}
    tailer.read(store,source);
    require(read(store.directory/L"sources"/L"2.log")=="x\n","Truncation preserves old evidence separately");
    std::filesystem::rename(source,root/L"fixture.rotated.log");
    {std::ofstream f(source,std::ios::binary);f<<"new generation\n";}
    tailer.read(store,source);
    require(read(store.directory/L"sources"/L"3.log")=="new generation\n","File replacement does not skip new source prefix");
    ULONG_PTR graphics{};Gdiplus::GdiplusStartupInput input;
    require(Gdiplus::GdiplusStartup(&graphics,&input,nullptr)==Gdiplus::Ok,"Image encoder initialization");
    Frame image{100,50,std::vector<std::uint32_t>(5000,0x308060)};
    require(saveJpeg(store,image,"screens/fixture.jpg"),"Encode synthetic game image");
    Gdiplus::GdiplusShutdown(graphics);
    auto jpg=read(store.directory/L"screens"/L"fixture.jpg");
    require(jpg.size()>100&&static_cast<unsigned char>(jpg[0])==255&&static_cast<unsigned char>(jpg[1])==216,"Valid JPEG header");
    require(sourceRoots(true).empty(),"Demo diagnostics do not collect real game logs");
    std::cout<<"Recorder tests passed; no keys sent, game processes changed, or live screenshots taken\n";
    return 0;
  }catch(const std::exception &e){std::cerr<<e.what()<<"\n";return 1;}
}
