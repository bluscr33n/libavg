//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "WrapHelper.h"

#include "../player/KeyEvent.h"
#include "../player/MouseEvent.h"
#include "../player/TouchEvent.h"
#include "../player/TangibleEvent.h"
#include "../player/Contact.h"
#include "../player/Publisher.h"
#include "../player/InputDevice.h"

#include <SDL2/SDL_events.h>
#include <boost/shared_ptr.hpp>
#include <string>

using namespace boost::python;
using namespace avg;
using namespace std;


class InputDeviceWrapper : public InputDevice, public wrapper<InputDevice>
{
    public:
        InputDeviceWrapper(const std::string& name,
                const DivNodePtr& pEventReceiverNode=DivNodePtr())
            : InputDevice(name, pEventReceiverNode)
        {
        }

        InputDeviceWrapper(const InputDevice& inputDevice)
            : InputDevice(inputDevice)
        {
        }

        virtual void start() 
        {
            override startMethod = this->get_override("start");
            if (startMethod) {
                startMethod();
            }
            InputDevice::start();
        }

        void default_start() 
        {
            return this->InputDevice::start();
        }

        virtual std::vector<EventPtr> pollEvents() 
        {
            return this->get_override("pollEvents")();
        }

};

void export_event()
{
    boost::python::to_python_converter<vector<TouchEventPtr>, 
        to_tuple<vector<TouchEventPtr> > >();
    boost::python::to_python_converter<vector<CursorEventPtr>, 
        to_tuple<vector<CursorEventPtr> > >();

    from_python_sequence<vector<EventPtr>, variable_capacity_policy>();

    enum_<int>("KeyModifier")
        .value("KEYMOD_NONE", KMOD_NONE)
        .value("KEYMOD_LSHIFT", KMOD_LSHIFT)
        .value("KEYMOD_RSHIFT", KMOD_RSHIFT)
        .value("KEYMOD_LCTRL", KMOD_LCTRL)
        .value("KEYMOD_RCTRL", KMOD_RCTRL)
        .value("KEYMOD_LALT", KMOD_LALT)
        .value("KEYMOD_RALT", KMOD_RALT)
        .value("KEYMOD_LGUI", KMOD_RALT)
        .value("KEYMOD_RGUI", KMOD_RALT)
        .value("KEYMOD_NUM", KMOD_NUM)
        .value("KEYMOD_CAPS", KMOD_CAPS)
        .value("KEYMOD_MODE", KMOD_MODE)
        .value("KEYMOD_CTRL", KMOD_CTRL)
        .value("KEYMOD_SHIFT", KMOD_SHIFT)
        .value("KEYMOD_ALT", KMOD_ALT)
        .value("KEYMOD_GUI", KMOD_GUI)
        .export_values()
    ;

    scope mainScope;

    scope eventScope = class_<Event, boost::noncopyable>("Event", init<Event::Type,
            Event::Source, optional<int> >())
        .add_property("type", &Event::getType)
        .add_property("source", &Event::getSource)
        .add_property("when", &Event::getWhen)
        .add_property("inputdevice", &Event::getInputDevice)
        .add_property("inputdevicename", make_function(&Event::getInputDeviceName,
                return_value_policy<copy_const_reference>()))
    ;

    enum_<Event::Type>("Type")
        .value("KEY_UP", Event::KEY_UP)
        .value("KEY_DOWN", Event::KEY_DOWN)
        .value("CURSOR_MOTION", Event::CURSOR_MOTION)
        .value("CURSOR_UP", Event::CURSOR_UP)
        .value("CURSOR_DOWN", Event::CURSOR_DOWN)
        .value("CURSOR_OVER", Event::CURSOR_OVER)
        .value("CURSOR_OUT", Event::CURSOR_OUT)
        .value("CUSTOM_EVENT", Event::CUSTOM_EVENT)
        .export_values()
    ;

    enum_<CursorEvent::Source>("Source")
        .value("MOUSE", CursorEvent::MOUSE)
        .value("TOUCH", CursorEvent::TOUCH)
        .value("TRACK", CursorEvent::TRACK)
        .value("TANGIBLE", CursorEvent::TANGIBLE)
        .value("CUSTOM", Event::CUSTOM)
        .value("NONE", Event::NONE)
        .export_values()
    ;

    scope oldScope1(mainScope);

    class_<CursorEvent, boost::shared_ptr<CursorEvent>, bases<Event> >("CursorEvent", 
            no_init)
        .add_property("source", &CursorEvent::getSource)
        .add_property("pos", &CursorEvent::getPos)
        .add_property("x", &CursorEvent::getXPosition)
        .add_property("y", &CursorEvent::getYPosition)
        .add_property("cursorid", &CursorEvent::getCursorID, &CursorEvent::setCursorID)
        .add_property("userid", &TouchEvent::getUserID)
        .add_property("jointid", &TouchEvent::getJointID)
        .add_property("node", &CursorEvent::getNode)
        .add_property("speed", make_function(&CursorEvent::getSpeed,
                return_value_policy<copy_const_reference>()))
        .add_property("contact", &CursorEvent::getContact)
    ;

    class_<KeyEvent, bases<Event> >("KeyEvent", no_init)
        .add_property("scancode", &KeyEvent::getScanCode)
        .add_property("keycode", &KeyEvent::getKeyCode)
        .add_property("keystring", make_function(&KeyEvent::getKeyString, 
                return_value_policy<copy_const_reference>()))
        .add_property("modifiers", &KeyEvent::getModifiers)
    ;    
    
    class_<MouseEvent, bases<CursorEvent> >("MouseEvent",
            init<Event::Type, bool, bool, bool, const IntPoint&, int,
                 optional<const glm::vec2&, int> >())
        .add_property("leftbuttonstate", &MouseEvent::getLeftButtonState)
        .add_property("middlebuttonstate", &MouseEvent::getMiddleButtonState)
        .add_property("rightbuttonstate", &MouseEvent::getRightButtonState)
        .add_property("button", &MouseEvent::getButton)
    ;

    class_<TouchEvent, bases<CursorEvent> >("TouchEvent", init<int, Event::Type,
            const IntPoint&, Event::Source, optional<const glm::vec2&> >())
        .add_property("area", &TouchEvent::getArea)
        .add_property("orientation", &TouchEvent::getOrientation)
        .add_property("eccentricity", &TouchEvent::getEccentricity)
        .add_property("center", make_function(&TouchEvent::getCenter,
                return_value_policy<copy_const_reference>()))
        .add_property("majoraxis", make_function(&TouchEvent::getMajorAxis,
                return_value_policy<copy_const_reference>()))
        .add_property("minoraxis", make_function(&TouchEvent::getMinorAxis,
                return_value_policy<copy_const_reference>()))
        .add_property("handorientation", &TouchEvent::getHandOrientation)
        .def("getRelatedEvents", &TouchEvent::getRelatedEvents)
        ;

    class_<TangibleEvent, bases<CursorEvent> >("TangibleEvent", init<int, int, 
            Event::Type, const IntPoint&, const glm::vec2&, float>())
        .add_property("markerid", &TangibleEvent::getMarkerID)
        .add_property("orientation", &TangibleEvent::getOrientation)
        ;

    object contactClass = class_<Contact, boost::shared_ptr<Contact>, bases<Publisher> >
            ("Contact", no_init)
        .add_property("id", &Contact::getID)
        .add_property("age", &Contact::getAge)
        .add_property("distancefromstart", &Contact::getDistanceFromStart)
        .add_property("motionangle", &Contact::getMotionAngle)
        .add_property("motionvec", &Contact::getMotionVec)
        .add_property("distancetravelled", &Contact::getDistanceTravelled)
        .add_property("events", &Contact::getEvents)
        .def("connectListener", &Contact::connectListener)
        .def("disconnectListener", &Contact::disconnectListener)
        ;
    exportMessages(contactClass, "Contact");

    class_<InputDevicePtr>("InputDevice")
    ;

    class_< InputDeviceWrapper,
            boost::shared_ptr<InputDeviceWrapper>,
            boost::noncopyable
    >("InputDevice", init<const std::string&, optional<const DivNodePtr&> >())
        .def("start", &InputDevice::start, &InputDeviceWrapper::default_start)
        .def("pollEvents", pure_virtual(&InputDevice::pollEvents))
        .add_property("name",
                      make_function(&InputDevice::getName,
                                    return_value_policy<copy_const_reference>()))
        .add_property("eventreceivernode",
                      make_function(&InputDevice::getEventReceiverNode,
                                    return_value_policy<copy_const_reference>()))
    ;
}
