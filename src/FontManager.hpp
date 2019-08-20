#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

#include <Text/FontFace.hpp>
#include <Utility/Result.hpp>
#include <Utility/StringId.hpp>

#include <Text/IFontManager.hpp>

namespace fs = std::filesystem;
using std::string;
using std::unordered_map;

namespace arc {

class IFontLibrary;

class FontManager : public IFontManager {
private:
  IFontLibrary &m_fontLibrary;

  fs::path m_fontSearchPath;

  unordered_map<StringId, FontFace> m_fontFaces;

public:
  FontManager(IFontLibrary &fontLibrary);
  ~FontManager() final;

  void setFontSearchPath(fs::path const &fontSearchPath);
  void loadFontFace(const StringId name, string const &file);
  FontFace const &getFontFace(const StringId name) const;

  fs::path const &getFontSearchPath() const;

  size_t getFontFacesCount() const;
  unordered_map<StringId, FontFace> const &getFontFaceTable() const;
};

} // namespace arc
