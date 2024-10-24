// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "World.h"

namespace vtkm_device {

World::World(VTKmDeviceGlobalState *s) : Object(ANARI_WORLD, s)
{
  s->objectCounts.worlds++;

  m_zeroGroup = new Group(s);
  m_zeroInstance = new Instance(s);
  m_zeroInstance->setParamDirect("group", m_zeroGroup.ptr);

  // never any public ref to these objects
  m_zeroGroup->refDec(helium::RefType::PUBLIC);
  m_zeroInstance->refDec(helium::RefType::PUBLIC);
}

World::~World()
{
  cleanup();
  deviceState()->objectCounts.worlds--;
}

bool World::getProperty(
    const std::string_view &name, ANARIDataType type, void *ptr, uint32_t flags)
{
  return Object::getProperty(name, type, ptr, flags);
}

void World::commit()
{
  cleanup();

  m_zeroSurfaceData = getParamObject<ObjectArray>("surface");
  m_zeroVolumeData = getParamObject<ObjectArray>("volume");

  m_addZeroInstance = m_zeroSurfaceData || m_zeroVolumeData;
  if (m_addZeroInstance) {
    reportMessage(
        ANARI_SEVERITY_DEBUG, "vtkm_device::World will add zero instance");
  }

  if (m_zeroSurfaceData) {
    reportMessage(ANARI_SEVERITY_DEBUG,
        "vtkm_device::World found %zu surfaces in zero instance",
        m_zeroSurfaceData->size());
    m_zeroGroup->setParamDirect("surface", getParamDirect("surface"));
  } else
    m_zeroGroup->removeParam("surface");

  if (m_zeroVolumeData) {
    reportMessage(ANARI_SEVERITY_DEBUG,
        "vtkm_device::World found %zu volumes in zero instance",
        m_zeroVolumeData->size());
    m_zeroGroup->setParamDirect("volume", getParamDirect("volume"));
  } else
    m_zeroGroup->removeParam("volume");

  m_zeroInstance->setParam("id", getParam<uint32_t>("id", ~0u));

  m_zeroGroup->commit();
  m_zeroInstance->commit();

  m_instanceData = getParamObject<ObjectArray>("instance");

  m_instances.clear();

  if (m_instanceData) {
    m_instanceData->removeAppendedHandles();
    if (m_addZeroInstance)
      m_instanceData->appendHandle(m_zeroInstance.ptr);
    std::for_each(m_instanceData->handlesBegin(),
        m_instanceData->handlesEnd(),
        [&](auto *o) {
          if (o && o->isValid())
            m_instances.push_back((Instance *)o);
        });
  } else if (m_addZeroInstance)
    m_instances.push_back(m_zeroInstance.ptr);

  if (m_instanceData)
    m_instanceData->addChangeObserver(this);
  if (m_zeroSurfaceData)
    m_zeroSurfaceData->addChangeObserver(this);
}

const std::vector<Instance *> &World::instances() const
{
  return m_instances;
}

void World::intersectVolumes(VolumeRay &ray) const
{
  const auto &insts = instances();
  for (uint32_t i = 0; i < insts.size(); i++)
  {
    const auto *inst = insts[i];
    inst->group()->intersectVolumes(ray);
    if (ray.volume)
      ray.instID = i;
  }
}


void World::cleanup()
{
  if (m_instanceData)
    m_instanceData->removeChangeObserver(this);
  if (m_zeroSurfaceData)
    m_zeroSurfaceData->removeChangeObserver(this);
}

} // namespace vtkm_device

VTKM_ANARI_TYPEFOR_DEFINITION(vtkm_device::World *);
