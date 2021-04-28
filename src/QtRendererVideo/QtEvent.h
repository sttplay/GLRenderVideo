#pragma once
#include <QEvent>
class QtEvent :public QEvent
{
public:
	enum Type
	{
		GL_Renderer = QEvent::User + 1,
	};
	QtEvent(Type type) :QEvent(QEvent::Type(type)) {};
};