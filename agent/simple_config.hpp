#pragma once
#include "agent/startup.hpp"
namespace owh {
inline std::wstring simpleDataDirectory(const StartupOptions &options) {
  if (!options.dataDir.empty()) return resolveDataDirectory(options);
  auto path=std::filesystem::path(dataDirectory(false)).parent_path()/L"OverwatchHelperSimple";
  std::filesystem::create_directories(path);return path.wstring();
}
inline void applySimpleDefaults(Config &c) {
  c.dryRun=false;c.holdShift=true;c.autoRecover=true;c.pulseShift=true;
  c.standbyOnInput=true;c.standbySeconds=10;c.cnPreset=true;c.semanticRecognition=true;
  c.server.clear();c.token.clear();c.steps.clear();
  std::string error;auto profile=embeddedAsset(202);
  if (profile.empty()) profile=defaultCNProfileJson();
  if (!parseCNProfile(profile,c.cnProfile,error)) throw std::runtime_error("Invalid embedded recognition profile");
  c.cnProfileSource="builtin";c.cnProfileError.clear();
}
inline std::wstring discoverSimpleNeac() {
  for (const auto *name:{L"OWNeacClient.exe",L"Overwatch.exe",L"NeacLoader.exe"}) {
    std::vector<DWORD> ids;if (!processIds(name,ids)) continue;
    for (auto pid:ids) {
      Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid));
      if (!process.h) continue;
      auto path=std::filesystem::path(fullProcessPath(process.h)).parent_path()/L"OWNeacClient.exe";
      std::error_code ec;
      if (validNeacPath(path.wstring()) && std::filesystem::is_regular_file(path,ec)) return path.wstring();
    }
  }
  for(auto root:{HKEY_LOCAL_MACHINE,HKEY_CURRENT_USER}) for(auto view:{KEY_WOW64_32KEY,KEY_WOW64_64KEY}) {
    HKEY key{};
    if(RegOpenKeyExW(root,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Overwatch",0,KEY_READ|view,&key)!=ERROR_SUCCESS) continue;
    wchar_t location[32768]{};DWORD size=sizeof(location),type{};
    auto result=RegQueryValueExW(key,L"InstallLocation",nullptr,&type,reinterpret_cast<BYTE*>(location),&size);RegCloseKey(key);
    if(result!=ERROR_SUCCESS || (type!=REG_SZ && type!=REG_EXPAND_SZ)) continue;
    for(auto suffix:{L"_retail_\\OWNeacClient.exe",L"OWNeacClient.exe"}) {
      auto path=std::filesystem::path(location)/suffix;std::error_code ec;
      if(validNeacPath(path.wstring()) && std::filesystem::is_regular_file(path,ec)) return path.wstring();
    }
  }
  return {};
}
inline void refreshSimplePaths(Config &c) {
  auto exists=[](const std::wstring &path) {std::error_code ec;return !path.empty() && std::filesystem::is_regular_file(path,ec);};
  const auto legacy=dataDirectory(false)+L"\\settings.ini";
  if (!exists(c.launcherPath)) {
    auto path=readIni(legacy,L"agent",L"launcher_path");
    if (!exists(path)) path=discoverLauncher();
    if (!path.empty()) c.launcherPath=path;
  }
  if (!validNeacPath(c.neacPath) || !exists(c.neacPath)) {
    auto path=readIni(legacy,L"agent",L"neac_path");
    if (!validNeacPath(path) || !exists(path)) path=discoverSimpleNeac();
    if (!path.empty()) c.neacPath=path;
  }
}
inline Config loadSimpleConfig(const std::wstring &directory,const std::wstring &developerDirectory) {
  auto c=loadConfig(directory,false);
  const auto developer=loadConfig(developerDirectory,false);
  if (!std::filesystem::exists(std::filesystem::path(directory)/L"settings.ini")) {
    c.launcherPath=developer.launcherPath;c.launcherArgs=developer.launcherArgs;
    c.neacPath=developer.neacPath;c.roomTarget=developer.roomTarget;c.mode=developer.mode;
  }
  refreshSimplePaths(c);
  applySimpleDefaults(c);return c;
}
inline int simpleModeIndex(const std::wstring &dir,const Config &c) {
  return std::clamp(number(readIni(dir+L"\\settings.ini",L"simple",L"mode",std::to_wstring(int(c.mode)*2).c_str())),0,3);
}
inline bool saveSimpleConfig(const std::wstring &dir,Config &c,int selected) {
  selected=std::clamp(selected,0,3);c.mode=WorkflowMode(selected/2);applySimpleDefaults(c);
  refreshSimplePaths(c);
  saveConfig(dir,c);
  return WritePrivateProfileStringW(L"simple",L"mode",std::to_wstring(selected).c_str(),(dir+L"\\settings.ini").c_str())!=FALSE;
}
} // namespace owh
