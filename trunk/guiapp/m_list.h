
#ifndef M_LISTH
#define M_LISTH

#include <stdlib.h>
#include <string.h>

//- List -----------------------------------------------------------------------

template <class Type> class TList;


template <class Type>
class TListItem {
	friend class TList<Type>;
private:

	TListItem<Type>* Next;
	

	Type*            Item;

protected:

	TListItem( Type *p ): Item(p)   { Next = NULL; }

	TListItem( Type *p, TListItem<Type>* p_next ): Item(p),Next(p_next)   {}
	

	virtual ~TListItem( void )   {}
	
public:

	Type* GetItem(void)   { return Item; }
};

//------------------------------------------------------------------

template <class Type>
class TFind {
public:

	virtual bool Is(Type*elem) = 0;
};


template <class Type>
class TIterator {
public:
	/**
	* 
	* @param elem ύλ
	*/
	virtual void Do(Type*elem) = 0;
};


/**

* @see TListItem
* @see TIterator
* @see TFind
*/
template <class Type>
class TList {
private:
	/**

	*/
	const bool      fItemDel;

	/**

	*/
	TListItem<Type> *Root;
	
	/**

	*/	
	TListItem<Type> *Last;


	TListItem<Type> **p_List;


	int             p_ListCount;
protected:

	int ListCount(void) {
		int c = 0;
		TListItem<Type> *p = Root;
		while( p != NULL )
		{
			c++;
			p = p->Next;
		}
		return c;
	}
	
	/**

	*/
	void Init(void) {
		Reset();
		int lcnt = ListCount();
		if ( lcnt ) {
			p_List = new TListItem<Type>*[lcnt];
		} else {
			p_List = NULL;
		}
		TListItem<Type> *p = Root;
		while ( p != NULL )
		{
			p_List[p_ListCount++] = p;
			p = p->Next;
		}
	}
	
	/**

	*/
	void Reset(void) {
		if( p_List != NULL ) {
			delete[] p_List;
			p_List = NULL;
			p_ListCount = 0;
		}
	}

	/**

	* @see TListItem
	*/
	inline void Add(TListItem<Type>* p) {
		if( isEmpty() )
			Root = Last = p;
		else {
			Last->Next = p;
			Last = p;
		}
	}
public:

	/**

	*/
	TList( const bool ItemDel = true ): fItemDel(ItemDel), p_List(NULL), p_ListCount(0) {
		Root = Last = NULL;
	}

	/**

	*/
	virtual ~TList( void ) {
		Reset();
		RemoveAll(fItemDel);
	}
	
	/**

	* @see TListItem
	*/
	TListItem<Type>*  hasObject(const Type& item);

	/**

	*/
	bool              isEmpty(void)   { return (Root==NULL); }

	/**

	*/
	void              RemoveAll(const bool fItemDel);

	/**

	*/
	void              Clear() { RemoveAll(true); }

	/**

	*/
	void              Add(TList<Type>* List, const bool fCopy);

	/**

	* @see TListItem
	*/
	TListItem<Type>*  Insert(TListItem<Type>* p, Type* p_item);

	/**

	*/
	TListItem<Type>*  Add(Type* p);

	/**

	*/
	bool              Del(Type* p);

	/**

	* @see TListItem
	*/
	bool              Del(TListItem<Type>* p);

	/**

	*/
	int               Count(void);

	/**

	*/
	Type*             Item(unsigned int Index);
	
	/**

	*/
	void              For(TIterator<Type>& Iterator);

	/**

	*/
	Type*             FindItem(TFind<Type>& Find);

	/**

	* @see TListItem
	*/
	TListItem<Type>*  Find(const Type *elem);

	/**

	* @see TFind
	* @see TListItem
	*/
	TListItem<Type>*  Find(TFind<Type>& Find);

	/**

	*/
	TListItem<Type>*  First(void) { return Root; }

	/**

	*/
	TListItem<Type>*  FindLast(void) { return Last; }

	/**

	*/
	TListItem<Type>*  Next(TListItem<Type>* p);
	
	/**

	*/
	Type* operator [](const int idx) const     { return Item(idx); }

	/**

	*/
	Type* operator [](const int idx)           { return Item(idx); }
};

