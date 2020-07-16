/*
Made by Mauricius

Part of my MUtilize repo: https://github.com/LegendaryMauricius/MUtilize
*/
#pragma once
#ifndef _SLOT_ARRAY_H
#define _SLOT_ARRAY_H

#include <vector>
#include <stdexcept>

/*
A random access class with the ability to free positions that are taken and find positions that are free without
invalidating iterators and references to taken positions.
Accessing a slot will automatically take it. If it was free before accessing it the iterators and references will be invalidated.
*/
template <class _T, class _Alloc = std::allocator<_T> >
class SlotArray
{
private:
	std::vector<_T, _Alloc> mSlots;
	std::vector<bool> mAreSlotsFree;// Whether a slot is free
	size_t mSize;

public:
    using value_type      = _T;
    using allocator_type  = _Alloc;
    using pointer         = _T*;
    using const_pointer   = const _T*;
    using reference       = _T&;
    using const_reference = const _T&;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;

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

		_IteratorImpl::value_type* mElement;
		const bool* mElementFree;

	public:
		inline _IteratorImpl() noexcept {}
		inline _IteratorImpl(const _IteratorImpl<_IteratorImpl::value_type>& it) noexcept :
			mElement(it.mElement),
			mElementFree(it.mElementFree)
		{}
		inline _IteratorImpl(_IteratorImpl::value_type* element, const bool* elementFree) noexcept :
			mElement(element),
			mElementFree(elementFree)
		{
		}
		inline _IteratorImpl::value_type* operator->() const {
			return mElement;
		}
		inline _IteratorImpl::reference operator*() const {
			return *mElement;
		}
		inline _IteratorImpl::reference operator[](size_t offset) {
			*(mElementFree + offset) = false;
			return *(mElement + offset);
		}
		inline _IteratorImpl::reference operator[](size_t offset) const {
			if (*(mElementFree + offset))
				throw std::out_of_range("Trying to access a free slot of a const SlotArray. "
					"Accessing the slot's content would need to take the slot first,"
					"which is impossible since the SlotArray iterator is const.");
			else
				return *(mElement+offset);
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator=(const _IteratorImpl<_IteratorImpl::value_type>& it) noexcept {
			mElementFree = it.mElementFree;
			mElement = it.mElement;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator+=(_IteratorImpl::difference_type offset) noexcept {
			mElementFree += offset;
			mElement += offset;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator-=(_IteratorImpl::difference_type offset) noexcept {
			mElementFree -= offset;
			mElement -= offset;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type> operator+(_IteratorImpl::difference_type offset) const noexcept {
			return _IteratorImpl<_IteratorImpl::value_type>(mElement + offset, mElementFree + offset);
		}
		inline _IteratorImpl<_IteratorImpl::value_type> operator-(_IteratorImpl::difference_type offset) const noexcept {
			return _IteratorImpl<_IteratorImpl::value_type>(mElement - offset, mElementFree - offset);
		}
		inline _IteratorImpl::difference_type operator-(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement - it.mElement);
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator++() noexcept {
			mElement++;
			mElementFree++;
			return *this;
		}
		inline _IteratorImpl<_IteratorImpl::value_type>& operator--() noexcept {
			mElement--;
			mElementFree--;
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
			return mElement == it.mElement;
		}
		inline bool operator!=(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return mWrapped != it.mWrapped;
		}
		inline bool operator<(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement < it.mElement);
		}
		inline bool operator>(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement > it.mElement);
		}
		inline bool operator<=(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement <= it.mElement);
		}
		inline bool operator>=(const _IteratorImpl<_IteratorImpl::value_type>& it) const noexcept {
			return (mElement >= it.mElement);
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
		SlotArray<_T>* mOwner;
		size_t mInd;

	public:
		inline _PersistentIteratorImpl() noexcept {}
		inline _PersistentIteratorImpl(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) noexcept :
			mOwner(it.mOwner),
			mInd(it.mInd)
		{}
		inline _PersistentIteratorImpl(size_t id, circular_queue<_PersistentIteratorImpl::value_type>* owner) noexcept :
			mInd(id),
			mOwner(owner)
		{
		}
		inline _PersistentIteratorImpl::value_type* operator->() const {
			return &mOwner[mInd];
		}
		inline _PersistentIteratorImpl::reference operator*() const {
			return mOwner[mInd];
		}
		inline _PersistentIteratorImpl::reference operator[](size_t offset) const {
			return mOwner[offset];
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) noexcept {
			mInd = it.mInd;
			mOwner = it.mOwner;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator+=(_PersistentIteratorImpl::difference_type offset) noexcept {
			mInd += offset;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator-=(_PersistentIteratorImpl::difference_type offset) noexcept {
			mInd -= offset;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type> operator+(_PersistentIteratorImpl::difference_type offset) const noexcept {
			return _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>(mInd + offset, mOwner);
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type> operator-(_PersistentIteratorImpl::difference_type offset) const noexcept {
			return _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>(mInd - offset, mOwner);
		}
		inline _PersistentIteratorImpl::difference_type operator-(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mInd - it.mInd;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator++() noexcept {
			mInd++;
			return *this;
		}
		inline _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& operator--() noexcept {
			mInd--;
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
			return mInd == it.mInd;
		}
		inline bool operator!=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mInd != it.mInd;
		}
		inline bool operator<(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mInd < it.mInd;
		}
		inline bool operator>(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mInd > it.mInd;
		}
		inline bool operator<=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mInd <= it.mInd;
		}
		inline bool operator>=(const _PersistentIteratorImpl<_PersistentIteratorImpl::value_type>& it) const noexcept {
			return mInd >= it.mInd;
		}
		inline operator bool() const noexcept {
			return mOwner->mSlots.begin() + mInd;
		}
	};

	using iterator = _IteratorImpl<value_type>;
	using const_iterator = _IteratorImpl<const value_type>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using persistent_iterator = _PersistentIteratorImpl<value_type>;
	using const_persistent_iterator = _PersistentIteratorImpl<const value_type>;
	using reverse_persistent_iterator = std::reverse_iterator<persistent_iterator>;
	using const_reverse_persistent_iterator = std::reverse_iterator<const_persistent_iterator>;

	/*
	Actual SlotArray methods
	*/

	static const size_t no_index = -1;

	SlotArray():
		mSize(0)
	{}

	_T& operator[](size_t slot)
	{
		if (slot >= mSlots.size())
		{
			mSlots.reserve(slot + 1);
			mAreSlotsFree.reserve(slot + 1);
			for (int i = mSlots.size(); i < slot; i++)
			{
				mSlots.push_back(_T());
				mAreSlotsFree.push_back(true);
			}
			mSlots.push_back(_T());
			mAreSlotsFree.push_back(false);
			mSize++;
			return mSlots.back();
		}
		else
		{
			if (mAreSlotsFree[slot]) mSize++;
			mAreSlotsFree[slot] = false;
			return mSlots[slot];
		}
	}

	const _T& operator[](size_t slot) const
	{
		if (slot < mSlots.size() && !mAreSlotsFree[slot])
		{
			return mSlots[slot];
		}
		else
		{
			throw std::out_of_range("Trying to access a free slot of a const SlotArray. "
				"Accessing the slot's content would need to take the slot first, which is impossible since the SlotArray is const.");
		}
	}

	size_t getFreeSlot() const
	{
		for (size_t i = 0; i < mSlots.size(); i++) {
			if (mAreSlotsFree[i])
				return i;
		}
		return mSlots.size();
	}

	void freeSlot(size_t slot)
	{
		if (slot < mSlots.size() && !mAreSlotsFree[slot]) {
			mAreSlotsFree[slot] = true;
			mSize--;
			while (mAreSlotsFree.size() && mAreSlotsFree.back()) {
				mSlots.pop_back();
				mAreSlotsFree.pop_back();
			}
		}
	}

	void clear()
	{
		mSlots.clear();
		mAreSlotsFree.clear();
		mSize = 0;
	}

	bool isSlotFree(size_t slot) const
	{
		if (slot < mSlots.size())
			return mAreSlotsFree[slot];
		return true;
	}

	size_t find(const _T& a) const
	{
		for (int i = 0; i < mSlots.size(); i++)
		{
			if (mSlots[i] == a) return i;
		}
		return no_index;
	}

	size_t slotCount() const
	{
		return mSlots.size();
	}

	size_t size() const
	{
		return mSize;
	}

	SlotArray<_T, _Alloc> operator=(const SlotArray<_T, _Alloc>& other)
	{
		mSlots = other.mSlots;
		mAreSlotsFree = other.mAreSlotsFree;
		mSize = other.mSize;
		return *this;
	}

	/*
	Normal iterators
	*/

	inline iterator begin() noexcept {
		return iterator(mSlots.data(), mAreSlotsFree.data());
	}

	inline iterator end() noexcept {
		return iterator(mSlots.data() + mSlots.size(), mAreSlotsFree.data() + mSlots.size());
	}

	inline const_iterator begin() const noexcept {
		return const_iterator(mSlots.data(), mAreSlotsFree.data());
	}

	inline const_iterator end() const noexcept {
		return const_iterator(mSlots.data() + mSlots.size(), mAreSlotsFree.data() + mSlots.size());
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
		return persistent_iterator(0, this);
	}

	inline persistent_iterator persistent_end() noexcept {
		return persistent_iterator(mSlots.size(), this);
	}

	inline const_persistent_iterator persistent_begin() const noexcept {
		return const_persistent_iterator(0, this);
	}

	inline const_persistent_iterator persistent_end() const noexcept {
		return const_persistent_iterator(mSlots.size(), this);
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


#endif