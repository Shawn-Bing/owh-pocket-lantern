#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
namespace owh {
inline std::string embeddedAsset(int id) {
  auto module=GetModuleHandleW(nullptr);
  auto resource=FindResourceW(module,MAKEINTRESOURCEW(id),MAKEINTRESOURCEW(10));
  if (!resource) return {};
  auto loaded=LoadResource(module,resource);
  auto data=LockResource(loaded);auto size=SizeofResource(module,resource);
  return data && size ? std::string(static_cast<const char*>(data),size) : std::string{};
}
inline std::filesystem::path embeddedOcrScript(const std::wstring &directory) {
  const auto bytes=embeddedAsset(201);
  if (bytes.empty()) return {};
  // Content-addressed names keep separately installed versions from overwriting
  // a script another OCR worker is still using. No installer or writable EXE dir.
  unsigned long long hash=14695981039346656037ull;
  for (unsigned char byte:bytes) hash=(hash^byte)*1099511628211ull;
  auto folder=std::filesystem::path(directory)/L"runtime";
  std::filesystem::create_directories(folder);
  auto path=folder/(L"ocr-"+std::to_wstring(hash)+L".ps1");
  std::ifstream input(path,std::ios::binary);std::ostringstream previous;previous<<input.rdbuf();input.close();
  if (previous.str()!=bytes) {
    std::ofstream output(path,std::ios::binary|std::ios::trunc);
    output.write(bytes.data(),std::streamsize(bytes.size()));output.close();
    if (!output) throw std::runtime_error("Cannot prepare embedded Windows OCR script");
  }
  return path;
}
} // namespace owh
