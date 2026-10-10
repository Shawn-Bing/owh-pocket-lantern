param([int]$Port=18765,[switch]$TryActualHelper,[string]$AlertAfter="")
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$output=Join-Path $root ('build/p2-5-local-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
$serverData=Join-Path $output 'server';$agentData=Join-Path $output 'agent'
New-Item -ItemType Directory -Path $serverData,$agentData -Force|Out-Null
$results=[Collections.Generic.List[object]]::new()
$server=$null;$agent=$null;$helper=$null;$stream=$null;$client=$null;$machine='p2-5-real-demo'
function Check($name,$condition,$detail) {
 $results.Add([pscustomobject]@{name=$name;result=$(if($condition){'PASS'}else{'FAIL'});detail=$detail})
 if(!$condition){throw "Validation failed: $name"}
}
Add-Type -AssemblyName System.Net.Http
Add-Type 'using System;using System.Runtime.InteropServices;public static class MonitorValidationWindow {
[DllImport("user32.dll")]public static extern uint GetWindowThreadProcessId(IntPtr w,out uint p);
[DllImport("user32.dll")]public static extern bool ShowWindow(IntPtr w,int c);
[DllImport("user32.dll")]public static extern bool PostMessage(IntPtr w,uint m,IntPtr a,IntPtr b);}'
$base="http://127.0.0.1:$Port"
function StartDashboard {
 $dashboardArgs=@('-listen',"127.0.0.1:$Port",'-data',('"'+$serverData+'"'));if($AlertAfter){$dashboardArgs+=@('-alert-after',$AlertAfter)}
 $script:server=Start-Process (Join-Path $root 'dist/owh-dashboard.exe') -ArgumentList $dashboardArgs -WindowStyle Hidden -PassThru -RedirectStandardError (Join-Path $output ('server-'+[guid]::NewGuid().ToString('N')+'.log'))
 for($n=0;$n -lt 40;$n++){if($server.HasExited){throw "Owned isolated dashboard exited"};try{if((Invoke-WebRequest "$base/healthz" -TimeoutSec 1).StatusCode -eq 200){if($server.HasExited){throw "Owned isolated dashboard exited"};return}}catch{};Start-Sleep -Milliseconds 100}
 throw 'Dashboard did not become healthy'
}
function Request($method,$path,$token,$body=$null) {
 $req=[Net.Http.HttpRequestMessage]::new([Net.Http.HttpMethod]::new($method),$base+$path)
 if($token){$req.Headers.Authorization=[Net.Http.Headers.AuthenticationHeaderValue]::new('Bearer',$token)}
 if($null -ne $body){$req.Content=[Net.Http.StringContent]::new(($body|ConvertTo-Json -Depth 30 -Compress),[Text.Encoding]::UTF8,'application/json')}
 try{$res=$client.SendAsync($req).GetAwaiter().GetResult();try{return @{code=[int]$res.StatusCode;text=$res.Content.ReadAsStringAsync().GetAwaiter().GetResult()}}finally{$res.Dispose()}}finally{$req.Dispose()}
}
function ReadSSELine {
 $pending=$stream.ReadLineAsync();if(!$pending.Wait(5000)){throw 'SSE line read timed out'};return $pending.GetAwaiter().GetResult()
}
function Status {$r=Request 'GET' '/api/status' $viewToken;if($r.code -ne 200){throw 'status failed'};return $r.text|ConvertFrom-Json}
function WaitAgent($expected) {
 for($n=0;$n -lt 60;$n++){
  $found=@((Status).agents|Where-Object machine -eq $machine)
  if($found.Count -eq 1 -and $found[0].v2.observation.running -eq $expected){return $found[0]}
  Start-Sleep -Milliseconds 200
 }
 throw 'Expected actual agent heartbeat not received'
}
function WindowCommand($command) {
 $agent.Refresh();$window=$agent.MainWindowHandle;[uint32]$owner=0
 [void][MonitorValidationWindow]::GetWindowThreadProcessId($window,[ref]$owner)
 if(!$window -or $owner -ne $agent.Id){throw 'Cannot verify isolated helper window identity'}
 [void][MonitorValidationWindow]::ShowWindow($window,0)
 [void][MonitorValidationWindow]::PostMessage($window,0x111,[IntPtr]$command,[IntPtr]::Zero)
}
try{
 $listener=[Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,$Port);$listener.Start();$listener.Stop()
 StartDashboard
 $agentToken=([IO.File]::ReadAllText((Join-Path $serverData 'agent-token.txt'))).Trim()
 $viewToken=([IO.File]::ReadAllText((Join-Path $serverData 'view-token.txt'))).Trim()
 $client=[Net.Http.HttpClient]::new();$client.Timeout=[TimeSpan]::FromSeconds(5)
 Check 'actual_dashboard_health' $true 'Existing dist binary, isolated loopback and data'
 Check 'token_roles' (((Request 'GET' '/api/status' $agentToken).code -eq 401)-and((Request 'POST' '/api/heartbeat' $viewToken @{}).code -eq 401)-and((Request 'GET' '/api/status' '').code -eq 401)) 'Agent read/View write/unauthenticated read rejected; no credentials printed'
 $ini="[agent]\r\ngame_exe=p2-5-no-such-demo.exe\r\nserver=$base\r\ntoken=$agentToken\r\nmachine=p2-5-real-demo\r\ndry_run=1\r\nauto_recover=0\r\nhold_shift=0\r\nstandby_on_input=0\r\nhardware_enabled=1\r\nhardware_cache=$agentData\missing-parent\hardware.json\r\n"
 [IO.File]::WriteAllText((Join-Path $agentData 'settings.ini'),$ini.Replace('\r\n',[Environment]::NewLine),[Text.UTF8Encoding]::new($false))
 try{
  if (!$TryActualHelper -and (Get-Content (Join-Path $root 'agent/overwatch-helper.manifest') -Raw).Contains('requireAdministrator')) {throw 'NOT TESTED: existing helper manifest requires administrator; no elevation approved or accepted. Use actual-main integration fixture.'}
  $agent=Start-Process (Join-Path $root 'dist/overwatch-helper-dev.exe') -ArgumentList @('--demo','--data-dir',('"'+$agentData+'"')) -WindowStyle Hidden -PassThru
  $helper=$agent
  $stopped=WaitAgent $false
  WindowCommand 101
  $started=WaitAgent $true
  Check 'actual_helper_start_and_http_v2' ($started.dry_run -and $started.v2.agent_pid -eq $agent.Id -and $started.monitor.state -ne 'IN_MATCH') 'Actual main/worker/telemetry; isolated demo dry-run with nonexistent target; no game launched'
  Check 'hardware_failure_isolation' ($null -eq $started.v2.hardware.cpu_package_temperature.value -and $started.v2.hardware.cpu_package_temperature.reason -and !$agent.HasExited) 'Nullable failed sensor does not stop actual helper'
  WindowCommand 102
  $stoppedAgain=WaitAgent $false
  Check 'actual_helper_stop_retracts' ($stoppedAgain.state -eq 'STOPPED' -and $stoppedAgain.monitor.state -ne 'IN_MATCH') 'Actual stop publishes running=false with revised session'
 }catch{$blocked=$_.Exception.Message.StartsWith('NOT TESTED: existing helper manifest');$results.Add([pscustomobject]@{name='actual_helper_start_stop';result=$(if($blocked){'BLOCKED'}else{'FAIL'});detail=$_.Exception.Message});if(!$blocked){throw}}
 # Non-elevated actual-main fixture runs telemetry() but no worker or input.
 $machine='p2-5-real-main-fixture'
 $agent=Start-Process (Join-Path $root 'build/windows/agent-telemetry-integration.exe') -ArgumentList @($base,('"'+(Join-Path $serverData 'agent-token.txt')+'"'),('"'+$agentData+'"')) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $output 'actual-main.log') -RedirectStandardError (Join-Path $output 'actual-main-error.log')
 $initial=WaitAgent $false
 $started=WaitAgent $true
 Check 'actual_main_telemetry_start' ($started.dry_run -and $started.v2.agent_pid -eq $agent.Id) 'Actual main.cpp telemetry thread; start ticket simulated; no worker/input/GUI/sensor process'
 Check 'actual_main_hardware_failure' ($null -eq $started.v2.hardware.cpu_package_power.value -and $started.v2.hardware.cpu_package_power.reason -eq 'bridge_cache_unavailable') 'Missing isolated cache does not block actual telemetry HTTP'
 $stopped=WaitAgent $false
 Check 'actual_main_stop' ($stopped.state -eq 'STOPPED' -and $stopped.monitor.state -ne 'IN_MATCH') 'Actual setRunning(false) and telemetry send stopped observation'
 if(!$agent.WaitForExit(6000)){throw 'Actual-main integration fixture did not exit'}
 Check 'actual_main_exit' ($agent.ExitCode -eq 0) 'Fixture confirms HTTP 200 and joins actual telemetry thread'
 # C++ serializes the full envelope; identity/UTC are adapted for wall-clock HTTP.
 # Game/window proof remains explicitly simulated, not a real screenshot.
 $fixture=(& (Join-Path $root 'build/windows/telemetry-v2-tests.exe') --fixture)|ConvertFrom-Json
 if($LASTEXITCODE -ne 0){throw 'C++ fixture failed'}
 $fixture.machine='p2-5-cpp-simulated';$fixture.v2.sent_at=[DateTime]::UtcNow.ToString('o');$fixture.v2.hardware.sampled_at=$fixture.v2.sent_at
 Check 'cpp_live_http_post' ((Request 'POST' '/api/heartbeat' $agentToken $fixture).code -eq 200) 'Actual C++ envelope; adapted UTC; simulated game/window facts'
 $firstSnapshot=Status
 $firstSnapshot|ConvertTo-Json -Depth 30|Set-Content (Join-Path $output "fresh-status.json") -Encoding utf8
 $first=@($firstSnapshot.agents|Where-Object machine -eq $fixture.machine)[0]
 Check 'cpp_get_in_match_cpu_null' ($first.monitor.state -eq 'IN_MATCH' -and $null -eq $first.v2.hardware.cpu_package_power.value -and $first.v2.hardware.cpu_package_power.reason -eq 'cpu_access_denied') 'Fresh simulated proof reaches GET independently from CPU failure'
 $sseReq=[Net.Http.HttpRequestMessage]::new([Net.Http.HttpMethod]::Get,$base+'/api/events')
 $sseReq.Headers.Authorization=[Net.Http.Headers.AuthenticationHeaderValue]::new('Bearer',$viewToken)
 $sseRes=$client.SendAsync($sseReq,[Net.Http.HttpCompletionOption]::ResponseHeadersRead).GetAwaiter().GetResult()
 $stream=[IO.StreamReader]::new($sseRes.Content.ReadAsStreamAsync().GetAwaiter().GetResult())
 $eventLine=ReadSSELine;$dataLine=ReadSSELine;$sse=$dataLine.Substring(6)|ConvertFrom-Json
 $sseAgent=@($sse.agents|Where-Object machine -eq $fixture.machine)[0]
 Check 'actual_sse_matches_get' ($eventLine -eq 'event: snapshot' -and $sseAgent.monitor.state -eq $first.monitor.state) 'Real HTTP SSE agrees with GET'
 $stream.Dispose();$stream=$null;$sseRes.Dispose();$sseReq.Dispose()
 $fixture.v2.sequence='2';$fixture.v2.sent_at_ms='10001';$fixture.v2.sent_at=[DateTime]::UtcNow.ToString('o')
 foreach($name in @('cpu_package_temperature','cpu_package_power','gpu_power')){$fixture.v2.hardware.$name.value=$null;$fixture.v2.hardware.$name.reason='validation_sensor_unavailable'}
 Check 'all_sensors_null_still_in_match' (((Request 'POST' '/api/heartbeat' $agentToken $fixture).code -eq 200)-and(@((Status).agents|Where-Object machine -eq $fixture.machine)[0].monitor.state -eq 'IN_MATCH')) 'Failed hardware never drives match state'
 $watch=[Diagnostics.Stopwatch]::StartNew();while($watch.Elapsed.TotalSeconds -lt 18){Start-Sleep -Milliseconds 200}
 $expired=@((Status).agents|Where-Object machine -eq $fixture.machine)[0]
 if($AlertAfter){Check 'actual_alert_threshold' ($expired.alert.enabled -and $expired.alert.reached -and $expired.alert.kind -eq 'UNVERIFIED') 'Actual server timer emits alert after scaled 2s threshold; game proof expires at 15s'}
 Check 'actual_server_expiry' ($expired.monitor.state -eq 'UNVERIFIED' -and $null -eq $expired.v2.hardware.gpu_power.value) 'Real elapsed time exceeds 15s without new proof'
 $fixture.v2.sequence='3';$fixture.v2.sent_at_ms='30000';$fixture.v2.sent_at=[DateTime]::UtcNow.ToString('o')
 foreach($name in @('last_confirmed_at_ms','last_observed_at_ms','last_reliable_visual_at_ms','process_observed_at_ms')){$fixture.v2.observation.$name='30000'}
 $fixture.v2.hardware.sampled_tick_ms='30000';$fixture.v2.hardware.sampled_at=$fixture.v2.sent_at
 Check 'new_proof_recovers' (((Request 'POST' '/api/heartbeat' $agentToken $fixture).code -eq 200)-and(@((Status).agents|Where-Object machine -eq $fixture.machine)[0].monitor.state -eq 'IN_MATCH')) 'New simulated proof recovers state'
 if($AlertAfter){Check 'actual_alert_resolved' (@((Status).agents|Where-Object machine -eq $fixture.machine)[0].alert.kind -eq 'NONE') 'New credible simulated IN_MATCH resolves alert'}
 Start-Sleep -Seconds 11
 Stop-Process -Id $server.Id;$server.WaitForExit();$server=$null
 StartDashboard
 $restarted=@((Status).agents|Where-Object machine -eq $fixture.machine)[0]
 Check 'actual_restart_unverified' ($restarted.monitor.state -eq 'UNVERIFIED' -and $null -eq $restarted.v2.hardware.gpu_power.value) 'Persisted IN_MATCH does not revive after actual server restart'
 if($AlertAfter){Check 'actual_alert_restart_disabled' (!$restarted.alert.enabled) 'Restart does not restore expected running or alert timer from disk'}
 Check 'dashboard_embedded_resources' (((Request 'GET' '/' '').code -eq 200)-and((Request 'GET' '/monitor.js' '').code -eq 200)-and((Request 'GET' '/app.js' '').code -eq 200)) 'Actual embedded resources; DOM behavior separately tested'
}finally{
 if($stream){$stream.Dispose()}
 foreach($ownedHelper in @($helper,$agent)){if($ownedHelper -and !$ownedHelper.HasExited){try{$ownedHelper.CloseMainWindow()|Out-Null;if(!$ownedHelper.WaitForExit(5000)){Stop-Process -Id $ownedHelper.Id}}catch{$results.Add([pscustomobject]@{name="isolated_helper_cleanup";result="BLOCKED";detail="OS denied helper cleanup; close only test PID $($ownedHelper.Id) manually"})}}}
 if($server -and !$server.HasExited){Stop-Process -Id $server.Id;$server.WaitForExit()}
 if($client){$client.Dispose()}
 $results|ConvertTo-Json -Depth 6|Set-Content (Join-Path $output 'results.json') -Encoding utf8
 $results|Format-Table name,result,detail -Wrap
 Write-Output ('Evidence directory: '+$output)
}
