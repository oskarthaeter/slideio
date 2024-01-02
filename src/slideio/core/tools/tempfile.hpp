// This file is part of slideio project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://slideio.com/license.html.
#pragma once
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace slideio {
class TempFile {
   public:
    explicit TempFile(fs::path path) : m_path(path) {
        if (fs::exists(path))
            fs::remove(path);
    }
    TempFile() : TempFile(static_cast<const char*>(nullptr)) {
    }
    explicit TempFile(const char* ext) {
        std::string pattern("%%%%-%%%%-%%%%-%%%%.");
        if (ext == nullptr || *ext == 0) {
            pattern += "temp";
        } else {
            pattern += ext;
        }
        m_path = fs::temp_directory_path();
        m_path /= fs::path(pattern);
    }
    const fs::path& getPath() const {
        return m_path;
    }
    ~TempFile() {
        if (fs::exists(m_path))
            fs::remove(m_path);
    }

   private:
    fs::path m_path;
};
}  // namespace slideio