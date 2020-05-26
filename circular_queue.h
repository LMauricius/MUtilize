/*
Made by Mauricius

Part of my MUtilize repo: https://github.com/LegendaryMauricius/MUtilize
The reason why this file uses the lowercase naming convention is to fit right in with the STL.
It's done similarly to the STL and compatible with STL functions and templates, and something that
should have been in the STL by default IMO. So why not make it look like an STD header?
*/

#pragma once

#include <vector>
#include <algorithm>

template <class _T, class _Alloc = std::allocator<_T> >
class circular_queue
{
private:
	size_t mSize;
	// Offset of the first element from the buffer begining
	size_t mBeginOffset;
	/*
	Each inserted element gets its ID that is by 1 greater then the last inserted element.
	In other words, ID is the ordinal number of an element since the last queue's total reset or consctruction.
	mIDOffset is used to store the ID of the front element in queue.
	Whenever an element is popped out, mBeginOffset is increased by 1 (if there is space in the buffer), so
	we also need to increase mIDOffset to compensate for that.
	mIDOffset is always increased since it's more of a 'virtual' fixed number unique to each element.
	*/
	size_t mIDOffset;
	std::vector<_T> mBuffer;

public:
	using value_type = _T;
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using allocator_type = _Alloc;
	using pointer = value_type*;
	using reference = value_type&;
	using const_pointer = const value_type*;
	using const_reference = const value_type&;

	/*
	Iterator definitions
	*/

	// normal iterator. This one gets invalidated when the queue's capacity changes
	template<class _ItT>
	class _IteratorImpl {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = _ItT;
		using difference_type = std::ptrdiff_t;
		using pointer = _IteratorImpl::value_type*;
		using reference = _IteratorImpl::value_type&;
		using const_pointer = const _IteratorImpl::value_type*;
		using const_reference = const _IteratorImpl::value_type&;
	private:
		_IteratorImpl::value_type *mElement, *mWrapPoint, *mBufferBegin;

		// Whether the _IteratorImpl::pointer has wrapped around the buffer end
		bool mWrapped;
		
		inline _IteratorImpl::value_type* calcOffsetPtr(std::ptrdiff_t offset) {
			return (
				mBufferBegin + (mElement - mBufferBegin + offset) % (mWrapPoint - mBufferBegin)
				);
		}

