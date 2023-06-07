#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

int main()
{
	std::ifstream ifs("./data/declus/declus.out");
	if (!ifs.is_open())
		return 0;
	std::string strline;
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);

	osg::Geode* geode1 = new osg::Geode();
	osg::Geode* geode2 = new osg::Geode();

	float x, y, z, p, s, w1, w2;
	
	while (std::getline(ifs,strline))
	{
		std::istringstream  iss(strline.c_str());
		if (iss >> z >> x >> y >> p >> s >> w1 >> w2)
		{
			{
				osg::Sphere* sp = new osg::Sphere(osg::Vec3(x, y, z), w1*50);
				osg::ShapeDrawable* sd = new osg::ShapeDrawable(sp);
				sd->setColor(osg::Vec4(1, 1, 0, 1));
				geode1->addDrawable(sd);
			}
			{
				osg::Sphere* sp = new osg::Sphere(osg::Vec3(x, y, z), w2 * 50);
				osg::ShapeDrawable* sd = new osg::ShapeDrawable(sp);
				sd->setColor(osg::Vec4(0, 1, 0, 1));
				geode2->addDrawable(sd);
			}
		}
	}

	osg::BoundingSphere bs = geode1->getBound();

	osg::MatrixTransform* mt1 = new osg::MatrixTransform(osg::Matrix::translate(bs.center() - osg::Vec3(bs.radius(), 0, 0)));
	osg::MatrixTransform* mt2 = new osg::MatrixTransform(osg::Matrix::translate(bs.center() + osg::Vec3(bs.radius(),0,  0)));
	mt1->addChild(geode1);
	mt2->addChild(geode2);

	osg::Group* root = new osg::Group();
	root->addChild(mt1);
	root->addChild(mt2);

	osgViewer::Viewer viewer;
	viewer.getCamera()->setClearColor(osg::Vec4(0, 0, 0, 1));
	viewer.setUpViewInWindow(300, 300, 800, 600);
	viewer.setSceneData(root);

	return viewer.run();
}