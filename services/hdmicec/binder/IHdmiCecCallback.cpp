#define LOG_NDEBUG 0
#define LOG_TAG "IHdmiCecCallback"

#include "IHdmiCecCallback.h"

namespace android {

class BpHdmiCecCallback: public BpInterface<IHdmiCecCallback> {
public:
    BpHdmiCecCallback(const sp<IBinder> &impl) :
        BpInterface<IHdmiCecCallback> (impl)
    {}

    virtual void notifyCallback(const hdmi_cec_event_t* event)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IHdmiCecCallback::getInterfaceDescriptor());
        data.writeInt32(event->eventType);
        if (((event->eventType & HDMI_EVENT_CEC_MESSAGE) != 0) || ((event->eventType
                & HDMI_EVENT_RECEIVE_MESSAGE) != 0)) {
            data.writeInt32(event->cec.initiator);
            data.writeInt32(event->cec.destination);
            data.writeInt32(event->cec.length);
            data.writeCString((char*) event->cec.body);
        } else if ((event->eventType & HDMI_EVENT_HOT_PLUG) != 0) {
            data.writeInt32(event->hotplug.connected);
            data.writeInt32(event->hotplug.port_id);
        } else if ((event->eventType & HDMI_EVENT_ADD_PHYSICAL_ADDRESS) != 0) {
            data.writeInt32(event->physicalAdd);
        }
        remote()->transact(NOTIFY_CALLBACK, data, &reply);
    }

};

IMPLEMENT_META_INTERFACE(HdmiCecCallback, "droidlogic.IHdmiCecCallback");

//----------------------------------------------------------------------------------------

status_t BnHdmiCecCallback::onTransact(uint32_t code, const Parcel &data, Parcel *reply,
        uint32_t flags) {
    LOGD("code = %d", code);
    switch (code) {
        case NOTIFY_CALLBACK: {
            CHECK_INTERFACE(IHdmiCecCallback, data, reply);
            hdmi_cec_event_t event;
            event.eventType = data.readInt32();
            if (((event.eventType & HDMI_EVENT_CEC_MESSAGE) != 0) || ((event.eventType
                    & HDMI_EVENT_RECEIVE_MESSAGE) != 0)) {
                event.cec.initiator = (cec_logical_address_t) data.readInt32();
                event.cec.destination = (cec_logical_address_t) data.readInt32();
                event.cec.length = data.readInt32();
                const char* body = data.readCString();
                memcpy(event.cec.body, body, event.cec.length);
            } else if ((event.eventType & HDMI_EVENT_HOT_PLUG) != 0) {
                event.hotplug.connected = data.readInt32();
                event.hotplug.port_id = data.readInt32();
            } else if ((event.eventType & HDMI_EVENT_ADD_PHYSICAL_ADDRESS) != 0) {
                event.physicalAdd = data.readInt32();
            }
            notifyCallback(&event);
            return NO_ERROR;
        }
        default: {
            return BBinder::onTransact(code, data, reply, flags);
        }
    }
    return NO_ERROR;
}

};//namespace android
