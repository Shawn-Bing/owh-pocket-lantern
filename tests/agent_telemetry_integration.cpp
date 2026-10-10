// Includes the shipped application and runs its real telemetry loop. No worker,
// screenshot capture, input, sensor process, GUI, or real game is started.
#define wWinMain unusedApplicationEntry
#include "agent/main.cpp"
#undef wWinMain
#include <fstream>
#include <iostream>
int main(int argc,char **argv) {
  if(argc<4) {std::cerr<<"usage: agent-telemetry-integration URL TOKEN_FILE OUTPUT_DIRECTORY\n";return 2;}
  try {
    directory=std::filesystem::absolute(argv[3]).wstring();
    std::filesystem::create_directories(directory);
    config=loadConfig(directory,true);
    config.server=wide(argv[1]);
    std::ifstream tokenFile(argv[2]);std::string privateToken;std::getline(tokenFile,privateToken);
    if(privateToken.size()<32) throw std::runtime_error("missing isolated agent token");
    config.token=wide(privateToken);config.machine=L"p2-5-real-main-fixture";
    config.dryRun=true;config.autoRecover=false;config.holdShift=false;
    config.hardwareEnabled=true;int activeSeconds=4;
    for(int i=4;i<argc;++i) {if(std::string(argv[i])=="--hardware-off")config.hardwareEnabled=false;else if(std::string(argv[i])=="--active-seconds" && i+1<argc) activeSeconds=std::stoi(argv[++i]);else throw std::runtime_error("unknown fixture option");}
    if(activeSeconds<1 || activeSeconds>3600) throw std::runtime_error("invalid fixture duration");config.gameExe=L"p2-5-no-such-demo.exe";
    failures.open(directory);
    if(config.hardwareEnabled) hardwareReader.start(directory+L"\\hardware.json");
    matchObserver.stop(0,GetTickCount64());
    std::thread network(telemetry);
    auto wait=[] {std::this_thread::sleep_for(std::chrono::seconds(4));};
    wait();std::cout<<"actual telemetry initial stopped\n"<<std::flush;
    // Simulate a start ticket, without running the automation worker or UI.
    generation=1;running=true;
    matchObserver.begin(1,WorkflowMode::CustomRoom,GetTickCount64());
    publish(AutomationState::DETECTING,L"isolated no-input integration fixture");
    std::this_thread::sleep_for(std::chrono::seconds(activeSeconds));std::cout<<"actual telemetry running with failed hardware\n"<<std::flush;
    setRunning(false);
    wait();
    bool accepted;{std::lock_guard lock(statusMutex);accepted=status.http==200;}
    quitting=true;networkWake.notify_all();network.join();hardwareReader.stop();
    if(!accepted) throw std::runtime_error("actual telemetry HTTP not accepted");
    std::cout<<"actual stop telemetry HTTP 200; no worker or input executed\n";return 0;
  }catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
}
