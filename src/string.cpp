/*
    string.cpp
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

#pragma hdrstop
#include "string.hpp"

#define MEMORY_STEP (64)

void string::init(void)
{
  data=NULL;
  data_empty[0]=NULL;
  current_size=0;
  actual_size=0;
  heap=GetProcessHeap();
}

void string::copy(const string& Value)
{
  if(Value.current_size && enlarge(Value.current_size))
  {
    memcpy(data,Value.data,Value.current_size*sizeof(wchar_t));
    current_size=Value.current_size;
    data[current_size-1]=0;
  }
  else
  {
    if (data)
    {
      current_size=1;
      data[current_size-1]=0;
    }
  }
}

void string::copy(const wchar_t *Value)
{
  size_t size=wcslen(Value)+1;
  if(enlarge(size))
  {
    memcpy(data,Value,size*sizeof(wchar_t));
    current_size=size;
    data[current_size-1]=0;
  }
}

void string::copy(const wchar_t *Value,size_t size)
{
  if(enlarge(size+1))
  {
    memcpy(data,Value,size*sizeof(wchar_t));
    current_size=size+1;
    data[current_size-1]=0;
  }
}

bool string::enlarge(size_t size)
{
  size_t new_actual_size=(size/MEMORY_STEP+((size%MEMORY_STEP)?1:0))*MEMORY_STEP;
  if(actual_size>=new_actual_size) return true;
  void *new_data;
  if(data)
    new_data=HeapReAlloc(heap,HEAP_ZERO_MEMORY,data,new_actual_size*sizeof(wchar_t));
  else
    new_data=HeapAlloc(heap,HEAP_ZERO_MEMORY,new_actual_size*sizeof(wchar_t));
  if(new_data)
  {
    data=(wchar_t *)new_data;
    actual_size=new_actual_size;
    return true;
  }
  return false;
}

string::string()
{
  init();
}

string::string(const wchar_t *Value)
{
  init();
  copy(Value);
}

string::string(const wchar_t *Value,size_t size)
{
  init();
  copy(Value,size);
}

string::string(const string& Value)
{
  init();
  copy(Value);
}

string::~string()
{
  if(data) HeapFree(heap,0,data);
}

string &string::operator=(const string& Value)
{
  if(this!=&Value)
    copy(Value);
  return *this;
}

string &string::operator=(const wchar_t *Value)
{
  copy(Value);
  return *this;
}

string &string::operator()(const wchar_t *Value,size_t size)
{
  copy(Value,size);
  return *this;
}

string::operator const wchar_t *() const
{
  if(data) return data;
  return data_empty;
}

wchar_t &string::operator[](size_t index)
{
  if(index>=current_size) return data_empty[0];
  return data[index];
}

size_t string::length(void) const
{
  return current_size?current_size-1:0;
}

void string::updsize(void)
{
  if (data) current_size=wcslen(data)+1;
  else current_size=0;
}

wchar_t *string::get(size_t size)
{
  if(size == (size_t)-1) 
  {
    if(data) return data;
  }
  else
  {
    if(enlarge(size) && data) return data;
  }
  return data_empty;
}

void string::clear(void)
{
  if(data&&actual_size)
  {
    current_size=1;
    data[current_size-1]=0;
  }
}

string& string::operator+=(wchar_t Value)
{
  if(Value && enlarge(current_size+(current_size?1:2)))
  {
    if(!current_size) current_size++;
    data[current_size-1]=Value;
    data[current_size++]=0;
  }
  return *this;
}

string& string::operator+=(const wchar_t *Value)
{
  if (Value && *Value)
  {
    size_t len=wcslen(Value);
    if (enlarge((current_size?current_size:1)+len))
    {
      memcpy(data+length(),Value,len*sizeof(wchar_t));
      if(!current_size) current_size++;
      current_size+=len;
      data[current_size-1]=0;
    }
  }
  return *this;
}

string operator+(const string& x,const string& y)
{
  string result;
  if(result.enlarge(x.length()+y.length()+1))
  {
    memcpy(result.data,x.data,x.length()*sizeof(wchar_t));
    memcpy(result.data+x.length(),y.data,y.length()*sizeof(wchar_t));
    result.current_size=x.length()+y.length()+1;
    result.data[result.current_size-1]=0;
  }
  return result;
}

string operator+(const string& x,const wchar_t *y)
{
  string result;
  if (y && *y)
  {
    size_t size=wcslen(y)+1;
    if (result.enlarge(x.length()+size))
    {
      memcpy(result.data,x.data,x.length()*sizeof(wchar_t));
      memcpy(result.data+x.length(),y,size*sizeof(wchar_t));
      result.current_size=x.length()+size;
      result.data[result.current_size-1]=0;
    }
  }
  return result;
}

int operator==(const string& x,const string& y)
{
  if(x.length()==y.length()) return memcmp((const wchar_t *)x,(const wchar_t *)y,x.length());
  else return x.length()-y.length();
}
