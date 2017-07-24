#include "MantidBeamline/Beamline.h"
#include "MantidBeamline/ComponentInfo.h"
#include "MantidBeamline/DetectorInfo.h"
#include <memory>

namespace Mantid {
namespace Beamline {

namespace {

template <typename T>
std::unique_ptr<T> copyOrNull(const std::unique_ptr<T> &source) {
  return source ? std::unique_ptr<T>(new T(*source))
                : std::unique_ptr<T>(nullptr);
}

bool bindTogether(ComponentInfo *compInfo, DetectorInfo *detInfo) {
  bool canBind = compInfo != nullptr && detInfo != nullptr;
  if (canBind) {
    compInfo->setDetectorInfo(detInfo);
    detInfo->setComponentInfo(compInfo);
  }
  return canBind;
}
}

Beamline::Beamline()
    : m_componentInfo(std::unique_ptr<ComponentInfo>(nullptr)),
      m_detectorInfo(std::unique_ptr<DetectorInfo>(nullptr)) {}

/**
 * Copy constructor.
 *
 * Behavior here is really important to ensure that cow is acheived
 * properly for internals of held ComponentInfos and DetectorInfos
 *
 * @param other
 */
Beamline::Beamline(const Beamline &other)
    : m_empty(other.m_empty),
      m_componentInfo(copyOrNull(other.m_componentInfo)),
      m_detectorInfo(copyOrNull(other.m_detectorInfo)) {
  bindTogether(m_componentInfo.get(), m_detectorInfo.get());
}

Beamline &Beamline::operator=(const Beamline &other) {
  if (&other != this) {
    m_componentInfo = copyOrNull(other.m_componentInfo);
    m_detectorInfo = copyOrNull(other.m_detectorInfo);
    m_empty = other.m_empty;
    bindTogether(m_componentInfo.get(), m_detectorInfo.get());
  }
  return *this;
}

Beamline::Beamline(ComponentInfo &&componentInfo, DetectorInfo &&detectorInfo)
    : m_empty(false), m_componentInfo(std::unique_ptr<ComponentInfo>(
                          new ComponentInfo(componentInfo))),
      m_detectorInfo(
          std::unique_ptr<DetectorInfo>(new DetectorInfo(detectorInfo))) {
  bindTogether(m_componentInfo.get(), m_detectorInfo.get());
}

const ComponentInfo &Beamline::componentInfo() const {
  return *m_componentInfo;
}

const DetectorInfo &Beamline::detectorInfo() const { return *m_detectorInfo; }

ComponentInfo &Beamline::mutableComponentInfo() { return *m_componentInfo; }

DetectorInfo &Beamline::mutableDetectorInfo() { return *m_detectorInfo; }

bool Beamline::empty() const { return m_empty; }
} // namespace Beamline
} // namespace Mantid