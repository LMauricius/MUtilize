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
};


#endif