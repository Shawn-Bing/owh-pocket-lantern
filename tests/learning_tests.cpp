#include "agent/learning.hpp"
#include <iostream>
#include <stdexcept>
using namespace owh;
void require(bool value,const char *message) { if (!value) throw std::runtime_error(message); }
void writeFixture(const std::filesystem::path &path,const std::string &bytes) {
  std::ofstream file(path,std::ios::binary|std::ios::trunc); file<<bytes; file.close();
  require(bool(file),"Write fixture");
}
int main(int argc,char **argv) {
  if (argc>1&&std::string(argv[1])=="--child-fixture") {
    int count{}; auto values=CommandLineToArgvW(GetCommandLineW(),&count);
    std::cout << (count>2 ? utf8(values[2]) : "missing") << std::endl;
    if (values) LocalFree(values);
    Sleep(30000); return 42;
  }
  try {
    const std::vector<std::wstring> arguments{L"",L"中文 路径",L"C:\\目录 with space\\",L"a\"b",L"\\\\\"",L"--model",L"$(literal);`literal`"};
    auto command=learning::commandLine(L"C:\\Program Files\\工具.exe",arguments);
    int count{}; auto parsed=CommandLineToArgvW(command.c_str(),&count);
    require(parsed&&count==int(arguments.size()+1),"Windows argument count");
    for (std::size_t i=0;i<arguments.size();++i)
      require(arguments[i]==parsed[i+1],"Windows quotes, Unicode and backslashes roundtrip without a shell");
    LocalFree(parsed);
    require(learning::localEndpoint(L"http://127.0.0.1:8080")&&learning::localEndpoint(L"http://localhost:8080/v1")&&
            learning::localEndpoint(L"http://[::1]:8080"),"Local model endpoints accepted");
    require(!learning::localEndpoint(L"https://remote.example/v1")&&
            !learning::localEndpoint(L"http://user:password@localhost:8080")&&
            !learning::localEndpoint(L"file:///C:/model"),"Nonlocal or credential-bearing endpoints rejected");
    const auto root=std::filesystem::current_path()/L"learning-test-data"/learning::uniqueName(L"test");
    std::filesystem::create_directories(root);
    const auto source=root/L"source.json", destination=root/L"cn-profile.json";
    CNProfile initial; std::string parseError;
    require(parseCNProfile(defaultCNProfileJson(),initial,parseError),"Default fixture parses");
    initial.name="original fixture"; writeFixture(destination,writeCNProfile(initial));
    auto replacement=initial; replacement.name="replacement fixture"; replacement.roomTarget=L"已标记的中文目标房间";
    writeFixture(source,writeCNProfile(replacement));
    std::wstring error; CNProfile imported;
    require(learning::importProfile(root.wstring(),source.wstring(),imported,error)&&imported.name==replacement.name&&imported.roomTarget==replacement.roomTarget,
            "A fully validated profile imports");
    unsigned backups{};
    for (const auto &file:std::filesystem::directory_iterator(root))
      if (file.path().filename().wstring().starts_with(L"cn-profile-backup-")) {
        std::string saved; CNProfile prior;
        require(learning::readBounded(file.path(),saved,error)&&parseCNProfile(saved,prior,parseError)&&prior.name==initial.name,
                "Previous profile is preserved in a unique backup"); ++backups;
      }
    require(backups==1,"Exactly one backup created for a successful replacement");
    std::string saved;
    require(learning::readBounded(destination,saved,error),"Read installed profile");
    writeFixture(source,"{invalid JSON");
    require(!learning::importProfile(root.wstring(),source.wstring(),imported,error),"Malformed profile rejected");
    std::string unchanged;
    require(learning::readBounded(destination,unchanged,error)&&unchanged==saved,"Rejected input preserves existing profile");
    writeFixture(source,std::string(learning::MaximumProfileBytes+1,' '));
    require(!learning::importProfile(root.wstring(),source.wstring(),imported,error),"Oversized profile rejected before parsing");
    require(learning::readBounded(destination,unchanged,error)&&unchanged==saved,"Oversized input preserves existing profile");
    replacement.name="blocked replacement"; writeFixture(source,writeCNProfile(replacement));
    {
      Handle lock(CreateFileW(destination.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
      require(lock.h!=INVALID_HANDLE_VALUE,"Lock destination against replacement");
      require(!learning::importProfile(root.wstring(),source.wstring(),imported,error),"Failed atomic replacement reported");
    }
    require(learning::readBounded(destination,unchanged,error)&&unchanged==saved,"Failed atomic replacement preserves original bytes");
    require(!learning::busy(),"Import never launches automation or a model process");
    // Own hidden window and child fixture only: no desktop input or game process.
    WNDCLASSEXW type{sizeof(type)}; type.lpfnWndProc=learning::procedure;
    type.hInstance=GetModuleHandleW(nullptr); type.lpszClassName=learning::WindowClass;
    require(RegisterClassExW(&type)!=0,"Register isolated learning window fixture");
    auto state=new learning::WindowState(); state->directory=root.wstring();
    HWND window=CreateWindowExW(WS_EX_NOACTIVATE,learning::WindowClass,L"Hidden learning fixture",WS_POPUP,
        -30000,-30000,780,644,nullptr,nullptr,type.hInstance,state);
    require(window!=nullptr,"Create learning controls without showing a window");
    learning::learningWindow=window;
    require(learning::isOpen(),"Open learning window blocks concurrent automation even before recording starts");
    require(state->text(learning::Endpoint)==L"http://127.0.0.1:8080", "UI defaults to the discovered local endpoint");
    const auto log=root/L"owned-child.log";
    const std::wstring payload=LR"(中文 路径\ with "quotes")";
    require(state->startChild(L"learning-tests.exe",{L"--child-fixture",payload},learning::ChildKind::Model,log,error),
            "Start only our fixed-location hidden child fixture");
    require(learning::busy(),"Child lifetime blocks automation");
    const auto began=GetTickCount64();
    while (GetTickCount64()-began<2000) {
      state->updateLog(); if (!state->lastLog.empty()) break; Sleep(10);
    }
    require(state->lastLog.find(payload)!=std::wstring::npos,"Owned child receives quoted Unicode arguments and writes its log");
    learning::cancel();
    MSG message{};
    while (PeekMessageW(&message,window,0,0,PM_REMOVE)) DispatchMessageW(&message);
    const auto cancelled=GetTickCount64();
    while (learning::busy()&&GetTickCount64()-cancelled<2000) { state->poll(); Sleep(10); }
    require(!learning::busy()&&state->child==learning::ChildKind::None,"F8 cancellation stops only the owned child and clears busy");
    SendMessageW(window,WM_CLOSE,0,0);
    require(!IsWindow(window)&&!learning::isOpen(),"Closing idle learning window releases controls and state");
    std::cout<<"Learning import backups, atomic failure preservation, local endpoint, Windows argv and owned-child cancellation tests passed\n";
    return 0;
  } catch (const std::exception &error) { std::cerr<<error.what()<<'\n'; return 1; }
}
