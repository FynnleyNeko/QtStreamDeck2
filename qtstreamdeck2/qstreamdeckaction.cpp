#include "qstreamdeckaction.h"

#include <QMetaEnum>
#include <QJsonArray>

#include "qstreamdeckplugin.h"

QStreamDeckAction::~QStreamDeckAction() {
	device_->plugin()->actions_.remove(actionContext_);
}

void QStreamDeckAction::init(QStreamDeckDevice *device, const QStreamDeckEvent &appearEvent) {
	const QJsonObject &json = appearEvent.json;
	const QJsonObject &payload = appearEvent.payload;

	device_ = device;
	actionContext_ = json["context"].toString();
	settings_ = payload["settings"].toObject();

	state_ = payload["state"].toInt();
	isInMultiAction_ = payload["isInMultiAction"].toBool();

	const auto coordJson = payload["coordinates"];
	coordinates_ = QPoint(coordJson["column"].toInt(), coordJson["row"].toInt());

	const auto controllerStr = payload["controller"].toString().toLower();
	controller_ = Controller(QMetaEnum::fromType<Controller>().keyToValue(controllerStr.toStdString().c_str()));

	device->plugin()->actions_.insert(actionContext_, this);
}

void QStreamDeckAction::updatePropertyInspector() {

}

void QStreamDeckAction::onEventReceived(const QStreamDeckEvent &e) {
	using ET = QStreamDeckEvent::EventType;

	// Update action state
	state_ = e.payload["state"].toInt();

	switch(e.eventType) {

		case ET::keyDown:
			isPressed_ = true;
			emit keyDown(e);
			break;

		case ET::keyUp:
			isPressed_ = false;
			emit keyUp(e);
			break;

		case ET::touchTap: {
			const auto tapPosJson = e.payload["tapPos"].toArray();
			emit touchTap(QPoint(tapPosJson[0].toInt(), tapPosJson[1].toInt()), e.payload["hold"].toBool(), e);
			break;
		}

		case ET::dialPress:
			isPressed_ = e.payload["pressed"].toBool();
			if(isPressed_)
				emit dialPressed(e);
			else
				emit dialReleased(e);
			break;

		case ET::dialRotate:
			emit dialRotated(e.payload["ticks"].toInt(), e);
			break;

		default:
			break;

	}
}
