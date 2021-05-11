/*=====================================================================
AnimatedTextureManager.h
------------------------
Copyright Glare Technologies Limited 2021 -
=====================================================================*/
#pragma once


#include "../opengl/OpenGLEngine.h"
#include "../opengl/WGL.h"
#include "../video/VideoReader.h"
#include <map>


class SubstrataVideoReaderCallback;
class MainWindow;
class WorldObject;
class QMediaPlayer;
class SubVideoSurface;
struct CreateVidReaderTask;

struct ID3D11Device;
struct IMFDXGIDeviceManager;


struct OpenGLAndD3DTex
{
	OpenGLTextureRef opengl_tex;
#ifdef _WIN32
	HANDLE interop_handle;
#endif
};


struct AnimatedTexData : public RefCounted
{ 
	AnimatedTexData();
	~AnimatedTexData();

	void shutdown(
#ifdef _WIN32
		WGL& wgl_funcs, HANDLE interop_device_handle
#endif
	);

	std::map<void*, OpenGLAndD3DTex> opengl_tex_for_d3d_tex;

	Reference<VideoReader> video_reader;
	int latest_tex_index;
	double in_anim_time; // Current time along timeline of video.  Doesn't change if video is paused.

	SubVideoSurface* video_surface;
	QMediaPlayer* media_player; // TEMP

	ThreadSafeQueue<FrameInfoRef> frameinfos;

	CircularBuffer<FrameInfoRef> vid_frame_queue;

	FrameInfoRef current_video_frame;
	FrameInfoRef current_audio_frame;
	//FrameInfoRef next_frame;

	Reference<CreateVidReaderTask> create_vid_reader_task;

	SubstrataVideoReaderCallback* callback;

	OpenGLTextureRef textures[2];


	Reference<TextureData> texdata;
	int cur_frame_i; // -1 = reached EOS

	bool encounted_error;
#ifdef _WIN32
	HANDLE locked_interop_tex_ob;
#endif
};

struct AnimatedTexObData // : public RefCounted
{
	std::vector<Reference<AnimatedTexData>> mat_animtexdata; // size() == ob.material.size()

	void process(MainWindow* main_window, OpenGLEngine* opengl_engine, WorldObject* ob, double anim_time, double dt);

	std::vector<float> temp_buf;//TEMP
};



//class AnimatedTextureManager
//{
//public:
//
//};