	public:
		inline _IteratorImpl() noexcept {}
		inline _IteratorImpl(const _IteratorImpl<_IteratorImpl::value_type>& it) noexcept :
			mWrapped(it.mWrapped),
			mElement(it.mElement),
			mWrapPoint(it.mWrapPoint),
			mBufferBegin(it.mBufferBegin)
		{}
		inline _IteratorImpl(_IteratorImpl::value_type* element, bool wrapped, _IteratorImpl::value_type* wrapPoint, _IteratorImpl::value_type* bufferBegin) noexcept :
			mElement(element),
			mWrapped(wrapped),
			mWrapPoint(wrapPoint),
			mBufferBegin(bufferBegin)
		{
		}
		inline _IteratorImpl::value_type* operator->() const {
			return mElement;
		}
		inline _IteratorImpl::reference operator*() const {
			return *mElement;
		}
		inline _IteratorImpl::reference operator[](size_t offset) const {
			return *calcOffsetPtr(offset);
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator=(const _IteratorImpl<_IteratorImpl::value_type>& it) noexcept {
			mWrapped = it.mWrapped;
			mElement = it.mElement;
			mWrapPoint = it.mWrapPoint;
			mBufferBegin = it.mBufferBegin;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator+=(_IteratorImpl::difference_type offset) noexcept {
			if (mElement + offset >= mWrapPoint) {
				mElement -= (mWrapPoint - mBufferBegin);
				mWrapped = true;
			}
			else if (mElement + offset < mBufferBegin) {
				mElement += (mWrapPoint - mBufferBegin);
				mWrapped = false;
			}
			mElement += offset;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator-=(_IteratorImpl::difference_type offset) noexcept {
			if (mElement - offset < mBufferBegin) {
				mElement += (mWrapPoint - mBufferBegin);
				mWrapped = false;
			}
			else if (mElement - offset >= mWrapPoint) {
				mElement -= (mWrapPoint - mBufferBegin);
				mWrapped = true;
			}
			mElement -= offset;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type> operator+(_IteratorImpl::difference_type offset) const noexcept {
			return iterator(mElement, mWrapped, mWrapPoint, mBufferBegin) += offset;
		}
		inline _IteratorImpl<_IteratorImpl::value_type> operator-(_IteratorImpl::difference_type offset) const noexcept {
			return iterator(mElement, mWrapped, mWrapPoint, mBufferBegin) -= offset;
		}
		inline _IteratorImpl::difference_type operator-(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (
				(mWrapped? mElement + (mWrapPoint - mBufferBegin): mElement) - 
				(it.mWrapped? it.mElement + (mWrapPoint - mBufferBegin) : it.mElement)
				);
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator++() noexcept {
			mElement++;
			if (mElement == mWrapPoint) {
				mElement = mBufferBegin;
				mWrapped = true;
			}
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator--() noexcept {
			if (mElement == mBufferBegin) {
				mElement = mWrapPoint;
				mWrapped = false;
			}
			mElement--;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type> operator++(int) noexcept {
			_IteratorImpl<_IteratorImpl::value_type> old = *this;
			(*this)++;
			return old;
		}
		inline _IteratorImpl<_IteratorImpl::value_type> operator--(int) noexcept {
			_IteratorImpl<_IteratorImpl::value_type> old = *this;
			(*this)--;
			return old;
		}
		inline bool operator==(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return mElement == it.mElement && mWrapped == it.mWrapped;
		}
		inline bool operator!=(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return mWrapped != it.mWrapped || mElement != it.mElement;
		}
		inline bool operator<(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement < it.mElement && mWrapped == it.mWrapped) || (!mWrapped && it.mWrapped);
		}
		inline bool operator>(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement > it.mElement && mWrapped == it.mWrapped) || (mWrapped && !it.mWrapped);
		}
		inline bool operator<=(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement <= it.mElement && mWrapped == it.mWrapped) || (!mWrapped && it.mWrapped);
		}
		inline bool operator>=(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement >= it.mElement && mWrapped == it.mWrapped) || (mWrapped && !it.mWrapped);
		}
		inline operator bool() const noexcept {
			return mElement;
		}
	};

	// persistent iterator, that is not invalidated as long as it's in the range
	friend class _PersistentIteratorImpl;

	template<class _ItT>
	class _PersistentIteratorImpl {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = _ItT;
		using difference_type = std::ptrdiff_t;
		using pointer = _PersistentIteratorImpl::value_type*;
		using reference = _PersistentIteratorImpl::value_type;

	private:
		circular_queue<_PersistentIteratorImpl::value_type> *mOwner;
		_PersistentIteratorImpl::value_type *mElement;
		// ID of the pointed element. This is used to calculate the _PersistentIteratorImpl::pointer to the actual element.
		size_t mID;

	public:
		inline _PersistentIteratorImpl() noexcept {}
		inline _PersistentIteratorImpl(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) noexcept :
			mElement(it.mElement),
			mOwner(it.mOwner),
			mID(it.mID)
		{}
		inline _PersistentIteratorImpl(size_t id, circular_queue<_PersistentIteratorImpl::value_type>* owner) noexcept :
			mID(id),
			mOwner(owner)
		{
		}
		inline _PersistentIteratorImpl::value_type* operator->() const {
			return mOwner->mBuffer.data() + ((mOwner->mBeginOffset + mID - mOwner->mIDOffset) % mOwner->mBuffer.capacity());
		}
		inline _PersistentIteratorImpl::reference operator*() const {
			return *(mOwner->mBuffer.data() + ((mOwner->mBeginOffset + mID - mOwner->mIDOffset) % mOwner->mBuffer.capacity()));
		}
		inline _PersistentIteratorImpl::reference operator[](size_t offset) const {
			return *(mOwner->mBuffer.data() + ((mOwner->mBeginOffset + mID + offset - mOwner->mIDOffset) % mOwner->mBuffer.capacity()));
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) noexcept {
			mID = it.mID;
			mElement = it.mElement;
			mOwner = it.mOwner;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator+=(_PersistentIteratorImpl::difference_type offset) noexcept {
			mID += offset;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator-=(_PersistentIteratorImpl::difference_type offset) noexcept {
			mID -= offset;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type> operator+(_PersistentIteratorImpl::difference_type offset) const noexcept {
			return _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>(mID + offset, mOwner);
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type> operator-(_PersistentIteratorImpl::difference_type offset) const noexcept {
			return _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>(mID - offset, mOwner);
		}
		inline _PersistentIteratorImpl::difference_type operator-(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& offset) const noexcept {
			return mID - offset.mId;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator++() noexcept {
			mID++;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator--() noexcept {
			mID--;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type> operator++(int) noexcept {
			_PersistentIteratorImpl<_PersistentIteratorImpl::value_type> old = *this;
			(*this)++;
			return old;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type> operator--(int) noexcept {
			_PersistentIteratorImpl<_PersistentIteratorImpl::value_type> old = *this;
			(*this)--;
			return old;
		}
		inline bool operator==(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mID == it.mID;
		}
		inline bool operator!=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mID != it.mID;
		}
		inline bool operator<(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mID < it.mID;
		}
		inline bool operator>(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mID > it.mID;
		}
		inline bool operator<=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mID <= it.mID;
		}
		inline bool operator>=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mID >= it.mID;
		}
		inline operator bool() const noexcept {
			return mElement;
		}
	};

	using iterator					= _IteratorImpl<value_type>;
	using const_iterator			= _IteratorImpl<const value_type>;
	using reverse_iterator			= std::reverse_iterator<iterator>;
	using const_reverse_iterator	= std::reverse_iterator<const_iterator>;
	using persistent_iterator				= _PersistentIteratorImpl<value_type>;
	using const_persistent_iterator			= _PersistentIteratorImpl<const value_type>;
	using reverse_persistent_iterator		= std::reverse_iterator<persistent_iterator>;
	using const_reverse_persistent_iterator	= std::reverse_iterator<const_persistent_iterator>;

	/*
	Queue public definitions
	*/

	circular_queue():
		mSize(0),
		mBeginOffset(0),
		mIDOffset(0)
	{}

	template <class InputIterator>
	circular_queue(InputIterator first, InputIterator last) :
		mSize(last - first),
		mBeginOffset(0),
		mIDOffset(0),
		mBuffer(first, last)
	{}
	
	circular_queue(size_t n, const_reference val):
		mSize(n),
		mBeginOffset(0),
		mIDOffset(0),
		mBuffer(n, val)
	{}

	circular_queue(std::initializer_list<value_type> il) :
		mSize(il.size()),
		mBeginOffset(0),
		mIDOffset(0),
		mBuffer(il)
	{}

	circular_queue<value_type>& operator=(const circular_queue<value_type> & other)
	{
		mIDOffset += mSize;
		mSize = other.mSize;
		mBeginOffset = other.mBeginOffset;
		mBuffer = other.mBuffer;
		return *this;
	}

	/*
	Capacity
	*/

	void reserve(size_t cap) {
		// we have to allocate a new buffer so we might as well make it more efficient
		if (mBuffer.capacity() < cap) {
			optimize_order();
			mBuffer.reserve(cap);
		}
	}

	void clear() {
		mIDOffset += mSize;
		mBuffer.clear();
		mSize = 0;
		mBeginOffset = 0;
	}

	// Resizes the buffer to fit the queue size. Possibly optimizes the order
	void shrink_to_fit() {
		optimize_order();
		mBuffer.resize(mSize);
	}

	// Reorders the elements linearly, so that the orders of elements in the queue corresponds to the order in the memory
	void optimize_order() {
		std::rotate(mBuffer.begin(), std::next(mBuffer.begin(), mBeginOffset), mBuffer.end());
		mBeginOffset = 0;
	}

	inline size_t size() const noexcept {
		return mSize;
	}

	inline size_t max_size() const noexcept {
		return mBuffer.max_size();
	}

	inline size_t capacity() const noexcept {
		return mBuffer.capacity();
	}

	inline bool empty() const noexcept {
		return mSize == 0;
	}

	/*
	Access
	*/

	inline reference operator[](size_t index) {
		return mBuffer[(mBeginOffset + index) % mBuffer.capacity()];
	}

	inline const_reference operator[](size_t index) const {
		return mBuffer[(mBeginOffset + index) % mBuffer.capacity()];
	}

	inline reference at(size_t index) {
		if (index < 0 || index >= mSize)
			throw std::out_of_range("Index out of range");
		return mBuffer.at((mBeginOffset + index) % mBuffer.capacity());
	}

	inline const_reference at(size_t index) const {
		if (index < 0 || index >= mSize)
			throw std::out_of_range("Index out of range");
		return mBuffer.at((mBeginOffset + index) % mBuffer.capacity());
	}

	inline value_type* data() {
		return mBuffer.data();
	}

	inline reference front() {
		return mBuffer[mBeginOffset];
	}

	inline const_reference front() const {
		return mBuffer[mBeginOffset];
	}

	/*
	Modification
	*/

	template <class InputIterator>
	void assign(InputIterator first, InputIterator last) {
		mIDOffset += mSize;
		mSize = last - first;
		mBeginOffset = 0;
		mBuffer.assign(first, last);
	}

	void assign(size_t n, const_reference val) {
		mIDOffset += mSize;
		mSize = n;
		mBeginOffset = 0;
		mBuffer.assign(n, val);
	}

	void assign(std::initializer_list<value_type> il) {
		mIDOffset += mSize;
		mSize = il.size();
		mBeginOffset = 0;
		mBuffer.assign(il);
	}

	template <class... _Args>
	void emplace_back(_Args&&... args) {
		// if the queue buffer is in one piece
		if (mBeginOffset + mSize < mBuffer.capacity() || mBeginOffset == 0) {
			mBuffer.emplace_back(args...);
		}
		// if we have free space in the buffer
		else if (mSize < mBuffer.capacity()) {
			value_type *obj = &mBuffer[(mBeginOffset + mSize) % mBuffer.size()];

			obj->~value_type();// destroy the prev object
			new (obj) value_type(args...);// construct the new object
		}
		// otherwise we have to allocate a new buffer so we might as well make it more efficient
		else {
			optimize_order();
			mBuffer.emplace_back(args...);
		}

		mSize++;
	}

	void push_back(const value_type& val) {
		emplace_back(val);
	}

	void push_back(value_type&& val) {
		emplace_back(std::move(val));
	}

	void pop_front() {
		mBeginOffset++;
		if (mBeginOffset == mBuffer.capacity())
			mBeginOffset = 0;
		mIDOffset++;
		mSize--;
	}

	void swap(circular_queue<value_type>& other) {
		std::swap(mSize, other.mSize);
		std::swap(mBeginOffset, other.mBeginOffset);
		std::swap(mIDOffset, other.mIDOffset);
		mBuffer.swap(other.mBuffer);
	}

	/*
	Comparisons
	*/

	inline bool operator==(const circular_queue<value_type>& other) const noexcept {
		if (mSize != other.mSize)
			return false;
		return std::equal(begin(), end(), other.begin(), other.end());
	}
	inline bool operator!=(const circular_queue<value_type>& other) const noexcept {
		return !((*this) == other);
	}
	inline bool operator<(const circular_queue<value_type>& other) const noexcept {
		return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
	}
	inline bool operator>(const circular_queue<value_type>& other) const noexcept {
		return (other < (*this));
	}
	inline bool operator<=(const circular_queue<value_type>& other) const noexcept {
		return !((*this) > other);
	}
	inline bool operator>=(const circular_queue<value_type>& other) const noexcept {
		return !((*this) < other);
	}

	/*
	Normal iterators
	*/

	inline iterator begin() noexcept {
		return iterator(mBuffer.data() + mBeginOffset, false, mBuffer.data() + mBuffer.capacity(), mBuffer.data());
	}

	inline iterator end() noexcept {
		return iterator((
			(mBeginOffset + mSize < mBuffer.capacity()) ?
				mBuffer.data() + mBeginOffset + mSize :
				mBuffer.data() + mBeginOffset + mSize - mBuffer.capacity()
			),
			(mBeginOffset + mSize >= mBuffer.capacity()), mBuffer.data() + mBuffer.capacity(), mBuffer.data());
	}

	inline const_iterator begin() const noexcept {
		return const_iterator(mBuffer.data() + mBeginOffset, false, mBuffer.data() + mBuffer.capacity(), mBuffer.data());
	}

	inline const_iterator end() const noexcept {
		return const_iterator((
			(mBeginOffset + mSize < mBuffer.capacity()) ?
				mBuffer.data() + mBeginOffset + mSize :
				mBuffer.data() + mBeginOffset + mSize - mBuffer.capacity()
			),
			(mBeginOffset + mSize >= mBuffer.capacity()), mBuffer.data() + mBuffer.capacity(), mBuffer.data());
	}

	inline const_iterator cbegin() const noexcept {
		return begin();
	}

	inline const_iterator cend() const noexcept {
		return end();
	}

	/*
	Reverse iterators
	*/

	inline reverse_iterator rbegin() noexcept {
		return reverse_iterator(end());
	}

	inline reverse_iterator rend() noexcept {
		return reverse_iterator(begin());
	}

	inline const_reverse_iterator rbegin() const noexcept {
		return reverse_iterator(end());
	}

	inline const_reverse_iterator rend() const noexcept {
		return reverse_iterator(begin());
	}

	inline const_reverse_iterator crbegin() const noexcept {
		return reverse_iterator(cend());
	}

	inline const_reverse_iterator crend() const noexcept {
		return reverse_iterator(cbegin());
	}

	/*
	Persistent iterators
	*/

	inline persistent_iterator persistent_begin() noexcept {
		return persistent_iterator(mIDOffset, this);
	}

	inline persistent_iterator persistent_end() noexcept {
		return persistent_iterator(mIDOffset + mSize, this);
	}

	inline const_persistent_iterator persistent_begin() const noexcept {
		return const_persistent_iterator(mIDOffset, this);
	}

	inline const_persistent_iterator persistent_end() const noexcept {
		return const_persistent_iterator(mIDOffset + mSize, this);
	}

	inline const_persistent_iterator persistent_cbegin() const noexcept {
		return persistent_begin();
	}

	inline const_persistent_iterator persistent_cend() const noexcept {
		return persistent_end();
	}

	/*
	Reverse persistent iterators
	*/

	inline reverse_persistent_iterator persistent_rbegin() noexcept {
		return reverse_persistent_iterator(persistent_end());
	}

	inline reverse_persistent_iterator persistent_rend() noexcept {
		return reverse_persistent_iterator(persistent_begin());
	}

	inline const_reverse_persistent_iterator persistent_rbegin() const noexcept {
		return reverse_persistent_iterator(persistent_end());
	}

	inline const_reverse_persistent_iterator persistent_rend() const noexcept {
		return reverse_persistent_iterator(persistent_begin());
	}

	inline const_reverse_persistent_iterator persistent_crbegin() const noexcept {
		return reverse_persistent_iterator(persistent_cend());
	}

	inline const_reverse_persistent_iterator persistent_crend() const noexcept {
		return reverse_persistent_iterator(persistent_cbegin());
	}
};