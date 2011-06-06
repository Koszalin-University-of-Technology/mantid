#include "MantidMDEvents/MDBox.h"
#include "MantidMDEvents/MDEvent.h"

namespace Mantid
{
namespace MDEvents
{

  //-----------------------------------------------------------------------------------------------
  /** Empty constructor */
  TMDE(MDBox)::MDBox() :
    IMDBox<MDE, nd>()
  {
  }

  //-----------------------------------------------------------------------------------------------
  /** ctor
   * @param controller :: BoxController that controls how boxes split
   * @param depth :: splitting depth of the new box.
   */
  TMDE(MDBox)::MDBox(BoxController_sptr controller, const size_t depth)
  {
    if (controller->getNDims() != nd)
      throw std::invalid_argument("MDBox::ctor(): controller passed has the wrong number of dimensions.");
    this->m_BoxController = controller;
    this->m_depth = depth;
  }

  //-----------------------------------------------------------------------------------------------
  /** Clear any points contained. */
  TMDE(
  void MDBox)::clear()
  {
    this->m_signal = 0.0;
    this->m_errorSquared = 0.0;
    data.clear();
  }

  //-----------------------------------------------------------------------------------------------
  /** Returns the number of dimensions in this box */
  TMDE(
  size_t MDBox)::getNumDims() const
  {
    return nd;
  }

  //-----------------------------------------------------------------------------------------------
  /** Returns the number of un-split MDBoxes in this box (including all children)
   * @return :: 1 always since this is just a MD Box*/
  TMDE(
  size_t MDBox)::getNumMDBoxes() const
  {
    return 1;
  }

  //-----------------------------------------------------------------------------------------------
  /** Returns the total number of points (events) in this box */
  TMDE(size_t MDBox)::getNPoints() const
  {
    return data.size();
  }

  //-----------------------------------------------------------------------------------------------
  /** Returns a reference to the events vector contained within.
   */
  TMDE(
  std::vector< MDE > & MDBox)::getEvents()
  {
    return data;
  }

  //-----------------------------------------------------------------------------------------------
  /** Allocate and return a vector with a copy of all events contained
   */
  TMDE(
  std::vector< MDE > * MDBox)::getEventsCopy()
  {
    std::vector< MDE > * out = new std::vector<MDE>();
    //Make the copy
    out->insert(out->begin(), data.begin(), data.end());
    return out;
  }

  //-----------------------------------------------------------------------------------------------
  /** Refresh the cache.
   * For MDBox, the signal/error is tracked on adding, so
   * this just calculates the centroid.
   */
  TMDE(
  void MDBox)::refreshCache(Kernel::ThreadScheduler * /*ts*/)
  {
  }


  //-----------------------------------------------------------------------------------------------
  /** Calculate and ccache the centroid of this box.
   */
  TMDE(
  void MDBox)::refreshCentroid(Kernel::ThreadScheduler * /*ts*/)
  {
    for (size_t d=0; d<nd; d++)
      this->m_centroid[d] = 0;

    // Signal was calculated before (when adding)
    // Keep 0.0 if the signal is null. This avoids dividing by 0.0
    if (this->m_signal == 0) return;

    typename std::vector<MDE>::const_iterator it_end = data.end();
    for(typename std::vector<MDE>::const_iterator it = data.begin(); it != it_end; it++)
    {
      const MDE & event = *it;
      double signal = event.getSignal();
      for (size_t d=0; d<nd; d++)
      {
        // Total up the coordinate weighted by the signal.
        this->m_centroid[d] += event.getCenter(d) * signal;
      }
    }

    // Normalize by the total signal
    for (size_t d=0; d<nd; d++)
      this->m_centroid[d] /= this->m_signal;
  }


  //-----------------------------------------------------------------------------------------------
  /** Calculate the statistics for each dimension of this MDBox, using
   * all the contained events
   * @param stats :: nd-sized fixed array of MDDimensionStats, reset to 0.0 before!
   */
  TMDE(
  void MDBox)::calculateDimensionStats(MDDimensionStats * stats) const
  {
    typename std::vector<MDE>::const_iterator it_end = data.end();
    for(typename std::vector<MDE>::const_iterator it = data.begin(); it != it_end; it++)
    {
      const MDE & event = *it;
      for (size_t d=0; d<nd; d++)
      {
        stats[d].addPoint( event.getCenter(d) );
      }
    }
  }



  //-----------------------------------------------------------------------------------------------
  /** Add a MDEvent to the box.
   * @param event :: reference to a MDEvent to add.
   * */
  TMDE(
  void MDBox)::addEvent( const MDE & event)
  {
    dataMutex.lock();
    this->data.push_back(event);

    // Keep the running total of signal and error
    double signal = event.getSignal();
    this->m_signal += signal;
    this->m_errorSquared += event.getErrorSquared();

#ifdef MDBOX_TRACKCENTROID_WHENADDING
    // Running total of the centroid
    for (size_t d=0; d<nd; d++)
      this->m_centroid[d] += event.getCenter(d) * signal;
#endif

    dataMutex.unlock();
  }

