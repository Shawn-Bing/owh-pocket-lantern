#include "core/semantic.hpp"
#include <cstdlib>
#include <iostream>
using namespace owh;
void require(bool value,const char *message) {
  if (!value) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
OcrFrame words(std::initializer_list<OcrWord> list,int width = 1280,int height = 720) {
  OcrFrame result{width,height,{}};
  for (auto word : list) result.lines.push_back({{word}});
  return result;
}
Frame screen() { return {1280,720,std::vector<std::uint32_t>(1280*720,0xff202838)}; }
void paint(Frame &frame,Rect roi,std::uint32_t color) {
  for (int y = int(roi.y*frame.height); y < int((roi.y+roi.h)*frame.height); ++y)
    for (int x = int(roi.x*frame.width); x < int((roi.x+roi.w)*frame.width); ++x)
      frame.pixels[std::size_t(y)*frame.width+x] = color;
}
int main() {
  auto frame = screen();
  SemanticContext launcher{true}, custom, death{false,WorkflowMode::Deathmatch};
  auto play = words({{L"守望先锋",{.2,.2,.2,.07}},{L"进入游戏",{.08,.82,.15,.05}}});
  paint(frame,{.06,.80,.2,.1},0xff007be8);
  auto result = recognizeSemantic(frame,play,launcher);
  require(result.observation.page == CNPage::Launch && result.observation.step.action == Action::Launch,
          "Launcher text and enabled button launch despite changed geometry");
  require(std::abs(result.observation.step.click.y-.845) < .001,"Actual OCR button coordinates are used");
  paint(frame,{.06,.80,.2,.1},0xff555555);
  require(recognizeSemantic(frame,play,launcher).observation.step.action == Action::Wait,"Disabled Play is not clicked");
  auto update = words({{L"守望先锋",{.2,.2,.2,.07}},{L"更新",{.08,.82,.15,.05}}});
  require(recognizeSemantic(frame,update,launcher).observation.step.action == Action::Wait,"Update is never mistaken for Play");
  require(!recognizeSemantic(frame,words({{L"进入游戏",{.08,.82,.15,.05}}}),launcher),"A button without game branding is not launched");
  auto sidebar = words({{L"守望先锋",{.02,.2,.14,.025}},{L"进入游戏",{.08,.82,.15,.05}}});
  paint(frame,{.06,.80,.2,.1},0xff007be8);
  require(!recognizeSemantic(frame,sidebar,launcher),"An unselected library label cannot authorize launching another game");
  auto largeSidebar = words({{L"守望先锋",{.02,.25,.2,.06}},{L"进入游戏",{.08,.82,.15,.05}}});
  require(!recognizeSemantic(frame,largeSidebar,launcher),"Larger rail fonts still cannot establish the main product");
  sidebar.lines.push_back({{{L"我的游戏",{.03,.1,.13,.03}}}});
  result = recognizeSemantic(frame,sidebar,launcher);
  require(result.observation.page == CNPage::LauncherSelect && std::abs(result.observation.step.click.y-.2125)<.001,
          "Library navigation clicks the Overwatch entry itself");
  auto password = words({{L"密码",{.25,.4,.1,.03}},{L"登录",{.45,.7,.1,.04}}});
  require(recognizeSemantic(frame,password,launcher).observation.step.action == Action::Ignore,"Account forms never receive input");
  require(recognizeSemantic(frame,password,custom).observation.step.action==Action::Restart,
          "A game login form restarts the verified game instead of waiting for account entry");
  require(recognizeSemantic(frame,password,death).observation.page==CNPage::LoginError,
          "Deathmatch also recovers from the game login form");
  require(!recognizeSemantic(frame,words({{L"密码",{.25,.4,.1,.03}}}),custom),
          "An isolated password word cannot establish a game login form");
  auto accountLogin=words({{L"电子邮箱",{.25,.4,.1,.03}},{L"登录",{.45,.7,.1,.04}}});
  require(recognizeSemantic(frame,accountLogin,custom).observation.step.action==Action::Restart,
          "Account and Login identify the game's saved-session failure even without password OCR");
  CNObservation oldLogin;oldLogin.page=CNPage::Login;oldLogin.score=.99;oldLogin.step.action=Action::Ignore;
  require(normalizeGameLoginRecovery(oldLogin,false).step.action==Action::Restart,
          "Saved old login calibration cannot keep the game idle");
  require(normalizeGameLoginRecovery(oldLogin,true).step.action==Action::Ignore,
          "Battle.net account login stays manual");
  ContinuousEvidence loginEvidence{300000};loginEvidence.observe(true,1000);loginEvidence.observe(true,4000);
  require(loginEvidence.elapsed(4000,GameLoginRecoveryDelay),"Confirmed game login recovers after three seconds rather than ninety");
  auto reconnect = words({{L"连接已断开",{.3,.3,.25,.03}},{L"重新连接",{.42,.6,.16,.04}}});
  require(recognizeSemantic(frame,reconnect,launcher).observation.step.action == Action::Click,"Saved session reconnect is actionable");
  auto home = words({{L"开始",{.45,.02,.07,.035}},{L"社交",{.6,.02,.08,.035}},{L"战令",{.75,.02,.08,.035}}});
  require(recognizeSemantic(frame,home,custom).observation.page == CNPage::Home,"Several home markers do not create false ambiguity");
  auto menu = words({{L"选项菜单",{.45,.28,.15,.04}},{L"社交互动",{.47,.36,.1,.03}},
                     {L"生涯概况",{.47,.47,.1,.03}},{L"离开比赛",{.47,.64,.1,.03}}});
  require(recognizeSemantic(frame,menu,custom).observation.step.action == Action::Escape,"New version menu closes by text");
  auto settings = words({{L"音频",{.28,.04,.03,.02}},{L"图像质量",{.09,.1,.05,.02}},
                         {L"全部恢复默认设置",{.82,.93,.1,.025}}});
  require(recognizeSemantic(frame,settings,custom).observation.step.action == Action::Escape,
          "Small-window settings tabs and default label provide a safe return path");
  require(recognizeSemantic(frame,menu,death).observation.page == CNPage::Menu,"A live match is not left from its menu");
  death.finished = true;
  require(recognizeSemantic(frame,menu,death).observation.page == CNPage::Leave,"Only a finished deathmatch can be left");
  auto confirmLeave = words({{L"确定要离开比赛？",{.3,.4,.3,.04}},{L"确定",{.4,.6,.06,.04}},{L"取消",{.55,.6,.06,.04}}});
  require(!recognizeSemantic(frame,confirmLeave,death),"A dialog alone cannot authorize leaving");
  death.leaving = true;
  require(recognizeSemantic(frame,confirmLeave,death).observation.page == CNPage::ConfirmLeave,"Our leave action authorizes its confirmation");
  death.finished = death.leaving = false;
  auto queue = words({{L"正在搜索比赛",{.36,.1,.25,.04}},{L"预计时间",{.4,.18,.15,.03}},{L"自由死斗",{.35,.24,.2,.04}}});
  require(recognizeSemantic(frame,queue,death).observation.page == CNPage::Queue,"Queue waits and does not cancel matchmaking");
  auto hero = words({{L"选择英雄",{.04,.2,.2,.05}},{L"选择",{.45,.86,.08,.04}}});
  paint(frame,{.42,.83,.14,.1},0xfff87820);
  require(recognizeSemantic(frame,hero,death).observation.page == CNPage::Hero,"Hero selection is handled before random input");
  require(recognizeSemantic(frame,hero,custom).observation.page==CNPage::Hero,
          "Custom-room hero selection also precedes room input");
  auto compactHero=words({{L"英信息",{.02,.95,.08,.02}},{L"选择",{.485,.90,.03,.03}}});
  require(recognizeSemantic(frame,compactHero,death).observation.page==CNPage::Hero &&
          recognizeSemantic(frame,compactHero,custom).observation.step.action==Action::Click,
          "Hero footer plus enabled selection handles a missing title and missing queue context");
  require(!recognizeSemantic(frame,words({{L"选择",{.485,.90,.03,.03}}}),death),
          "A selection button alone does not identify hero selection");
  require(!recognizeSemantic(frame,words({{L"英信息",{.02,.95,.08,.02}},{L"装备",{.485,.90,.03,.03}}}),death),
          "Hero gallery equipment is not a game selection screen");
  auto rosterFrame=screen();
  auto paintRoster=[](Frame &target,int rows) {
    for (int row=0;row<rows;++row) for (int tile=0;tile<20;++tile) {
      const double x=.08+tile*.04,y=.74+row*.058;
      paint(target,{x,y,.027,.002},0xffeeeeee);
      paint(target,{x,y+.050,.027,.002},0xffeeeeee);
    }
  };
  paintRoster(rosterFrame,2);paint(rosterFrame,{.47,.89,.06,.055},0xfff87820);
  auto blurredHero=words({{L"回聊夭英息",{.02,.95,.13,.02}},{L"继续",{.488,.901,.024,.021}}});
  auto rosterHero=recognizeSemantic(rosterFrame,blurredHero,death);
  require(rosterHero.observation.page==CNPage::Hero && rosterHero.heroRoster &&
          rosterHero.observation.step.action==Action::Click && rosterHero.observation.step.click.x>.48 &&
          rosterHero.observation.step.click.x<.52,"Two portrait rows recover Continue despite unreadable footer without queue trust");
  require(recognizeSemantic(rosterFrame,blurredHero,custom).observation.page==CNPage::Hero,
          "Custom rooms use the same portrait proof");
  require(!recognizeSemantic(screen(),blurredHero,death),"Unreadable footer plus Continue alone is not a hero screen");
  auto oneRow=screen();paintRoster(oneRow,1);paint(oneRow,{.47,.89,.06,.055},0xfff87820);
  require(!recognizeSemantic(oneRow,blurredHero,death),"A single portrait row cannot prove hero selection");
  auto disabledRoster=rosterFrame;paint(disabledRoster,{.47,.89,.06,.055},0xff555555);
  require(!recognizeSemantic(disabledRoster,blurredHero,death),"Portraits do not authorize a disabled Continue button");
  require(!recognizeSemantic(rosterFrame,words({{L"装备",{.488,.901,.024,.021}}}),death),
          "Portrait grids with equipment controls remain outside game selection");
  auto footerContinue=compactHero;footerContinue.lines.back().words.front().text=L"继续";
  require(recognizeSemantic(frame,footerContinue,death).observation.page==CNPage::Hero,
          "Legible hero information also supports Continue");
  Frame largeRoster{1920,1080,std::vector<std::uint32_t>(1920*1080,0xff202838)};
  paintRoster(largeRoster,2);paint(largeRoster,{.47,.89,.06,.055},0xfff87820);
  auto largeWords=blurredHero;largeWords.width=1920;largeWords.height=1080;
  require(recognizeSemantic(largeRoster,largeWords,death).observation.page==CNPage::Hero,
          "Portrait geometry scales to 1080p client dimensions");
  auto details = words({{L"自由死斗",{.06,.2,.2,.06}},{L"开始",{.45,.86,.08,.04}}});
  require(recognizeSemantic(frame,details,death).observation.page == CNPage::DeathmatchStart,"Mode details start a new queue");
  auto modernMore=words({{L"更多模式",{.805,.125,.056,.023}},
                         {L"自定游戏",{.145,.637,.082,.035}},
                         {L"休闲比赛",{.11,.125,.053,.022}},
                         {L"街机先锋",{.62,.125,.06,.022}}});
  auto deathRoute=recognizeSemantic(frame,modernMore,death);
  require(deathRoute.observation.page==CNPage::Play && deathRoute.observation.step.click.x>.6 &&
          deathRoute.observation.step.click.x<.72,"Deathmatch follows the confirmed Arcade tab, never the Unranked tab");
  require(recognizeSemantic(frame,modernMore,custom).observation.page==CNPage::More,"Custom routing remains separate");
  modernMore.lines.back().words[0].text=L"街机先";
  require(recognizeSemantic(frame,modernMore,death).observation.step.action==Action::Click,"Truncated Arcade text can recover with mode-menu evidence");
  require(!recognizeSemantic(frame,words({{L"街枳先",{.62,.125,.06,.022}}}),death),"Partial Arcade text alone cannot authorize navigation");
  auto smallWindow=words({{L"休闲比不",{.075,.19,.16,.063}},
                          {L"&机先",{.61,.116,.035,.019}}});
  const auto compactRoute=recognizeSemantic(frame,smallWindow,death);
  require(compactRoute.observation.page==CNPage::Play && compactRoute.observation.step.action==Action::Click &&
          compactRoute.observation.step.click.x>.6 && compactRoute.observation.step.click.x<.7,
          "1080p window OCR keeps the Arcade route when its icon obscures the first glyph");
  require(!recognizeSemantic(frame,words({{L"机先",{.61,.116,.035,.019}}}),death),
          "An isolated truncated Arcade label cannot authorize a click");
  require(!recognizeSemantic(frame,words({{L"休闲比不",{.075,.19,.16,.063}},
                                        {L"机先",{.61,.5,.035,.019}}}),death),
          "A truncated label outside the navigation strip cannot authorize a tab click");
  modernMore.lines.pop_back();
  require(recognizeSemantic(frame,modernMore,death).observation.step.action==Action::Wait,"Missing Arcade cannot fall back to Unranked or More");
  auto detailedWords=modernMore;detailedWords.lines.push_back({{{L"街机先锋",{.62,.125,.06,.022}}}});
  auto recovered=recognizeSemanticDetail(frame,modernMore,detailedWords,death,recognizeSemantic(frame,modernMore,death));
  require(recovered.observation.step.action==Action::Click && recovered.observation.step.click.x>.6,"Detailed OCR recovers Arcade");
  auto arcadePage=words({{L"街机先锋",{.04,.2,.2,.06}},{L"街机先锋",{.62,.125,.06,.022}},
                        {L"自由死斗",{.55,.62,.18,.045}},{L"更多模式",{.805,.125,.056,.023}}});
  require(recognizeSemantic(frame,arcadePage,death).observation.page==CNPage::More &&
          recognizeSemantic(frame,arcadePage,death).observation.step.click.y>.5,"The Deathmatch card wins over its persistent Arcade tab");
  arcadePage.lines.erase(arcadePage.lines.begin()+2);
  require(recognizeSemantic(frame,arcadePage,death).observation.step.action==Action::Wait,"Already-open Arcade waits for the mode card instead of repeated tab clicks");
  auto upperArcade=words({{L"街机先锋",{.04,.13,.20,.06}},
                         {L"街机先锋",{.62,.125,.06,.022}},
                         {L"自由死斗",{.55,.62,.18,.045}}});
  const auto directCard=recognizeSemantic(frame,upperArcade,death);
  require(directCard.observation.page==CNPage::More && directCard.observation.step.action==Action::Click &&
          directCard.observation.step.click.y>.5,"Arcade title plus its tab do not make the Deathmatch card ambiguous without More Modes");
  upperArcade.lines.pop_back();
  require(recognizeSemantic(frame,upperArcade,death).observation.step.action==Action::Wait,
          "An upper-left Arcade title is an open page, never another entrance to click");
  require(recognizeSemantic(frame,words({{L"街机先锋",{.04,.13,.20,.06}}}),death).observation.step.action==Action::Wait,
          "An Arcade heading with no card waits instead of clicking the heading repeatedly");
  auto ordinaryArcade=words({{L"街机先锋",{.62,.125,.06,.022}}});
  require(recognizeSemantic(frame,ordinaryArcade,death).observation.step.action==Action::Wait,
          "A persistent Arcade tab alone cannot authorize repeat clicks while card text is missing");
  require(recognizeSemantic(frame,words({{L"街机先锋",{.62,.60,.12,.04}}}),death).observation.step.action==Action::Click,
          "A body Arcade entrance card remains clickable when the mode menu is compact");
  auto accumulatedArcade=ordinaryArcade;
  auto detailedArcade=words({{L"街机先锋",{.04,.2,.2,.06}},
                            {L"自由死斗",{.55,.62,.18,.045}}});
  auto nextArcade=recognizeSemanticSupplement(frame,accumulatedArcade,detailedArcade,death,
                                             recognizeSemantic(frame,ordinaryArcade,death));
  auto tabCrop=words({{L"街机先锋",{.62,.125,.06,.022}}});
  nextArcade=recognizeSemanticSupplement(frame,accumulatedArcade,tabCrop,death,nextArcade);
  require(nextArcade.observation.page==CNPage::More && nextArcade.observation.step.action==Action::Click &&
          nextArcade.observation.step.click.y>.5,"A later navigation crop preserves the detailed Deathmatch card");
  accumulatedArcade=ordinaryArcade;detailedArcade.lines.pop_back();
  nextArcade=recognizeSemanticSupplement(frame,accumulatedArcade,detailedArcade,death,
                                        recognizeSemantic(frame,ordinaryArcade,death));
  nextArcade=recognizeSemanticSupplement(frame,accumulatedArcade,tabCrop,death,nextArcade);
  require(nextArcade.observation.page==CNPage::More && nextArcade.observation.step.action==Action::Wait,
          "A later navigation crop preserves the already-open Arcade title instead of clicking again");
  auto heroConfirm=words({{L"请选择英雄",{.04,.2,.2,.05}},{L"空格 确认选择",{.45,.86,.18,.04}}});
  paint(frame,{.4,.82,.3,.12},0xff202838);
  require(recognizeSemantic(frame,heroConfirm,death).observation.step.action==Action::Enter,"Disabled confirmation first selects the highlighted hero");
  paint(frame,{.4,.82,.3,.12},0xfff87820);
  require(recognizeSemantic(frame,heroConfirm,death).observation.step.action==Action::Click,"Enabled confirmation is clicked before gameplay input");
  death.endorsementAttempted=true;
  auto ended = words({{L"比赛结束",{.35,.14,.3,.06}},{L"重新匹配",{.4,.86,.2,.04}}});
  paint(frame,{.38,.83,.24,.1},0xfff87820);
  require(recognizeSemantic(frame,ended,death).observation.step.action == Action::Click,"Results requeue using the real button");
  death.endorsementAttempted=false;
  auto resultScreen=words({{L"总览奖励",{.01,.02,.08,.03}},
                           {L"再次誹队",{.40,.9,.08,.03}},{L"继续",{.54,.9,.06,.03}}});
  auto resultFrame=screen();paint(resultFrame,{.37,.88,.14,.07},0xffffffff);
  paint(resultFrame,{.53,.88,.10,.07},0xfff87820);
  require(recognizeSemantic(resultFrame,resultScreen,death).observation.step.action==Action::Endorse,
          "Results overview opens endorsement before requeue without a remembered end banner");
  death.endorsementAttempted=true;
  auto nextQueue=recognizeSemantic(resultFrame,resultScreen,death);
  require(nextQueue.observation.page==CNPage::Results && nextQueue.observation.step.action==Action::Click &&
          nextQueue.observation.step.click.x<.51,"The white Again Queue button works after endorsement");
  paint(resultFrame,{.37,.88,.14,.07},0xff555555);
  require(recognizeSemantic(resultFrame,resultScreen,death).observation.step.action==Action::Wait,
          "Disabled requeue remains a wait rather than a click");
  require(!recognizeSemantic(frame,words({{L"再次誹队",{.40,.9,.08,.03}}}),death),
          "Truncated requeue alone cannot establish results");
  auto endorseFrame=screen();paint(endorseFrame,{.25,.81,.15,.08},0xffffffff);
  paint(endorseFrame,{.44,.81,.15,.08},0xffffffff);paint(endorseFrame,{.46,.855,.10,.025},0xffff5b10);
  auto endorseWords=words({{L"赞赏一名对手",{.42,.74,.16,.04}}});
  auto endorsementOptions=endorsementCards(endorseFrame);
  require(endorsementOptions.size()==1 && endorsementOptions.front().x<.40,"Already-endorsed orange cards are excluded");
  auto endorseAction=recognizeSemantic(endorseFrame,endorseWords,death);
  require(endorseAction.observation.page==CNPage::Endorse && endorseAction.observation.step.action==Action::Click &&
          endorseAction.observation.step.click.x<.40,"The remaining player's actual nameplate bounds supply the click");
  auto thanksWords=words({{L"感谢赞赏！",{.44,.74,.13,.04}}});
  require(recognizeSemantic(endorseFrame,thanksWords,death).observation.step.action==Action::Escape &&
          recognizeSemantic(endorseFrame,thanksWords,death).observation.page==CNPage::Endorse,
          "The zero-remaining thank-you page closes even when clickable-looking portraits remain");
  death.endorsementClicks=2;
  require(recognizeSemantic(endorseFrame,endorseWords,death).observation.step.action==Action::Escape,
          "After two clicks the endorsement dialog closes even if it still displays a card");
  require(recognizeSemantic(screen(),endorseWords,death).observation.step.action==Action::Escape,
          "No eligible cards cannot strand requeue");
  death.endorsementAttempted=false;death.endorsementClicks=0;
  auto browse = words({{L"浏览游戏",{.06,.1,.2,.05}},{L"最稳轮换版",{.14,.62,.2,.04}},{L"自动加入",{.48,.86,.13,.04}}});
  paint(frame,{.45,.83,.2,.1},0xfff87820);
  result = recognizeSemantic(frame,browse,custom);
  require(result.observation.page == CNPage::Browser && result.observation.step.action==Action::DoubleClick && result.observation.hasCard,"Target card double click does not require a readable Browse footer");
  require(std::abs(result.observation.card.y-.64) < .001,"Target card may move between versions");
  auto mergedJoin = browse;
  mergedJoin.lines.pop_back();
  mergedJoin.lines.push_back({{{L"浏览游戏",{.29,.86,.15,.04}},
                              {L"自动",{.48,.86,.065,.04}},{L"加入",{.545,.86,.065,.04}}}});
  auto mergedResult = recognizeSemantic(frame,mergedJoin,custom);
  require(mergedResult.observation.page == CNPage::Browser && mergedResult.observation.step.action == Action::DoubleClick,
          "The browser footer opens the target mode by double click");
  require(mergedResult.observation.hasCard &&
          std::abs(mergedResult.observation.step.click.x-mergedResult.observation.card.x)<.001 &&
          std::abs(mergedResult.observation.step.click.y-mergedResult.observation.card.y)<.001,
          "The target card, rather than the Auto Join button, receives the double click");
  paint(frame,{.45,.83,.2,.1},0xff555555);
  require(recognizeSemantic(frame,mergedJoin,custom).observation.step.action == Action::DoubleClick,
          "A disabled Auto Join button does not block direct target browsing");
  paint(frame,{.45,.83,.2,.1},0xfff87820);
  auto reversedAliases = UiText(mergedJoin).one({L"加入",L"自动加入"},{.15,.74,.8,.25});
  require(reversedAliases && std::abs(reversedAliases->roi.w-.13)<.001,
          "Nested alias deduplication preserves the complete label regardless of alias order");
  auto twoButtons = words({});
  twoButtons.lines.push_back({{{L"自动加入",{.48,.86,.016,.04}},{L"加入游戏",{.503,.86,.016,.04}}}});
  require(!UiText(twoButtons).one({L"自动加入",L"加入游戏",L"加入"},{.15,.74,.8,.25}),
          "Separate neighboring buttons remain ambiguous even when their centers are close");
  auto noHeading = words({{L"筛选",{.08,.20,.05,.035}},{L"人气",{.06,.035,.06,.03}},
                          {L"托比昂战令最稳轮换版",{.14,.62,.28,.04}},{L"自动加入",{.48,.86,.13,.04}}});
  auto fallbackBrowser = recognizeSemantic(frame,noHeading,custom);
  require(fallbackBrowser.observation.page == CNPage::Browser && fallbackBrowser.observation.step.action==Action::DoubleClick && fallbackBrowser.evidence.size() >= 3,
          "Filter and category controls confirm a browser without its large heading and both remain evidence");
  auto onlyFilter = noHeading; onlyFilter.lines.erase(onlyFilter.lines.begin()+1);
  require(!recognizeSemantic(frame,onlyFilter,custom),"A filter control alone cannot establish the browser");
  auto onlyCategory = noHeading; onlyCategory.lines.erase(onlyCategory.lines.begin());
  require(!recognizeSemantic(frame,onlyCategory,custom),"A category control alone cannot establish the browser");
  auto onlyRoomAndJoin = words({{L"托比昂战令最稳轮换版",{.14,.62,.28,.04}},
                               {L"自动加入",{.48,.86,.13,.04}}});
  require(!recognizeSemantic(frame,onlyRoomAndJoin,custom),"A room title and join button alone cannot establish the browser");
  require(recognizeSemantic(frame,browse,death).observation.step.action == Action::Escape,"Deathmatch cannot join a custom lobby");
  auto duplicate = browse; duplicate.lines.push_back({{{L"最稳轮换版",{.55,.5,.2,.04}}}});
  auto instances = recognizeSemantic(frame,duplicate,custom);
  require(instances.observation.step.action == Action::DoubleClick && instances.observation.cards.size() == 2,
          "Two instances of the same full mode title are selectable");
  require(std::abs(instances.observation.card.y-.52)<.001,"Instance ordering is stable from top to bottom");
  auto differentModes = words({{L"浏览游戏",{.06,.1,.2,.05}},
                              {L"托比昂战令最稳轮换版",{.14,.62,.2,.04}},
                              {L"其他游戏最稳轮换版",{.55,.5,.2,.04}}});
  require(recognizeSemantic(frame,differentModes,custom).observation.step.action == Action::Wait,
          "A shared search phrase cannot mix distinct full mode titles");
  differentModes.lines.push_back({{{L"最稳轮换版",{.72,.3,.2,.04}}}});
  require(recognizeSemantic(frame,differentModes,custom).observation.cards.size() == 1,
          "An exact full title takes priority over longer substring matches");
  auto description = words({{L"浏览游戏",{.06,.1,.2,.05}},
                            {L"最稳轮换版请进QQ群",{.2,.65,.2,.015}}});
  require(!recognizeSemantic(frame,description,custom).observation.hasCard,
          "Small description text is not mistaken for a card title");
  auto ordinaryBrowserWords = words({{L"浏览游戏",{.06,.1,.2,.05}},
                                    {L"托比昂战令最稳轮换版",{.14,.62,.2,.04}}});
  auto ordinaryBrowser = recognizeSemantic(frame,ordinaryBrowserWords,custom);
  auto displacedHeading = words({{L"浏览游戏",{.09,.1,.2,.05}}});
  require(ordinaryBrowser.observation.page == CNPage::Browser &&
          !recognizeSemantic(frame,words({{L"浏览游戏",{.06,.1,.2,.05}},
                                         {L"浏览游戏",{.09,.1,.2,.05}},
                                         {L"最稳轮换版",{.14,.62,.2,.04}}}),custom),
          "Displaced OCR duplicates can invalidate a combined browser heading");
  require(recognizeSemanticDetail(frame,ordinaryBrowserWords,displacedHeading,custom,ordinaryBrowser)
              .observation.page == CNPage::Browser,
          "Failed high-detail merging preserves an actionable ordinary browser");
  auto newMode = words({{L"另一个最稳轮换版",{.55,.5,.2,.04}}});
  require(recognizeSemanticDetail(frame,ordinaryBrowserWords,newMode,custom,ordinaryBrowser)
              .observation.step.action == Action::Wait,
          "High-detail evidence of different full modes retains the ambiguity wait");
  require(recognizeSemanticDetail(frame,words({}),ordinaryBrowserWords,custom,{})
              .observation.page == CNPage::Browser,
          "High detail can recover a page absent from the ordinary pass");
  auto chat = words({{L"比赛结束",{.1,.94,.2,.02}},{L"死斗",{.1,.91,.05,.02}}});
  require(!recognizeSemantic(frame,chat,death),"Chat messages cannot fake a result or a HUD");
  auto hudText = words({{L"自由死斗",{.4,.04,.2,.04}}});
  require(!recognizeSemantic(frame,hudText,death),"Mode title without a HUD cannot start random input");
  paint(frame,{.08,.85,.1,.006},0xfff4f4f4);
  paint(frame,{.78,.85,.03,.055},0xfff4f4f4);
  require(recognizeSemantic(frame,hudText,death).observation.page == CNPage::Combat,"HUD shape and deathmatch text together establish combat");
  auto noTitle = words({});
  require(!recognizeSemantic(frame,noTitle,death),"A generic HUD without a confirmed deathmatch cannot start keys");
  require(!recognizeSemantic(frame,noTitle,custom),"A generic HUD cannot establish the selected custom room");
  custom.trustedCustomRoom = true;
  auto liveRoom = recognizeSemantic(frame,noTitle,custom);
  require(liveRoom.observation.page == CNPage::Room && liveRoom.observation.step.action == Action::Ready && liveRoom.liveRoomHud,
          "A verified target join plus the live HUD establishes the custom room");
  require(!learnSemantic(frame,liveRoom,custom),"A dynamic room HUD never becomes a persistent target-room template");
  auto loadingFrame = screen();
  auto loading = words({{L"地图工坊岛屿夜间",{.47,.81,.42,.08}}});
  require(!recognizeSemantic(loadingFrame,loading,custom) && !recognizeSemantic(loadingFrame,noTitle,custom),
          "Map titles and blank loading frames do not confirm joining even during a pending target join");
  require(recognizeSemantic(frame,menu,custom).observation.page == CNPage::Menu,
          "A menu over a trusted live HUD still ends room observation");
  require(recognizeSemantic(frame,home,custom).observation.page == CNPage::Home,
          "Home text must override HUD-like menu art even when the custom room is trusted");
  require(recognizeSemantic(frame,settings,custom).observation.page == CNPage::Menu,
          "Settings text must override HUD-like controls while target-room trust is pending");
  auto modesWithHud = words({{L"更多模式",{.5,.2,.2,.04}},{L"游戏模式",{.08,.1,.2,.04}}});
  require(recognizeSemantic(frame,modesWithHud,custom).observation.page == CNPage::Play,
          "Mode-selection text must override a generic trusted HUD shape");
  auto customModesWithHud = words({{L"更多模式",{.5,.2,.2,.04}},{L"自定游戏",{.3,.4,.2,.04}}});
  require(recognizeSemantic(frame,customModesWithHud,custom).observation.page == CNPage::More,
          "The custom-game entry must override a generic trusted HUD shape");
  auto emptyBrowserWithHud = words({{L"浏览游戏",{.06,.1,.2,.05}}});
  require(recognizeSemantic(frame,emptyBrowserWithHud,custom).observation.page == CNPage::Browser,
          "An empty browser must remain a browser while pending room trust and HUD-like art coexist");
  require(recognizeSemantic(frame,browse,custom).observation.page == CNPage::Browser,
          "A target room card and join button must remain actionable before the trusted HUD fallback");
  custom.trustedCustomRoom = false;
  death.trustedDeathmatch = true;
  require(recognizeSemantic(frame,noTitle,death).observation.page == CNPage::Combat,"Confirmed deathmatch queue supports an unlabeled live HUD");
  auto respawn = words({{L"按空格键重生",{.38,.7,.23,.05}}});
  require(recognizeSemantic(frame,respawn,death).observation.step.action == Action::Space,"Trusted deathmatch accepts combined respawn prompts");
  death.trustedDeathmatch = false;
  require(!recognizeSemantic(frame,respawn,death),"A respawn prompt alone cannot establish the selected mode");
  Frame scoreboardFrame=frame;
  paint(scoreboardFrame,{.365,.08,.27,.035},0xff168fc3);
  paint(scoreboardFrame,{.365,.12,.27,.035},0xffb41c41);
  require(deathmatchScoreboard(scoreboardFrame),"Stacked individual blue and red ranking bands identify the current deathmatch layout");
  Frame secondPlace=frame;paint(secondPlace,{.365,.08,.27,.035},0xffb41c41);paint(secondPlace,{.365,.12,.27,.035},0xff168fc3);
  require(deathmatchScoreboard(secondPlace),"Individual ranking evidence survives the player moving below an opponent");
  auto directCombat=recognizeSemantic(scoreboardFrame,noTitle,death);
  require(directCombat.observation.page==CNPage::Combat && directCombat.verifiedDeathmatch,"Existing deathmatch can be recognized without a remembered queue");
  require(recognizeSemantic(scoreboardFrame,home,death).observation.page==CNPage::Home,"Menu text wins over ranking-like artwork");
  require(recognizeSemantic(scoreboardFrame,heroConfirm,death).observation.page==CNPage::Hero,"Hero confirmation completes before any combat input");
  Frame leaderboardOnly=screen();paint(leaderboardOnly,{.365,.08,.27,.035},0xff168fc3);paint(leaderboardOnly,{.365,.12,.27,.035},0xffb41c41);
  require(!recognizeSemantic(leaderboardOnly,noTitle,death),"Leaderboard requires a real health and ability HUD");
  Frame oneBand=frame;paint(oneBand,{.365,.08,.27,.035},0xff168fc3);
  require(!deathmatchScoreboard(oneBand),"One colored title cannot identify deathmatch");
  Frame sideBySide=frame;paint(sideBySide,{.24,.08,.2,.035},0xff168fc3);paint(sideBySide,{.53,.08,.2,.035},0xffb41c41);
  require(!deathmatchScoreboard(sideBySide),"Side-by-side team scores are not individual rankings");
  DeathmatchSession session(42);
  session.observe(CNPage::Combat,1000);
  require(!session.canInput(1000),"One frame cannot start game keys");
  session.observe(CNPage::Combat,2000);
  require(session.canInput(2000),"Two current combat frames permit game keys");
  auto stroke = session.next(2000);
  require(stroke.duration >= 100 && stroke.duration <= 500 && !session.canInput(2100),"Random hold duration and pacing are bounded");
  session.observe(CNPage::None,2300);
  require(!session.canInput(2400),"Unknown frames stop random input immediately");
  session.observe(CNPage::Results,3000);
  require(session.finished() && !session.canInput(3000),"Results stop input and arm requeue");
  session.acted(CNPage::Leave,true);
  require(session.leaving(),"Leave confirmation needs our preceding action");
  session.acted(CNPage::ConfirmLeave,true);
  require(session.leaving(),"A sent confirmation remains retryable until the dialog actually closes");
  session.observe(CNPage::Queue,4000);
  require(!session.finished() && !session.canInput(4000),"New queue clears the previous match");
  require(!session.trusted(),"An arbitrary queue alone does not authorize random input");
  session.acted(CNPage::DeathmatchStart,true); session.observe(CNPage::Queue,4100);
  require(session.trusted(),"Our deathmatch request and observed queue establish the mode");
  session.observe(CNPage::Results,4200); session.acted(CNPage::Results,true,Action::Click);
  session.observe(CNPage::Results,4300); session.observe(CNPage::Queue,4400);
  require(session.trusted(),"Requeue requests survive residual results and an unlabeled next queue");
  session.observe(CNPage::Results,4500); session.acted(CNPage::Results,true,Action::Escape);
  session.observe(CNPage::Queue,4600);
  require(!session.trusted(),"Escaping results does not itself establish a new matchmaking request");
  session.observe(CNPage::Combat,5000); session.observe(CNPage::Combat,16000);
  require(!session.canInput(16000),"A long gap in observation requires new confirmation");
  session.reset(); require(!session.finished() && !session.trusted() && !session.canInput(10000),"Stop and process replacement reset the session");
  CNJoinGate join;
  join.acted(CNPage::Browser,true,1000); join.acted(CNPage::Join,true,1100);
  require(!join.observe(CNPage::None,1300).joined && !join.bursting(1300),"Loading cancels the burst without claiming success");
  require(join.awaitingRoom(70000),"A verified target request survives the observed 69-second Workshop load");
  require(!join.awaitingRoom(1000) && !join.awaitingRoom(181101),"Pending room trust is time-bounded and rejects clock rollback");
  require(!join.observe(CNPage::Menu,1500).joined && !join.awaitingRoom(1600),
          "An error/menu transition clears the pending target request");
  CNJoinGate slowJoin;
  slowJoin.acted(CNPage::Browser,true,1000); slowJoin.acted(CNPage::Join,true,2000);
  slowJoin.observe(CNPage::None,9000); slowJoin.observe(CNPage::None,40000);
  require(slowJoin.awaitingRoom(71000) && !slowJoin.observe(CNPage::Room,71000).joined &&
          slowJoin.observe(CNPage::Room,73000).joined && !slowJoin.awaitingRoom(74000),
          "Only two positive HUD observations after a long load confirm joining and consume pending trust");
  slowJoin.acted(CNPage::Browser,true,100000); slowJoin.acted(CNPage::Join,false,101000);
  require(!slowJoin.awaitingRoom(102000),"A failed join click cannot authorize a generic HUD");
  for (Millis t = 10000; t < 35000; t += 5000) {
    join.acted(CNPage::Browser,true,t); join.acted(CNPage::Join,true,t);
    join.observe(CNPage::Join,t+5000);
  }
  require(join.gaveUp() && join.retryAt() == 50000,"Full rooms get a bounded burst and a pause");
  CNObservation selected; selected.page = CNPage::Join; selected.step.action = Action::Click;
  require(join.prepare(selected,36000,{.2,.6}).step.action == Action::Wait,"Pause cannot click a full room");
  require(join.prepare(selected,50000,{.2,.6}).page == CNPage::Browser && !join.gaveUp(),"Automatic retry reselects the target after backoff");
  CNJoinGate rotating;
  CNObservation options = selected; options.hasCard = true; options.cards = {{.45,.33},{.15,.62}};
  rotating.acted(CNPage::Browser,true,1000); rotating.acted(CNPage::Join,true,2000);
  rotating.observe(CNPage::Join,7000);
  auto nextRoom = rotating.prepare(options,7000,options.cards.front());
  require(nextRoom.page == CNPage::Browser && std::abs(nextRoom.step.click.y-.62)<.001,
          "A failed burst reselects the next same-mode instance");
  require(parseRoomCapacity(L"１１ ／ １２")->players==11 &&
          parseRoomCapacity(L" 0 / 12 ")->maximum==12,"Capacity parsing preserves slash and fullwidth digits");
  for (auto bad:{L"112",L"11的2",L"13/12",L"1/0",L"1/129",L"1/2/3",L"-1/12",L"1O/12"})
    require(!parseRoomCapacity(bad),"Unreadable or impossible capacities are never selectable");
  auto roomList=words({{L"托比昂战令最稳轮换版",{.30,.09,.42,.08}},
      {L"当前地图",{.49,.24,.07,.02}},{L"创建新的大厅",{.37,.85,.09,.03}},
      {L"托比昂战令最稳轮换版",{.19,.32,.12,.018}},{L"12/12",{.77,.32,.035,.018}},
      {L"托比昂战令最稳轮换版",{.19,.36,.12,.018}},{L"11/12",{.77,.36,.035,.018}},
      {L"托比昂战令最稳轮换版",{.19,.40,.12,.018}},{L"9／12",{.77,.40,.035,.018}},
      {L"托比昂战令最稳轮换版",{.19,.44,.12,.018}},{L"112",{.77,.44,.035,.018}}});
  auto available=recognizeSemantic(frame,roomList,custom);
  require(available.observation.page==CNPage::RoomList && available.observation.step.action==Action::DoubleClick &&
          available.observation.cards.size()==2 && std::abs(available.observation.step.click.y-.409)<.001,
          "Room list skips full/unknown rows and prioritizes the room with most vacant slots");
  auto anotherMode=custom;anotherMode.target=L"其他模式";
  require(recognizeSemantic(frame,roomList,anotherMode).observation.step.action==Action::Wait,"A different mode title cannot authorize room entry");
  auto noHeader=roomList;noHeader.lines.erase(noHeader.lines.begin()+1);
  require(recognizeSemantic(frame,noHeader,custom).observation.page==CNPage::RoomList &&
          recognizeSemantic(frame,noHeader,custom).observation.step.action==Action::Wait,
          "Incomplete list OCR waits and cannot be mistaken for a joined lobby");
  auto fullList=roomList;
  for (auto &line:fullList.lines) for (auto &word:line.words)
    if (word.roi.x>.7) word.text=L"12/12";
  require(recognizeSemantic(frame,fullList,custom).observation.step.action==Action::Wait,
          "All full rooms wait without clicking");
  auto conflictingList=roomList;conflictingList.lines.push_back({{{L"12/12",{.77,.40,.035,.018}}}});
  auto unambiguous=recognizeSemantic(frame,conflictingList,custom);
  require(unambiguous.observation.cards.size()==1 && std::abs(unambiguous.observation.step.click.y-.369)<.001,
          "Conflicting OCR counts invalidate only their own row");
  auto orphanCount=words({{L"11/12",{.77,.60,.035,.018}}});
  require(vacantRoomRows(orphanCount).empty(),"A capacity without a matching row title cannot be clicked");
  auto splitCount=words({{L"托比昂战令最稳轮换版",{.19,.40,.12,.018}}});
  splitCount.lines.push_back({{{L"11",{.77,.40,.015,.018}},{L"/",{.785,.40,.005,.018}},{L"12",{.79,.40,.015,.018}}}});
  require(vacantRoomRows(splitCount,L"托比昂战令最稳轮换版").size()==1,"Capacity split into OCR words remains readable");
  // P2.7: row geometry is independent of OCR Line boundaries and pixels.
  auto splitLines=words({{L"托比昂战令最稳轮换版",{.19,.28,.12,.018}},
    {L"１１",{.77,.28,.015,.018}},{L"／",{.785,.28,.005,.018}},{L"１２",{.79,.28,.015,.018}},
    {L"托比昂战令最稳轮换版",{.19,.79,.12,.018}},{L"11 / 12",{.77,.79,.035,.018}},
    {L"另一个最稳轮换版",{.19,.5,.12,.018}},{L"0/12",{.77,.5,.035,.018}},
    {L"0/12",{.51,.28,.035,.018}}});
  for(auto size:{Point{640,360},Point{1024,576},Point{1920,1080}}){
    splitLines.width=int(size.x);splitLines.height=int(size.y);
    auto rows=vacantRoomRows(splitLines,L"托比昂战令最稳轮换版");
    require(rows.size()==2 && center(rows.front().title).y<.30 && center(rows.back().title).y>.78,
      "First/last rows and normalized geometry survive resolution changes; other modes/columns are excluded");
  }
  auto unknown=roomList;for(auto &line:unknown.lines)for(auto &word:line.words)if(word.roi.x>.7)word.text=L"112";
  auto unknownResult=recognizeSemantic(frame,unknown,custom);
  require(unknownResult.roomList.state==RoomRowState::UnknownCapacity && unknownResult.observation.cards.empty(),"Unknown capacity is distinct from full and absent");
  require(unknownResult.roomList.rawCapacitySummary.find("112")!=std::string::npos &&
      redactRoomCapacity(L"11的2")=="11?2" && redactRoomCapacity(L"１２ ／ １２")=="12?/?12" &&
      redactRoomCapacity(std::wstring(100,L'8')).size()==24,"Raw occupancy diagnostics preserve bounded digits/slash and redact all other OCR characters");
  auto confirmedFilter=custom;confirmedFilter.onlyJoinableFilterConfirmed=true;
  require(recognizeSemantic(frame,unknown,confirmedFilter).observation.cards.size()==4,"Only current-session explicit filter confirmation permits unknown-capacity fallback");
  for(auto impossible:{L"13/12",L"11/0",L"129/128"}){
    auto contradiction=unknown;for(auto &line:contradiction.lines)for(auto &word:line.words)if(word.roi.x>.7)word.text=impossible;
    auto result=recognizeSemantic(frame,contradiction,confirmedFilter);
    require(result.observation.cards.empty()&&result.roomList.ambiguous==4,"Impossible counts remain ambiguous even with filter confirmation");
  }
  require(recognizeSemantic(frame,fullList,confirmedFilter).observation.cards.empty(),"Explicitly full rows never enter filter fallback");
  auto nearRows=words({{L"目标模式",{.19,.4,.12,.02}},{L"目标模式",{.19,.412,.12,.02}},
                       {L"11/12",{.77,.406,.035,.02}}});
  require(vacantRoomRows(nearRows).empty(),"A count with two possible row owners cannot authorize either candidate");
  RoomListRefresh refresh;
  require(!refresh.due(true,false,false,1000)&&!refresh.due(true,false,false,15999)&&refresh.due(true,false,false,16000),"Empty room list refreshes after 15 seconds");
  CNJoinGate refreshJoin;refreshJoin.acted(CNPage::RoomList,true,16000,Action::Click);refresh.acted(true,16000);
  require(!refreshJoin.awaitingRoom(16001)&&refresh.count()==1&&!refresh.due(true,false,false,30000),"Refresh request cannot establish target-join source and has cooldown");
  require(!refresh.due(true,true,false,31000)&&!refresh.due(true,false,true,31000),"Fresh candidates and pending joins take priority over refreshing");
  require(refresh.due(true,false,false,31000),"Unchanged list is allowed another limited refresh");
  refresh.acted(false,31000);require(!refresh.due(true,false,false,60000)&&refresh.due(true,false,false,61000),"Failed input uses exponential refresh backoff");
  refresh.reset();require(!refresh.due(false,false,false,70000)&&!refresh.due(true,false,false,70000),"Stop/mode reset removes scheduled refresh");
  refresh.observe(true,false,true,true);
  require(refresh.unknownExhausted()&&refresh.due(true,true,false,85000),"Exhausted unknown candidates request a real refresh despite remaining visible rows");
  refresh.observe(true,false,true,false);
  require(refresh.unknownExhausted(),"Elapsed join backoff alone cannot re-arm unknown fallback");
  refresh.acted(false,85000);require(refresh.unknownExhausted(),"Failed refresh cannot re-arm fallback");
  refresh.acted(true,115000);require(!refresh.unknownExhausted(),"Only sent verified refresh re-arms bounded unknown candidates");
  auto appearing=available.observation;refreshJoin.prepare(appearing,80000,appearing.card);
  refreshJoin.acted(CNPage::RoomList,true,80000,Action::DoubleClick);
  require(refreshJoin.awaitingRoom(80001)&&!refreshJoin.observe(CNPage::Room,81000).joined&&refreshJoin.observe(CNPage::Room,82000).joined,"Room appearing after refresh still needs a target request and two live confirmations");
  auto maskFrame=Frame{1024,576,std::vector<std::uint32_t>(1024*576,0xff202838)};
  auto paintMask=[&](const char *const *mask,int w,int h,int x0,int y0){for(int y=0;y<h;++y)for(int x=0;x<w;++x)
    maskFrame.pixels[std::size_t(y0+y)*1024+x0+x]=mask[y][x]=='1'?0xffeeeeee:0xff202838;};
  paintMask(RoomTitleMask,101,12,195,165);
  require(bool(roomTitleGlyph(maskFrame,171./576)),"Complete first-row glyph template matches");
  paintMask(RoomTitleMaskSecond,101,12,195,208);
  require(bool(roomTitleGlyph(maskFrame,214./576)),"Complete last-row subpixel variant matches");
  for(int y=166;y<175;++y)for(int x=215;x<225;++x)maskFrame.pixels[std::size_t(y)*1024+x]=0xff202838;
  require(!roomTitleGlyph(maskFrame,171./576),"Changing one character cannot hide in whole-title similarity");
  paintMask(RoomTitleMask,101,12,195,165);paint(maskFrame,{.30,166./576,.02,9./576},0xffeeeeee);
  require(!roomTitleGlyph(maskFrame,171./576),"Full target plus a suffix is rejected");
  paint(maskFrame,{.30,166./576,.02,9./576},0xff202838);paint(maskFrame,{.18,166./576,.01,9./576},0xffeeeeee);
  require(!roomTitleGlyph(maskFrame,171./576),"Full target with additional prefix is rejected");
  paint(maskFrame,{.18,166./576,.01,9./576},0xff202838);paint(maskFrame,{.19,.28,.06,.04},0xff202838);
  require(!roomTitleGlyph(maskFrame,171./576),"Occluded/truncated title is rejected");
  auto unsupported=maskFrame;unsupported.height=700;unsupported.pixels.resize(1024*700);
  require(!roomTitleGlyph(unsupported,171./576),"Unsupported client aspect never uses a title template");
  paint(maskFrame,{800./1024,110./576,32./1024,18./576},0xffe85524);
  for(int y=0;y<18;++y)for(int x=0;x<32;++x)if(RoomRefreshMask[y][x]=='1')maskFrame.pixels[std::size_t(110+y)*1024+800+x]=0xffffffff;
  require(bool(roomRefreshGlyph(maskFrame,{.49,140./576,.07,.02})),"Orange double-arrow control is structurally anchored to the table");
  paint(maskFrame,{800./1024,110./576,32./1024,18./576},0xff555555);
  require(!roomRefreshGlyph(maskFrame,{.49,140./576,.07,.02}),"Disabled or missing refresh glyph is never clicked");
  auto joiningText=words({{L"正在尝试自动加入",{.3,.35,.4,.05}},{L"返回",{.4,.6,.1,.04}}});
  require(recognizeSemantic(frame,joiningText,custom).observation.page==CNPage::Joining &&
          recognizeSemantic(frame,joiningText,custom).observation.step.action==Action::Wait,
          "The joining overlay waits instead of issuing more clicks");
  CNJoinGate direct;
  direct.acted(CNPage::RoomList,true,1000);
  require(direct.prepare(available.observation,4000,{}).step.action==Action::Wait,
          "A direct double click waits for its result");
  direct.observe(CNPage::RoomList,6000);
  require(std::abs(direct.prepare(available.observation,6000,{}).step.click.y-.369)<.001,
          "An unchanged list retries a different vacant row after five seconds");
  direct.acted(CNPage::RoomList,true,7000);direct.observe(CNPage::Joining,8000);
  direct.observe(CNPage::None,40000);
  require(direct.awaitingRoom(71000) && !direct.observe(CNPage::Room,71000).joined &&
          direct.observe(CNPage::Room,73000).joined,
          "Direct entry survives loading and requires two positive room observations");
  CNJoinGate directBackoff;
  for (Millis t=1000;t<=21000;t+=5000) {
    directBackoff.acted(CNPage::RoomList,true,t);directBackoff.observe(CNPage::RoomList,t+5000);
  }
  require(directBackoff.gaveUp() && directBackoff.retryAt()==41000 &&
          directBackoff.prepare(available.observation,27000,{}).step.action==Action::Wait &&
          directBackoff.prepare(available.observation,41000,{}).step.action==Action::DoubleClick,
          "Five failed direct entries trigger a fifteen-second pause then resume");
  require(!learnSemantic(frame,available,custom),"Room occupancy is never reused from persistent calibration");
  Action parsedAction{};CNPage parsedPage{};
  require(cnActionFromName("double_click",parsedAction) && parsedAction==Action::DoubleClick &&
          cnPageFromName("room_list",parsedPage) && parsedPage==CNPage::RoomList &&
          cnPageFromName("joining",parsedPage) && parsedPage==CNPage::Joining,
          "New actions and pages survive profile serialization");
  // Cached anchors carry glyph structure: swapping the text while leaving its
  // background and mean brightness intact must invalidate the cached action.
  Frame patterned = screen();
  for (int y = 0; y < patterned.height; ++y)
    for (int x = 0; x < patterned.width; ++x)
      patterned.pixels[std::size_t(y)*patterned.width+x] = ((x/4+y/3)%2) ? 0xffeeeeee : 0xff222222;
  SemanticObservation calibration;
  calibration.observation.page = CNPage::Home; calibration.observation.step.action = Action::Click;
  calibration.observation.step.label = L"主界面"; calibration.observation.step.click = {.5,.03};
  calibration.evidence = {{.1,.1,.1,.1},{.6,.2,.1,.1}};
  auto learned = learnSemantic(patterned,calibration,custom);
  require(bool(learned),"Two structured anchors can be calibrated");
  SemanticCache cache; require(cache.remember(*learned),"New calibration is stored");
  auto moved = *learned; moved.semantic.observation.step.click = {.6,.03}; moved.anchors[0].roi.x += .1;
  SemanticCache layouts; layouts.remember(*learned);
  require(layouts.remember(moved) && layouts.pages.size() == 2,"Identical labels at moved positions retain the new layout");
  require(bool(cache.match(patterned,custom)),"Unchanged glyphs reuse a calibration");
  auto oldDeathMenu=*learned; oldDeathMenu.context=death; oldDeathMenu.semantic.observation.page=CNPage::Play;
  SemanticCache oldDeathCache; oldDeathCache.pages.push_back(oldDeathMenu);
  require(!oldDeathCache.match(patterned,death),"Old deathmatch tab caches cannot hide changed cards");
  require(!oldDeathCache.remember(oldDeathMenu) && !learnSemantic(patterned,oldDeathMenu.semantic,death),
          "Persistent tab glyphs do not become deathmatch navigation templates");
  SemanticCache cleanedDeathCache; std::string cleanedDeathError;
  require(parseSemanticCache(writeSemanticCache(oldDeathCache),cleanedDeathCache,cleanedDeathError) && cleanedDeathCache.pages.empty(),
          "Legacy deathmatch tab decisions are ignored on reload");
  auto legacyRoom=*learned;legacyRoom.semantic.observation.page=CNPage::Room;
  SemanticCache legacyRoomCache;legacyRoomCache.pages.push_back(legacyRoom);
  require(!legacyRoomCache.match(patterned,custom),"Legacy room title caches cannot hide the new browser list footer");
  require(!cache.match(patterned,death),"Room and deathmatch caches are isolated");
  auto changed = patterned;
  for (auto &pixel : changed.pixels) pixel ^= 0x00ffffff;
  require(!cache.match(changed,custom),"Different glyph structure invalidates a cache");
  SemanticCache loaded; std::string error;
  require(parseSemanticCache(writeSemanticCache(cache),loaded,error) && bool(loaded.match(patterned,custom)),"Persisted calibration survives restart");
  auto searchCalibration = calibration; searchCalibration.observation.page = CNPage::BrowserSearch;
  searchCalibration.observation.step.action = Action::Text;
  require(!learnSemantic(patterned,searchCalibration,custom),"Missing-target search states are always freshly recognized");
  auto oldBrowser = *learned; oldBrowser.semantic.observation.page = CNPage::Browser;
  oldBrowser.semantic.observation.hasCard = true;
  oldBrowser.semantic.observation.card = {.65,.25};
  oldBrowser.semantic.observation.cards = {{.65,.25}};
  require(!learnSemantic(patterned,oldBrowser.semantic,custom),
          "Browser selection is not cached across an unchanged title and changed footer");
  SemanticCache browserCache;
  require(!browserCache.remember(oldBrowser),"New browser calibration cannot hide the next Join state");
  browserCache.pages.push_back(oldBrowser);
  auto selectedFrame = patterned; paint(selectedFrame,{.45,.83,.2,.1},0xfff87820);
  require(!browserCache.match(selectedFrame,custom),
          "Old browser entries cannot hide a newly enabled join footer");
  require(parseSemanticCache(writeSemanticCache(browserCache),loaded,error) && loaded.pages.empty(),
          "Persisted transient browser entries are discarded on load");
  auto listCalibration = calibration; listCalibration.observation.page = CNPage::Join;
  listCalibration.observation.hasCard = true; listCalibration.observation.card = {.65,.25};
  listCalibration.observation.cards = {{.65,.25},{.15,.15}};
  auto listLearned = learnSemantic(patterned,listCalibration,custom);
  require(bool(listLearned),"Room list evidence can use a guarded fast path");
  SemanticCache listCache; listCache.remember(*listLearned);
  require(!listCache.match(patterned,custom),"Legacy Auto Join caches require fresh OCR for the new entry route");
  auto changedList = patterned; paint(changedList,{.78,.7,.08,.03},0xffeeeeee);
  require(!listCache.match(changedList,custom),"Animated legacy lists cannot bypass the new entry route");
  auto changedTitle = patterned; paint(changedTitle,{.6,.2,.1,.1},0xff222222);
  require(!listCache.match(changedTitle,custom),"A changed target title or position invalidates rapid clicks");
  require(parseSemanticCache(writeSemanticCache(listCache),loaded,error) && !loaded.match(changedList,custom) &&
          loaded.pages.front().semantic.observation.cards.size() == 2,
          "Cached same-mode candidates survive restart and tolerate animated background");
  auto legacy = cache; legacy.pages.push_back(*listLearned);
  auto legacySource = writeSemanticCache(legacy);
  legacySource.replace(legacySource.find("\"version\":3"),11,"\"version\":2");
  require(parseSemanticCache(legacySource,loaded,error) && loaded.pages.size() == 1 && loaded.match(patterned,custom),
          "Version 2 keeps saved navigation but relearns obsolete room pages");
  require(!parseSemanticCache("{\"format\":\"owh-semantic-cache\",\"version\":99,\"pages\":[]}",loaded,error),"Unsupported cache format is rejected");
  OcrFrame parsed;
  require(!parseOcrFrame("{\"error\":\"OCR unavailable\"}",parsed,error),"OCR errors cannot produce actions");
  require(!parseOcrFrame("{\"width\":1280,\"height\":720,\"lines\":[{\"words\":[{\"text\":\"Play\",\"roi\":[-1,0,1,1]}]}]}",parsed,error),"Invalid OCR geometry is rejected");
  std::cout << "Semantic vision and workflow scenarios passed\n";
}
