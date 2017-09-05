/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2013 Bernd Schoeler <brjohn@brother-john.net>
**
** This file is part of Photivo.
**
** Photivo is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 3
** as published by the Free Software Foundation.
**
** Photivo is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Photivo.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************
**
** Shout-outs go out to Tim Day, whose article "LRU cache implementation in C++"
** <http://timday.bitbucket.org/lru.html> was a great inspiration for this
** implementation.
**
*******************************************************************************/

#include "ptThumbCache.h"
#include "../ptMutexLocker.h"
#include "../ptImage8.h"

//------------------------------------------------------------------------------
ptThumbCache::ptThumbCache(uint AMaxSizeBytes):
  FCapacity(AMaxSizeBytes),
  FOccupancy(0)
{
#ifdef QT_DEBUG
  printf("Thumbnail image cache size: %d bytes\n", FCapacity);
#endif
}

//------------------------------------------------------------------------------

ptThumbCache::~ptThumbCache() {}

//------------------------------------------------------------------------------
/*! Immediately empties the entire cache. */
void ptThumbCache::clear() {
  ptMutexLocker hLock(&FMutex);
  FAccess.clear();
  FCache.clear();
  FCache.squeeze();
  FOccupancy = 0;
}

//------------------------------------------------------------------------------
/*!
  Returns the image matching AThumbId from the cache, or an empty TThumbPtr if
  no matching image can be found.
*/
TThumbPtr ptThumbCache::find(const TThumbId& AThumbId) {
  const auto hIdHash = this->makeHashable(AThumbId);
  ptMutexLocker hLock(&FMutex);
  auto hDataIter = FCache.find(hIdHash);

  // cache hit: move item to the end of the access tracker list, then return the thumbnail
  if (hDataIter != FCache.end()) {
    FAccess.erase(hDataIter->AccessIter);
    hDataIter->AccessIter = FAccess.insert(FAccess.end(), hIdHash);
    return hDataIter->Data;
  }

  // cache miss: return empty thumbnail
  return TThumbPtr();
}

//------------------------------------------------------------------------------
/*! Inserts a new image into the cache. The image must not already be present in the cache. */
void ptThumbCache::insert(const TThumbId& AThumbId, TThumbPtr AThumbData) {
  const auto hIdHash = this->makeHashable(AThumbId);
  ptMutexLocker hLock(&FMutex);

  // Abort when the cache already contains the item. Reasons:
  // - Caching an item twice makes no sense, only wastes cache capacity.
  // - Inserting into QHash would replace the existing item, but adding to the acces tracker
  //   would not, thus making hash list and tracker go out of sync.
  if (FCache.contains(hIdHash) || (FCapacity == 0)) {
    return;
  }

  FOccupancy += AThumbData->sizeBytes();
  this->evict();

  FCache.insert(hIdHash, {AThumbData,
                          AThumbData->sizeBytes(),
                          FAccess.insert(FAccess.end(), hIdHash)});
}

//------------------------------------------------------------------------------
// If the cache is filled over capacity, removes as many least-recently-used items
// as needed to get the cache at or below capacity again.
// Actually the cache never really holds too much data. Only the occupancy figure may
// exceed capacity because it is calculated first.
void ptThumbCache::evict() {
  if (FAccess.isEmpty())
    return;

  while (FOccupancy > FCapacity) {
    if (FCache.isEmpty()) {
      FOccupancy = 0;
      return;
    }

    auto hDataIter = FCache.find(FAccess.first());

    Q_ASSERT_X(hDataIter != FCache.end(), __PRETTY_FUNCTION__,
               QString("Head of access tracker not in cache: " + FAccess.first()).toLocal8Bit().data());

    FOccupancy -= hDataIter->Size;
    FCache.erase(hDataIter);
    FAccess.removeFirst();
  }
}

//------------------------------------------------------------------------------
// Helper to make a hashable string from a TThumbId
ptThumbCache::TThumbKey ptThumbCache::makeHashable(const TThumbId& AThumbId) const {
  return AThumbId.FilePath + AThumbId.Timestamp.toString() + QString::number(AThumbId.LongEdgeSize);
}
