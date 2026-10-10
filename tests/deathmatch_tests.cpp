#include "core/deathmatch.hpp"
#include <iostream>
#include <stdexcept>
using namespace owh;
void require(bool ok,const char *why) { if (!ok) throw std::runtime_error(why); }
int main() {
  DeathmatchSession game(42);
  game.acted(CNPage::DeathmatchStart,true);
  game.observe(CNPage::Hero,1000);
  require(game.trusted(),"Our start followed directly by hero selection establishes deathmatch");
  game.observe(CNPage::Combat,2000);
  require(!game.canInput(2000),"One combat frame cannot start keys");
  game.observe(CNPage::Combat,6500);
  require(game.canInput(6500),"Two current combat frames survive a realistic slow OCR interval");
  require(!game.canInput(8101),"A stale last combat frame cannot authorize input");
  auto stroke=game.next(6500);
  require(stroke.duration>=100 && stroke.duration<=500 && !game.canInput(6600),"Random key pacing remains bounded");
  require(game.confirmedCombat(6600),"A paced random-key gap does not hide current combat confirmation");
  require(stroke.rightClick==bool(stroke.duration&1),"Mouse side follows the reversed sample's hold-duration parity");
  bool sawLeft=false,sawRight=false;
  DeathmatchSession sequence(77);
  for (int i=0;i<128;++i) {
    auto next=sequence.next(10000+i*3000);
    require(next.rightClick==bool(next.duration&1),"Each key is paired with the correct mouse side");
    sawLeft|=!next.rightClick;sawRight|=next.rightClick;
  }
  require(sawLeft && sawRight,"Both mouse buttons participate in the random sequence");
  game.observe(CNPage::Respawn,7000);
  require(!game.canInput(7000) && game.trusted(),"Respawn stops random input and keeps the selected mode");
  game.observe(CNPage::Combat,7500);
  require(!game.canInput(7500),"Respawn needs fresh combat reconfirmation");
  game.observe(CNPage::Combat,9500);
  require(game.canInput(9500),"Fresh combat confirmation resumes after respawn");
  game.observe(CNPage::Menu,10000);
  require(!game.canInput(10000),"Menus pause random input immediately");
  game.observe(CNPage::Results,12000);
  require(game.finished() && !game.trusted() && !game.canInput(12000),"Results ends the prior input session");
  game.acted(CNPage::Results,true,Action::Click);
  game.observe(CNPage::Results,13000);
  game.observe(CNPage::Hero,14000);
  require(game.trusted() && !game.finished(),"Requeue also tolerates skipping the queue screen");
  game.observe(CNPage::Combat,15000); game.observe(CNPage::Combat,17000);
  require(game.canInput(17000),"A new match can begin after results");
  game.observe(CNPage::None,18000);
  require(!game.canInput(18000),"Unknown frames cannot retain key authorization");
  game.observe(CNPage::Combat,19000); game.observe(CNPage::Combat,30000);
  require(!game.canInput(30000),"Unobserved long gaps reset combat confirmation");
  game.reset(); game.observe(CNPage::Hero,31000);
  require(!game.trusted(),"Unrelated hero selection cannot establish selected mode");
  game.acted(CNPage::DeathmatchStart,false); game.observe(CNPage::Hero,32000);
  require(!game.trusted(),"A failed start cannot establish selected mode");
  game.acted(CNPage::DeathmatchStart,true);game.observe(CNPage::Home,33000);game.observe(CNPage::Hero,34000);
  require(!game.trusted(),"Returning home revokes pending mode identity");
  game.reset();game.observe(CNPage::Results,40000);
  require(game.finished() && !game.endorsementAttempted(),"A new result allows one endorsement opening");
  game.acted(CNPage::Results,false,Action::Endorse);
  require(!game.endorsementAttempted(),"A failed N press remains retryable");
  game.acted(CNPage::Results,true,Action::Endorse);game.observe(CNPage::Results,41000);
  require(game.endorsementAttempted(),"Residual results cannot reopen N after a successful press");
  game.observe(CNPage::Endorse,42000);game.acted(CNPage::Endorse,true);game.acted(CNPage::Endorse,true);
  require(game.endorsementClicks()==2 && !game.canInput(42000),"Endorsement counts selections and stops gameplay input");
  game.acted(CNPage::Results,true);game.observe(CNPage::Queue,43000);game.observe(CNPage::Hero,44000);
  require(game.trusted() && !game.finished(),"Post-endorsement requeue preserves the next hero transition");
  game.observe(CNPage::Combat,45000);game.observe(CNPage::Combat,46000);
  require(game.canInput(46000) && !game.endorsementAttempted() && game.endorsementClicks()==0,
          "Next-match combat resumes input and resets only the next result's endorsement budget");
  DeathmatchSession closing;
  closing.observe(CNPage::Endorse,50000);
  closing.acted(CNPage::Endorse,true,Action::Click,50100);
  require(!closing.endorsementCloseReady(50500),"First endorsement keeps the next card available");
  closing.acted(CNPage::Endorse,false,Action::Click,50200);
  require(closing.endorsementClicks()==1 && !closing.endorsementCloseReady(50600),
          "Failed card submission cannot finish endorsements");
  closing.acted(CNPage::Endorse,true,Action::Click,50300);
  require(!closing.endorsementCloseReady(50599) && closing.endorsementCloseReady(50600),
          "Second card schedules Escape after the UI settles");
  closing.observe(CNPage::None,50700);
  require(closing.endorsementCloseReady(50700),"Missing the changed Thanks title cannot strand the panel");
  closing.endorsementCloseAttempted(false,50700);
  require(!closing.endorsementCloseReady(51699) && closing.endorsementCloseReady(51700),
          "Failed Escape remains retryable with a one-second pause");
  closing.endorsementCloseAttempted(true,51700);
  require(!closing.endorsementCloseReady(53000),"Accepted Escape cannot toggle a following page repeatedly");
  auto armClose=[](DeathmatchSession &session,Millis now) {
    session.reset();session.observe(CNPage::Endorse,now);
    session.acted(CNPage::Endorse,true,Action::Click,now+100);
    session.acted(CNPage::Endorse,true,Action::Click,now+200);
  };
  armClose(closing,60000);closing.cancelEndorsementClose();
  require(!closing.endorsementCloseReady(61000),"Intervening manual input cancels the final automatic Escape");
  armClose(closing,70000);closing.observe(CNPage::Results,70400);
  require(!closing.endorsementCloseReady(70500),"A returned Results page needs no extra Escape");
  armClose(closing,80000);
  require(!closing.endorsementCloseReady(90201),"A stale endorsement transaction cannot press Escape later");
  armClose(closing,100000);
  for (int i=0;i<3;++i) {
    require(closing.endorsementCloseReady(100500+i*1000),"Escape retries remain available within their bound");
    closing.endorsementCloseAttempted(false,100500+i*1000);
  }
  require(!closing.endorsementCloseReady(104000),"Repeated Escape failures consume a finite retry budget");
  armClose(closing,110000);closing.acted(CNPage::Endorse,true,Action::Escape,110500);
  require(!closing.endorsementCloseReady(111000),"A recognized Escape also completes the pending transaction");
  std::cout << "Deathmatch queue, hero, combat, respawn, results and requeue regressions passed" << std::endl;
}
