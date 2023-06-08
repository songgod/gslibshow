#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/TransferFunction>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

bool getInt(const std::string& str, int& value)
{
	std::istringstream iss(str.c_str());
	if (!(iss >> value))
		return false;
	return true;
}

osg::TransferFunction1D tf;


osg::Geode* createSource()
{
	std::ifstream ifs("./data/kt3d/cluster.dat");
	if (!ifs.is_open())
		return nullptr;
	std::string strline;
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);

	osg::Geode* geodepoints = new osg::Geode();

	float x, y, z,p;

	while (std::getline(ifs, strline))
	{
		std::istringstream  iss(strline.c_str());
		if (iss >>  x >> y >>z>>p)
		{
			osg::Sphere* sp = new osg::Sphere(osg::Vec3(x, y, z), 10);
			osg::ShapeDrawable* sd = new osg::ShapeDrawable(sp);
			sd->setColor(tf.getColor(p));
			geodepoints->addDrawable(sd);
		}
	}

	return geodepoints;
}

osg::Geode* createResult()
{
	std::ifstream ifs("./data/kt3d/cluster.out");
	if (!ifs.is_open())
		return nullptr;

	std::string strline;
	int i = 0;
	std::getline(ifs, strline);
	if (strline.empty())
		return nullptr;
	std::getline(ifs, strline);
	if (!getInt(strline, i) || i != 7)
		return nullptr;
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float True = 0.f;
	float Estimate = 0.f;
	float EstimationVariance = 0.f;
	float Error = 0.f;
	i = 0;

	osg::Geode* geodepoints = new osg::Geode();
	osg::Geometry* geom = new osg::Geometry();
	osg::Vec3Array* vetexies = new osg::Vec3Array();
	osg::Vec4Array* colors = new osg::Vec4Array();
	std::vector<float> values;
	while (std::getline(ifs, strline))
	{
		std::istringstream oss(strline);
		oss >> x >> y >> z >> True >> Estimate >> EstimationVariance >> Error;
		values.push_back(Estimate);
		vetexies->push_back(osg::Vec3(x, y, z));
		i++;
	}

	float fMin = *std::min_element(values.begin(), values.end());
	float fMax = *std::max_element(values.begin(), values.end());

	tf.setColor(fMin, osg::Vec4(1, 0, 0, 1));
	tf.setColor((fMin + fMax) / 2, osg::Vec4(0, 1, 0, 1));
	tf.setColor(fMax, osg::Vec4(0, 0, 1, 1));

	for (size_t i = 0; i < values.size(); i++)
	{
		colors->push_back(tf.getColor(values[i]));
	}

	geom->setVertexArray(vetexies);
	geom->setColorArray(colors);
	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::DrawArrays::POINTS,0,vetexies->size()));
	geom->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	geodepoints->addDrawable(geom);

	return geodepoints;
}

int main()
{
	
	osg::Node* result = createResult();
	osg::Node* source = createSource();
	if (!source && !result)
		return 0;

	osg::Group* root = new osg::Group();
	if(source) root->addChild(source);
	if(result) root->addChild(result);

	osgViewer::Viewer viewer;
	viewer.getCamera()->setClearColor(osg::Vec4(0, 0, 0, 1));
	viewer.setUpViewInWindow(300, 300, 800, 600);
	viewer.setSceneData(root);

	return viewer.run();
}