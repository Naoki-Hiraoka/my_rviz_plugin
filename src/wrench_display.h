//from https://github.com/ros-visualization/rviz/blob/kinetic-devel/src/rviz/default_plugin/wrench_display.h

/*
  Public Domain Dedication

Copyright-Only Dedication (based on United States law) or Public Domain
Certification

The person or persons who have associated work with this document (the
"Dedicator" or "Certifier") hereby either (a) certifies that, to the best
of his knowledge, the work of authorship identified is in the public
domain of the country from which the work is published, or (b)
hereby dedicates whatever copyright the dedicators holds in the work
of authorship identified below (the "Work") to the public domain. A
certifier, moreover, dedicates any copyright interest he may have in
the associated work, and for these purposes, is described as a
"dedicator" below.

A certifier has taken reasonable steps to verify the copyright
status of this work. Certifier recognizes that his good faith efforts
may not shield him from liability if in fact the work certified is not
in the public domain.

Dedicator makes this dedication for the benefit of the public at
large and to the detriment of the Dedicator's heirs and successors.
Dedicator intends this dedication to be an overt act of relinquishment
in perpetuity of all present and future rights under copyright law,
whether vested or contingent, in the Work. Dedicator understands that
such relinquishment of all rights includes the relinquishment of all
rights to enforce (by lawsuit or otherwise) those copyrights in the
Work.

Dedicator recognizes that, once placed in the public domain, the Work
may be freely reproduced, distributed, transmitted, used, modified,
built upon, or otherwise exploited by anyone for any purpose, commercial
or non-commercial, and in any way, including by methods that have not
yet been invented or conceived.


 */

#ifndef MY_RVIZ_PLUGIN_WRENCHSTAMPED_DISPLAY_H
#define MY_RVIZ_PLUGIN_WRENCHSTAMPED_DISPLAY_H

#ifndef Q_MOC_RUN
#include <boost/circular_buffer.hpp>
#endif

#include <geometry_msgs/WrenchStamped.h>
#include <rviz/message_filter_display.h>

namespace Ogre
{
class SceneNode;
}

namespace rviz
{
class ColorProperty;
class ROSTopicStringProperty;
class FloatProperty;
class IntProperty;
}

namespace rviz
{
  class WrenchVisual;
}

namespace my_rviz_plugin
{

class WrenchStampedDisplay: public rviz::MessageFilterDisplay<geometry_msgs::WrenchStamped>
{
    Q_OBJECT
public:
    // Constructor.  pluginlib::ClassLoader creates instances by calling
    // the default constructor, so make sure you have one.
    WrenchStampedDisplay();
    virtual ~WrenchStampedDisplay();

protected:
    // Overrides of public virtual functions from the Display class.
    virtual void onInitialize();
    virtual void reset();

private Q_SLOTS:
    // Helper function to apply color and alpha to all visuals.
    void updateColorAndAlpha();
    void updateHistoryLength();

private:
  // Function to handle an incoming ROS message.
  void processMessage( const geometry_msgs::WrenchStamped::ConstPtr& msg );
  
  // Storage for the list of visuals par each joint intem
  // Storage for the list of visuals.  It is a circular buffer where
  // data gets popped from the front (oldest) and pushed to the back (newest)
  boost::circular_buffer<boost::shared_ptr<rviz::WrenchVisual> > visuals_;

  // Property objects for user-editable properties.
  rviz::ColorProperty *force_color_property_, *torque_color_property_;
  rviz::FloatProperty *alpha_property_, *force_scale_property_, *torque_scale_property_, *width_property_;
  rviz::IntProperty *history_length_property_;
};
} // end namespace rviz_plugin_tutorials

#endif // MY_RVIZ_PLUGIN_WRENCHSTAMPED_DISPLAY_H
