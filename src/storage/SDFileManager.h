#pragma once
#include <M5Cardputer.h>
#include <SD.h>
#include <SPI.h>
#include <vector>

// SD card SPI pin definitions for M5Cardputer / StampS3 base
#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

// File info structure
struct FileInfo {
    String name;        // File name
    String path;        // Full path
    bool isDirectory;   // Is directory
    size_t size;        // File size (0 for dir)
    
    FileInfo() : name(""), path(""), isDirectory(false), size(0) {}
    FileInfo(const String& n, const String& p, bool isDir, size_t s = 0) 
        : name(n), path(p), isDirectory(isDir), size(s) {}
};

// SD card file manager
class SDFileManager {
private:
    bool initialized;
    String currentPath;
    struct DirCtx { File dir; String path; };
    std::vector<DirCtx> scanStack_;
    String scanExt_;
    bool scanActive_ = false;
    
    // Normalize path
    String normalizePath(const String& path) {
        if (path.isEmpty() || path == "/") return "/";
        
        String normalized = path;
        if (!normalized.startsWith("/")) normalized = "/" + normalized;
        if (normalized.endsWith("/") && normalized.length() > 1) {
            normalized = normalized.substring(0, normalized.length() - 1);
        }
        return normalized;
    }
    
public:
    SDFileManager() : initialized(false), currentPath("/") {}
    
    // Initialize SD card
    bool initialize() {
        if (initialized) return true;
        
        SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
        
        if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
            return false;
        }
        
