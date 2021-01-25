#ifndef __SEEED_FS__
#define __SEEED_FS__


/*
    ________________________________________________________________________________________________________________________________________
    |    Flags         |                            Meaning                                                                                  |
    |----------------------------------------------------------------------------------------------------------------------------------------|
    | FA_READ          | Specifies read access to the object. Data can be read from the file.                                                |
    | FA_WRITE         | Specifies write access to the object. Data can be written to the file. Combine with FA_READ for read-write access.  |
    | FA_OPEN_EXISTING | Opens the file. The function fails if the file is not existing. (Default)                                           |
    | FA_CREATE_NEW    | Creates a new file. The function fails with FR_EXIST if the file is existing.                                       |
    | FA_CREATE_ALWAYS | Creates a new file. If the file is existing, it will be truncated and overwritten.                                  |
    | FA_OPEN_ALWAYS   | Opens the file if it is existing. If not, a new file will be created.                                               |
    | FA_OPEN_APPEND   | Same as FA_OPEN_ALWAYS except the read/write pointer is set end of the file.                                        |
    |__________________|_____________________________________________________________________________________________________________________|
*/

/*
    _____________________________________________________
    |    POSIX    |               FatFs                   |
    |-------------|---------------------------------------|
    |"r"          |FA_READ                                |
    |"r+"         |FA_READ | FA_WRITE                     |
    |"w"          |FA_CREATE_ALWAYS | FA_WRITE            |
    |"w+"         |FA_CREATE_ALWAYS | FA_WRITE | FA_READ  |
    |"a"          |FA_OPEN_APPEND | FA_WRITE              |
    |"a+"         |FA_OPEN_APPEND | FA_WRITE | FA_READ    |
    |"wx"         |FA_CREATE_NEW | FA_WRITE               |
    |"w+x"        |FA_CREATE_NEW | FA_WRITE | FA_READ     |
    |_____________|_______________________________________|
*/



#define FILE_READ FA_READ
#define FILE_WRITE (FA_CREATE_ALWAYS | FA_WRITE | FA_READ)
#define FILE_APPEND (FA_OPEN_APPEND | FA_WRITE)

#include "fatfs/ff.h"

/*
#if _LFN_UNICODE
#include <wchar.h>
#define tcscpy      wcscpy
#define tcscat      wcscat
#define tcslen      wcslen
#define tcschr      wcschr
#define tcsrchr     wcsrchr
#define tcscmp      wcscmp
#define tcscasecmp  wcscasecmp
#else
#define tcscpy      strcpy
#define tcscat      strcat
#define tcslen      strlen
#define tcschr      strchr
#define tcsrchr     strrchr
#define tcscmp      strcmp
#define tcscasecmp  strcasecmp
#endif
*/


static inline void tcscpy(TCHAR* dst, const TCHAR* src)
{
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

static inline void tcscat(TCHAR* dst, const TCHAR* src)
{
    while (*dst) dst++;
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

static inline int tcslen(const TCHAR* str)
{
    const TCHAR* begin = str;
    while (*str) str++;
    return  str - begin;
}

static inline TCHAR* tcschr(const TCHAR* str, int c)
{
    while (*str && (*str != c))   str++;
    return  *str == c ? (TCHAR*)str : NULL;
}

static inline TCHAR* tcsrchr(const TCHAR* str, int c)
{
    const TCHAR* begin = str;
    while (*str)   str++;
    while ( (begin <= str) && (*str != c))   str--;
    return  begin <= str ? (TCHAR*)str : NULL;
}

static inline int tcscmp(const TCHAR* str1, const TCHAR* str2)
{
    int i   = 0;
    while ( str1[i] && str2[i] && (str1[i] == str2[i])) i++;
    return  str1[i] == str2[i] ? 0 : str1[i] < str2[i] ? -1 : 1;
}

static inline int tcscasecmp(const TCHAR* str1, const TCHAR* str2)
{
    int i   = 0;
    while ( str1[i] && str2[i] && (tolower(str1[i]) == tolower(str2[i])))   i++;
    return  str1[i] == str2[i] ? 0 : str1[i] < str2[i] ? -1 : 1;
}




namespace fs {

    enum SeekMode {
        SeekSet = 0,
        SeekCur = 1,
        SeekEnd = 2
    };


    class File : public Stream {
      private:
        TCHAR _name[_MAX_LFN + 2]; // file name
        FIL* _file;  // underlying file pointer
        DIR* _dir;  // if open a dir
        FILINFO* _fno; // for traverse directory

      public:
        File(FIL f, const TCHAR* name);     // wraps an underlying SdFile
        File(DIR d, const TCHAR* name);
        File(void);      // 'empty' constructor
        ~File();
        virtual size_t write(uint8_t);
        virtual size_t write(const uint8_t* buf, size_t size);
        virtual int read();
        virtual int peek();
        virtual int available();
        virtual void flush();
        long read(void* buf, uint32_t nbyte);
        boolean seek(uint32_t pos);
        bool seek(uint32_t pos, SeekMode mode);
        uint32_t position();
        uint32_t size();
        void close();
        operator bool();
        TCHAR* name();

        boolean isDirectory(void);
        File openNextFile(uint8_t mode = FA_READ);
        void rewindDirectory(void);

        using Print::write;
    };
    class FS {
      protected:
        FATFS root;
      public:
        // Open the specified file/directory with the supplied mode (e.g. read or
        // write, etc). Returns a File object for interacting with the file.
        // Note that currently only one file can be open at a time.
        File open(const TCHAR* filepath, uint8_t mode = FILE_READ);
        File open(const TCHAR* filepath, const char* mode);

        // Methods to determine if the requested file path exists.
        boolean exists(const TCHAR* filepath);

        // Create the requested directory heirarchy--if intermediate directories
        // do not exist they will be created.
        boolean mkdir(const TCHAR* filepath);
        boolean rename(const TCHAR* pathFrom, const TCHAR* pathTo);

        // Delete the file.
        boolean remove(const TCHAR* filepath);
        boolean rmdir(const TCHAR* filepath);
    };
};
using namespace fs;

#include "SD/Seeed_SD.h"

#endif