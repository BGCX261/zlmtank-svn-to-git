/**  
* @file m_str.cpp  
*/   
   
#include <STRING.H>    
#include <STDLIB.H>    
#include <STDIO.H>    
#include <TIME.H>    
   
#ifdef TARGET_UNIX    
        #include <SYS time.h>    
        #include <SYS stat.h>    
        #include <UNISTD.H>    
        #include <CTYPE.H>    
   
#ifndef strnicmp    
        #define strnicmp(x,y,z) strncasecmp(x,y,z)    
#endif    
   
#ifndef stricmp    
        #define stricmp(x,y) strcasecmp(x,y)    
#endif    
#endif    
   
#include "m_str.h"    
   
TStr::TStr(void)   
{   
  Data = new char[Size = 256];   
  Data[Count = 0] = '\0';   
}   
   
TStr::TStr(const TStr& src)   
{   
  Data = new char[Size = ((Count = src.Count) + 1)];   
  strncpy(Data,src.Data,Count);   
  Data[Count] = '\0';   
}   
   
TStr::TStr(const char* src, int len)   
{   
  if( len < 0 && src != NULL )   
    len = strlen(src);   
  Data = new char[Size = ((Count = len) + 1)];   
  if( src != NULL )   
    strncpy(Data,src,Count);   
  Data[Count] = '\0';   
}   
   
TStr::TStr(char src)   
{   
  Data = new char[Size = 64];   
  Data[0] = src;   
  Data[Count = 1] = '\0';   
}   
   
TStr::TStr(int src)   
{   
  char tmp[128];   
  sprintf( tmp, "%d", src );   
  Data = new char[ (Size = (Count = strlen(tmp)) + 1)];   
  strcpy( Data, tmp );   
}   
   
TStr::TStr(unsigned int src)   
{   
  char tmp[128];   
  sprintf( tmp, "%d", src );   
  Data = new char[ (Size = (Count = strlen(tmp)) + 1)];   
  strcpy( Data, tmp );   
}   
   
TStr::TStr(unsigned long src)   
{   
  char tmp[128];   
  sprintf( tmp, "%ld", src );   
  Data = new char[ (Size = (Count = strlen(tmp)) + 1)];   
  strcpy( Data, tmp );   
}   
   
TStr::TStr(long src)   
{   
  char tmp[128];   
  sprintf( tmp, "%ld", src );   
  Data = new char[ (Size = (Count = strlen(tmp)) + 1)];   
  strcpy( Data, tmp );   
}   
   
TStr::TStr(double src)   
{   
  char tmp[128];   
  sprintf( tmp, "%f", src );   
  Data = new char[ (Size = (Count = strlen(tmp)) +1)];   
  strcpy( Data, tmp );   
}   
   
TStr::~TStr(void)   
{   
  delete[] Data;   
}   
   
void TStr::CheckSize(int NewCount)   
{   
  if( Size <= NewCount ) {   
    if( Size > 4096 )   
      Size = 4096;   
    char* p = new char[Size = NewCount + 1 + Size];   
    if( Count > 0 )   
      strncpy(p,Data,Count);   
    p[Count] = '\0';   
    delete[] Data;   
    Data = p;   
  }   
}   
   
TStr& TStr::operator =(const TStr& rhs)   
{   
  CheckSize(rhs.Count + 1);   
  strncpy(Data,rhs.Data,rhs.Count);   
  Data[Count = rhs.Count] = '\0';   
  return *this;   
}   
   
TStr& TStr::operator +=(const TStr& rhs)   
{   
  if( rhs.Count > 0 ) {   
    CheckSize(Count + rhs.Count + 1);   
    strncpy(Data+Count,rhs.Data,rhs.Count);   
    Data[Count += rhs.Count] = '\0';   
  }   
  return *this;   
}   
   
TStr TStr::operator +(const TStr& rhs) const   
{   
  TStr Res(*this);   
  Res += rhs;   
  return Res;   
}   
   
bool TStr::operator ==(const TStr& rhs) const   
{   
  if( rhs.Count != Count )   
    return false;   
  return strnicmp(Data,rhs.Data,Count) == 0;   
}   
   
bool TStr::operator !=(const TStr& rhs) const   
{   
  return !(rhs == *this);   
}   
   
