#ifndef __SCENE_H__
#define __SCENE_H__

class Scene
{
public:
	void RootUpdate();
	void RootDraw();

	Scene();
	virtual ~Scene();
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void OnResize(unsigned int width, unsigned int height);
};

#endif // __SCENE_H__