/*

    SD - a slightly more friendly wrapper for fatfs.


    Created by hongtai.liu 13 July 2019


*/
#include <Arduino.h>
#include <Seeed_FS.h>

namespace fs {

    File::File(FIL f, const TCHAR* n) {
        //is a file.
        _file = new FIL(f);
        if (_file) {
            _dir = NULL;
            tcscpy( _name, n);
        }
        _fno = NULL;
    }

    File::File(DIR d, const TCHAR* n) {
        // is a directory
        _dir = new DIR(d);
        if (_dir) {
            _file = NULL;
            tcscpy( _name, n);
        }
        _fno = NULL;
    }

    File::File(void) {
        _file = NULL;
        _dir = NULL;
        _name[0] = 0;
        //Serial.print("Created empty file object");
        _fno = NULL;
    }

    File::~File() {
        close();
        delete _fno;
    }

    // returns a pointer to the file name
    TCHAR* File::name(void) {
        return _name;
    }

    // a directory is a special type of file
    boolean File::isDirectory(void) {
        FRESULT ret = FR_OK;
        FILINFO v_fileinfo;

        if (_dir && !_file) {
            return true;
        }
        if ((ret = f_stat(_name, &v_fileinfo)) == FR_OK) {
            if (v_fileinfo.fattrib & AM_DIR) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
        return false;
    }

    size_t File::write(uint8_t val) {
        return write(&val, 1);
    }

    size_t File::write(const uint8_t* buf, size_t size) {
        UINT t;
        if (!_file) {
            return 0;
        }
        if (f_write(_file, buf, size, &t) == -1) { // if success, return the number of bytes have written.
            return 0;
        } else {
            return t;
        }
    }

    //return the peek value
    int File::peek() {
        if (!_file) {
            return 0;
        }

        int c = read();
        if (c != -1) {
            f_lseek(_file, f_tell(_file) - 1);
        }
        return c;
    }

    // read a value
    int File::read() {
        uint8_t val;
        if (_file) {
            return read(&val, 1) == 1 ? val : -1;
        }
        return -1;
    }

    long File::read(void* buf, uint32_t nbyte) {
        UINT t;
        if (!_file) {
            return 0;
        }
        if (f_read(_file, buf, nbyte, &t) == -1) {
            return 0;
        } else {
            return t;
        }
    }

    int File::available() {
        if (!_file) {
            return 0;
        }

        return !f_eof(_file);
    }

    void File::flush() {
        if (_file) {
            f_sync(_file);
        }
    }

    boolean File::seek(uint32_t pos) {
        if (!_file) {
            return false;
        }

        return f_lseek(_file, pos);
    }

    bool File::seek(uint32_t pos, SeekMode mode) {
        if (!_file) {
            return false;
        }
        switch (mode) {
            case SeekSet:
                return f_lseek(_file, pos);
                break;
            case SeekCur:
                f_lseek(_file, f_tell(_file) + pos);
                break;
            case SeekEnd:
                return f_lseek(_file, f_size(_file) - pos);
                break;
            default : return false;
                break;
        }
        return false;
    }

    uint32_t File::position() {
        if (!_file) {
            return -1;
        }
        return f_tell(_file);
    }

    uint32_t File::size() {
        return f_size(_file);
    }

    void File::close() {
        if (_dir) {
            f_closedir(_dir);
            delete _dir;
            _dir = NULL;
        }
        if (_file) {
            f_close(_file);
            delete _file;
            _file = NULL;
        }
    }

    // allows you to recurse into a directory
    File File::openNextFile(uint8_t mode) {
        FRESULT res;
        UINT i;
        TCHAR path[257];

        tcscpy(path, _name);
        if (!_fno) {
            _fno = new FILINFO;
        }

        for (; _fno;) {
            res = f_readdir(_dir, _fno); /* Read a directory item */
            if (res != FR_OK || _fno->fname[0] == 0) {
                break;    /* Break on error or end of dir */
            }
            if (_fno->fattrib == 255) {
                continue;    /*ignore if the addr was removed*/
            }

            i = tcslen(path);
            if (i && path[i - 1] != '/') {
                path[i++] = '/';
            }
            tcscpy( path + i, _fno->fname );

            if (_fno->fattrib & AM_DIR) {
                /* It is a directory */
                DIR dir;
                if ((res = f_opendir(&dir, path)) == FR_OK) {
                    return File(dir, path);
                } else {
                    return File();
                }
            } else {
                FIL file;
                if ((res = f_open(&file, path, mode)) == FR_OK) {
                    return File(file, path);
                } else {
                    return File();
                }
            }
        }
        return File();
    }

    void File::rewindDirectory(void) {
        if (isDirectory()) {
            f_rewinddir(_dir);
        }
    }

    File::operator bool() {
        if (_file || _dir) {
            return true;
        }
        return false;
    }

    File FS::open(const TCHAR* filepath, uint8_t mode) {
        FRESULT ret = FR_OK;
        FILINFO v_fileinfo;

        if (!tcscmp(filepath, _T("/"))) {
            DIR dir;
            if ((ret = f_opendir(&dir, _T("/"))) == FR_OK) {
                return File(dir, filepath);
            } else {
                return File();
            }
        }

        if ((ret = f_stat(filepath, &v_fileinfo)) == FR_OK) {
            if (v_fileinfo.fattrib & AM_DIR) {
                DIR dir;
                if ((ret = f_opendir(&dir, filepath)) == FR_OK) {
                    return File(dir, filepath);
                } else {
                    return File();
                }
            } else {
                FIL file;
                if ((ret = f_open(&file, filepath, mode)) == FR_OK) {
                    return File(file, filepath);
                } else {
                    return File();
                }
            }
        } else {
            FIL file;
            if ((ret = f_open(&file, filepath, mode)) == FR_OK) {
                return File(file, filepath);
            } else {
                return File();
            }
        }
    }

    File FS::open(const TCHAR* filepath, const char* mode) {
        if (strlen(mode) > 3) {
            return File();
        }
        if (!strcmp(mode, "r")) {
            return open(filepath, FA_READ);
        }
        if (!strcmp(mode, "r+")) {
            return open(filepath, FA_READ | FA_WRITE);
        }
        if (!strcmp(mode, "w")) {
            return open(filepath, FA_CREATE_ALWAYS | FA_WRITE);
        }
        if (!strcmp(mode, "w+")) {
            return open(filepath, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
        }
        if (!strcmp(mode, "a")) {
            return open(filepath, FA_OPEN_APPEND | FA_WRITE);
        }
        if (!strcmp(mode, "a+")) {
            return open(filepath, FA_OPEN_APPEND | FA_WRITE | FA_READ);
        }
        if (!strcmp(mode, "wx")) {
            return open(filepath, FA_CREATE_NEW | FA_WRITE);
        }
        if (!strcmp(mode, "w+x")) {
            return open(filepath, FA_CREATE_NEW | FA_WRITE | FA_READ);
        }

        return File();
    }

    boolean FS::exists(const TCHAR* filepath) {
        FRESULT ret = FR_OK;
        FILINFO v_fileinfo;
        if ((ret = f_stat(filepath, &v_fileinfo)) == FR_OK) {
            return true;
        } else {
            return false;
        }
    }

    boolean FS::mkdir(const TCHAR* filepath) {
        FRESULT ret = FR_OK;
        ret = f_mkdir(filepath);
        if (ret == FR_OK) {
            return true;
        } else {
            return false;
        }
    }

    boolean FS::rename(const TCHAR* pathFrom, const TCHAR* pathTo) {
        FRESULT ret = FR_OK;
        ret = f_rename(pathFrom, pathTo);
        if (ret == FR_OK) {
            return true;
        } else {
            return false;
        }
    }

    boolean FS::rmdir(const TCHAR* filepath) {
        TCHAR file[_MAX_LFN + 2];
        FRESULT status;
        DIR dj;
        FILINFO fno;
        status = f_findfirst(&dj, &fno, filepath, _T("*"));
        while (status == FR_OK && fno.fname[0]) {
            tcscpy( file, filepath );
            tcscat( file, _T("/") );
            tcscat( file, fno.fname );

            if (fno.fattrib & AM_DIR) {
                rmdir(file);
            } else {
                remove(file);
            }
            status = f_findnext(&dj, &fno);
        }
        f_closedir(&dj);
        if (remove(filepath)) {
            return true;
        } else {
            return false;
        }
    }

    boolean FS::remove(const TCHAR* filepath) {
        FRESULT ret = FR_OK;
        ret = f_unlink(filepath);
        if (ret == FR_OK) {
            return true;
        } else {
            return false;
        }
    }
}; // namespace fs