  //-----------------------------------------------------------------------------------------------
  /** Add several events. No bounds checking is made!
   *
   * @param events :: vector of events to be copied.
   * @param start_at :: index to start at in vector
   * @param stop_at :: index to stop at in vector (exclusive)
   * @return the number of events that were rejected (because of being out of bounds)
   */
  TMDE(
  size_t MDBox)::addEvents(const std::vector<MDE> & events, const size_t start_at, const size_t stop_at)
  {
    dataMutex.lock();
    typename std::vector<MDE>::const_iterator start = events.begin()+start_at;
    typename std::vector<MDE>::const_iterator end = events.begin()+stop_at;
    // Copy all the events
    this->data.insert(this->data.end(), start, end);

    //Running total of signal/error
    for(typename std::vector<MDE>::const_iterator it = start; it != end; it++)
    {
      double signal = it->getSignal();
      this->m_signal += signal;
      this->m_errorSquared += it->getErrorSquared();

#ifdef MDBOX_TRACKCENTROID_WHENADDING
    // Running total of the centroid
    for (size_t d=0; d<nd; d++)
      this->m_centroid[d] += it->getCenter(d) * signal;
#endif
    }

    dataMutex.unlock();
    return 0;
  }

  //-----------------------------------------------------------------------------------------------
  /** Perform centerpoint binning of events.
   * @param bin :: MDBin object giving the limits of events to accept.
   * @param fullyContained :: optional bool array sized [nd] of which dimensions are known to be fully contained (for MDSplitBox)
   */
  TMDE(
  void MDBox)::centerpointBin(MDBin<MDE,nd> & bin, bool * fullyContained) const
  {
    if (fullyContained)
    {
      // For MDSplitBox, check if we've already found that all dimensions are fully contained
      size_t d;
      for (d=0; d<nd; ++d)
      {
        if (!fullyContained[d]) break;
      }
      if (d == nd)
      {
//        std::cout << "MDBox at depth " << this->m_depth << " was fully contained in bin " << bin.m_index << ".\n";
        // All dimensions are fully contained, so just return the cached total signal instead of counting.
        bin.m_signal += this->m_signal;
        bin.m_errorSquared += this->m_errorSquared;
        return;
      }
    }

    typename std::vector<MDE>::const_iterator it = data.begin();
    typename std::vector<MDE>::const_iterator it_end = data.end();

    // For each MDEvent
    for (; it != it_end; ++it)
    {
      // Go through each dimension
      size_t d;
      for (d=0; d<nd; ++d)
      {
        // Check that the value is within the bounds given. (Rotation is for later)
        coord_t x = it->getCenter(d);
        if (x < bin.m_min[d])
          break;
        if (x >= bin.m_max[d])
          break;
      }
      // If the loop reached the end, then it was all within bounds.
      if (d == nd)
      {
        // Accumulate error and signal
        bin.m_signal += it->getSignal();
        bin.m_errorSquared += it->getErrorSquared();
      }
    }
  }


  /** Integrate the signal within a sphere; for example, to perform single-crystal
   * peak integration.
   * The CoordTransform object could be used for more complex shapes, e.g. "lentil" integration, as long
   * as it reduces the dimensions to a single value.
   *
   * @param radiusTransform :: nd-to-1 coordinate transformation that converts from these
   *        dimensions to the distance (squared) from the center of the sphere.
   * @param radiusSquared :: radius^2 below which to integrate
   * @param[out] signal :: set to the integrated signal
   * @param[out] errorSquared :: set to the integrated squared error.
   */
  TMDE(
  void MDBox)::integrateSphere(CoordTransform & radiusTransform, const coord_t radiusSquared, signal_t & signal, signal_t & errorSquared) const
  {
    typename std::vector<MDE>::const_iterator it = data.begin();
    typename std::vector<MDE>::const_iterator it_end = data.end();

    // For each MDEvent
    for (; it != it_end; ++it)
    {
      coord_t out[nd];
      radiusTransform.apply(it->getCenter(), out);
      if (out[0] < radiusSquared)
      {
        signal += it->getSignal();
        errorSquared += it->getErrorSquared();
      }
    }
  }

  /** Find the centroid of all events contained within by doing a weighted average
   * of their coordinates.
   *
   * @param radiusTransform :: nd-to-1 coordinate transformation that converts from these
   *        dimensions to the distance (squared) from the center of the sphere.
   * @param radiusSquared :: radius^2 below which to integrate
   * @param[out] centroid :: array of size [nd]; its centroid will be added
   * @param[out] signal :: set to the integrated signal
   */
  TMDE(
  void MDBox)::centroidSphere(CoordTransform & radiusTransform, const coord_t radiusSquared, coord_t * centroid, signal_t & signal) const
  {
    typename std::vector<MDE>::const_iterator it = data.begin();
    typename std::vector<MDE>::const_iterator it_end = data.end();

    // For each MDEvent
    for (; it != it_end; ++it)
    {
      coord_t out[nd];
      radiusTransform.apply(it->getCenter(), out);
      if (out[0] < radiusSquared)
      {
        double eventSignal = it->getSignal();
        signal += eventSignal;
        for (size_t d=0; d<nd; d++)
          centroid[d] += it->getCenter(d) * eventSignal;
      }
    }
  }


}//namespace MDEvents

}//namespace Mantid