void TStr::Insert(const TStr& str, int index)   
{   
  if( index > Count )   
    index = Count;   
  CheckSize(index + str.Count);   
  strncpy(Data+index,str.Data,str.Count);   
  Data[Count += str.Count] = '\0';   
}   
   
void TStr::Delete(int index, int count)   
{   
  if( index < 0 )   
    index = 0;   
  if( (count+index) > this->Count )   
    count = this->Count - index;   
  strncpy(Data+index,Data+index+count,this->Count-index-count);   
  Data[this->Count -= count] = '\0';   
}   
   
void TStr::Set(const char* src, int count)   
{   
  if( count == -1 )   
    count = strlen(src);   
  if( count != 0 ) {   
    CheckSize(count+1);   
    strncpy(Data,src,count);   
    Data[this->Count = count] = '\0';   
  }   
}   
   
void TStr::Append(const char* src, int count)   
{   
  if( count == -1 )   
    count = strlen(src);   
  if( count != 0 ) {   
    CheckSize(count+this->Count+1);   
    strncpy(Data+Count,src,count);   
    Data[this->Count += count] = '\0';   
  }   
}   
   
void TStr::Append(const char src)   
{   
  CheckSize(Count+1);   
  Data[Count] = src;   
  Data[Count += 1] = '\0';   
}   
   
bool TStr::Compare(const TStr& src)   
{   
  return (*this) == src;   
}   
   
bool TStr::Compare(const char* src, int count)   
{   
    if( count == -1 )   
      count = strlen(src);   
    if ( count != this->Count )   
      return false;   
    if( this->Count == 0 )   
      return true;   
    return strnicmp(Data,src,this->Count) == 0;   
}   
   
void TStr::Clear()   
{   
  Count = 0;   
}   
   
void TStr::SetLength(int newLength)   
{   
  if( Size != newLength+1 ) {   
    delete[] Data;   
    Data = new char[Size = newLength+1];   
    for( int i = 0; i < Size; i++ )   
      Data[i] = '?';   
  }   
  Data[Count = newLength] = '\0';   
}   
   
int TStr::ToInt() const    
{   
  return atoi(Data);   
}   
   
bool TStr::ToBool() const    
{   
    if (atoi(Data)) {   
        return true;   
    } else {   
        return false;   
    }   
}   
   
int TStr::ToIntDef(int defaultValue) const   
{   
  if( Count == 0 )   
    return defaultValue;   
  int r = ToInt();   
  if( Data[0] != '0' && r == 0 )   
    return defaultValue;   
  return r;   
}   
   
double TStr::ToDouble() const   
{   
  return atof(Data);   
}   
   
TStr TStr::Word(int word)   
{   
  int i = 0;   
  int start = 0;   
  while( i < Count )   
  {   
    if( Data[i] == ' ' || Data[i] == '\t' ) {   
      word--;   
      if( word < 0 ) {   
        TStr res(Data+start,i-start);   
        return res;   
      }   
      while( i < Count && (Data[i] == ' ' || Data[i] == '\t') )   
        i++;   
      start = i;   
    } else   
      i++;   
  }   
  if( start < i )   
    return TStr(Data+start,i-start-1);   
  return "";   
}   
   
TStr TStr::Right(int c)   
{   
  if( Count > c )   
    return TStr(Data+Count-c,c);   
  return *this;   
}   
   
int TStr::Pos(const TStr &sub)   
{   
  int i = 0;   
  int len = Count - sub.Count;   
  while( i <= len )   
  {   
    if( strnicmp(Data+i,sub.Data,sub.Count) == 0 )    
      return i;   
    i++;   
  }   
  return -1;   
}   
   
int TStr::RigthPos(const TStr &sub)   
{   
  int i = Count - sub.Count;   
  while( i >= 0 )   
  {   
    if( strnicmp(Data+i,sub.Data,sub.Count) == 0 )    
      return i;   
    i--;   
  }   
  return -1;   
}   
   
TStr TStr::Replace(const TStr &sub, const TStr &replace)   
{   
  TStr r;   
  int i = 0;   
  int len = Length() - sub.Count;   
  while( i < Count )   
  {   
    if( i <= len && strnicmp(Data+i,sub.Data,sub.Count) == 0 ) {   
      r += replace;   
      i += sub.Count;   
    } else {   
      r += Data[i];   
      i++;   
    }   
  }   
  return r;   
}   
   
TStr TStr::Copy(int c, int len)   
{   
  if( len > Count - c )   
    len = Count - c;   
  return TStr(Data+c,len);   
}   
   
