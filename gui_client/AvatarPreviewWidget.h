#pragma once


#include "../utils/IncludeWindows.h" // This needs to go first for NOMINMAX.
#include "../opengl/OpenGLEngine.h"
#include "../maths/vec2.h"
#include "../maths/vec3.h"
#include "../utils/Timer.h"
#include "../utils/Reference.h"
#include "../utils/RefCounted.h"
#include <QtCore/QEvent>
#include <QtOpenGL/QGLWidget>
#include <map>


namespace Indigo { class Mesh; }
class TextureServer;
class EnvEmitter;


class AvatarPreviewWidget : public QGLWidget
{
	Q_OBJECT        // must include this if you use Qt signals/slots

public:
	AvatarPreviewWidget(QWidget *parent = 0);
	~AvatarPreviewWidget();

	void addObject(const Reference<GLObject>& object);
	void addOverlayObject(const Reference<OverlayObject>& object);

	void setEnvMat(OpenGLMaterial& mat);
	
protected:

	virtual void initializeGL();
	virtual void resizeGL(int w, int h); // Gets called whenever the widget has been resized (and also when it is shown for the first time because all newly created widgets get a resize event automatically).
	virtual void paintGL(); // Gets called whenever the widget needs to be updated.

	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);
	
	void showEvent(QShowEvent* e);

	void buildMaterial(OpenGLMaterial& mat);

signals:;
	void cameraUpdated();
	void widgetShowSignal();

private:
	QPoint mouse_prev_pos;
	QPoint mouse_move_origin;

	std::string indigo_base_dir;

	float viewport_aspect_ratio;
	int viewport_w, viewport_h;

	float cam_phi, cam_theta, cam_dist;
public:
	TextureServer* texture_server_ptr;
	Reference<OpenGLEngine> opengl_engine;
};
