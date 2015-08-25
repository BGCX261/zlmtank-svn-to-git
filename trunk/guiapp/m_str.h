/** 
* @file m_str.h 
*/ 
 
#ifndef m_stringH 
#define m_stringH 
 
/** 
*/ 
class TStr 
{ 
friend TStr operator +(const char*, const TStr& rhs); 
friend TStr StrToSQL(const TStr&); 
friend TStr StrToHTML(const TStr&); 
friend TStr HTMLToStr(const TStr&); 
private: 
  int  Count; 
  int  Size; 
  char *Data; 
  void CheckSize(int NewCount); 
public: 
  TStr(void); 
  TStr(const TStr& src); 
  TStr(const char* src, int len = -1); 
  TStr(char src); 
  TStr(int src); 
  TStr(unsigned int src); 
  TStr(long src); 
  TStr(unsigned long src); 
  TStr(double src); 
  ~TStr(void); 
 
  // operator 
  TStr& operator = (const TStr& rhs); 
  TStr& operator +=(const TStr& rhs); 
  TStr  operator + (const TStr& rhs) const; 
  bool  operator ==(const TStr& rhs) const; 
  bool  operator !=(const TStr& rhs) const; 
  char  operator [](const int idx) const     { return Data[idx]; } 
  char& operator [](const int idx)           { return Data[idx]; } 
 
//  TStr&       sprintf(const char* format, ...); 
//  int         printf(const char* format, ...); 
 
  char*       c_str() const { return Data; } 
 
  int         Length()  const { return Count; } 
  bool        isEmpty() const { return Count == 0; }; 
 
  void        Insert(const TStr& str, int index); 
  void        Delete(int index, int count); 
  void        SetLength(int newLength); 
  void        Set(const char* src, int count = -1); 
  void        Append(const char* src, int count = -1); 
  void        Append(const char src); 
  bool        Compare(const char* src, int count = -1); 
  bool        Compare(const TStr&); 
  void        Clear(); 
  TStr        Trim() { return this->TrimLeft().TrimRight(); } 
  TStr        TrimLeft(); 
  TStr        TrimRight(); 
  TStr        Copy(int, int); 
  TStr        CopyAfter(int); 
  TStr        CopyAfter(const TStr &sub); 
  TStr        CopyBefore(const TStr &sub); 
  TStr        CopyBefore(int); 
  TStr        GetBefore(char sub, const bool subdel = true) { return GetBefore(TStr(sub),subdel); } 
  TStr        GetBefore(const TStr &sub, const bool subdel = true); 
  TStr        GetBefore(int Index, const bool subdel = true); 
  TStr        Replace(const TStr &sub, const TStr &replace); 
  int         Pos(const TStr &sub); 
  int         RigthPos(const TStr &sub); 
 
  int         ToInt() const; 
  bool		  ToBool() const; 
  int         ToIntDef(int) const; 
  double      ToDouble() const; 
 
  TStr        Word(int); 
  TStr        Right(int); 
}; 
 
extern TStr operator +(const char*, const TStr& rhs); 
 
void   StrToBin(TStr src, char *s, int size); 
TStr   BinToStr(const char *s, int size); 
TStr   StrToEscape(const char *s, int size); 
void   EscapeToStr(TStr src, char *s, int size); 
TStr   StrToSQL(const TStr &src); 
TStr   StrToHTML(const TStr &src); 
TStr   HTMLToStr(const TStr &src); 
TStr   ToUpperCase(TStr src); 
TStr   ToLowerCase(TStr src); 
TStr   IntToStr(int src); 
TStr   LongToStrHex(long src); 
TStr   LongToStr(long src); 
int    StrToIntDef(TStr src, int def); 
int    StrToInt(TStr src); 
double StrToDouble(TStr src); 
// 
TStr   MailTo(); 
 
// ostream& operator << (ostream& os, const TStr& arg); 
// istream& operator >> (istream& is, TStr& arg); 
 
#endif 