TStr TStr::TrimLeft()   
{   
  int i = 0;   
  while( i < Count && ( Data[i] == ' ' || Data[i] == 13 || Data[i] == 10) )   
    i++;   
  if( i > 0 )   
    return Copy(i,Count-i);   
  return *this;   
}   
   
TStr TStr::TrimRight()   
{   
  int i = Count-1;   
  while( i > 0 && ( Data[i] == ' ' || Data[i] == 13 || Data[i] == 10) )   
    i--;   
  if( i < Count-1 )   
    return Copy(0,i+1);   
  return *this;   
}   
   
TStr TStr::GetBefore(const TStr &sub, const bool subdel)   
{   
  TStr r;   
  int i = Pos(sub);   
  if( i < 0 ) {   
    r = *this;   
    Count = 0;   
  } else {   
    r = CopyBefore(i);   
    if( subdel )   
      i += sub.Count;   
    Delete(0,i);   
  }   
  return r;   
}   
   
TStr TStr::GetBefore(int Index, const bool subdel)   
{   
  TStr r;   
  if( Index < 0 ) {   
    r = *this;   
    Count = 0;   
  } else {   
    r = CopyBefore(Index);   
    Delete(0,Index);   
  }   
  return r;   
}   
   
TStr TStr::CopyBefore(const TStr &sub)   
{   
  int i = Pos(sub);   
  if( i < 0 )   
    return *this;   
  return CopyBefore(i);   
}   
   
TStr TStr::CopyBefore(int c)   
{   
  if( c >= 0 )   
    if( c < Count )   
      return TStr(Data,c);   
    else   
      return TStr(Data,Count);   
  return "";   
}   
   
TStr TStr::CopyAfter(const TStr &sub)   
{   
  int i = Pos(sub);   
  if( i < 0 )   
    return "";   
  return CopyAfter(i+sub.Count-1);   
}   
   
TStr TStr::CopyAfter(int c)   
{   
  if( c < 0 )   
    c = 0;   
  if( c < Count )   
    return TStr(Data+c+1,Count - c - 1);   
  return "";   
}   
   
TStr operator +(const char* s, const TStr& rhs)   
{   
  TStr Result(s);   
  Result += rhs;   
  return Result;   
}   
   
// --    
   
TStr ToUpperCase(TStr src)   
{   
#ifdef TARGET_WIN32    
  TStr s(src);   
  src = strupr(src.c_str());   
#endif    
   
#ifdef TARGET_UNIX    
    int len = strlen ( src.c_str() ), a;   
    char *buf = new char [ len + 1 ];   
    char *str = src.c_str();   
       
    for ( a = 0; a < len; a ++ )   
    buf[a] = toupper( str[a] );   
   
    buf[a] = 0;   
       
    src = buf;   
    delete buf;   
#endif    
   
  return src;   
}   
   
TStr ToLowerCase(TStr src)   
{   
#ifdef TARGET_WIN32    
  TStr s(src);   
  src = strlwr(src.c_str());   
#endif    
   
#ifdef TARGET_UNIX    
    int len = strlen ( src.c_str() ), a;   
    char *buf = new char [ len + 1 ];   
    char *str = src.c_str();   
   
    for ( a = 0; a < len; a ++ )   
    buf[a] = tolower( str[a] );   
   
    buf[a] = 0;   
       
    src = buf;   
    delete buf;   
#endif    
   
  return src;   
}   
   
TStr DoubleToStr(double src)   
{   
  char s[1024];   
  sprintf( s, "%f", src );   
  return TStr(s);   
}   
   
TStr LongToStr(long src)   
{   
  char s[1024];   
  sprintf( s, "%ld", src );   
  return TStr(s);   
}   
   
TStr LongToStrHex(long src)   
{   
  char s[1024];   
  sprintf( s, "%lx", src );   
  return TStr(s);   
}   
   
TStr IntToStr(int src)   
{   
  char s[1024];   
  sprintf( s, "%d", src );   
  return TStr(s);   
}   
   
int StrToInt(TStr src)   
{   
  return src.ToInt();   
}   
   
int StrToIntDef(TStr src, int def)   
{   
  return src.ToIntDef(def);   
}   
   
double StrToDouble(TStr src)   
{   
  return src.ToDouble();   
}   
   
