#include "console_writer.h"

#include <unistd.h>
#include <iostream>

namespace gcheck {
    
int ConsoleWriter::GetWidth() {
    int w = 0;
#if defined(_WIN32) || defined(WIN32)  
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    if(csbi.dwSize.X > 0) {
        terminating_newline_ = false;
        w =  csbi.dwSize.X;
    } else if(csbi.srWindow.Right - csbi.srWindow.Left > 0) {
        terminating_newline_ = false;
        w =  csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else if(csbi.dwMaximumWindowSize.X > 0) {
        terminating_newline_ = true;
        w =  csbi.dwMaximumWindowSize.X;
    } else {
        terminating_newline_ = true;
    }
#else //lets hope it is unix
    winsize size;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
    
    w = size.ws_col;
#endif
    return w < 5 ? w = 128 : w;
}
    
std::vector<int> ConsoleWriter::CalcWidths(const std::vector<std::string>& strs, std::vector<int> widths) {
    if(widths.size() < strs.size())
        widths.resize(strs.size(), 0);
    
    auto it2 = widths.begin();
    for(auto it = strs.begin(); it != strs.end(); it++, it2++) {
        const std::string& str = *it;
        int& max_width = *it2;
        for(size_t pos = 0, pos2 = 0; pos2 != std::string::npos; pos = pos2+1) {
            pos2 = str.find('\n', pos);
            int width = pos2 == std::string::npos ? str.length()-pos : pos2-pos;
            if(max_width < width)
                max_width = width;
        }
    }
    
    return widths;
}
    
void ConsoleWriter::WriteRow(int width, const std::vector<std::string>& cells, const std::vector<int>& widths, const std::vector<int>& cuts) {
    
    if(width < 5) {
        width = 128;
    }
    
    int totalwidth = 0;
    for(auto& e : widths)
        totalwidth += e;
    if(totalwidth > width) 
        totalwidth = width;
    
    std::vector<std::vector<std::string>> parts;
    
    std::vector<size_t> poss(cells.size(), 0);
    unsigned int num_done = 0;
    while(num_done != poss.size()) {
        parts.push_back({});
        std::vector<std::string>& row = parts[parts.size()-1];
        for(unsigned int i = 0; i < poss.size(); i++) {
            if(poss[i] != std::string::npos) {
                size_t end = cells[i].find('\n', poss[i]);
                if(end != std::string::npos) {
                    row.push_back(cells[i].substr(poss[i], end-poss[i]));
                    poss[i] = end+1;
                } else {
                    row.push_back(cells[i].substr(poss[i]));
                    poss[i] = end;
                    num_done++;
                }
            } else {
                row.push_back("");
            }
        }
    }
    
    int pos = 0;
    for(auto it = cuts.begin(); it != cuts.end(); it++) {
        if(pos == 0)
            std::cout << "+";
        else std::cout << "  ";
        int t_width = (pos == 0 ? 1 : 2);
        for(int i = 0; i < *it; ++i) {
            int w = std::min(widths[pos+i], totalwidth - (pos == 0 ? 2 : 3));
            t_width += w + 1;
            std::cout << std::string(w, '-') << '+';
        }
        if(terminating_newline_ || t_width != totalwidth)
            std::cout << std::endl;
        for(auto& row : parts) {
            
            std::string pad = "|";
            if(pos != 0)
                pad = "  ";
            std::cout << pad;
            
            if(row[pos].length() > totalwidth-pad.length()-1) {
                size_t len = totalwidth-pad.length()-1;
                SetColor(BrightBlack);
                std::cout << row[pos].substr(0, len);
                
                size_t p = len;
                len = totalwidth-pad.length()-2;
                for(; p < row[pos].length(); p += len) {
                    SetColor(Black);
                    std::cout << '|' << pad << "  ";
                    SetColor(BrightBlack);
                    std::cout << row[pos].substr(p, len);
                }
                SetColor(Black);
                std::cout << std::string(std::max(p-row[pos].length(), 1ul)-1, ' ') << '|';
            } else {
                t_width = (pos == 0 ? 1 : 2);
                for(int i = 0; i < *it; ++i) {
                    SetColor(BrightBlack);
                    std::cout << row[pos+i];
                    SetColor(Black);
                    int w = std::min(widths[pos+i], totalwidth - (pos == 0 ? 2 : 3));;
                    t_width += w + 1;
                    std::cout << std::string(w-row[pos+i].length(), ' ') << '|';
                }
                if(terminating_newline_ || t_width != totalwidth)
                    std::cout << std::endl;
            }
        }
        if(cuts.size() > 1 && it == cuts.end()-1)
            std::cout << std::endl;
        pos += *it;
    }
}

void ConsoleWriter::SetColor(Color color) {
    if(use_colors_) {
        std::cout.flush();
#if defined(_WIN32) || defined(WIN32)  
        if(color == Original)
            color = Black;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, (WORD)color);
#else //lets hope it is unix
        if(color == Original)
            std::cout << "\033[0m";
        else
            std::cout << "\033[48;5;" + std::to_string((int)color) + "m";
#endif
    }
}
    
void ConsoleWriter::WriteSeparator() {
    int width = GetWidth();
    std::cout << std::string(width, '*');
    if(terminating_newline_)
        std::cout << std::endl;
}

void ConsoleWriter::SetHeaders(const std::vector<std::string>& headers) {
    headers_ = headers;
    header_widths_ = CalcWidths(headers);
}

void ConsoleWriter::WriteRow(const std::vector<std::string>& cells) {
    WriteRows(std::vector<std::vector<std::string>>(1, cells));
}
    
void ConsoleWriter::WriteRows(const std::vector<std::vector<std::string>>& cells) {
    
    int width = GetWidth();
    std::vector<int> widths = header_widths_;
    for(auto& r : cells)
        widths = CalcWidths(r, widths);
    
    std::vector<int> cuts;
    int cut_width = 1, counter = 0;
    for(auto& e : widths) {
        cut_width += e + 1;
        if(cut_width > width && counter != 0) {
            cut_width = e + 3;
            cuts.push_back(counter);
            counter = 0;
        }
        counter++;
    }
    cuts.push_back(counter);
    
    if(headers_.size() != 0) {
        WriteRow(width, headers_, widths, cuts);
    }
    for(auto& r : cells) {
        WriteRow(width, r, widths, cuts);
    }
    for(int i = 0; i < cuts[0]; i++) {
        std::cout << '+' << std::string(widths[i], '-');
    }
    std::cout << '+' << std::endl;
}

}