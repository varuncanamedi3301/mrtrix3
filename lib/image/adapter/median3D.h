/*
    Copyright 2008 Brain Research Institute, Melbourne, Australia

    Written by J-Donald Tournier, 27/06/08.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef __image_adapter_median3D_h__
#define __image_adapter_median3D_h__

#include "image/adapter/voxel.h"

namespace MR
{
  namespace Image
  {
    namespace Adapter 
    {


      template <class VoxelType> 
        class Median3D : public Voxel<VoxelType> {
        public:
          Median3D (const VoxelType& parent) : 
            Voxel<VoxelType> (parent) {
              set_extent (std::vector<int>(1,3));
            }

          Median3D (const VoxelType& parent, const std::vector<int>& extent) : 
            Voxel<VoxelType> (parent) {
              set_extent (extent);
            }

          typedef typename VoxelType::value_type value_type;
          typedef Median3D voxel_type;

          void set_extent (const std::vector<int>& extent) {
            for (size_t i = 0; i < extent.size(); ++i)
              if (! (extent[i] & int(1)))
                throw Exception ("expected odd number for extent");
            if (extent.size() != 1 && extent.size() != 3)
              throw Exception ("unexpected number of elements specified in extent");
            if (extent.size() == 1)
              extent_ = std::vector<int> (3, extent[0]);
            else 
              extent_ = extent;

            v.resize (extent_[0]*extent_[1]*extent_[2]);

            DEBUG ("median3D adapter for image \"" + name() + "\" initialised with extent " + str(extent_));

            for (size_t i = 0; i < 3; ++i)
              extent_[i] = (extent_[i]-1)/2;
          }



          value_type& value () {

            const ssize_t old_pos [3] = { (*this)[0], (*this)[1], (*this)[2] };
            const ssize_t from[3] = {
              (*this)[0] < extent_[0] ? 0 : (*this)[0] - extent_[0], 
              (*this)[1] < extent_[1] ? 0 : (*this)[1] - extent_[1], 
              (*this)[2] < extent_[2] ? 0 : (*this)[2] - extent_[2]
            };
            const ssize_t to[3] = {
              (*this)[0] >= dim(0)-extent_[0] ? dim(0) : (*this)[0]+extent_[0]+1, 
              (*this)[1] >= dim(1)-extent_[1] ? dim(1) : (*this)[1]+extent_[1]+1, 
              (*this)[2] >= dim(2)-extent_[2] ? dim(2) : (*this)[2]+extent_[2]+1
            };

            const size_t n = (to[0]-from[0])*(to[1]-from[1])*(to[2]-from[2]);
            const size_t m = n/2 + 1;

            size_t nc = 0;
            result = -INFINITY;

            for (ssize_t k = from[2]; k < to[2]; ++k) {
              (*this)[2] = k;
              for (ssize_t j = from[1]; j < to[1]; ++j) {
                (*this)[1] = j;
                for (ssize_t i = from[0]; i < to[0]; ++i) {
                  (*this)[0] = i;
                  const value_type val = parent_vox.value();
                  if (nc < m) {
                    v[nc] = val;
                    if (v[nc] > result) result = val;
                    ++nc;
                  }
                  else if (val < result) {
                    size_t x;
                    for (x = 0; v[x] != result; ++x);
                    v[x] = val;
                    result = -INFINITY;
                    for (x = 0; x < m; x++)
                      if (v[x] > result) result = v[x];
                  }
                }
              }
            }

            if ((n+1) & 1) {
              value_type t = result = -INFINITY;
              for (size_t i = 0; i < m; ++i) {
                if (v[i] > result) {
                  t = result;
                  result = v[i];
                }
                else if (v[i] > t) t = v[i];
              }
              result = (result+t)/2.0;
            }

            (*this)[0] = old_pos[0];
            (*this)[1] = old_pos[1];
            (*this)[2] = old_pos[2];

            return result;
          }

          using Voxel<VoxelType>::name;
          using Voxel<VoxelType>::dim;
          using Voxel<VoxelType>::operator[];

        protected:
          using Voxel<VoxelType>::parent_vox;
          std::vector<int> extent_;
          std::vector<value_type> v;
          value_type result;
        };

    }
  }
}


#endif

