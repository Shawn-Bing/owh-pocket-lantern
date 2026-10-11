#include "core/match_evidence.hpp"
#include <cstdlib>
#include <future>
#include <iostream>
#include <thread>
using namespace owh;
void require(bool ok, const char *why) {
  if (!ok) { std::cerr << "FAIL: " << why << '\n'; std::exit(1); }
}
Frame screen() { return {1280,720,std::vector<std::uint32_t>(1280*720,0xff202838)}; }
void paint(Frame &f, Rect roi, std::uint32_t color) {
  for (int y=int(roi.y*f.height); y<int((roi.y+roi.h)*f.height); ++y)
    for (int x=int(roi.x*f.width); x<int((roi.x+roi.w)*f.width); ++x)
      f.pixels[std::size_t(y)*f.width+x]=color;
}
Frame hud(bool scoreboard=false) {
  auto f=screen();
  paint(f,{.08,.85,.1,.006},0xfff4f4f4); paint(f,{.78,.85,.03,.055},0xfff4f4f4);
  if (scoreboard) {
    paint(f,{.365,.08,.27,.035},0xff168fc3); paint(f,{.365,.12,.27,.035},0xffb41c41);
  }
  return f;
}
OcrFrame words(std::initializer_list<OcrWord> list={}) {
  OcrFrame result{1280,720,{}};
  for (auto word:list) result.lines.push_back({{word}});
  return result;
}
const MatchWindow visible{false,false,true,false};
struct Fixture {
  MatchObserver monitor{"test-instance"};
  MatchTicket ticket;
  MatchProcess process{123,456};
  CNJoinGate join;
  CNRoomStart explicitStart;
  DeathmatchSession deathmatch;
  SemanticContext context;
  std::uint64_t sequence{};
  explicit Fixture(WorkflowMode mode=WorkflowMode::CustomRoom) {
    context.mode=mode;
    // Mirrors potentially stale automation trust: the independent adapter must
    // not let the recognizer's generic HUD output prove its own mode.
    context.trustedCustomRoom=context.trustedDeathmatch=true;
    ticket=monitor.begin(7,mode,100); monitor.selectProcess(ticket,process,100);
  }
  MatchEvidence evidence(const Frame &frame, const OcrFrame &ocr, Millis now) {
    auto semantic=recognizeSemantic(frame,ocr,context);
    return matchEvidence(frame,semantic,{},context.mode,process,join,explicitStart,deathmatch,now);
  }
  MatchSnapshot observe(const Frame &frame, const OcrFrame &ocr, Millis at, Millis delivered=0) {
    auto e=evidence(frame,ocr,delivered ? delivered : at);
    require(monitor.observe(ticket,process,at,++sequence,delivered ? delivered : at,visible,e),
            "Current independently sampled frame is accepted");
    if (context.mode==WorkflowMode::Deathmatch) deathmatch.observe(e.page,at);
    join.observe(e.page,at);
    return monitor.snapshot(delivered ? delivered : at);
  }
  void joined(Millis now=500) { join.acted(CNPage::Join,true,now); }
  void declared(Millis now=500) { explicitStart.begin(process.pid,process.created,now); }
  void inRoom() {
    joined(); require(observe(hud(),words(),1000).state==MatchState::Unverified,"First HUD waits");
    require(observe(hud(),words(),2000).state==MatchState::InMatch,"Two HUDs plus target join confirm");
  }
};
int main() {
  MatchObserver configured{"config-test",MatchObserverConfig{7000,9000,1000,20000}};
  require(configured.snapshot(100).evidenceTtlMs==7000 && configured.snapshot(100).processTtlMs==9000,
          "Snapshot exports central TTL configuration without transport constants");
  // 1. A real CNJoinGate request + two semantic HUDs; titles alone are lobbies.
  Fixture room; room.inRoom();
  require(room.monitor.snapshot(2000).source==MatchSource::TargetJoin,"Target join provenance survives join gate reset");
  auto named=words({{L"最稳轮换版",{.3,.1,.3,.06}},{L"玩家",{.2,.4,.1,.04}}});
  Fixture lobby;
  auto namedState=lobby.observe(screen(),named,1000);
  require(namedState.state==MatchState::OutOfMatch && namedState.source==MatchSource::TargetTitle,"Named target lobby is provenance, not a running match");
  lobby.observe(hud(),words(),2000);
  require(lobby.observe(hud(),words(),3000).state==MatchState::InMatch,"Named lobby provenance plus two live HUDs confirms");
  Fixture manual;
  Step ready; ready.action=Action::Ready; RoomSession automationRoom;
  automationRoom.observe(&ready,true);
  require(automationRoom.active(),"Legacy manual Ready still establishes automation room");
  require(manual.observe(hud(),words(),1000).state==MatchState::Unverified &&
          manual.observe(hud(),words(),2000).lastError=="mode_source_unverified","Manual Ready and inherited active cannot prove target mode");
  auto fakeBuiltin=CNObservation{}; fakeBuiltin.page=CNPage::Room; fakeBuiltin.step=ready;
  require(matchEvidence(hud(),{},fakeBuiltin,WorkflowMode::CustomRoom,manual.process,
      manual.join,manual.explicitStart,manual.deathmatch,3000).kind==MatchVisualKind::Unknown,"Arbitrary Ready template cannot supply trustworthy live HUD");

  // P2.7 uses the exact production ordering: observer before join reset.
  Fixture longRoom;longRoom.join.acted(CNPage::RoomList,true,500,Action::DoubleClick);
  auto steadyHud=hud();
  for(Millis at=1000;at<=602000;at+=1000){
    longRoom.monitor.selectProcess(longRoom.ticket,longRoom.process,at);
    auto result=longRoom.observe(steadyHud,words(),at);
    if(at>=2000)require(result.state==MatchState::InMatch&&result.source==MatchSource::TargetJoin&&result.lastConfirmedAt==at,"Ten minutes of fresh actual-order fixtures retain independent target source");
  }
  require(!longRoom.join.awaitingRoom(602000),"Join gate resets independently of retained observer source");
  Fixture manualRecover;
  manualRecover.observe(steadyHud,words(),1000);
  require(manualRecover.observe(steadyHud,words(),2000).lastError=="mode_source_unverified","Ordinary start and HUD alone are still unverified");
  manualRecover.declared(2500);
  require(manualRecover.observe(steadyHud,words(),3000).state==MatchState::Unverified&&
    manualRecover.observe(steadyHud,words(),4000).source==MatchSource::ExplicitStart&&
    manualRecover.monitor.snapshot(4000).state==MatchState::InMatch,"Explicit process-bound manual confirmation plus two fresh HUDs recovers");
  Fixture refreshOnly;refreshOnly.join.acted(CNPage::RoomList,true,500,Action::Click);
  refreshOnly.observe(steadyHud,words(),1000);
  require(refreshOnly.observe(steadyHud,words(),2000).state==MatchState::Unverified,"Refresh cannot become an automatic room source");
  // 2. Queue, requested backfill hero selection, explicit start, and scoreboard.
  auto queue=words({{L"正在搜索比赛",{.36,.1,.25,.04}},{L"预计时间",{.4,.18,.15,.03}},{L"自由死斗",{.35,.24,.2,.04}}});
  Fixture dm(WorkflowMode::Deathmatch);
  require(dm.observe(screen(),queue,1000).source==MatchSource::DeathmatchQueue,"Recognized deathmatch queue supplies mode");
  dm.observe(hud(),words(),2000);
  require(dm.observe(hud(),words(),3000).state==MatchState::InMatch,"Two combat observations after queue confirm");
  auto hero=words({{L"选择英雄",{.04,.2,.2,.05}}});
  Fixture backfill(WorkflowMode::Deathmatch); backfill.deathmatch.acted(CNPage::DeathmatchStart,true);
  require(backfill.observe(screen(),hero,1000).source==MatchSource::DeathmatchQueue,"Requested mode then hero supports backfill");
  backfill.observe(hud(),words(),2000);
  require(backfill.observe(hud(),words(),3000).state==MatchState::InMatch,"Backfill requires both HUD observations");
  Fixture arbitrary(WorkflowMode::Deathmatch);
  arbitrary.observe(hud(),words(),1000);
  require(arbitrary.deathmatch.trusted(),"Existing automation's Combat sets trusted");
  require(arbitrary.observe(hud(),words(),2000).lastError=="mode_source_unverified","Combat cannot self-authorize the observer through trusted()");
  Fixture ranked(WorkflowMode::Deathmatch);
  ranked.observe(hud(true),words(),1000);
  require(ranked.observe(hud(true),words(),2000).state==MatchState::InMatch,"Live individual scoreboard supplies independent mode proof");
  Fixture declared(WorkflowMode::Deathmatch); declared.declared();
  declared.observe(hud(),words(),1000);
  require(declared.observe(hud(),words(),2000).state==MatchState::InMatch,"Explicit identity-bound start still needs two HUDs");

  // 3. Loading and short black frames immediately remove green and break pairs.
  auto history=room.monitor.snapshot(2000).lastConfirmedAt;
  require(room.observe(screen(),words(),2500).state==MatchState::Unverified,"Loading/black frame is not in-match");
  require(room.monitor.snapshot(2500).lastConfirmedAt==history,"Unknown cannot refresh confirmation time");
  require(room.observe(hud(),words(),3000).state==MatchState::Unverified,"After loading one HUD is insufficient");
  require(room.observe(hud(),words(),4000).state==MatchState::InMatch,"Two fresh HUDs recover after short load");

  // 4. Definite menu/results immediately exit; a menu preserves mode briefly.
  auto menu=words({{L"选项菜单",{.45,.28,.15,.04}},{L"社交互动",{.47,.36,.1,.03}}});
  require(room.observe(hud(),menu,5000).state==MatchState::OutOfMatch,"Menu text defeats HUD-looking pixels");
  auto results=words({{L"比赛结束",{.3,.2,.3,.06}}});
  auto ended=dm.observe(screen(),results,4000);
  require(ended.state==MatchState::OutOfMatch && ended.source==MatchSource::None,"Results revokes mode provenance");
  dm.observe(hud(),words(),5000);
  require(dm.observe(hud(),words(),6000).state==MatchState::Unverified,"Old deathmatch cannot revive after results without new source");

  // 5. Login failure is a current, positive non-match page.
  auto login=words({{L"连接已断开",{.3,.3,.25,.03}}});
  auto disconnected=room.observe(screen(),login,6000);
  require(disconnected.state==MatchState::OutOfMatch && disconnected.source==MatchSource::None,"Login error clears match and source");

  // 6. Exit, restart, PID reuse, unavailable creation time, and late old identity.
  Fixture processes; processes.inRoom();
  auto oldId=processes.monitor.snapshot(2000).sessionId;
  processes.monitor.selectProcess(processes.ticket,{},3000);
  require(processes.monitor.snapshot(3000).state==MatchState::Offline,"Only confirmed game absence is OFFLINE");
  require(processes.monitor.snapshot(18000).state==MatchState::Offline,"Fresh process-absence boundary is inclusive");
  require(processes.monitor.snapshot(18001).state==MatchState::Unverified,"Blocked process scanner cannot claim OFFLINE forever");
  auto oldProcess=processes.process; processes.process.created++;
  processes.monitor.selectProcess(processes.ticket,processes.process,4000);
  require(processes.monitor.snapshot(4000).state==MatchState::Unverified &&
          processes.monitor.snapshot(4000).sessionId!=oldId,"PID reuse starts a new unverified session");
  auto oldEvidence=processes.evidence(hud(),words(),4100);
  require(!processes.monitor.observe(processes.ticket,oldProcess,4100,50,4100,visible,oldEvidence),"Delayed old process identity is rejected");
  processes.monitor.selectProcess(processes.ticket,{123,0},5000);
  require(processes.monitor.snapshot(5000).state==MatchState::Unverified,"PID without creation time cannot verify identity");

  Fixture failedProcess; failedProcess.inRoom();
  failedProcess.monitor.processUnverified(failedProcess.ticket,3000,"process_scan_failed");
  auto uncertainProcess=failedProcess.monitor.snapshot(3000);
  require(uncertainProcess.state==MatchState::Unverified && uncertainProcess.source==MatchSource::None &&
          uncertainProcess.processObservedAt==100 && uncertainProcess.lastConfirmedAt==2000,
          "Process scan failure revokes current authority without inventing fresh process or visual evidence");

  // 7. Minimize, capture failure, independent snapshot TTL, and source retention.
  Fixture minimized; minimized.inRoom();
  MatchWindow hidden{true,false,false,true};
  minimized.monitor.unavailable(minimized.ticket,minimized.process,3000,hidden,"minimized");
  auto unavailable=minimized.monitor.snapshot(3000);
  require(unavailable.state==MatchState::Unverified && unavailable.window.minimized &&
          unavailable.window.backgroundInput && unavailable.lastConfirmedAt==2000,"Minimizing immediately revokes green without changing historical evidence");
  minimized.monitor.unavailable(minimized.ticket,minimized.process,4000,visible,"bitblt_failed");
  require(minimized.monitor.snapshot(4000).lastError=="bitblt_failed","Capture failure is diagnostic, not evidence");
  Fixture ttl; ttl.inRoom();
  require(ttl.monitor.snapshot(17000).state==MatchState::InMatch,"15-second evidence boundary is inclusive");
  require(ttl.monitor.snapshot(17001).state==MatchState::Unverified,"Snapshot independently expires a blocked worker");
  require(ttl.monitor.snapshot(17001).source==MatchSource::TargetJoin,"Evidence TTL does not erase short-lived mode lineage");
  require(ttl.observe(hud(),words(),18000).state==MatchState::Unverified,"Expired evidence requires a new pair");
  require(ttl.observe(hud(),words(),19000).state==MatchState::InMatch,"Brief minimize can recover from retained provenance");
  require(ttl.monitor.snapshot(199001).source==MatchSource::None,"Mode lineage also expires after three unobserved minutes");
  require(ttl.observe(hud(),words(),200000).lastError=="mode_source_unverified","Long invisible interval cannot inherit active automation");
  ttl.declared(201000); ttl.observe(hud(),words(),201001);
  require(ttl.observe(hud(),words(),202001).state==MatchState::InMatch,"Explicit current-match start restores expired provenance");

  // 8. Unknown OCR and ambiguity break continuity; delayed and duplicate frames.
  Fixture unknown; unknown.joined(); unknown.observe(hud(),words(),1000);
  unknown.observe(screen(),words({{L"无法归类",{.2,.3,.2,.04}}}),1500);
  require(unknown.observe(hud(),words(),2000).state==MatchState::Unverified,"Unknown OCR breaks consecutive confirmation");
  require(unknown.observe(hud(),words(),3000).state==MatchState::InMatch,"Fresh independent observations re-establish match");
  Fixture duplicate; duplicate.declared();
  auto e=duplicate.evidence(hud(),words(),1000);
  require(duplicate.monitor.observe(duplicate.ticket,duplicate.process,1000,1,1000,visible,e),"First frame accepted");
  require(!duplicate.monitor.observe(duplicate.ticket,duplicate.process,1000,2,1001,visible,e),"Same capture time is not a new frame");
  require(!duplicate.monitor.observe(duplicate.ticket,duplicate.process,1100,1,1100,visible,e),"Same sequence is not a new frame");
  require(duplicate.monitor.snapshot(1100).state==MatchState::Unverified,"Duplicates cannot turn green");
  require(duplicate.monitor.observe(duplicate.ticket,duplicate.process,1200,3,17201,visible,e),"Late OCR is recorded with original capture age");
  require(duplicate.monitor.snapshot(17201).state==MatchState::Unverified &&
          duplicate.monitor.snapshot(17201).lastConfirmedAt==0,"OCR delay cannot refresh old pixels");
  auto ambiguous=matchEvidence(hud(),{}, {},WorkflowMode::CustomRoom,duplicate.process,
      duplicate.join,duplicate.explicitStart,duplicate.deathmatch,18000,true);
  require(ambiguous.kind==MatchVisualKind::Unknown,"Ambiguous templates cannot create evidence");

  // 9. Remote focus changes and covering windows do not confer gameplay proof.
  Fixture remote; remote.inRoom();
  MatchWindow covered{false,true,true,false};
  remote.monitor.unavailable(remote.ticket,remote.process,3000,covered,"occluded");
  require(remote.monitor.snapshot(3000).state==MatchState::Unverified && remote.monitor.snapshot(3000).window.occluded,"Overlay blocks observation even if game owns focus");
  MatchWindow unfocused{false,false,false,false};
  remote.monitor.unavailable(remote.ticket,remote.process,4000,unfocused,"not_foreground");
  require(remote.monitor.snapshot(4000).state==MatchState::Unverified,"Remote connection/focus loss cannot retain green");
  remote.monitor.automation(remote.ticket,AutomationState::ACTIVE);
  require(remote.monitor.snapshot(4000).state==MatchState::Unverified,"Automation ACTIVE is only an independent status field");

  // 10. Deterministic old worker return after stop/restart, including same-generation restart.
  Fixture race; race.inRoom(); auto stale=race.ticket;
  auto staleEvidence=race.evidence(hud(),words(),3000);
  std::promise<void> release; auto resumed=release.get_future();
  std::promise<bool> result; auto committed=result.get_future();
  std::thread late([&] { resumed.wait(); result.set_value(race.monitor.observe(stale,race.process,3000,50,5000,visible,staleEvidence)); });
  race.monitor.stop(8,3100);
  require(race.monitor.snapshot(3100).state==MatchState::Unverified &&
          race.monitor.snapshot(3100).automation==AutomationState::STOPPED,"Stop is unverified, not proof the game exited");
  auto restarted=race.monitor.begin(9,WorkflowMode::Deathmatch,4000);
  race.monitor.selectProcess(restarted,race.process,4000);
  release.set_value(); late.join();
  require(!committed.get(),"Late previous worker cannot commit into restarted run");
  require(!race.monitor.automation(stale,AutomationState::ACTIVE),"Old worker cannot overwrite snapshot automation");
  require(race.monitor.snapshot(5000).mode==WorkflowMode::Deathmatch &&
          race.monitor.snapshot(5000).state==MatchState::Unverified,"New mode/session remains untouched");
  auto manualRestart=race.monitor.begin(9,WorkflowMode::Deathmatch,6000);
  require(manualRestart!=restarted && !race.monitor.observe(restarted,race.process,6001,51,6001,visible,staleEvidence),"Manual restart revokes observation even without automation generation change");

  std::cout << "Match observer: ten deterministic scenario groups passed\n";
}
