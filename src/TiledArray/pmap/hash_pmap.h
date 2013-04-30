/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2013  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TILEDARRAY_PMAP_HASH_PMAP_H__INCLUDED
#define TILEDARRAY_PMAP_HASH_PMAP_H__INCLUDED

#include <TiledArray/pmap/pmap.h>
#include <TiledArray/madness.h>

namespace TiledArray {
  namespace detail {

    /// Process map base class
    class HashPmap : public Pmap<std::size_t> {
    public:
      typedef Pmap<std::size_t>::key_type key_type;
      typedef Pmap<std::size_t>::const_iterator const_iterator;

      HashPmap(madness::World& world, std::size_t size) :
          rank_(world.rank()),
          procs_(world.size()),
          size_(size),
          seed_(0ul),
          local_()
      { }

    private:

      HashPmap(const HashPmap& other) :
          rank_(other.rank_),
          procs_(other.procs_),
          size_(other.size_),
          seed_(0ul),
          local_()
      { }

    public:

      virtual ~HashPmap() { }

      /// Initialize the hashing seed and local iterator
      virtual void set_seed(madness::hashT seed = 0ul) {
        seed_ = seed;

        for(key_type i = 0ul; i < size_; ++i) {
          if(HashPmap::owner(i) == rank_)
            local_.push_back(i);
        }
      }

      /// Create a copy of this pmap

      /// \return A shared pointer to the new object
      virtual std::shared_ptr<Pmap<key_type> > clone() const {
        return std::shared_ptr<Pmap<key_type> >(new HashPmap(*this));
      }

      /// Key owner

      /// \return The \c ProcessID of the process that owns \c key .
      virtual ProcessID owner(const key_type& key) const {
        madness::hashT seed = seed_;
        madness::hash_combine(seed, key);
        return seed % procs_;
      }


      /// Local size accessor

      /// \return The number of local elements
      virtual std::size_t local_size() const { return local_.size(); }

      /// Local elements

      /// \return \c true when there are no local elements, otherwise \c false .
      virtual bool empty() const { return local_.empty(); }

      /// Begin local element iterator

      /// \return An iterator that points to the beginning of the local element set
      virtual const_iterator begin() const { return local_.begin(); }


      /// End local element iterator

      /// \return An iterator that points to the beginning of the local element set
      virtual const_iterator end() const { return local_.end(); }

      /// Local element vector accessor

      /// \return A const reference to a vector of local elements
      virtual const std::vector<key_type>& local() const { return local_; }

    private:
      const std::size_t rank_;
      const std::size_t procs_;
      const std::size_t size_;
      std::size_t seed_;
      std::vector<key_type> local_;
    }; // class HashPmap

  } // namespace detail
}  // namespace TiledArray


#endif // TILEDARRAY_PMAP_HASH_PMAP_H__INCLUDED