template<class Type>
bool TList<Type>::Del(Type* Item) {
  Reset();
  if( Last->Item == Item )
    return Del(Last);
  else {
    TListItem<Type>* p = First();
    while( p != NULL )
    {
      if( p->Item == Item )
        return Del(p);
      p = Next(p);
    }
  }
  return false;
}

template<class Type>
bool TList<Type>::Del(TListItem<Type>* p) {
  if( p == NULL )
    return false;
 
  if ( Count() == 1 ) {
  	RemoveAll(fItemDel);
	return true;
  }
  
  Reset();
  TListItem<Type>* p_next = p->Next;
  if( p_next == NULL ) {
    TListItem<Type>* p_prev = Root;
    while( p_prev->Next != p )
      if( (p_prev = p_prev->Next) == NULL )
        return false;
    Last = p_prev;
    p_prev->Next = NULL;
    if( fItemDel )
      delete p->Item;
    delete p;
  } else {
    if( fItemDel )
      delete p->Item;
    p->Item = p_next->Item;
    p->Next = p_next->Next;
    if( Last == p_next )
      Last = p;
    delete p_next;
  }
  return true;
}

template<class Type>
inline TListItem<Type>* TList<Type>::Next(TListItem<Type>* p) {
  if( p != NULL )
    p = p->Next;
  return p;
}

template<class Type>
inline TListItem<Type>* TList<Type>::Add(Type* p)
{
  Reset();
  TListItem<Type>* p_list = new TListItem<Type>(p);
  Add(p_list);
  return p_list;
}

template<class Type>
inline TListItem<Type>* TList<Type>::Insert(TListItem<Type>* p, Type* p_item)
{
  Reset();
  TListItem<Type>* p_list = new TListItem<Type>(p);
  if( p == NULL ) {
    p_list->Next = Root;
    if( Last == Root )
      Last = p_list;
    Root = p_list;
  } else {
    p_list->Next = p->Next;
    p->Next = p_list;
    if( Last == p )
      Last = p_list;
  }
  return p_list;
}

template<class Type>
void TList<Type>::RemoveAll( const bool fItemDel ) {
  Reset();
  while( Root != NULL)
  {
    TListItem<Type>* p = Root->Next;
    if( fItemDel )
      delete Root->Item;
    delete Root;
    Root = p;
  }
  Root = Last = NULL;
}

template<class Type>
void TList<Type>::Add(TList<Type>* List, const bool fCopy) {
  Reset();
  TListItem<Type>* p = List->First();
  while( p != NULL)
  {
    if( fCopy ) {
      Type* p_item = new Type(*(p->GetItem()));
      Add(p_item);
    } else
      Add(p->GetItem());
    p = List->Next(p);
  }
}

template<class Type>
int TList<Type>::Count(void) {
  if( p_List == NULL )
    Init();
  return p_ListCount;
}

template<class Type>
TListItem<Type>* TList<Type>::Find(TFind<Type>& Find) {
  TListItem<Type> *p = Root;
  while( p != NULL)
  {
    if( Find.Is(p->Item) )
      return p;
    p = p->Next;
  }
  return NULL;
}

template<class Type>
TListItem<Type>* TList<Type>::Find(const Type *p_item) {
  TListItem<Type> *p = Root;
  while( p != NULL)
  {
    if( p->Item == p_item )
      return p;
    p = p->Next;
  }
  return NULL;
}

template<class Type>
Type* TList<Type>::FindItem(TFind<Type>& Find) {
  TListItem<Type> *p = Find(Find);
  if( p != NULL )
    return p->Item;
  return NULL;
}

template<class Type>
TListItem<Type>* TList<Type>::hasObject(const Type& item) {
  if( isEmpty() )
    return NULL;
  else {
    TListItem<Type> *p = Root;
    while( p != NULL )
    {
      if( item == *(p->Item) )
        return p;
      else
        p = p->Next;
    }
    return NULL;
  }
}

template<class Type>
void TList<Type>::For(TIterator<Type>& Iterator) {
  TListItem<Type> *p = Root;
  while( p != NULL)
  {
    Iterator.Do(p->Item);
    p = p->Next;
  }
}

template<class Type>
Type* TList<Type>::Item( unsigned int Index ) {
  if( p_List == NULL )
    Init();
  return p_List[Index]->Item;
}

//- Array ----------------------------------------------------------------------

/**

*/
template <class Type>
class TArray
{
private:
	
