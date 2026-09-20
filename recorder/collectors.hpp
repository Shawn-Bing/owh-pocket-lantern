#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include "recorder/storage.hpp"
#include <iphlpapi.h>
#include <icmpapi.h>
#include <psapi.h>
#include <winevt.h>
#include <shlobj.h>
#include <gdiplus.h>
#include <thread>

namespace owh::rec {
inline std::uint64_t ticks(FILETIME t) { return (std::uint64_t(t.dwHighDateTime)<<32)|t.dwLowDateTime; }
inline std::wstring environment(const wchar_t *name) {
  wchar_t value[32768]{}; GetEnvironmentVariableW(name,value,32768); return value;
}
inline std::wstring lower(std::wstring text) {
  std::transform(text.begin(),text.end(),text.begin(),[](wchar_t c){return wchar_t(towlower(c));});
  return text;
}
struct Processes {
  bool ok{};
  std::set<DWORD> ids;
  std::string json, gameIdentity;
};
inline Processes processes(bool demo) {
  Processes result; std::ostringstream out; out<<"[";
  Handle snap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0));
  PROCESSENTRY32W e{}; e.dwSize=sizeof(e);
  if (snap.h==INVALID_HANDLE_VALUE || !Process32FirstW(snap.h,&e)) {result.json="null";return result;}
  bool comma=false;
  const auto game=demo?L"owh-demo.exe":L"overwatch.exe";
  do {
    auto name=lower(e.szExeFile);
    bool wanted=name==game || name==L"owneacclient.exe" || name==L"neacloader.exe" ||
      name==L"battle.net.exe" || name==L"overwatch-helper.exe" || name==L"owh-recorder.exe" || name==L"agent.exe";
    if (!wanted || (demo && name!=game && name!=L"owh-recorder.exe")) continue;
    Handle h(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,e.th32ProcessID));
    if (name==L"agent.exe" && (!h.h || lower(fullProcessPath(h.h)).find(L"battle.net")==std::wstring::npos)) continue;
    result.ids.insert(e.th32ProcessID);
    if (comma) out<<","; comma=true;
    FILETIME born{},exit{},kernel{},user{};
    bool cpu=h.h && GetProcessTimes(h.h,&born,&exit,&kernel,&user);
    PROCESS_MEMORY_COUNTERS_EX memory{}; memory.cb=sizeof(memory);
    bool mem=h.h && GetProcessMemoryInfo(h.h,reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&memory),sizeof(memory));
    DWORD handles{}; bool handleOk=h.h && GetProcessHandleCount(h.h,&handles);
    out<<"{\"name\":"<<jsonString(name)<<",\"pid\":"<<e.th32ProcessID<<",\"parent_pid\":"<<e.th32ParentProcessID
       <<",\"threads\":"<<e.cntThreads<<",\"created_ft\":"<<(cpu?jsonString(std::to_string(ticks(born))):"null")
       <<",\"cpu_ms_total\":"<<(cpu?std::to_string((ticks(kernel)+ticks(user))/10000):"null")
       <<",\"working_set\":"<<(mem?std::to_string(memory.WorkingSetSize):"null")
       <<",\"private_bytes\":"<<(mem?std::to_string(memory.PrivateUsage):"null")
       <<",\"handles\":"<<(handleOk?std::to_string(handles):"null")<<"}";
    if (name==game) result.gameIdentity+=std::to_string(e.th32ProcessID)+":"+(cpu?std::to_string(ticks(born)):"unknown")+";";
  } while (Process32NextW(snap.h,&e));
  result.ok=GetLastError()==ERROR_NO_MORE_FILES; out<<"]"; result.json=out.str(); return result;
}
inline std::string address(int family,const void *bytes) {
  char text[INET6_ADDRSTRLEN]{};
  return InetNtopA(family,const_cast<void *>(bytes),text,sizeof(text))?text:"";
}
inline std::string endpoints(const std::set<DWORD> &ids) {
  std::ostringstream out; out<<"["; bool comma=false;
  for (ULONG family:{ULONG(AF_INET),ULONG(AF_INET6)}) for (bool tcp:{true,false}) {
    DWORD size{};
    auto query=[&](void *p,DWORD *n) {
      return tcp?GetExtendedTcpTable(p,n,FALSE,family,TCP_TABLE_OWNER_PID_ALL,0)
                :GetExtendedUdpTable(p,n,FALSE,family,UDP_TABLE_OWNER_PID,0);
    };
    DWORD error=query(nullptr,&size);
    std::vector<BYTE> data;
    if (error==ERROR_INSUFFICIENT_BUFFER && size<16*1024*1024) {
      data.resize(size); error=query(data.data(),&size);
    }
    auto begin=[&] {if(comma)out<<",";comma=true;};
    if (error!=NO_ERROR || data.empty()) {
      begin(); out<<"{\"protocol\":"<<jsonString(tcp?"tcp":"udp")<<",\"family\":"<<family
                 <<",\"error\":"<<error<<"}"; continue;
    }
    auto row=[&](DWORD pid,const std::string &local,DWORD port,const std::string &remote,DWORD remotePort,DWORD state,DWORD localScope=0,DWORD remoteScope=0) {
      if (!ids.count(pid)) return;
      begin(); out<<"{\"pid\":"<<pid<<",\"protocol\":"<<jsonString(tcp?"tcp":"udp")<<",\"family\":"<<family
        <<",\"local\":"<<jsonString(local)<<",\"local_port\":"<<ntohs(u_short(port))
        <<",\"local_scope\":"<<localScope<<",\"remote_scope\":"<<remoteScope;
      if (tcp) out<<",\"remote\":"<<jsonString(remote)<<",\"remote_port\":"<<ntohs(u_short(remotePort))<<",\"state\":"<<state;
      out<<"}";
    };
    if(tcp && family==AF_INET) {
      auto table=reinterpret_cast<MIB_TCPTABLE_OWNER_PID *>(data.data());
      for(DWORD i=0;i<table->dwNumEntries;++i) {auto &r=table->table[i];row(r.dwOwningPid,address(AF_INET,&r.dwLocalAddr),r.dwLocalPort,address(AF_INET,&r.dwRemoteAddr),r.dwRemotePort,r.dwState);}
    } else if(tcp) {
      auto table=reinterpret_cast<MIB_TCP6TABLE_OWNER_PID *>(data.data());
      for(DWORD i=0;i<table->dwNumEntries;++i) {auto &r=table->table[i];row(r.dwOwningPid,address(AF_INET6,r.ucLocalAddr),r.dwLocalPort,address(AF_INET6,r.ucRemoteAddr),r.dwRemotePort,r.dwState,r.dwLocalScopeId,r.dwRemoteScopeId);}
    } else if(family==AF_INET) {
      auto table=reinterpret_cast<MIB_UDPTABLE_OWNER_PID *>(data.data());
      for(DWORD i=0;i<table->dwNumEntries;++i) {auto &r=table->table[i];row(r.dwOwningPid,address(AF_INET,&r.dwLocalAddr),r.dwLocalPort,"",0,0);}
    } else {
      auto table=reinterpret_cast<MIB_UDP6TABLE_OWNER_PID *>(data.data());
      for(DWORD i=0;i<table->dwNumEntries;++i) {auto &r=table->table[i];row(r.dwOwningPid,address(AF_INET6,r.ucLocalAddr),r.dwLocalPort,"",0,0,r.dwLocalScopeId);}
    }
  }
  out<<"]"; return out.str();
}
inline std::string interfaces() {
  PMIB_IF_TABLE2 table{}; DWORD error=GetIfTable2(&table);
  if(error) return "{\"error\":"+std::to_string(error)+"}";
  std::ostringstream out; out<<"["; bool comma=false;
  for(ULONG i=0;i<table->NumEntries;++i) {
    auto &r=table->Table[i]; if(r.Type==IF_TYPE_SOFTWARE_LOOPBACK)continue;
    if(comma)out<<",";comma=true;
    out<<"{\"index\":"<<r.InterfaceIndex<<",\"alias\":"<<jsonString(std::wstring(r.Alias))
       <<",\"type\":"<<r.Type<<",\"status\":"<<r.OperStatus
       <<",\"rx_bytes\":"<<r.InOctets<<",\"tx_bytes\":"<<r.OutOctets
       <<",\"rx_errors\":"<<r.InErrors<<",\"tx_errors\":"<<r.OutErrors
       <<",\"rx_discards\":"<<r.InDiscards<<",\"tx_discards\":"<<r.OutDiscards
       <<",\"link_rx_bps\":"<<r.ReceiveLinkSpeed<<",\"link_tx_bps\":"<<r.TransmitLinkSpeed<<"}";
  }
  FreeMibTable(table); out<<"]";return out.str();
}
struct Routes { DWORD error{}; std::vector<std::string> gateways; std::string json; };
inline Routes routes() {
  Routes r; ULONG size=16384; std::vector<BYTE> bytes(size);
  auto get=[&]{return GetAdaptersAddresses(AF_UNSPEC,GAA_FLAG_INCLUDE_GATEWAYS,nullptr,reinterpret_cast<IP_ADAPTER_ADDRESSES *>(bytes.data()),&size);};
  r.error=get(); if(r.error==ERROR_BUFFER_OVERFLOW && size<1024*1024){bytes.resize(size);r.error=get();}
  if(r.error){r.json="null";return r;}
  std::ostringstream out;out<<"[";bool comma=false;
  for(auto a=reinterpret_cast<IP_ADAPTER_ADDRESSES *>(bytes.data());a;a=a->Next) {
    if(a->IfType==IF_TYPE_SOFTWARE_LOOPBACK)continue;
    if(comma)out<<",";comma=true;
    out<<"{\"index\":"<<a->IfIndex<<",\"status\":"<<a->OperStatus<<",\"v4_metric\":"<<a->Ipv4Metric<<",\"gateways\":[";
    bool comma2=false;
    for(auto g=a->FirstGatewayAddress;g;g=g->Next) {
      auto sa=g->Address.lpSockaddr;std::string ip;
      if(sa->sa_family==AF_INET) ip=address(AF_INET,&reinterpret_cast<sockaddr_in *>(sa)->sin_addr);
      else if(sa->sa_family==AF_INET6) ip=address(AF_INET6,&reinterpret_cast<sockaddr_in6 *>(sa)->sin6_addr);
      if(ip.empty())continue;if(comma2)out<<",";comma2=true;out<<jsonString(ip);
      if(sa->sa_family==AF_INET && a->OperStatus==IfOperStatusUp && r.gateways.size()<4 &&
         std::find(r.gateways.begin(),r.gateways.end(),ip)==r.gateways.end())r.gateways.push_back(ip);
    }
    out<<"],\"dns\":[";comma2=false;
    for(auto d=a->FirstDnsServerAddress;d;d=d->Next){auto sa=d->Address.lpSockaddr;std::string ip;
      if(sa->sa_family==AF_INET)ip=address(AF_INET,&reinterpret_cast<sockaddr_in *>(sa)->sin_addr);
      else if(sa->sa_family==AF_INET6)ip=address(AF_INET6,&reinterpret_cast<sockaddr_in6 *>(sa)->sin6_addr);
      if(!ip.empty()){if(comma2)out<<",";comma2=true;out<<jsonString(ip);}}
    out<<"]}";
  }
  out<<"]";r.json=out.str();return r;
}
inline DWORD ping(Store &store,HANDLE icmp,const std::string &target,const char *role) {
  IN_ADDR ip{}; if(InetPtonA(AF_INET,target.c_str(),&ip)!=1)return ERROR_INVALID_PARAMETER;
  char payload[]="OWH diagnostic";
  std::vector<BYTE> response(sizeof(ICMP_ECHO_REPLY)+sizeof(payload)+32);
  auto at=stamp(); auto start=GetTickCount64();
  DWORD count=IcmpSendEcho(icmp,ip.S_un.S_addr,payload,sizeof(payload),nullptr,response.data(),DWORD(response.size()),600);
  DWORD status=count?reinterpret_cast<ICMP_ECHO_REPLY *>(response.data())->Status:GetLastError();
  store.text("probes.jsonl","{"+at+",\"target\":"+jsonString(target)+",\"role\":"+jsonString(role)+
    ",\"status\":"+std::to_string(status)+",\"elapsed_ms\":"+std::to_string(GetTickCount64()-start)+
    ",\"rtt_ms\":"+(count && status==IP_SUCCESS?std::to_string(reinterpret_cast<ICMP_ECHO_REPLY *>(response.data())->RoundTripTime):"null")+"}\n");
  return status;
}
class SystemEvents {
  std::map<std::wstring,std::set<std::wstring>> seen_;
  std::set<std::wstring> failed_;
  Millis lastPoll_{};
public:
  void collect(Store &store) {
    auto now=GetTickCount64();
    auto lookback=lastPoll_?std::min<Millis>(now-lastPoll_+120000,18ULL*3600000+120000):120000;
    lastPoll_=now;
    const std::vector<std::pair<std::wstring,std::wstring>> channels={
      {L"System",L"*[System[TimeCreated[timediff(@SystemTime) <= 120000] and (Level<=3 or Provider[@Name='Microsoft-Windows-Kernel-Power'] or Provider[@Name='Microsoft-Windows-Power-Troubleshooter'] or Provider[@Name='Microsoft-Windows-Kernel-General'])]]"},
      {L"Application",L"*[System[TimeCreated[timediff(@SystemTime) <= 120000] and (Provider[@Name='Application Error'] or Provider[@Name='Application Hang'] or Provider[@Name='Windows Error Reporting'])]]"},
      {L"Microsoft-Windows-WLAN-AutoConfig/Operational",L"*[System[TimeCreated[timediff(@SystemTime) <= 120000]]]"},
      {L"Microsoft-Windows-NetworkProfile/Operational",L"*[System[TimeCreated[timediff(@SystemTime) <= 120000]]]"}};
    for(const auto &[channel,query]:channels) {
      auto expanded=query;auto position=expanded.find(L"120000");
      expanded.replace(position,6,std::to_wstring(lookback));
      EVT_HANDLE result=EvtQuery(nullptr,channel.c_str(),expanded.c_str(),EvtQueryChannelPath|EvtQueryForwardDirection);
      if(!result){auto error=GetLastError();if(failed_.insert(channel).second)store.event("event_channel_unavailable","\"channel\":"+jsonString(channel)+",\"error\":"+std::to_string(error));continue;}
      unsigned total=0; EVT_HANDLE events[16]{};DWORD count{};
      while(total<512 && EvtNext(result,16,events,200,0,&count)) {
        total+=count;
        for(DWORD i=0;i<count;++i) {
          DWORD needed{},properties{};
          EvtRender(nullptr,events[i],EvtRenderEventXml,0,nullptr,&needed,&properties);
          if(needed && needed<=512*1024) {
            std::vector<wchar_t> buffer(needed/2+1);
            if(EvtRender(nullptr,events[i],EvtRenderEventXml,needed,buffer.data(),&needed,&properties)) {
              std::wstring xml=buffer.data();auto keyStart=xml.find(L"<EventRecordID>");
              std::wstring key=keyStart==std::wstring::npos?xml:xml.substr(keyStart,xml.find(L"</EventRecordID>",keyStart)-keyStart);
              auto &seen=seen_[channel];
              auto small=lower(xml);
              bool relevant=channel!=L"Application" || small.find(L"overwatch")!=std::wstring::npos ||
                small.find(L"neac")!=std::wstring::npos || small.find(L"battle.net")!=std::wstring::npos;
              if(relevant && seen.insert(key).second)store.text("windows-events.jsonl","{"+stamp()+",\"channel\":"+jsonString(channel)+",\"xml\":"+jsonString(xml)+"}\n");
              if(seen.size()>4096)seen.clear();
            }
          }
          EvtClose(events[i]);
        }
      }
      if(total>=512)store.event("event_query_limit","\"channel\":"+jsonString(channel));
      EvtClose(result);
    }
  }
};
inline std::vector<std::filesystem::path> sourceRoots(bool demo) {
  if(demo)return {};
  std::vector<std::filesystem::path> paths;
  PWSTR docs{};
  if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents,0,nullptr,&docs))){paths.emplace_back(std::filesystem::path(docs)/L"Overwatch"/L"Logs");CoTaskMemFree(docs);}
  auto local=std::filesystem::path(environment(L"LOCALAPPDATA"));
  paths.push_back(local/L"Battle.net"/L"Logs");
  paths.push_back(local/L"OverwatchHelper");
  auto configured=readIni((local/L"OverwatchHelper"/L"settings.ini").wstring(),L"agent",L"neac_path");
  std::filesystem::path retail;
  if(validNeacPath(configured))retail=std::filesystem::path(configured).parent_path();
  else {auto game=findProcess(L"Overwatch.exe");Handle h(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,game.pid));if(h.h)retail=std::filesystem::path(fullProcessPath(h.h)).parent_path();}
  if(!retail.empty()){paths.push_back(retail/L"log");paths.push_back(retail/L"logs");}
  return paths;
}
inline void collectSources(Store &store,Tailer &tailer,const std::vector<std::filesystem::path> &roots) {
  for(const auto &root:roots) {
    std::error_code error;
    std::vector<std::pair<std::filesystem::file_time_type,std::filesystem::path>> files;
    std::filesystem::recursive_directory_iterator it(root,std::filesystem::directory_options::skip_permission_denied,error),end;
    if(error){store.event("source_directory_unavailable","\"path\":"+jsonString(root.wstring())+",\"error\":"+std::to_string(error.value()));continue;}
    unsigned scanned=0;
    for(;it!=end && scanned<512;it.increment(error)) {
      if(error)break;++scanned;
      if(it.depth()>=2)it.disable_recursion_pending();
      if(!it->is_regular_file(error))continue;
      auto name=lower(it->path().filename().wstring());
      bool log=name.ends_with(L".log") || name==L"agent.log.1";
      if(!log)continue;
      auto time=it->last_write_time(error);if(!error)files.emplace_back(time,it->path());
    }
    std::sort(files.begin(),files.end(),[](const auto &a,const auto &b){return a.first>b.first;});
    if(files.size()>16)files.resize(16);
    for(auto &file:files)tailer.read(store,file.second);
    if(scanned>=512)store.event("source_scan_limit","\"path\":"+jsonString(root.wstring()));
  }
}
inline bool saveJpeg(Store &store,const Frame &frame,const std::filesystem::path &name) {
  using namespace Gdiplus;
  if(frame.pixels.empty())return false;
  int width=std::min(960,frame.width),height=std::max(1,frame.height*width/frame.width);
  Bitmap source(frame.width,frame.height,frame.width*4,PixelFormat32bppRGB,
                reinterpret_cast<BYTE *>(const_cast<std::uint32_t *>(frame.pixels.data())));
  Bitmap small(width,height,PixelFormat24bppRGB);
  {Graphics g(&small);g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
   if(g.DrawImage(&source,0,0,width,height)!=Ok)return false;}
  UINT count{},size{};GetImageEncodersSize(&count,&size);if(!size)return false;
  std::vector<BYTE> codecs(size);auto info=reinterpret_cast<ImageCodecInfo *>(codecs.data());
  GetImageEncoders(count,size,info);CLSID jpeg{};bool found=false;
  for(UINT i=0;i<count;++i)if(wcscmp(info[i].MimeType,L"image/jpeg")==0){jpeg=info[i].Clsid;found=true;break;}
  if(!found)return false;
  IStream *stream{};if(FAILED(CreateStreamOnHGlobal(nullptr,TRUE,&stream)))return false;
  EncoderParameters parameters{};parameters.Count=1;ULONG quality=75;
  parameters.Parameter[0].Guid=EncoderQuality;parameters.Parameter[0].Type=EncoderParameterValueTypeLong;
  parameters.Parameter[0].NumberOfValues=1;parameters.Parameter[0].Value=&quality;
  bool ok=small.Save(stream,&jpeg,&parameters)==Ok;
  HGLOBAL memory{};STATSTG stat{};
  if(ok)ok=SUCCEEDED(GetHGlobalFromStream(stream,&memory)) && SUCCEEDED(stream->Stat(&stat,STATFLAG_NONAME));
  if(ok){auto bytes=GlobalLock(memory);ok=bytes && store.write(name,bytes,std::size_t(stat.cbSize.QuadPart),false);if(bytes)GlobalUnlock(memory);}
  stream->Release();return ok;
}
} // namespace owh::rec
