"use strict";
const assert=require("node:assert/strict");
const fs=require("node:fs"),vm=require("node:vm");
const monitor=require("./web/monitor.js");
const clone=o=>JSON.parse(JSON.stringify(o));
const snapshot={now:"2026-10-10T17:00:00.000Z",offline_after_seconds:90,events:[]};
const agent={machine:"fixture",seen:snapshot.now,state:"ACTIVE",detail:"运行中",dry_run:false,score:0.95,uptime:20,restarts:0,version:"0.5.13",monitor:{state:"IN_MATCH",reason:"",evidence_remaining_ms:15000,process_remaining_ms:15000,hardware_remaining_ms:15000,clock_trusted:true,host_online:true},v2:{observation:{state:"IN_MATCH",running:true,game_pid:1,game_created:"134000000000000001",confidence:"REPEATED_VISUAL",source:"TARGET_JOIN",mode:"CUSTOM_ROOM",window:{foreground:true,minimized:false,occluded:false,background_input:false}},hardware:{sampled_at:snapshot.now,gpu_power:{value:40.344,unit:"watts",source:"nvml",sensor:"GPU",reason:""}}}};
snapshot.agents=[agent];
assert.equal(monitor.evaluate(agent,snapshot,0,true).healthy,true);
assert.equal(monitor.evaluate(agent,snapshot,15000,true).state,"UNVERIFIED");
assert.equal(monitor.evaluate(agent,snapshot,0,false).healthy,false);
assert.equal(monitor.evaluate({state:"ACTIVE",seen:snapshot.now},snapshot,0,true).state,"UNVERIFIED");
assert.equal(monitor.evaluate(agent,snapshot,0,true,16000).state,"UNVERIFIED");
assert.equal(monitor.evaluate(agent,snapshot,0,true,Infinity).healthy,false);
assert.equal(monitor.elapsed(2,3600000),3600000); // Mac sleep: perf may pause
assert.equal(monitor.elapsed(5000,-10000),5000); // wall clock rollback
assert.equal(monitor.evaluate(agent,snapshot,monitor.elapsed(2,3600000),true).healthy,false);
assert.equal(monitor.transport(snapshot,Date.parse(snapshot.now)-2000),Infinity);
assert.equal(monitor.evaluate(agent,snapshot,0,true,monitor.transport(snapshot,Date.parse(snapshot.now)+16000)).healthy,false);
assert.equal(monitor.sensor(agent.v2.hardware.gpu_power,false).value,null);
assert.equal(monitor.sensor({value:null,source:"lhm",reason:"cpu_access_denied"},true).reason,"cpu_access_denied");
for(const flag of ["minimized","occluded","background_input"]){const a=clone(agent);a.v2.observation.window[flag]=true;assert.equal(monitor.evaluate(a,snapshot,0,true).healthy,false)}
const offline=clone(agent);offline.v2.observation.state=offline.monitor.state="OFFLINE";offline.v2.observation.game_pid=0;
assert.equal(monitor.evaluate(offline,snapshot,0,true).state,"OFFLINE");
assert.equal(monitor.evaluate(offline,snapshot,15000,true).state,"UNVERIFIED");

