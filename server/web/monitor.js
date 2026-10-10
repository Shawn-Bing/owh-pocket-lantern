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
  return {evaluate, sensor, transport, elapsed};
})();
if (typeof module !== "undefined" && module.exports) module.exports = MonitorState;
