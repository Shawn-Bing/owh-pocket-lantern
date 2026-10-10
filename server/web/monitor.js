"use strict";
// Shared by the browser and Node regression tests. All ages use elapsed local
// monotonic time plus transport age; ACTIVE never grants match confirmation.
const MonitorState = (() => {
  const finite = n => typeof n === "number" && Number.isFinite(n) && n >= 0;
  function elapsed(performanceDelta, wallDelta) { return Math.max(0, finite(performanceDelta) ? performanceDelta : Infinity, Number.isFinite(wallDelta) ? wallDelta : Infinity); }
  function transport(snapshot, wallNow) {
    const at = Date.parse(snapshot.now);
    if (!Number.isFinite(at) || wallNow < at - 1000) return Infinity;
    return Math.max(0, wallNow - at);
  }
  function evaluate(agent, snapshot, elapsedMS, connected, transitMS = 0) {
    const elapsed = finite(elapsedMS) && finite(transitMS) ? elapsedMS + transitMS : Infinity;
    const age = Date.parse(snapshot.now) - Date.parse(agent.seen) + elapsed;
    const hostOnline = Number.isFinite(age) && age >= 0 && age <= Math.min(90, snapshot.offline_after_seconds) * 1000;
    const m = agent.monitor, o = agent.v2?.observation;
    let state = "UNVERIFIED", reason = !connected ? "connection_unavailable" : !hostOnline ? "host_offline" : m?.reason || "legacy_protocol";
    const evidence = m && finite(m.evidence_remaining_ms) ? Math.min(15000,m.evidence_remaining_ms) - elapsed : 0;
    const process = m && finite(m.process_remaining_ms) ? Math.min(15000,m.process_remaining_ms) - elapsed : 0;
    if (connected && hostOnline && m?.host_online && m.clock_trusted && o?.running) {
      const visible = o.window?.foreground && !o.window?.minimized && !o.window?.occluded && !o.window?.background_input;
      if (m.state === "IN_MATCH" && o.state === "IN_MATCH" && o.game_pid > 0 && /^[1-9][0-9]{0,19}$/.test(o.game_created) && o.confidence === "REPEATED_VISUAL" && ["TARGET_TITLE","TARGET_JOIN","EXPLICIT_START","DEATHMATCH_QUEUE","DEATHMATCH_SCOREBOARD"].includes(o.source) && visible && evidence > 0 && process > 0) state = "IN_MATCH";
      else if (m.state === "OUT_OF_MATCH" && o.state === "OUT_OF_MATCH" && visible && evidence > 0 && process > 0) state = "OUT_OF_MATCH";
      else if (m.state === "OFFLINE" && o.state === "OFFLINE" && o.game_pid === 0 && process > 0) state = "OFFLINE";
      else if (m.state !== "UNVERIFIED") reason = "evidence_expired";
    }
    const hardwareFresh = connected && hostOnline && m && finite(m.hardware_remaining_ms) && m.hardware_remaining_ms > elapsed;
    return {state, reason, hostOnline, connected, healthy: state === "IN_MATCH" && !agent.dry_run, hardwareFresh, evidenceRemainingMS:Math.max(0,evidence), ageSeconds:Math.max(0,age/1000)};
  }
  function sensor(reading, fresh) {
    if (!fresh) return {value:null, reason:"sample_expired", source:reading?.source || "", sensor:reading?.sensor || ""};
    if (!reading || typeof reading.value !== "number" || !Number.isFinite(reading.value)) return {value:null, reason:reading?.reason || "sensor_unavailable", source:reading?.source || "", sensor:reading?.sensor || ""};
    return reading;
  }
  function alert(agent, elapsedMS, connected) {
    const a=agent.alert;
    if (!connected || !finite(elapsedMS) || elapsedMS > 20000) return {text:"告警待同步（服务器未连接或快照过期）",className:"alert-pending"};
    if (!a?.enabled) {
      const reasons={legacy_protocol:"旧协议未上报运行状态",dry_run:"演练模式不启用告警",assistant_stopped:"助手已停止",awaiting_fresh_running_heartbeat:"等待新的运行心跳"};
      return {text:"告警未启用："+(reasons[a?.reason]||"等待状态同步"),className:"alert-inactive"};
    }
    if (a.kind==="NONE") return {text:"上次同步：对局证据有效 · 无连续异常",className:"alert-inactive"};
    const kinds={OUT_OF_MATCH:a.reason==="game_process_absent"?"上次同步：游戏进程不存在":"上次同步：已确认不在局内",UNVERIFIED:"上次同步：无法验证局内",OFFLINE:"上次同步：主机心跳失联"};
    const seconds=Math.floor((finite(a.duration_ms)?a.duration_ms:0)/1000+elapsedMS/1000);
    const thresholdSeconds=(finite(a.threshold_ms)?a.threshold_ms:300000)/1000;
    const threshold=thresholdSeconds>=60?`${Math.floor(thresholdSeconds/60)} 分钟${thresholdSeconds%60?` ${thresholdSeconds%60} 秒`:""}`:`${thresholdSeconds} 秒`;
    return {text:(a.reached?"异常告警 · ":"异常计时 · ")+(kinds[a.kind]||"上次同步：状态未知")+`；连续未确认局内 ${Math.floor(seconds/60)} 分 ${seconds%60} 秒（阈值 ${threshold}）`,className:a.reached?"alert-reached":"alert-timing"};
  }
  return {evaluate, sensor, transport, elapsed, alert};
})();
if (typeof module !== "undefined" && module.exports) module.exports = MonitorState;