// Exercise the real app's rendering and connection transitions without a DOM
// package, browser driver, network, or tokens. Only used DOM primitives are faked.
class Element {
 constructor(tag){this.tag=tag;this.children=[];this.dataset={};this.className="";this.textContent="";this.value="";this.listeners={};this.classList={toggle:(name,on)=>{const set=new Set(this.className.split(" ").filter(Boolean));on?set.add(name):set.delete(name);this.className=[...set].join(" ")}}}
 append(...nodes){this.children.push(...nodes)}
 replaceChildren(...nodes){this.children=[...nodes]}
 addEventListener(name,fn){this.listeners[name]=fn}
 querySelector(selector){return this.querySelectorAll(selector)[0]||null}
 querySelectorAll(selector){const name=selector.slice(1),out=[];for(const c of this.children){if(c.className.split(" ").includes(name))out.push(c);out.push(...c.querySelectorAll(selector))}return out}
}
const elements=new Map(),get=id=>{if(!elements.has(id))elements.set(id,new Element("div"));return elements.get(id)};
let perf=0,wall=Date.parse(snapshot.now);
class TestDate extends Date {static now(){return wall}}
const context=vm.createContext({document:{getElementById:get,createElement:tag=>new Element(tag)},performance:{now:()=>perf},Date:TestDate,location:{origin:"http://127.0.0.1:8765"},setInterval:()=>0,setTimeout:()=>0,clearTimeout:()=>{},AbortController,TextDecoder,AbortSignal,fetch:async()=>{throw Error("offline")}});
vm.runInContext(fs.readFileSync(require.resolve("./web/monitor.js"),"utf8"),context);
vm.runInContext(fs.readFileSync(require.resolve("./web/app.js"),"utf8"),context);
context.fixture=clone(snapshot);vm.runInContext('render(fixture);connection("实时连接",true);',context);
assert.equal(get("healthy").textContent,1);
vm.runInContext('connection("连接中断",false)',context);
assert.equal(get("healthy").textContent,0);assert.equal(get("agents").children[0].querySelector(".match-label").textContent,"局内状态未验证");
vm.runInContext('connection("实时连接",true)',context);assert.equal(get("healthy").textContent,1);
wall+=3600000;perf+=2;vm.runInContext('updateAges()',context);assert.equal(get("healthy").textContent,0);
// Replaying the exact same server snapshot does not reset local expiry.
vm.runInContext('render(fixture);connection("实时连接",true)',context);assert.equal(get("healthy").textContent,0);
vm.runInContext('disconnect()',context);assert.equal(get("healthy").textContent,"0");
assert.equal(get("dashboard").hidden,true);
console.log("Monitor browser tests passed: TTL, transport, sleep, v1, sensors, disconnect, replay, logout");

// Alert display never treats an old cached alert as live after disconnect.
const alertAgent=clone(agent);
alertAgent.alert={enabled:true,episode_id:"episode-1",kind:"OUT_OF_MATCH",reason:"visually_out_of_match",duration_ms:300000,type_duration_ms:1000,threshold_ms:300000,reached:true};
assert.match(monitor.alert(alertAgent,0,true).text,/连续未确认局内 5 分/);
assert.doesNotMatch(monitor.alert(alertAgent,0,true).text,/不在局内已/);
assert.equal(monitor.alert(alertAgent,0,false).className,"alert-pending");
assert.equal(monitor.alert(alertAgent,20001,true).className,"alert-pending");
for(const reason of ["legacy_protocol","dry_run","assistant_stopped","awaiting_fresh_running_heartbeat"]){
 const a=clone(alertAgent);a.alert={enabled:false,reason};assert.equal(monitor.alert(a,0,true).className,"alert-inactive");
}
wall+=1000;perf+=1000;
context.alertFixture=clone(snapshot);context.alertFixture.now=new Date(wall).toISOString();context.alertFixture.agents=[alertAgent];
vm.runInContext('render(alertFixture);connection("实时连接",true)',context);
assert.match(get("agents").children[0].querySelector(".alert-detail").textContent,/异常告警/);
vm.runInContext('connection("连接中断",false)',context);
assert.match(get("agents").children[0].querySelector(".alert-detail").textContent,/告警待同步/);
console.log("Alert browser tests passed: mixed duration, disconnect, expiry, stop, dry-run, legacy and actual rendering");

// Alert type describes the last server sync; local short TTL never becomes a current visual claim.
assert.equal(monitor.evaluate(alertAgent,snapshot,15001,true).state,"UNVERIFIED");
assert.match(monitor.alert(alertAgent,15001,true).text,/上次同步：已确认不在局内/);
assert.doesNotMatch(monitor.alert(alertAgent,15001,true).text,/当前已确认/);

const noneAlert=clone(alertAgent);noneAlert.alert.kind="NONE";assert.match(monitor.alert(noneAlert,15001,true).text,/上次同步：对局证据有效/);
for(const [ms,label] of [[2000,"阈值 2 秒"],[90000,"阈值 1 分钟 30 秒"],[300000,"阈值 5 分钟"]]){const a=clone(alertAgent);a.alert.threshold_ms=ms;assert.ok(monitor.alert(a,0,true).text.includes(label))}
