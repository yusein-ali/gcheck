
#include <vector>
#include <string>

#if defined(_WIN32) || defined(WIN32)  
    #include <windows.h>
#else //lets hope it's unix
    #include <sys/ioctl.h>
#endif

namespace gcheck {

class ConsoleWriter {
public:
#if defined(_WIN32) || defined(WIN32)  
    enum Color : int {
        Original,
        Black = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        BrightBlack = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_INTENSITY,
        White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
        Green = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_GREEN,
        Red = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED
    };
#else //lets hope it supports ansi codes
    enum Color : int {
        Original,
        Black = 0,
        BrightBlack = 236,
        White = 15,
        Green = 22,
        Red = 1
    };
#endif
private:
    
    std::vector<std::string> headers_;
    std::vector<int> header_widths_;
    
    bool terminating_newline_ = false;
    bool use_colors_ = false; // TODO: determine if terminal supports colors
    
    int GetWidth();
    
    std::vector<int> CalcWidths(const std::vector<std::string>& strs, std::vector<int> widths = std::vector<int>());
    
    void WriteRow(int width, const std::vector<std::string>& cells, const std::vector<int>& widths, const std::vector<int>& cuts);
public:
    
    void SetColor(Color color);
    
    void WriteSeparator();
    
    void SetHeaders(const std::vector<std::string>& headers);

    void WriteRow(const std::vector<std::string>& cells);
    
    void WriteRows(const std::vector<std::vector<std::string>>& cells);
};

}