	const bool fItemDel;

	
	Type** fArray;

	
	unsigned int fCount;

	
	unsigned int fSize;
protected:
public:

	
	TArray( const int Size = 1000, const bool ItemDel = true ): fItemDel(ItemDel)   { fArray = new Type*[fSize=Size]; fCount = 0; }

	
	virtual ~TArray( void )   { RemoveAll(fItemDel); delete[] fArray; }
	

	void         RemoveAll(const bool ItemDel);


	inline int   Add(Type* p);

	/**
	*/
	int          Count(void) { return fCount; }

	/**
	*/
	Type*        Item(unsigned int Index) { 
		if ( Index < 0 || Index >= fCount ) {
			return NULL;
		}
		return fArray[Index];
	}

	/**
	*/
	bool         Del(unsigned int Index);
};

template<class Type>
bool TArray<Type>::Del( unsigned int Index ) {
	if ( Index < 0 || Index >= fCount ) {
		return false;
	}

	if( fItemDel )
		delete fArray[Index];
	
	fCount--;
	memcpy(fArray+Index, fArray+Index+1, sizeof(Type*)*(fCount-Index));
	return true;
}

template<class Type>
void TArray<Type>::RemoveAll( const bool ItemDel ) {
	if( ItemDel )
		for( unsigned int i = 0; i < fCount; i++ )
			delete fArray[i];
		fCount = 0;
}

template<class Type>
inline int TArray<Type>::Add(Type *p) {
	if( fCount >= fSize ) {
		int NewSize = fCount + fCount / 2;
		Type** pArray = new Type*[NewSize];
		memcpy(pArray,fArray,sizeof(Type*)*fSize);
		delete[] fArray;
		fArray = pArray;
		fCount = fSize;
		fSize = NewSize;
	}

	fArray[fCount] = p;
	return fCount++;
}

//- Chain ----------------------------------------------------------------------


template <class Type> class TChain;

/**
*/
template <class Type>
class TChainItem {
	friend class TChain<Type>;
private:
	/**
	*/
	TChainItem<Type>* Prev;

	/**
	*/
	TChainItem<Type>* Next;

	/**
	*/
	Type*             Item;
protected:

	/**
	*/
	TChainItem( Type *p ): Item(p)   { Prev = Next = NULL; }

	/**
	*/
	TChainItem( Type *p, TChainItem<Type>* p_prev, TChainItem<Type>* p_next ): Item(p),Prev(p_prev),Next(p_next)   {}

	/**
	*/
	virtual ~TChainItem( void )   {}
public:

	/**
	*/
	Type* GetItem(void)   { return Item; }
};



template <class Type>
class TChain {
private:
	
	const bool      fItemDel;


	TChainItem<Type> *Root;

		
	TChainItem<Type> *Last;
	
	
	int             Count;
protected:

	/**
	*/
	inline void Add(TChainItem<Type>* p) {
		if( isEmpty() )
			Root = Last = p;
		else {
			Last->Next = p;
			p->Prev = Last;
			Last = p;
		}
	}

public:
	
	/**
	*/
	TChain( const bool ItemDel = true ): fItemDel(ItemDel) {
		Root = Last = NULL;
	}
	
	/**
	*/
	virtual ~TChain( void ) {
		RemoveAll(fItemDel);
	}
	
	/**
	*/
	TChainItem<Type>*  hasObject(const Type& item);

	/**
	*/
	bool               isEmpty(void)   { return (Root==NULL); }

	/**
	*/
	void               RemoveAll(const bool fItemDel);

	/**
	*/
	void               Clear() { RemoveAll(true); }

	/**
	*/
	void               Add(TChain<Type>* Chain, const bool fCopy);

	/**
	*/
	TChainItem<Type>*  Insert(TChainItem<Type>* p, Type* p_item);

	/**
	*/
	TChainItem<Type>*  Add(Type* p);

	/**
	*/
	bool               Del(Type* p);

	/**
	*/
	bool               Del(TChainItem<Type>* p);

	/**
	*/
	bool               DelFirst();
	
	/**
	*/
	void               For(TIterator<Type>& Iterator);

	/**
	*/
	Type*              FindItem(TFind<Type>& Find);

	/**
	*/
	TChainItem<Type>*  Find(TFind<Type>& Find);

