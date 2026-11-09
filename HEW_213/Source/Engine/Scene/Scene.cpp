#include "Scene.h"

Scene::Scene()
{
}
Scene::~Scene()
{
}

void Scene::RootUpdate()
{
	Update();
}
void Scene::RootDraw()
{
	Draw();
}

void Scene::OnResize(unsigned int width, unsigned int height)
{
}