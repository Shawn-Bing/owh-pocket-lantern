// .NET Framework 4.8; no NuGet packages and no driver/service mutation APIs.
using System;
using System.Collections;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using Microsoft.Win32.SafeHandles;

namespace Owh.Sensors {
sealed class SensorFailure : Exception {
    internal readonly string Code;
    internal SensorFailure(string code) { Code = code; }
}
sealed class Reading {
    internal double? Value;
    internal string Unit, Source, Sensor = "", Reason;
    internal Reading(string unit, string source, string reason) { Unit=unit; Source=source; Reason=reason; }
    internal static Reading Valid(double value, string unit, string source, string sensor, double max) {
        if (Double.IsNaN(value) || Double.IsInfinity(value) || value < 0 || value > max)
            return new Reading(unit, source, "sensor_out_of_range");
        return new Reading(unit, source, "") { Value=value, Sensor=sensor };
    }
}
sealed class CpuResult {
    internal Reading Temperature = new Reading("celsius", "lhm-pawnio", "cpu_backend_error");
    internal Reading Power = new Reading("watts", "lhm-pawnio", "cpu_backend_error");
    internal static CpuResult Missing(string reason) {
        return new CpuResult { Temperature=new Reading("celsius","lhm-pawnio",reason), Power=new Reading("watts","lhm-pawnio",reason) };
    }
}
// One bounded worker per backend. A stuck native call consumes one background
// thread; later attempts return timeout rather than creating more threads.
sealed class Worker<T> {
    readonly object gate = new object();
    readonly AutoResetEvent wake = new AutoResetEvent(false);
    readonly Func<T> collect;
    long request, completed;
    bool busy;
    T result;
    internal readonly ManualResetEvent Done = new ManualResetEvent(false);
    internal Worker(Func<T> collect) {
        this.collect=collect;
        new Thread(Run) { IsBackground=true, Name="owh-sensor-backend" }.Start();
    }
    internal bool Request(long id) {
        lock(gate) {
            if(busy) return false;
            busy=true; request=id; Done.Reset(); wake.Set(); return true;
        }
    }
    void Run() {
        for(;;) {
            wake.WaitOne();
            T value=collect();
            lock(gate) { result=value; completed=request; busy=false; Done.Set(); }
        }
    }
    internal T Read(long id) { lock(gate) return completed==id ? result : default(T); }
}
static class Native {
    [DllImport("kernel32.dll")] internal static extern ulong GetTickCount64();
    [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
    internal static extern IntPtr LoadLibraryEx(string path, IntPtr file, uint flags);
    [DllImport("kernel32.dll", CharSet=CharSet.Ansi)] internal static extern IntPtr GetProcAddress(IntPtr library, string name);
    [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
    internal static extern SafeFileHandle CreateFile(string path, uint access, uint share, IntPtr security, uint disposition, uint flags, IntPtr template);
    [DllImport("advapi32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
    internal static extern IntPtr OpenSCManager(string machine, string database, uint access);
    [DllImport("advapi32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
    internal static extern IntPtr OpenService(IntPtr manager, string name, uint access);
    [DllImport("advapi32.dll", SetLastError=true)]
    internal static extern bool QueryServiceStatus(IntPtr service, out ServiceStatus status);
    [DllImport("advapi32.dll")] internal static extern bool CloseServiceHandle(IntPtr handle);
    [StructLayout(LayoutKind.Sequential)]
    internal struct ServiceStatus { internal uint Type, State, Accepted, Win32Exit, ServiceExit, Checkpoint, WaitHint; }
    internal static bool PawnRunning() {
        IntPtr manager=OpenSCManager(null,null,1); if(manager==IntPtr.Zero) return false;
        try {
            IntPtr service=OpenService(manager,"PawnIO",4); if(service==IntPtr.Zero) return false;
            try { ServiceStatus status; return QueryServiceStatus(service,out status) && status.State==4; }
            finally { CloseServiceHandle(service); }
        } finally { CloseServiceHandle(manager); }
    }
}
sealed class Nvml {
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int Init();
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int Count(out uint count);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int Handle(uint index, out IntPtr device);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int Uuid(IntPtr device, StringBuilder uuid, uint length);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int Power(IntPtr device, out uint milliwatts);
    IntPtr library, device;
    Power power;
    string uuid="", failure;
    readonly string selected;
    internal Nvml(string selected) { this.selected=selected; }
    T Api<T>(string name) where T:class {
        IntPtr address=Native.GetProcAddress(library,name);
        if(address==IntPtr.Zero) throw new SensorFailure("nvml_api_missing");
        return Marshal.GetDelegateForFunctionPointer(address,typeof(T)) as T;
    }
    void Initialize() {
        // Absolute system location only; never load DLLs from CWD/PATH.
        library=Native.LoadLibraryEx(Path.Combine(Environment.SystemDirectory,"nvml.dll"),IntPtr.Zero,0x800);
        if(library==IntPtr.Zero) throw new SensorFailure("nvml_library_missing");
        int code=Api<Init>("nvmlInit_v2")(); if(code!=0) throw new SensorFailure("nvml_init_"+code);
        uint count; code=Api<Count>("nvmlDeviceGetCount_v2")(out count);
        if(code!=0) throw new SensorFailure("nvml_enumeration_"+code);
        if(count==0) throw new SensorFailure("no_nvidia_gpu");
        if(count>32) throw new SensorFailure("gpu_limit");
        if(count!=1 && selected=="") throw new SensorFailure("gpu_selection_required");
        Handle get=Api<Handle>("nvmlDeviceGetHandleByIndex_v2");
        Uuid id=Api<Uuid>("nvmlDeviceGetUUID");
        for(uint index=0; index<count; ++index) {
            IntPtr candidate; if(get(index,out candidate)!=0) continue;
            var text=new StringBuilder(96); if(id(candidate,text,96)!=0) continue;
            if(selected=="" || String.Equals(text.ToString(),selected,StringComparison.Ordinal)) {
                device=candidate; uuid=text.ToString(); break;
            }
        }
        if(device==IntPtr.Zero) throw new SensorFailure("gpu_not_found");
        power=Api<Power>("nvmlDeviceGetPowerUsage");
    }
    internal Reading Sample() {
        try {
            if(failure!=null) return new Reading("watts","nvml",failure);
            if(power==null) Initialize();
            uint milliwatts; int code=power(device,out milliwatts);
            if(code!=0) return new Reading("watts","nvml","nvml_power_"+code) { Sensor=uuid };
            return Reading.Valid(milliwatts/1000.0,"watts","nvml",uuid,3000);
        } catch(SensorFailure e) { failure=e.Code; }
          catch { failure="nvml_backend_error"; }
        return new Reading("watts","nvml",failure);
    }
}
sealed class Cpu {
    // Audited installed PawnIO-only build. Unknown/updated builds fail closed;
    // a version string alone does not prove Computer.Open is driver-safe.
    const string AuditedHash="85C97A4069BA38422B022DBCD4D4797D17FEB0BCBC670B555AC5E65B1AD81B63";
    readonly string directory;
    object computer, hardware;
    string failure;
    internal Cpu(string directory, bool enabled) { this.directory=directory; if(!enabled) failure="cpu_disabled"; }
    static object Property(object value, string name) { return value.GetType().GetProperty(name).GetValue(value,null); }
    void Initialize() {
        if(directory=="" || !Directory.Exists(directory)) throw new SensorFailure("lhm_library_missing");
        string path=Path.Combine(directory,"LibreHardwareMonitorLib.dll");
        if(!File.Exists(path)) throw new SensorFailure("lhm_library_missing");
        string hash;
        using(var input=File.OpenRead(path)) using(var sha=SHA256.Create())
            hash=BitConverter.ToString(sha.ComputeHash(input)).Replace("-","");
        if(hash!=AuditedHash) throw new SensorFailure("lhm_build_not_audited");
        if(!Native.PawnRunning()) throw new SensorFailure("pawnio_not_running");
        // OPEN_EXISTING only. No service start/create, driver install or fallback.
        using(var handle=Native.CreateFile(@"\\?\GLOBALROOT\Device\PawnIO",3,3,IntPtr.Zero,3,0x80,IntPtr.Zero)) {
            if(handle.IsInvalid) throw new SensorFailure(Marshal.GetLastWin32Error()==5 ? "cpu_access_denied" : "pawnio_device_unavailable");
        }
        AppDomain.CurrentDomain.AssemblyResolve += Resolve;
        Assembly assembly=Assembly.LoadFrom(path);
        if(assembly.GetType("LibreHardwareMonitor.Hardware.Ring0")!=null ||
           assembly.GetType("LibreHardwareMonitor.Hardware.KernelDriver")!=null ||
           assembly.GetType("LibreHardwareMonitor.PawnIo.PawnIo")==null)
            throw new SensorFailure("lhm_unsafe_backend");
        Type type=assembly.GetType("LibreHardwareMonitor.Hardware.Computer",true);
        computer=Activator.CreateInstance(type);
        foreach(string name in new[]{"IsCpuEnabled","IsGpuEnabled","IsMemoryEnabled","IsMotherboardEnabled","IsStorageEnabled","IsControllerEnabled","IsNetworkEnabled","IsPsuEnabled","IsPowerMonitorEnabled","IsBatteryEnabled"}) {
            PropertyInfo property=type.GetProperty(name);
            if(property==null) throw new SensorFailure("lhm_api_unsupported");
            property.SetValue(computer,name=="IsCpuEnabled",null);
        }
        type.GetMethod("Open").Invoke(computer,null);
        int count=0;
        foreach(object item in (IEnumerable)Property(computer,"Hardware")) {
            if(Property(item,"HardwareType").ToString()=="Cpu") { hardware=item; ++count; }
        }
        if(count!=1) throw new SensorFailure("cpu_selection_required");
        // Only the audited AMD Family17 package path is validated presently.
        // Its PawnIO module must be loaded: LHM can otherwise turn failed
        // kernel reads into zero-filled buffers.
        if(hardware.GetType().FullName!="LibreHardwareMonitor.Hardware.Cpu.Amd17Cpu")
            throw new SensorFailure("cpu_model_not_validated");
        RequireModule();
    }
    Assembly Resolve(object sender, ResolveEventArgs args) {
        string name=new AssemblyName(args.Name).Name;
        // Dependencies of the audited CPU path, loaded only from the selected
        // existing installation, never downloaded or copied into FanControl.
        if(name!="System.Memory" && name!="System.Runtime.CompilerServices.Unsafe") return null;
        string path=Path.Combine(directory,name+".dll");
        return File.Exists(path) ? Assembly.LoadFrom(path) : null;
    }
    void RequireModule() {
        FieldInfo field=hardware.GetType().GetField("_pawnModule",BindingFlags.NonPublic|BindingFlags.Instance);
        object module=field==null ? null : field.GetValue(hardware);
        field=module==null ? null : module.GetType().GetField("_pawnIo",BindingFlags.NonPublic|BindingFlags.Instance);
        object pawn=field==null ? null : field.GetValue(module);
        if(pawn==null || !(bool)Property(pawn,"IsLoaded")) throw new SensorFailure("pawnio_module_unavailable");
    }
    static Reading Read(object sensor, string unit, double max) {
        object value=Property(sensor,"Value");
        if(value==null) return new Reading(unit,"lhm-pawnio","sensor_no_value") { Sensor=Property(sensor,"Identifier").ToString() };
        return Reading.Valid(Convert.ToDouble(value,CultureInfo.InvariantCulture),unit,"lhm-pawnio",Property(sensor,"Identifier").ToString(),max);
    }
    internal CpuResult Sample() {
        try {
            if(failure!=null) return CpuResult.Missing(failure);
            if(computer==null) Initialize();
            if(!Native.PawnRunning()) return CpuResult.Missing("pawnio_not_running");
            RequireModule();
            hardware.GetType().GetMethod("Update").Invoke(hardware,null);
            var result=CpuResult.Missing("sensor_not_found");
            foreach(object sensor in (IEnumerable)Property(hardware,"Sensors")) {
                string kind=Property(sensor,"SensorType").ToString(), name=Property(sensor,"Name").ToString();
                if(kind=="Temperature" && (name=="Core (Tctl/Tdie)" || name=="CPU Package"))
                    result.Temperature=Read(sensor,"celsius",200);
                if(kind=="Power" && (name=="Package" || name=="CPU Package"))
                    result.Power=Read(sensor,"watts",3000);
            }
            // Zero package power/temperature is not trusted for this backend:
            // failed LHM kernel reads can yield numeric zero without an error.
            if(result.Temperature.Value==0) result.Temperature=new Reading("celsius","lhm-pawnio","cpu_zero_unverified");
            if(result.Power.Value==0) result.Power=new Reading("watts","lhm-pawnio","cpu_zero_unverified");
            return result;
        } catch(SensorFailure e) { failure=e.Code; }
          catch(UnauthorizedAccessException) { failure="cpu_access_denied"; }
          catch { failure="cpu_backend_error"; }
        return CpuResult.Missing(failure);
    }
}
static class Program {
    static string Q(string value) {
        return "\""+value.Replace("\\","\\\\").Replace("\"","\\\"").Replace("\r","").Replace("\n","")+"";
    }
    static string Quote(string value) { return Q(value)+"\""; }
    static string Encode(Reading r) {
        return "{\"value\":"+(r.Value.HasValue ? r.Value.Value.ToString("0.###",CultureInfo.InvariantCulture) : "null")+
            ",\"unit\":"+Quote(r.Unit)+",\"source\":"+Quote(r.Source)+",\"sensor\":"+Quote(r.Sensor)+",\"reason\":"+Quote(r.Reason)+"}";
    }
    static string Encode(CpuResult cpu, Reading gpu, ulong tick, long sequence, int interval, string sampledAt) {
        return "{\"sampled_at\":"+Quote(sampledAt)+
            ",\"sampled_tick_ms\":"+Quote(tick.ToString(CultureInfo.InvariantCulture))+
            ",\"sequence\":"+Quote(sequence.ToString(CultureInfo.InvariantCulture))+
            ",\"interval_ms\":"+interval+",\"stale_after_ms\":"+(3*interval)+
            ",\"cpu_package_temperature\":"+Encode(cpu.Temperature)+",\"cpu_package_power\":"+Encode(cpu.Power)+",\"gpu_power\":"+Encode(gpu)+"}";
    }
    static string Local(string value) {
        string path=Path.GetFullPath(value);
        if(path.StartsWith(@"\\",StringComparison.Ordinal)) throw new ArgumentException("Local paths required");
        return path;
    }
    static void Publish(string output,string json) {
        // Same-volume replacement ensures readers see an entire old/new sample.
        string temp=output+".tmp";
        using(var stream=new FileStream(temp,FileMode.Create,FileAccess.Write,FileShare.None)) {
            byte[] bytes=new UTF8Encoding(false).GetBytes(json);
            stream.Write(bytes,0,bytes.Length); stream.Flush(true);
        }
        if(File.Exists(output)) File.Replace(temp,output,null);
        else File.Move(temp,output);
    }
    static string DefaultLhm() {
        foreach(string root in new[]{Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86),Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles)}) {
            string dir=Path.Combine(root,"FanControl");
            if(File.Exists(Path.Combine(dir,"LibreHardwareMonitorLib.dll"))) return dir;
        }
        return "";
    }
    static int Main(string[] args) {
        try {
            string output="",directory=DefaultLhm(),gpu="";
            int interval=5000,samples=0; bool cpu=true,selfTest=false;
            for(int i=0;i<args.Length;++i) {
                switch(args[i]) {
                    case "--output": output=Local(args[++i]); break;
                    case "--lhm-dir": directory=Local(args[++i]); break;
                    case "--gpu-uuid": gpu=args[++i]; if(gpu.Length>96) throw new ArgumentException(); break;
                    case "--interval-ms": interval=Int32.Parse(args[++i],CultureInfo.InvariantCulture); break;
                    case "--samples": samples=Int32.Parse(args[++i],CultureInfo.InvariantCulture); if(samples<1) throw new ArgumentException(); break;
                    case "--no-cpu": cpu=false; break;
                    case "--self-test": selfTest=true; break;
                    default: throw new ArgumentException();
                }
            }
            if(selfTest) return SelfTest();
            if(output=="" || interval<3000 || interval>60000) throw new ArgumentException();
            Directory.CreateDirectory(Path.GetDirectoryName(output));
            // Tick-based sequence is monotonic across bridge process restarts
            // on the same boot; only one writer may own a cache.
            using(var mutex=new Mutex(false,@"Local\OWH.Sensor."+HashPath(output))) {
                bool owns;
                try { owns=mutex.WaitOne(0); } catch(AbandonedMutexException) { owns=true; }
                if(!owns) { Console.Error.WriteLine("cache_already_owned"); return 3; }
                try {
                    var cpuBackend=new Cpu(directory,cpu); var gpuBackend=new Nvml(gpu);
                    var cpuWorker=new Worker<CpuResult>(cpuBackend.Sample);
                    var gpuWorker=new Worker<Reading>(gpuBackend.Sample);
                    long sequence=(long)Native.GetTickCount64(); int taken=0;
                    for(;;) {
                        ulong started=Native.GetTickCount64(); ++sequence;
                        string sampledAt=DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fffZ",CultureInfo.InvariantCulture);
                        bool cr=cpuWorker.Request(sequence),gr=gpuWorker.Request(sequence);
                        var handles=new WaitHandle[]{cpuWorker.Done,gpuWorker.Done};
                        if(cr && gr) WaitHandle.WaitAll(handles,1500);
                        else if(cr) cpuWorker.Done.WaitOne(1500);
                        else if(gr) gpuWorker.Done.WaitOne(1500);
                        CpuResult c=cpuWorker.Read(sequence) ?? CpuResult.Missing("cpu_sample_timeout");
                        Reading g=gpuWorker.Read(sequence) ?? new Reading("watts","nvml","gpu_sample_timeout");
                        Publish(output,Encode(c,g,started,sequence,interval,sampledAt));
                        if(samples>0 && ++taken>=samples) return 0;
                        ulong elapsed=Native.GetTickCount64()-started;
                        if(elapsed<(ulong)interval) Thread.Sleep(interval-(int)elapsed);
                    }
                } finally { mutex.ReleaseMutex(); }
            }
        } catch(ArgumentException) {
            Console.Error.WriteLine("Usage: owh-sensors.exe --output <local-cache.json> [--lhm-dir <FanControl directory>] [--gpu-uuid GPU-...] [--interval-ms 3000..60000] [--samples N] [--no-cpu]");
            return 2;
        } catch { Console.Error.WriteLine("sensor_bridge_failed"); return 1; }
    }
    static string HashPath(string path) {
        using(var sha=SHA256.Create()) return BitConverter.ToString(sha.ComputeHash(Encoding.UTF8.GetBytes(path.ToUpperInvariant()))).Replace("-","");
    }
    static int SelfTest() {
        int calls=0;
        var release=new ManualResetEvent(false);
        var worker=new Worker<string>(delegate { Interlocked.Increment(ref calls); release.WaitOne(); return "done"; });
        if(!worker.Request(1) || worker.Done.WaitOne(30) || worker.Request(2) || worker.Read(1)!=null) return 10;
        release.Set(); if(!worker.Done.WaitOne(1000) || worker.Read(1)!="done" || worker.Read(2)!=null || calls!=1) return 11;
        if(Encode(new Reading("watts","nvml","missing")).IndexOf("\"value\":null",StringComparison.Ordinal)<0) return 12;
        Console.WriteLine("sensor worker timeout/late-result/null tests passed"); return 0;
    }
}
}