TStr StrToHTML(const TStr &src)   
{   
  TStr s;   
  int i = 0;   
  while( i < src.Count )   
  {   
    if( src.Data[i] == '<' )   
      s.Append("<");   
    else   
    if( src.Data[i] == '>' )   
      s.Append(">");   
    else   
    if( src.Data[i] == '"' )   
      s.Append("\"");   
    else   
    if( src.c_str()[i] == '&' )   
      s.Append("&");   
    else   
      s.Append(src.Data[i]);   
    i++;   
  }   
  return s;   
}   
   
TStr HTMLToStr(const TStr &src)   
{   
  TStr s;   
  int i = 0;   
  while( i < src.Count )   
  {   
    if( stricmp(src.Data+i,"<") == 0 ) {   
      s.Append("<");   
      i += 3;   
    } else   
    if( stricmp(src.Data+i,">") == 0 ) {   
      s.Append(">");   
      i += 3;   
    } else   
    if( stricmp(src.Data+i,"\"") == 0 ) {   
      s.Append("\"");   
      i += 5;   
    } else   
    if( stricmp(src.Data+i,"&") == 0 ) {   
      s.Append("&");   
      i += 4;   
    } else   
      s.Append(src.Data[i]);   
    i++;   
  }   
  return s;   
}   
   
TStr StrToSQL(const TStr &src)   
{   
  TStr s;   
  int i = 0;   
  while( i < src.Count )   
  {   
    if( src.Data[i] == '\0' )   
      s.Append("\\0'");   
    else   
    if( src.Data[i] == '\'' )   
      s.Append("\\'");   
    else   
    if( src.Data[i] == '\"' )   
      s.Append("\\\"");   
    else   
    if( src.Data[i] == '\n' )   
      s.Append("\\n");   
    else   
    if( src.Data[i] == '\r' )   
      s.Append("\\r");   
    else   
    if( src.Data[i] == '\t' )   
      s.Append("\\t");   
    else   
    if( src.Data[i] == '\\' )   
      s.Append("\\\\");   
    else   
      s.Append(src.Data[i]);   
    i++;   
  }   
  return s;   
}   
   
//NULL ASCII 0.  represent this by `\0'    
//\    ASCII 92, Represent this by `\\'    
//'    ASCII 39, Represent this by `\''    
//"    ASCII 34, Represent this by `\"'    
   
TStr StrToEscape(const char *s, int size)   
{   
  TStr r;   
  for( int i = 0; i < size; i++ )   
    if( s[i] == 0 )   
      r += "\\0";   
    else   
    if( s[i] == 92 )   
      r += "\\\\";   
    else   
    if( s[i] == 39 )   
      r += "\\'";   
    else   
    if( s[i] == 34 )   
      r += "\\\"";   
    else   
      r += s[i];   
  return r;   
}   
   
void EscapeToStr(TStr src, char *s, int size)   
{   
  int i = 0, j = 0;   
  while( i < src.Length() && j < size )   
  {   
    char c = src[i];   
    if( (i+1) < src.Length() ) {   
      if( src[i] == '\\' && src[i+1] == '0' ) {   
        c = 0;   
        i++;   
      } else   
      if( src[i] == '\\' && src[i+1] == '\\' ) {   
        c = 92;   
        i++;   
      } else   
      if( src[i] == '\\' && src[i+1] == '\'' ) {   
        c = 39;   
        i++;   
      } else   
      if( src[i] == '\\' && src[i+1] == '"' ) {   
        c = 34;   
        i++;   
      }   
    }   
    i++;   
    s[j++] = c;;   
  }   
}   
   
TStr BinToStr(const char *s, int size)   
{   
  TStr r;   
  for( int i = 0; i < size; i++ )   
    if( s[i] == 92 )   
      r += "\\\\";   
    else   
    if( s[i] == 0 )   
      r += "\\0";   
    else   
      r += s[i];   
  return r;   
}   
   
void StrToBin(TStr src, char *s, int size)   
{   
  int i = 0, j = 0;   
  while( i < src.Length() && j < size )   
  {   
    char c = src[i];   
    if( (i+1) < src.Length() ) {   
      if( src[i] == '\\' && src[i+1] == '0' ) {   
        c = 0;   
        i++;   
      } else   
      if( src[i] == '\\' && src[i+1] == '\\' ) {   
        c = 92;   
        i++;   
      }   
    }   
    i++;   
    s[j++] = c;   
  }   
}   

