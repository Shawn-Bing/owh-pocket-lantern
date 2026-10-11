param(
 [ValidateRange(1024,65535)][int]$Port=18767,
 [ValidateRange(1,3600)][int]$ActiveSeconds=4,
 [switch]$OnlyHardwareOn,
 [ValidateRange(100,60000)][int]$SampleIntervalMs=500,
 [ValidateRange(0,1800)][int]$WarmupSeconds=10,
 [ValidateRange(0,300)][int]$StatusIntervalSeconds=0
)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$out=Join-Path $root ('build/monitor-performance-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $out -Force|Out-Null
Add-Type -AssemblyName System.Net.Http
$records=[Collections.Generic.List[object]]::new()
$encoding=[Text.UTF8Encoding]::new($false)
$server=$null;$fixture=$null;$bridge=$null;$http=$null;$sse=$null;$sampleWriter=$null
function SaveResults {$records|ConvertTo-Json -Depth 12|Set-Content (Join-Path $out 'performance.json') -Encoding utf8}
function StopOwned($process) {
 if($process -and !$process.HasExited){Stop-Process -Id $process.Id -ErrorAction Stop;$process.WaitForExit()}
}
try {
 $probe=[Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,$Port);$probe.Start();$probe.Stop()
 $dashboardPath=Join-Path $out 'dashboard.exe'
 Copy-Item -LiteralPath (Join-Path $root 'dist/owh-dashboard.exe') -Destination $dashboardPath
 $binaryHashes=[ordered]@{}
 foreach($path in @($dashboardPath,(Join-Path $root 'build/windows/agent-telemetry-integration.exe'),(Join-Path $root 'dist/owh-sensors.exe'))){
  $binaryHashes[(Split-Path $path -Leaf)]=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
 }
 $phases=if($OnlyHardwareOn){@($true)}else{@($false,$true)}
 foreach($enabled in $phases){
  $phase=if($enabled){'hardware_on'}else{'hardware_off'}
  $phaseDir=Join-Path $out $phase;$agentDir=Join-Path $phaseDir 'agent';$data=Join-Path $phaseDir 'server'
  New-Item -ItemType Directory -Path $agentDir -Force|Out-Null
  # Each phase gets a fresh dashboard heap, agent cache and event history.
  $server=Start-Process $dashboardPath -ArgumentList @('-listen',"127.0.0.1:$Port",'-data',('"'+$data+'"')) -PassThru -WindowStyle Hidden -RedirectStandardError (Join-Path $phaseDir 'server.log')
  $healthy=$false
  for($n=0;$n -lt 40;$n++){
   if($server.HasExited){throw 'Owned isolated dashboard exited during startup'}
   try{$healthy=(Invoke-WebRequest "http://127.0.0.1:$Port/healthz" -TimeoutSec 1).StatusCode -eq 200}catch{}
   if($healthy){break};Start-Sleep -Milliseconds 100
  }
  if(!$healthy -or $server.HasExited){throw 'Owned isolated dashboard did not become healthy'}
  $view=([IO.File]::ReadAllText((Join-Path $data 'view-token.txt'))).Trim()
  $http=[Net.Http.HttpClient]::new();$http.Timeout=[TimeSpan]::FromSeconds(5)
  Copy-Item -LiteralPath (Join-Path $root 'build/windows/agent-telemetry-integration.exe') -Destination (Join-Path $agentDir 'agent.exe')
  if($enabled){
   Copy-Item -LiteralPath (Join-Path $root 'dist/owh-sensors.exe') -Destination (Join-Path $agentDir 'sensor.exe')
   $cache=Join-Path $agentDir 'hardware.json'
   $bridge=Start-Process (Join-Path $agentDir 'sensor.exe') -ArgumentList @('--output',('"'+$cache+'"'),'--interval-ms','5000','--samples',[string]([Math]::Ceiling(($ActiveSeconds+30)/5)+1)) -PassThru -WindowStyle Hidden -RedirectStandardError (Join-Path $phaseDir 'bridge.log')
  }
  $fixtureArgs=@("http://127.0.0.1:$Port",('"'+(Join-Path $data 'agent-token.txt')+'"'),('"'+$agentDir+'"'),'--active-seconds',[string]$ActiveSeconds)
  if(!$enabled){$fixtureArgs+='--hardware-off'}
  $fixture=Start-Process (Join-Path $agentDir 'agent.exe') -ArgumentList $fixtureArgs -PassThru -WindowStyle Hidden -RedirectStandardOutput (Join-Path $phaseDir 'fixture.log') -RedirectStandardError (Join-Path $phaseDir 'fixture-error.log')
  $sseRequest=[Net.Http.HttpRequestMessage]::new([Net.Http.HttpMethod]::Get,"http://127.0.0.1:$Port/api/events")
  $sseRequest.Headers.Authorization=[Net.Http.Headers.AuthenticationHeaderValue]::new('Bearer',$view)
  $sseResponse=$http.SendAsync($sseRequest,[Net.Http.HttpCompletionOption]::ResponseHeadersRead).GetAwaiter().GetResult()
  if([int]$sseResponse.StatusCode -ne 200){throw 'Benchmark SSE authentication failed'}
  $sse=[IO.StreamReader]::new($sseResponse.Content.ReadAsStreamAsync().GetAwaiter().GetResult())
  $pending=$sse.ReadLineAsync()
  $sampleWriter=[IO.StreamWriter]::new((Join-Path $phaseDir 'samples.jsonl'),$false,$encoding);$sampleWriter.AutoFlush=$true
  $cpuStart=@{};$cpuLast=@{};$rtt=[Collections.Generic.List[double]]::new();$samples=[Collections.Generic.List[object]]::new()
  $payload=0L;$sseEvents=0;$getRequests=0;$heartbeatSequence='';$sensorSequence='';$previousAt=0.0;$lastGet=-1e9;$lastSse=0.0;$largestGap=0.0
  $startedUTC=[DateTime]::UtcNow.ToString('o');$clock=[Diagnostics.Stopwatch]::StartNew()
  Write-Output ("Started $phase active=$ActiveSeconds warmup=$WarmupSeconds evidence=$phaseDir")
  while(!$fixture.HasExited -and $clock.Elapsed.TotalSeconds -lt ($ActiveSeconds+30)){
   $iterationAt=$clock.Elapsed.TotalMilliseconds
   if($server.HasExited){throw 'Owned dashboard exited during measurement'}
   if($enabled -and $bridge.HasExited){throw 'Owned sensor bridge exited during measurement'}
   $at=$clock.Elapsed.TotalSeconds;$span=$at-$previousAt;$cpuDelta=0.0;$totalWS=0L;$threads=0;$handles=0
   $perProcess=[Collections.Generic.List[object]]::new()
   foreach($process in @($server,$fixture,$bridge)){
    if($process -and !$process.HasExited){
     $process.Refresh();$key=[string]$process.Id;$cpu=$process.TotalProcessorTime.TotalSeconds
     if(!$cpuStart.ContainsKey($key)){$cpuStart[$key]=$cpu}
     $delta=if($cpuLast.ContainsKey($key)){$cpu-$cpuLast[$key]}else{0.0}
     $cpuDelta+=$delta;$cpuLast[$key]=$cpu
     $totalWS+=$process.WorkingSet64;$threads+=$process.Threads.Count;$handles+=$process.HandleCount
     $perProcess.Add([pscustomobject]@{
      role=$(if($process.Id -eq $server.Id){'server'}elseif($process.Id -eq $fixture.Id){'agent'}else{'bridge'})
      pid=$process.Id;working_set_bytes=$process.WorkingSet64;private_bytes=$process.PrivateMemorySize64
      threads=$process.Threads.Count;handles=$process.HandleCount;cpu_seconds=$cpu
      cpu_percent_one_core=$(if($previousAt -gt 0 -and $span -gt 0){100*$delta/$span}else{0.0})
     })
    }
   }
   if($at-$lastGet -ge $StatusIntervalSeconds){
    $request=[Net.Http.HttpRequestMessage]::new([Net.Http.HttpMethod]::Get,"http://127.0.0.1:$Port/api/status")
    $request.Headers.Authorization=[Net.Http.Headers.AuthenticationHeaderValue]::new('Bearer',$view)
    $timer=[Diagnostics.Stopwatch]::StartNew();$response=$http.SendAsync($request).GetAwaiter().GetResult()
    try{
     $body=$response.Content.ReadAsStringAsync().GetAwaiter().GetResult();$rtt.Add($timer.Elapsed.TotalMilliseconds)
     if([int]$response.StatusCode -ne 200){throw 'Benchmark status failed'}
     $payload+=[Text.Encoding]::UTF8.GetByteCount($body);$getRequests++;$snapshot=$body|ConvertFrom-Json
     $observed=@($snapshot.agents|Where-Object machine -eq 'p2-5-real-main-fixture')
     if($observed.Count){$heartbeatSequence=[string]$observed[0].v2.sequence;if($enabled){$sensorSequence=[string]$observed[0].v2.hardware.sequence}}
    }finally{$response.Dispose();$request.Dispose()}
    $lastGet=$at
   }
   while($pending.IsCompleted){
    $line=$pending.GetAwaiter().GetResult();if($null -eq $line){throw 'SSE closed during measurement'}
    $payload+=[Text.Encoding]::UTF8.GetByteCount($line)+1
    if($line -eq 'event: snapshot'){$sseEvents++;$largestGap=[Math]::Max($largestGap,$at-$lastSse);$lastSse=$at}
    $pending=$sse.ReadLineAsync()
   }
   if($at-$lastSse -gt 35){throw 'No SSE snapshot observed for more than 35 seconds'}
   $entry=[pscustomobject]@{
    elapsed_seconds=$at;working_set_bytes=$totalWS;threads=$threads;handles=$handles;cpu_delta_seconds=$cpuDelta
    cpu_percent_one_core=$(if($previousAt -gt 0 -and $span -gt 0){100*$cpuDelta/$span}else{0.0})
    processes=$perProcess;sse_connections_owned=1;sse_snapshots=$sseEvents;get_requests=$getRequests
    get_sse_response_body_bytes=$payload;last_heartbeat_sequence=$heartbeatSequence;last_sensor_sequence=$sensorSequence
   }
   $samples.Add($entry);$sampleWriter.WriteLine(($entry|ConvertTo-Json -Depth 6 -Compress));$previousAt=$at
   Start-Sleep -Milliseconds ([int][Math]::Max(1,$SampleIntervalMs-($clock.Elapsed.TotalMilliseconds-$iterationAt)))
  }
  if(!$fixture.WaitForExit(3000)){throw 'Fixture timed out'}
  if($fixture.ExitCode -ne 0){throw 'Fixture telemetry failed'}
  $elapsed=$clock.Elapsed.TotalSeconds;$cpuSeconds=0.0
  foreach($key in $cpuStart.Keys){$cpuSeconds+=$cpuLast[$key]-$cpuStart[$key]}
  $steady=@($samples|Where-Object {$_.elapsed_seconds -ge $WarmupSeconds -and $_.elapsed_seconds -le ($ActiveSeconds+3.5)})
  $hardware=$null
  if($enabled -and (Test-Path -LiteralPath $cache)){
   $hardware=Get-Content -LiteralPath $cache -Raw|ConvertFrom-Json
   if($hardware.gpu_power.sensor){$hardware.gpu_power.sensor='local NVML GPU (identity omitted)'}
  }
  $records.Add([pscustomobject]@{
   result='PASS';phase=$phase;started_at_utc=$startedUTC;active_seconds=$ActiveSeconds;elapsed_seconds=$elapsed
   binary_sha256=$binaryHashes;warmup_seconds=$WarmupSeconds;sample_interval_ms=$SampleIntervalMs
   status_interval_seconds=$StatusIntervalSeconds;sample_count=$samples.Count;steady_samples=$steady
   cpu_seconds_observed=$cpuSeconds;cpu_percent_one_core_average=100*$cpuSeconds/$elapsed
   cpu_percent_one_core_peak=($samples.cpu_percent_one_core|Measure-Object -Maximum).Maximum
   cpu_percent_machine_average=100*$cpuSeconds/$elapsed/[Environment]::ProcessorCount;logical_processors=[Environment]::ProcessorCount
   combined_working_set_bytes_min=($samples.working_set_bytes|Measure-Object -Minimum).Minimum
   combined_working_set_bytes_max=($samples.working_set_bytes|Measure-Object -Maximum).Maximum
   get_requests=$getRequests;get_rtt_ms_average=($rtt|Measure-Object -Average).Average;get_rtt_ms_maximum=($rtt|Measure-Object -Maximum).Maximum
   dashboard_get_sse_response_body_bytes_sent=$payload;get_sse_payload_bytes_per_second=$payload/$elapsed
   network_accounting='Measured GET/SSE response bodies only; excludes heartbeat upload, HTTP headers, TCP, ZeroTier and NIC counters.'
   dashboard_total_network_bytes_sent=$null;dashboard_total_network_bytes_received=$null;total_network_result='NOT MEASURED'
   sse_connections_owned=1;sse_connection_attempts=1;sse_snapshots=$sseEvents;sse_max_observed_snapshot_gap_seconds=$largestGap
   sse_closed_unexpectedly=$false;hardware_sample=$hardware;whole_machine_power_watts=$null;whole_machine_power_result='NOT MEASURED'
  })
  SaveResults
  $sampleWriter.Dispose();$sampleWriter=$null;$sse.Dispose();$sse=$null;$sseResponse.Dispose();$sseRequest.Dispose()
  $http.Dispose();$http=$null;StopOwned $bridge;$bridge=$null;StopOwned $fixture;$fixture=$null;StopOwned $server;$server=$null
  Write-Output ("Completed $phase elapsed=$([Math]::Round($elapsed,2)) samples=$($samples.Count) SSE=$sseEvents")
 }
}catch{
 [IO.File]::WriteAllText((Join-Path $out 'failure.txt'),$_.Exception.Message,$encoding);throw
}finally{
 if($sampleWriter){$sampleWriter.Dispose()}
 if($sse){$sse.Dispose()};if($http){$http.Dispose()}
 foreach($process in @($fixture,$bridge,$server)){try{StopOwned $process}catch{Write-Warning ("Cannot close owned test PID "+$process.Id)}}
 SaveResults
 $records|Select-Object phase,elapsed_seconds,cpu_percent_one_core_average,cpu_percent_one_core_peak,combined_working_set_bytes_max,get_rtt_ms_average,sse_snapshots|Format-Table
 Write-Output ('Evidence directory: '+$out)
}