	/**
	*/
	TChainItem<Type>*  Find(const Type *elem);

	/**
	*/
	TChainItem<Type>*  First(void) { return Root; }
	
	/**
	*/
	TChainItem<Type>*  FindLast(void) { return Last; }
	
	/**
	*/
	TChainItem<Type>*  Next(TChainItem<Type>* p);
	
	/**
	*/
	TChainItem<Type>*  Prev(TChainItem<Type>* p);
};

template<class Type>
bool TChain<Type>::Del(Type* Item) {
  TChainItem<Type>* p = Find(Item);
  if( p != NULL )
    return Del(p);
  return false;
}

template<class Type>
bool TChain<Type>::Del(TChainItem<Type>* p) {
  if( p == NULL )
    return false;
  if( Root == p )
    Root = p->Next;
  if( Last == p )
    Last = p->Prev;
  if( p->Prev != NULL )
    p->Prev->Next = p->Next;
  if( p->Next != NULL )
    p->Next->Prev = p->Prev;
  if( fItemDel )
    delete p->Item;
  delete p;
  return true;
}

template<class Type>
bool TChain<Type>::DelFirst() {
  return Del(First());
}

template<class Type>
inline TChainItem<Type>* TChain<Type>::Prev(TChainItem<Type>* p) {
  if( p != NULL )
    p = p->Prev;
  return p;
}

template<class Type>
inline TChainItem<Type>* TChain<Type>::Next(TChainItem<Type>* p) {
  if( p != NULL )
    p = p->Next;
  return p;
}

template<class Type>
inline TChainItem<Type>* TChain<Type>::Add(Type* p)
{
  TChainItem<Type>* p_ = new TChainItem<Type>(p);
  Add(p_);
  return p_;
}

template<class Type>
void TChain<Type>::RemoveAll( const bool fItemDel ) {
  while( Root != NULL)
  {
    TChainItem<Type>* p = Root->Next;
    if( fItemDel )
      delete Root->Item;
    delete Root;
    Root = p;
  }
  Root = Last = NULL;
}

template<class Type>
void TChain<Type>::Add(TChain<Type>* Chain, const bool fCopy) {
  TChainItem<Type>* p = Chain->First();
  while( p != NULL)
  {
    if( fCopy ) {
      Type* p_item = new Type(*(p->GetItem()));
      Add(p_item);
    } else
      Add(p->GetItem());
    p = Chain->Next(p);
  }
}

template<class Type>
inline TChainItem<Type>* TChain<Type>::Insert(TChainItem<Type>* p, Type* p_item)
{
  TChainItem<Type>* p_ = new TChainItem<Type>(p);
  if( p == NULL ) {
    p_->Next = Root;
    if( Root != NULL )
      Root->Prev = p_item;
    if( Last == Root )
      Last = p_;
    Root = p_;
  } else {
    p_->Next = p->Next;
    p->Prev = p;
    p->Next = p_;
    if( Last == p )
      Last = p;
  }
  return p;
}

template<class Type>
Type* TChain<Type>::FindItem(TFind<Type>& Find) {
  TChainItem<Type>* p = Find(Find);
  if( p != NULL )
    return p->Item;
  return NULL;
}

template<class Type>
TChainItem<Type>* TChain<Type>::Find(const Type *p_item) {
  TChainItem<Type> *p = Root;
  while( p != NULL)
  {
    if( p->Item == p_item )
      return p;
    p = p->Next;
  }
  return NULL;
}

template<class Type>
TChainItem<Type>* TChain<Type>::Find(TFind<Type>& Find) {
  TChainItem<Type> *p = Root;
  while( p != NULL)
  {
    if( Find.Is(p->Item) )
      return p;
    p = p->Next;
  }
  return NULL;
}

template<class Type>
TChainItem<Type>* TChain<Type>::hasObject(const Type& item) {
  if( isEmpty() )
    return NULL;
  else {
    TChainItem<Type> *p = Root;
    while( p != NULL )
    {
      if( *(p->Item) == item )
        return p;
      else
        p = p->Next;
    }
    return NULL;
  }
}

template<class Type>
void TChain<Type>::For(TIterator<Type>& Iterator) {
  TChainItem<Type> *p = Root;
  while( p != NULL)
  {
    Find.Do(p->Item);
    p = p->Next;
  }
}

#endif

