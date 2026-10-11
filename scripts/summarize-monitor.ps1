param(
 [Parameter(Mandatory=$true)][string]$EvidenceDirectory,
 [string]$OutputPath='docs/validation/p2-6-performance.json'
)
$ErrorActionPreference='Stop'
$encoding=[Text.UTF8Encoding]::new($false)
$evidence=(Resolve-Path -LiteralPath $EvidenceDirectory).Path
$records=@(Get-Content -LiteralPath (Join-Path $evidence 'performance.json') -Raw|ConvertFrom-Json)
if($records.Count -ne 2 -or @($records|Where-Object result -ne 'PASS').Count){throw 'Two completed PASS phases are required'}
function Rounded($value) {if($null -eq $value){return $null};return [Math]::Round([double]$value,6)}
function RangeStats($values) {
 $stats=$values|Measure-Object -Minimum -Maximum -Average
 return [ordered]@{first=$values[0];last=$values[-1];minimum=$stats.Minimum;maximum=$stats.Maximum;mean=(Rounded $stats.Average)}
}
function WindowSummary($window) {
 $window=@($window)
 if($window.Count -lt 2){throw 'At least two samples are required per window'}
 $duration=$window[-1].elapsed_seconds-$window[0].elapsed_seconds
 $roles=@($window.processes.role|Sort-Object -Unique)
 $processStats=[ordered]@{}
 foreach($role in $roles){
  $points=@($window|ForEach-Object {$sample=$_;$process=@($sample.processes|Where-Object role -eq $role);if($process.Count -eq 1){[pscustomobject]@{at=$sample.elapsed_seconds;value=$process[0]}}})
  if($points.Count -ne $window.Count){throw ('Missing role within window: '+$role)}
  $first=$points[0].value;$last=$points[-1].value
  $processStats[$role]=[ordered]@{
   cpu_percent_one_core_mean=(Rounded (100*($last.cpu_seconds-$first.cpu_seconds)/$duration))
   cpu_percent_one_core_peak=(Rounded (($points.value.cpu_percent_one_core|Measure-Object -Maximum).Maximum))
   private_bytes=(RangeStats @($points.value.private_bytes))
   working_set_bytes=(RangeStats @($points.value.working_set_bytes))
   threads=(RangeStats @($points.value.threads));handles=(RangeStats @($points.value.handles))
   private_endpoint_bytes_per_minute=(Rounded (($last.private_bytes-$first.private_bytes)*60/$duration))
   working_set_endpoint_bytes_per_minute=(Rounded (($last.working_set_bytes-$first.working_set_bytes)*60/$duration))
  }
 }
 $cpu=0.0;foreach($role in $roles){$cpu+=$processStats[$role].cpu_percent_one_core_mean}
 return [ordered]@{
  first_elapsed_seconds=(Rounded $window[0].elapsed_seconds);last_elapsed_seconds=(Rounded $window[-1].elapsed_seconds)
  sample_count=$window.Count;cpu_percent_one_core_mean=(Rounded $cpu)
  cpu_percent_one_core_peak=(Rounded (($window.cpu_percent_one_core|Measure-Object -Maximum).Maximum))
  combined_working_set_bytes=(RangeStats @($window.working_set_bytes))
  get_requests_delta=($window[-1].get_requests-$window[0].get_requests)
  sse_snapshots_delta=($window[-1].sse_snapshots-$window[0].sse_snapshots)
  get_sse_response_body_bytes_delta=($window[-1].get_sse_response_body_bytes-$window[0].get_sse_response_body_bytes)
  processes=$processStats
 }
}
$phases=[Collections.Generic.List[object]]::new()
foreach($record in $records){
 $rawPath=Join-Path (Join-Path $evidence $record.phase) 'samples.jsonl'
 $raw=@(Get-Content -LiteralPath $rawPath|ForEach-Object {$_|ConvertFrom-Json})
 if($raw.Count -ne $record.sample_count){throw 'Raw sample count differs from completion record'}
 $steady=@($raw|Where-Object {$_.elapsed_seconds -ge $record.warmup_seconds -and $_.elapsed_seconds -lt $record.active_seconds})
 $windows=[Collections.Generic.List[object]]::new()
 for($start=[int]$record.warmup_seconds;$start -lt [int]$record.active_seconds;$start+=300){
  $end=[Math]::Min($start+300,[int]$record.active_seconds)
  $window=@($steady|Where-Object {$_.elapsed_seconds -ge $start -and $_.elapsed_seconds -lt $end})
  if($window.Count -ge 2){$summary=WindowSummary $window;$summary['nominal_from_seconds']=$start;$summary['nominal_to_seconds']=$end;$windows.Add($summary)}
 }
 $hardware=$null
 if($record.hardware_sample){
  $hardware=[ordered]@{}
  foreach($name in @('cpu_package_temperature','cpu_package_power','gpu_power')){
   $reading=$record.hardware_sample.$name
   $hardware[$name]=[ordered]@{value=$reading.value;unit=$reading.unit;source=$reading.source;reason=$reading.reason}
  }
 }
 $steadySummary=WindowSummary $steady
 $phases.Add([ordered]@{
  phase=$record.phase;result=$record.result;started_at_utc=$record.started_at_utc
  active_seconds=$record.active_seconds;elapsed_seconds=(Rounded $record.elapsed_seconds)
  warmup_seconds=$record.warmup_seconds;sample_interval_ms=$record.sample_interval_ms;status_interval_seconds=$record.status_interval_seconds
  sample_count=$record.sample_count;logical_processors=$record.logical_processors;binary_sha256=$record.binary_sha256
  whole_run=[ordered]@{
   cpu_percent_one_core_average=(Rounded $record.cpu_percent_one_core_average);cpu_percent_one_core_peak=(Rounded $record.cpu_percent_one_core_peak)
   cpu_percent_machine_average=(Rounded $record.cpu_percent_machine_average)
   get_requests=$record.get_requests;get_rtt_ms_average=(Rounded $record.get_rtt_ms_average);get_rtt_ms_maximum=(Rounded $record.get_rtt_ms_maximum)
   get_sse_response_body_bytes_sent=$record.dashboard_get_sse_response_body_bytes_sent;get_sse_payload_bytes_per_second=(Rounded $record.get_sse_payload_bytes_per_second)
   sse_connections_owned=$record.sse_connections_owned;sse_connection_attempts=$record.sse_connection_attempts
   sse_snapshots=$record.sse_snapshots;sse_max_observed_snapshot_gap_seconds=(Rounded $record.sse_max_observed_snapshot_gap_seconds)
   sse_closed_unexpectedly=$record.sse_closed_unexpectedly
  }
  steady_state=$steadySummary;five_minute_windows=$windows;hardware_sample=$hardware
 })
}
$output=[ordered]@{
 schema_version=1;scope='Isolated real dashboard plus actual-main telemetry fixture; optional real sensor bridge. No automation worker, OCR, GUI or real-game proof.'
 phase_result_scope='PASS denotes completed timed run with accepted telemetry and continuous owned SSE; it is not a memory-stability or leak verdict.'
 source_evidence_directory=(Split-Path $evidence -Leaf)
 conditions='Sequential hardware_off/on runs, each fresh dashboard and cache; existing real Overwatch/FanControl/GameViewer may run in background. Not a controlled whole-machine A/B.'
 cpu_accounting='Owned server+agent(+bridge) process CPU only; one core is 100%. Script/test tools and unrelated processes excluded. Machine percent divides by logical_processors.'
 steady_accounting='Elapsed samples >= warmup and < active_seconds; excludes startup warmup and shutdown tail. CPU means use cumulative process CPU difference between first and last sample.'
 window_accounting='Five-minute nominal half-open windows; first/last observed samples and sample counts reported. Endpoint slopes describe changes, not leak diagnoses.'
 sse_accounting='One owned independent SSE connection; snapshots drained every sample interval. Observed gaps have sample-interval quantization.'
 network_accounting='GET/SSE response bodies only; heartbeat uploads, HTTP headers, TCP, ZeroTier and NIC counters excluded.'
 total_network_bytes_sent=$null;total_network_bytes_received=$null;total_network_result='NOT MEASURED'
 whole_machine_power_watts=$null;whole_machine_power_result='NOT MEASURED'
 phases=$phases
}
$output|ConvertTo-Json -Depth 12|Set-Content -LiteralPath $OutputPath -Encoding utf8
Write-Output ('Sanitized summary: '+$OutputPath)