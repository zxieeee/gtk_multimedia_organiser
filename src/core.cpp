#include "../include/core.h"
#include <filesystem>
#include <iostream>

// TODO: Implement system level calls to check change in direcotry INOTIFY and
// Windows read directory
std::string fileOperations::getfile_ext(const std::filesystem::path &filename) {
  std::string ext = filename.extension().string();
  if (!ext.empty() && ext.front() == '.') {
    return ext.substr(1);
  } else
    return "";
}
bool isdirectorychanged(const std::string &path,
                        std::filesystem::file_time_type &lastWriteTime) {
  std::filesystem::file_time_type newWriteTime =
      std::filesystem::last_write_time(path);
  if (newWriteTime != lastWriteTime) {
    lastWriteTime = newWriteTime;
    return true;
  }
  return false;
}

std::filesystem::path
fileOperations::createFolder(const std::string &DEST_PATH,
                             const std::filesystem::path &folder) {
  std::filesystem::path dest_path(DEST_PATH);
  if (!std::filesystem::exists(DEST_PATH / folder)) {
    std::filesystem::create_directory(DEST_PATH / folder);
  }
  dest_path = dest_path / folder;
  return dest_path;
}

int fileOperations::attemptToMove(const std::filesystem::path &file_path,
                                  const std::filesystem::path &dest_path) {
  if (std::filesystem::exists(file_path)) {
    try {
      std::filesystem::rename(file_path, dest_path / file_path.filename());

    } catch (const std::exception &e) {
      std::cerr << "Error moving file: " << file_path << " - " << e.what()
                << std::endl;
      return 1;
    }
  }
  return 0;
}

// path fileOperations::getPathfromConfig(const configV &configV_umap) {
//   return SOURCE, DEST_PATH;
// }
// TODO: Implement getPathFromConfig function

void fileOperations::defaultMoveBehaviour(const std::string &SOURCE,
                                          const std::string &DEST_PATH) {
  std::filesystem::path SourceDir(SOURCE);
  for (const auto &file : std::filesystem::directory_iterator(SOURCE)) {
    if (std::filesystem::is_regular_file(file)) {
      std::string ext = getfile_ext(file);
      std::filesystem::path file_path(file);
      std::filesystem::path folder(ext + "_folder");
      std::filesystem::path dest_path = createFolder(DEST_PATH, folder);
      SUCCESS = attemptToMove(file_path, dest_path);
    }
  }
}

void fileOperations::moveFileinDir(const std::string &SOURCE,
                                   const std::string &DEST_PATH,
                                   const key_value &key_umap) {
  std::filesystem::path dest_path;
  std::filesystem::path SourceDir(SOURCE);
  for (const auto &pair : key_umap) {
    if (pair.first != "source_path" || pair.first != "dest_path") {
      std::filesystem::path folder(pair.first);
      for (const auto &extension : pair.second) {
        for (const auto &file : std::filesystem::directory_iterator(SOURCE)) {
          if (std::filesystem::is_regular_file(file)) {
            std::filesystem::path file_path(file);
            if (file_path.extension() == extension) {
              dest_path = createFolder(DEST_PATH, folder);
              SUCCESS = attemptToMove(file_path, dest_path);
            }
          }
        }
      }
    }
  }
}

std::vector<std::string>
fileOperations::organiseFolder(const std::string &CONFIG) {
  std::vector<std::string> SOURCEs;
  configV configV_umap;
  key_value keyvalue;
  std::string SOURCE;
  std::string DEST_PATH;
  configV_umap = parseConfigFile(CONFIG);

  if (!configV_umap.empty()) {
    for (const auto &section : configV_umap) {
      if (section.first == "settings") {

        // TODO:implement settings

      } else {
        if (configV_umap.count(section.first) &&
            configV_umap.at(section.first).count("source_path")) {
          SOURCE = configV_umap.at(section.first).at("source_path")[0];
          SOURCEs.push_back(SOURCE);
          if (configV_umap.count(section.first) &&
              configV_umap.at(section.first).count("dest_path")) {
            DEST_PATH = configV_umap.at(section.first).at("dest_path")[0];
          } else {
            DEST_PATH = SOURCE;
          }
        }
        if (std::filesystem::exists(SOURCE)) {
          if (section.second.size() > 1) {
            moveFileinDir(SOURCE, DEST_PATH, section.second);
          } else {
            defaultMoveBehaviour(SOURCE, DEST_PATH);
          }
        } else {
          continue;
        }
      }
    }
  }
  return SOURCEs;
}
