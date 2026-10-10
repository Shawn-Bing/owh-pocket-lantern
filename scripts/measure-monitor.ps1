param([int]$Port=18767,[int]$ActiveSeconds=4,[switch]$OnlyHardwareOn)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$out=Join-Path $root ('build/p2-5-performance-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $out -Force|Out-Null
$server=$null;$fixture=$null;$bridge=$null;$http=$null;$sse=$null
$records=[Collections.Generic.List[object]]::new()
try {
 $listener=[Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,$Port);$listener.Start();$listener.Stop()
 Copy-Item -LiteralPath (Join-Path $root "dist/owh-dashboard.exe") -Destination (Join-Path $out "dashboard.exe")
 $data=Join-Path $out 'server'
 $server=Start-Process (Join-Path $out 'dashboard.exe') -ArgumentList @('-listen',"127.0.0.1:$Port",'-data',('"'+$data+'"')) -PassThru -WindowStyle Hidden -RedirectStandardError (Join-Path $out 'server.log')
 for($n=0;$n -lt 40;$n++){try{if((Invoke-WebRequest "http://127.0.0.1:$Port/healthz" -TimeoutSec 1).StatusCode -eq 200){break}}catch{};Start-Sleep -Milliseconds 100}
 if($server.HasExited){throw 'Owned isolated dashboard exited during startup'}
 $view=([IO.File]::ReadAllText((Join-Path $data 'view-token.txt'))).Trim()
 Add-Type -AssemblyName System.Net.Http
 $http=[Net.Http.HttpClient]::new();$http.Timeout=[TimeSpan]::FromSeconds(5)
 $phases=if($OnlyHardwareOn){@($true)}else{@($false,$true)}
 foreach($enabled in $phases){
  $phase=if($enabled){'hardware_on'}else{'hardware_off'}
  $agentDir=Join-Path $out $phase;New-Item -ItemType Directory -Path $agentDir -Force|Out-Null
  Copy-Item -LiteralPath (Join-Path $root 'dist/owh-sensors.exe') -Destination (Join-Path $agentDir 'sensor.exe')
  Copy-Item -LiteralPath (Join-Path $root 'build/windows/agent-telemetry-integration.exe') -Destination (Join-Path $agentDir 'agent.exe')
  if($enabled){
   $cache=Join-Path $agentDir 'hardware.json'
   # A new per-run cache path; never shares a writer with the real helper.
   $bridge=Start-Process (Join-Path $agentDir 'sensor.exe') -ArgumentList @('--output',('"'+$cache+'"'),'--interval-ms','5000','--samples',[string]([Math]::Ceiling(($ActiveSeconds+16)/5)+1)) -PassThru -WindowStyle Hidden -RedirectStandardError (Join-Path $agentDir 'bridge.log')
  }
  $argsList=@("http://127.0.0.1:$Port",('"'+(Join-Path $data 'agent-token.txt')+'"'),('"'+$agentDir+'"'))
  if(!$enabled){$argsList+='--hardware-off'}
  $argsList+=@('--active-seconds',[string]$ActiveSeconds)
  $fixture=Start-Process (Join-Path $agentDir 'agent.exe') -ArgumentList $argsList -PassThru -WindowStyle Hidden -RedirectStandardOutput (Join-Path $agentDir 'fixture.log')
  $sseRequest=[Net.Http.HttpRequestMessage]::new([Net.Http.HttpMethod]::Get,"http://127.0.0.1:$Port/api/events")
  $sseRequest.Headers.Authorization=[Net.Http.Headers.AuthenticationHeaderValue]::new('Bearer',$view)
  $sseResponse=$http.SendAsync($sseRequest,[Net.Http.HttpCompletionOption]::ResponseHeadersRead).GetAwaiter().GetResult()
  $sse=[IO.StreamReader]::new($sseResponse.Content.ReadAsStreamAsync().GetAwaiter().GetResult())
  $pending=$sse.ReadLineAsync()
  $cpuStart=@{};$cpuLast=@{};$ws=[Collections.Generic.List[double]]::new();$cpuPeaks=[Collections.Generic.List[double]]::new();$rtt=[Collections.Generic.List[double]]::new()
  $payload=0L;$sseEvents=0;$sampleCount=0;$samples=[Collections.Generic.List[object]]::new();$clock=[Diagnostics.Stopwatch]::StartNew();$previousAt=0.0
  while(!$fixture.HasExited -and $clock.Elapsed.TotalSeconds -lt ($ActiveSeconds+16)){
   $cpuDelta=0.0;$totalWS=0.0;$threads=0;$handles=0;$perProcess=[Collections.Generic.List[object]]::new()
   foreach($process in @($server,$fixture,$bridge)){
    if($process -and !$process.HasExited){
     $process.Refresh();$idKey=[string]$process.Id;$cpu=$process.TotalProcessorTime.TotalSeconds
     if(!$cpuStart.ContainsKey($idKey)){$cpuStart[$idKey]=$cpu}
     if($cpuLast.ContainsKey($idKey)){$cpuDelta+=$cpu-$cpuLast[$idKey]}
     $cpuLast[$idKey]=$cpu;$totalWS+=$process.WorkingSet64;$threads+=$process.Threads.Count;$handles+=$process.HandleCount;$perProcess.Add([pscustomobject]@{role=$(if($process.Id -eq $server.Id){"server"}elseif($process.Id -eq $fixture.Id){"agent"}else{"bridge"});pid=$process.Id;working_set_bytes=$process.WorkingSet64;private_bytes=$process.PrivateMemorySize64;threads=$process.Threads.Count;handles=$process.HandleCount})
    }
   }
   $at=$clock.Elapsed.TotalSeconds
   if($previousAt -gt 0 -and $at -gt $previousAt){$cpuPeaks.Add(100*$cpuDelta/($at-$previousAt))}
   $previousAt=$at;$ws.Add($totalWS);$samples.Add([pscustomobject]@{elapsed_seconds=$at;working_set_bytes=$totalWS;threads=$threads;handles=$handles;cpu_delta_seconds=$cpuDelta;processes=$perProcess})
   $request=[Net.Http.HttpRequestMessage]::new([Net.Http.HttpMethod]::Get,"http://127.0.0.1:$Port/api/status")
   $request.Headers.Authorization=[Net.Http.Headers.AuthenticationHeaderValue]::new('Bearer',$view)
   $timer=[Diagnostics.Stopwatch]::StartNew()
   $response=$http.SendAsync($request).GetAwaiter().GetResult()
   $text=$response.Content.ReadAsStringAsync().GetAwaiter().GetResult();$rtt.Add($timer.Elapsed.TotalMilliseconds)
   if([int]$response.StatusCode -ne 200){throw 'Benchmark status failed'}
   $payload+=[Text.Encoding]::UTF8.GetByteCount($text);$response.Dispose();$request.Dispose()
   while($pending.IsCompleted){
    $line=$pending.GetAwaiter().GetResult();if($null -eq $line){throw 'SSE closed during short run'}
    $payload+=[Text.Encoding]::UTF8.GetByteCount($line)+1
    if($line -eq 'event: snapshot'){$sseEvents++}
    $pending=$sse.ReadLineAsync()
   }
   $sampleCount++;Start-Sleep -Milliseconds 500
  }
  if(!$fixture.WaitForExit(3000)){throw 'Fixture timed out'}
  if($fixture.ExitCode -ne 0){throw 'Fixture telemetry failed'}
  $elapsed=$clock.Elapsed.TotalSeconds;$cpuSeconds=0.0
  foreach($key in $cpuStart.Keys){$cpuSeconds+=$cpuLast[$key]-$cpuStart[$key]}
  $steady=@($samples|Where-Object {$_.elapsed_seconds -ge 10 -and $_.elapsed_seconds -le ($ActiveSeconds+3.5)})
  $sample=$null;if($enabled -and (Test-Path $cache)){$sample=Get-Content $cache -Raw|ConvertFrom-Json}
  $records.Add([pscustomobject]@{phase=$phase;active_seconds=$ActiveSeconds;steady_samples=$steady;elapsed_seconds=$elapsed;sample_interval_ms=500;samples=$sampleCount;cpu_seconds_observed=$cpuSeconds;cpu_percent_one_core_average=100*$cpuSeconds/$elapsed;cpu_percent_one_core_peak=($cpuPeaks|Measure-Object -Maximum).Maximum;cpu_percent_machine_average=100*$cpuSeconds/$elapsed/[Environment]::ProcessorCount;logical_processors=[Environment]::ProcessorCount;combined_working_set_bytes_min=($ws|Measure-Object -Minimum).Minimum;combined_working_set_bytes_max=($ws|Measure-Object -Maximum).Maximum;get_rtt_ms_average=($rtt|Measure-Object -Average).Average;get_rtt_ms_maximum=($rtt|Measure-Object -Maximum).Maximum;get_sse_payload_bytes=$payload;get_sse_payload_bytes_per_second=$payload/$elapsed;sse_snapshots=$sseEvents;sse_closed_unexpectedly=$false;hardware_sample=$sample;whole_machine_power_watts=$null})
  $sse.Dispose();$sse=$null;$sseResponse.Dispose();$sseRequest.Dispose()
  if($bridge -and !$bridge.HasExited){Stop-Process -Id $bridge.Id;$bridge.WaitForExit()};$bridge=$null
 }
}finally{
 foreach($process in @($fixture,$bridge,$server)){if($process -and !$process.HasExited){Stop-Process -Id $process.Id}}
 if($sse){$sse.Dispose()};if($http){$http.Dispose()}
 $records|ConvertTo-Json -Depth 12|Set-Content (Join-Path $out 'performance.json') -Encoding utf8
 $records|Select-Object phase,elapsed_seconds,cpu_percent_one_core_average,cpu_percent_one_core_peak,combined_working_set_bytes_max,get_rtt_ms_average,sse_snapshots|Format-Table
 Write-Output ('Evidence directory: '+$out)
}