        initialized = true;
        currentPath = "/";
        return true;
    }
    
    // Check if initialized
    bool isInitialized() const { return initialized; }
    bool isScanning() const { return scanActive_; }
    
    // Get current path
    String getCurrentPath() const { return currentPath; }
    
    // Set current path
    bool setCurrentPath(const String& path) {
        if (!initialized) return false;
        
        String newPath = normalizePath(path);
        if (!exists(newPath) || !isDirectory(newPath)) return false;
        
        currentPath = newPath;
        return true;
    }
    
    // Exists
    bool exists(const String& path) {
        if (!initialized) return false;
        File file = SD.open(path);
        bool result = (bool)file;
        file.close();
        return result;
    }
    
    // Is directory
    bool isDirectory(const String& path) {
        if (!initialized) return false;
        File file = SD.open(path);
        bool result = file && file.isDirectory();
        file.close();
        return result;
    }
    
    // List directory contents
    bool listDirectory(const String& dirPath, FileInfo* fileList, int& fileCount, int maxFiles) {
        if (!initialized) return false;
        
        fileCount = 0;
        String targetPath = dirPath.isEmpty() ? currentPath : normalizePath(dirPath);
        
        File dir = SD.open(targetPath);
        if (!dir || !dir.isDirectory()) {
            dir.close();
            return false;
        }
        
        // Add parent directory entry (if not root)
        if (targetPath != "/" && fileCount < maxFiles) {
            fileList[fileCount++] = FileInfo("..", "", true, 0);
        }
        
        dir.rewindDirectory();
        File file = dir.openNextFile();
        
        while (file && fileCount < maxFiles) {
            String fileName = file.name();
            String displayName = fileName;
            
            // Extract name
            int lastSlash = displayName.lastIndexOf('/');
            if (lastSlash != -1) {
                displayName = displayName.substring(lastSlash + 1);
            }
            
            // Skip hidden or empty
            if (displayName.length() == 0 || displayName.startsWith(".")) {
                file.close();
                file = dir.openNextFile();
                continue;
            }
            
            // Build full path
            String fullPath = targetPath;
            if (!fullPath.endsWith("/")) fullPath += "/";
            fullPath += displayName;
            
            bool isDir = file.isDirectory();
            size_t fileSize = isDir ? 0 : file.size();
            
            fileList[fileCount++] = FileInfo(displayName, fullPath, isDir, fileSize);
            
            file.close();
            file = dir.openNextFile();
        }
        
        dir.close();
        return true;
    }
    
    // List current directory
    bool listCurrentDirectory(FileInfo* fileList, int& fileCount, int maxFiles) {
        return listDirectory(currentPath, fileList, fileCount, maxFiles);
    }
    
    // Enter directory
    bool enterDirectory(const String& dirName) {
        if (!initialized) return false;
        
        String newPath;
        if (dirName == "..") {
            // Back to parent
            if (currentPath == "/") return false;
            
            int lastSlash = currentPath.lastIndexOf('/');
            if (lastSlash <= 0) {
                newPath = "/";
            } else {
                newPath = currentPath.substring(0, lastSlash);
            }
        } else {
            // Enter child
            newPath = currentPath;
            if (!newPath.endsWith("/")) newPath += "/";
            newPath += dirName;
        }
        
        return setCurrentPath(newPath);
    }
    
    // Read file content
    String readFile(const String& filePath) {
        if (!initialized) return "";
        
        File file = SD.open(filePath);
        if (!file || file.isDirectory()) {
            file.close();
            return "";
        }
        
        String content = "";
        while (file.available()) {
            content += (char)file.read();
        }
        
        file.close();
        return content;
    }

    bool writeFile(const String& filePath, const String& content) {
        if (!initialized) return false;
        String p = normalizePath(filePath);
        if (exists(p)) SD.remove(p);
        File file = SD.open(p, FILE_WRITE);
        if (!file) return false;
        bool ok = file.print(content);
        file.flush();
        file.close();
        return ok;
    }

    bool createFile(const String& filePath, const String& content = "") {
        return writeFile(filePath, content);
    }

    bool deletePath(const String& path) {
        if (!initialized) return false;
        String p = normalizePath(path);
        if (isDirectory(p)) return SD.rmdir(p);
        return SD.remove(p);
    }
    
    // Scan all files (recursive)
    bool scanAllFiles(FileInfo* fileList, int& fileCount, int maxFiles, const String& extension = "") {
        if (!initialized) return false;
        fileCount = 0;
        return scanDirectoryRecursive("/", fileList, fileCount, maxFiles, extension);
    }
    
    // Recursive directory scan
    bool scanDirectoryRecursive(const String& dirPath, FileInfo* fileList, int& fileCount, int maxFiles, const String& extension = "") {
        if (!initialized || fileCount >= maxFiles) return false;
        
        File dir = SD.open(dirPath);
        if (!dir || !dir.isDirectory()) {
            dir.close();
            return false;
        }
        
        File file = dir.openNextFile();
        while (file && fileCount < maxFiles) {
            String fileName = file.name();
            String filePath = file.path();
            bool isDir = file.isDirectory();
            
            if (isDir) {
                // Recurse into subdirectory
                scanDirectoryRecursive(filePath, fileList, fileCount, maxFiles, extension);
            } else {
                // Check extension
                bool shouldAdd = extension.isEmpty();
                if (!extension.isEmpty()) {
                    String lowerName = fileName;
                    lowerName.toLowerCase();
                    String lowerExt = extension;
                    lowerExt.toLowerCase();
                    shouldAdd = lowerName.endsWith(lowerExt);
                }
                
                if (shouldAdd) {
                    size_t fileSize = file.size();
                    fileList[fileCount++] = FileInfo(fileName, filePath, false, fileSize);
                }
            }
            
            file.close();
            file = dir.openNextFile();
        }
        
        dir.close();
        return true;
    }
    
    // Get file extension
    String getFileExtension(const String& fileName) {
        int lastDot = fileName.lastIndexOf('.');
        if (lastDot == -1) return "";
        return fileName.substring(lastDot);
    }
    
    // Is audio file
    bool isAudioFile(const String& fileName) {
        String ext = getFileExtension(fileName);
        ext.toLowerCase();
        return ext == ".mp3" || ext == ".wav" || ext == ".m4a" || ext == ".aac";
    }

    bool beginScan(const String& root, const String& extension) {
        if (!initialized) return false;
        while (!scanStack_.empty()) {
            if (scanStack_.back().dir) scanStack_.back().dir.close();
            scanStack_.pop_back();
        }
        String r = normalizePath(root.length() ? root : String("/"));
        File d = SD.open(r);
        if (!d || !d.isDirectory()) { d.close(); return false; }
        DirCtx ctx; ctx.dir = d; ctx.path = r;
        scanStack_.push_back(ctx);
        scanExt_ = extension;
        scanActive_ = true;
        return true;
    }

    bool stepScan(FileInfo* outList, int& outCount, int maxOut) {
        if (!initialized || !scanActive_) { outCount = 0; return false; }
        outCount = 0;
        int processed = 0;
        while (!scanStack_.empty() && outCount < maxOut) {
            DirCtx& ctx = scanStack_.back();
            File entry = ctx.dir.openNextFile();
            if (!entry) {
                ctx.dir.close();
                scanStack_.pop_back();
                continue;
            }
            processed++;
            String name = entry.name();
            String path = entry.path();
            bool isDir = entry.isDirectory();
            if (isDir) {
                File sub = SD.open(path);
                if (sub && sub.isDirectory()) {
                    DirCtx subctx; subctx.dir = sub; subctx.path = path;
                    scanStack_.push_back(subctx);
                } else {
                    sub.close();
                }
            } else {
                bool add = scanExt_.isEmpty();
                if (!scanExt_.isEmpty()) {
                    String nm = name; nm.toLowerCase();
                    String ex = scanExt_; ex.toLowerCase();
                    add = nm.endsWith(ex);
                }
                if (add) {
                    String display = name;
                    int lastSlash = display.lastIndexOf('/');
                    if (lastSlash != -1) display = display.substring(lastSlash + 1);
                    size_t sz = entry.size();
                    outList[outCount++] = FileInfo(display, path, false, sz);
                }
            }
            entry.close();
            if (processed >= maxOut * 4) break;
        }
        if (scanStack_.empty()) { scanActive_ = false; }
        return scanActive_;
    }
};
