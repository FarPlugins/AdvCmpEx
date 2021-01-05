/*
    string.hpp
    Copyright (C) 2002-2008 zg

    с моими уточнениями :-)
*/
/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#include <windows.h>

class string
{
  private:
    size_t current_size;
    size_t actual_size;
    wchar_t *data;
    wchar_t data_empty[1];
    HANDLE heap;
    void init(void);
    void copy(const string& Value);
    void copy(const wchar_t *Value);
    void copy(const wchar_t *Value,size_t size);
    bool enlarge(size_t size);
  public:
    string();
    string(const wchar_t *Value);
    string(const wchar_t *Value,size_t size);
    string(const string& Value);
    ~string();
    string &operator=(const string& Value);
    string &operator=(const wchar_t *Value);
    string &operator()(const wchar_t *Value,size_t size);
    operator const wchar_t *() const;
    wchar_t &operator[](size_t index);
    size_t length(void) const;
    void updsize(void);    //необходимо вызывать после воздействия на data извне
    wchar_t *get(size_t size = (size_t)-1);
    void clear(void);
    string& operator+=(wchar_t Value);
    string& operator+=(const wchar_t *Value);
    friend string operator+(const string& x,const string& y);
    friend string operator+(const string& x,const wchar_t *y);
};
    int operator==(const string& x,const string& y);
