#include "FolderMethods.h"
#include "../../../AppPlatform.h"
#include <algorithm>
#include <set>
#include <cctype>

#ifndef WIN32

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

int _mkdir( const char* name ) {
	return mkdir(name, 0755);
}

int _access( const char* name, int mode ) {
	return access(name, mode);
}

int _errno() {
	return errno;
}

#else
	#include <io.h>
	#include <direct.h>
	#include <windows.h>
#endif

bool exists(const char* name) {
    return _access(name, 0) == 0;
}

bool createFolderIfNotExists( const char* name ) {
	if (exists(name))
        return true;

    int errorCode = 0;
	if ((errorCode = _mkdir(name)) != 0) {
        LOGI("FAILED to create folder %s, %d! Escape plan?\n", name, _errno());
        return false;
    }

    LOGI("Created folder %s\n", name);
    return true;
}

int getRemainingFileSize( FILE* fp ) {
	if (!fp) return 0;
	int current = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int end = ftell(fp);
	fseek(fp, current, SEEK_SET);
	return end - current;
}

int getFileSize( const char* filename ) {
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		return -1;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	return size;
}

bool createTree( const char* base, const char* tree[], int treeLength ) {
	if (!createFolderIfNotExists(base))
		return false;

	std::string p(base);
	for (int i = 0; i < treeLength; ++i && tree[i]) {
		p += tree[i];
		if (!createFolderIfNotExists(p.c_str()))
			return false;
	}
	return true;
}

static bool endsWithCaseInsensitive(const std::string& str, const std::string& suffix) {
    if (suffix.empty()) return true;
    if (str.length() < suffix.length()) return false;
    size_t offset = str.length() - suffix.length();
    for (size_t i = 0; i < suffix.length(); ++i) {
        if (tolower((unsigned char)str[offset + i]) != tolower((unsigned char)suffix[i]))
            return false;
    }
    return true;
}

std::vector<std::string> listDataFiles(AppPlatform* platform, const std::string& dir, const std::string& extension) {
    std::set<std::string> resultSet;
    std::vector<std::string> result;

    std::string diskPath = dir;
    std::string assetPath = dir;

    if (dir.find("data/") == 0) {
        assetPath = dir;
    } else {
        diskPath = "data/" + dir;
        assetPath = dir;
    }

    // 1. Real folder on disk
#ifdef WIN32
    std::string searchPath = diskPath + "/*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string fname = findData.cFileName;
                if (endsWithCaseInsensitive(fname, extension)) {
                    resultSet.insert(fname);
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* d = opendir(diskPath.c_str());
    if (d) {
        struct dirent* entry;
        while ((entry = readdir(d)) != NULL) {
            std::string fname = entry->d_name;
            if (fname != "." && fname != "..") {
                if (endsWithCaseInsensitive(fname, extension)) {
                    resultSet.insert(fname);
                }
            }
        }
        closedir(d);
    }
#endif

    // 2. Packaged assets (e.g. Android AssetManager)
    if (platform) {
        StringVector assetFiles = platform->listAssetFiles(assetPath);
        for (size_t i = 0; i < assetFiles.size(); ++i) {
            const std::string& fname = assetFiles[i];
            if (endsWithCaseInsensitive(fname, extension)) {
                resultSet.insert(fname);
            }
        }
    }

    for (std::set<std::string>::const_iterator it = resultSet.begin(); it != resultSet.end(); ++it) {
        result.push_back(*it);
    }

    return result;
}
