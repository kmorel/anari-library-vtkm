// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
#include "scene/World.h"
#include "../VTKmTypes.h"

namespace vtkm_device {

struct PixelSample
{
  float4 color;
  float depth;
  uint32_t primId{~0u};
  uint32_t objId{~0u};
  uint32_t instId{~0u};
};

struct Renderer : public Object
{
  Renderer(VTKmDeviceGlobalState *s);
  ~Renderer() override;
  static Renderer *createInstance(
      std::string_view subtype, VTKmDeviceGlobalState *d);
  void commit() override;

  PixelSample renderSample(const float2 &screen, Ray ray, const World &w) const;
  float4 shadeRay(const float2 &screen,
                  const Ray &ray,
                  const VolumeRay &vray,
                  const World &w) const;
  float4 background() const;

 private:
  //float4 m_bgColor{float3(0.f), 1.f};
  float4 m_bgColor{0.f, 0.f, 0.f, 1.f};
};

} // namespace vtkm_device

VTKM_ANARI_TYPEFOR_SPECIALIZATION(vtkm_device::Renderer *, ANARI_RENDERER);
