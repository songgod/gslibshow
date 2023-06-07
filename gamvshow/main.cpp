#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/LineStipple>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>
#include <osg/io_utils>

osg::Node* createPoints()
{
	std::ifstream ifs("./data/gamv/cluster.dat");
	if (!ifs.is_open())
		return nullptr;
	std::string strline;
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);

	osg::Geode* geodepoints = new osg::Geode();

	float x, y, p;

	while (std::getline(ifs, strline))
	{
		std::istringstream  iss(strline.c_str());
		if (iss >>p>> x >> y)
		{
			osg::Sphere* sp = new osg::Sphere(osg::Vec3(x, y, 0), 10);
			osg::ShapeDrawable* sd = new osg::ShapeDrawable(sp);
			sd->setColor(osg::Vec4(1, 1, 0, 1));
			geodepoints->addDrawable(sd);
		}
	}

	return geodepoints;
}

osg::Geometry* createLine(const std::vector<osg::Vec3>& points, bool bStrip=false, const osg::Vec4& color=osg::Vec4(1,1,0,1))
{
	osg::Geometry* geom = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vex = new osg::Vec3Array;
	for (size_t i = 0; i < points.size(); i++)
	{
		vex->push_back(points[i]);
	}
	geom->setVertexArray(vex);
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	colors->push_back(color);
	geom->setColorArray(colors);
	geom->setColorBinding(osg::Geometry::BIND_OVERALL);
	osg::ref_ptr<osg::PrimitiveSet> primitiveSet = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vex->size());
	geom->addPrimitiveSet(primitiveSet);

	if (bStrip)
	{
		osg::LineStipple* linestipple = new osg::LineStipple;
		linestipple->setFactor(1);
		linestipple->setPattern(0xf0f0);
		geom->getOrCreateStateSet()->setAttributeAndModes(linestipple, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
	}

	return geom;
}

template<typename vec2Type>
double ParallelogramAreaT(const vec2Type& vec1, const vec2Type& vec2)
{
	return vec1.x() * vec2.y() - vec1.y() * vec2.x();
}

template<typename vec2Type>
bool LineIntersectT(const vec2Type& cutlinestart, const vec2Type& cutlineend, const vec2Type& linestart, const vec2Type& lineend, vec2Type& cutpoint)
{
	double d, d1, d2, s1, s2;
	if (osg::minimum(cutlinestart.x(), cutlineend.x()) >= osg::maximum(linestart.x(), lineend.x()))
	{
		return false;
	}
	if (osg::maximum(cutlinestart.x(), cutlineend.x()) <= osg::minimum(linestart.x(), lineend.x()))
	{
		return false;
	}
	if (osg::minimum(cutlinestart.y(), cutlineend.y()) >= osg::maximum(linestart.y(), lineend.y()))
	{
		return false;
	}
	if (osg::maximum(cutlinestart.y(), cutlineend.y()) <= osg::minimum(linestart.y(), lineend.y()))
	{
		return false;
	}
	s1 = (cutlinestart - cutlineend).length();
	if (s1 <= 0)
	{
		return false;
	}
	s2 = (linestart - lineend).length();
	if (s2 <= 0)
	{
		return false;
	}
	d1 = ParallelogramAreaT<vec2Type>((cutlinestart - linestart), (lineend - linestart));
	d2 = ParallelogramAreaT<vec2Type>((cutlineend - linestart), (lineend - linestart));
	d = d1 * d2;
	if (d > 0)
	{
		return false;
	}
	d1 = ParallelogramAreaT<vec2Type>((linestart - cutlinestart), (cutlineend - cutlinestart));
	d2 = ParallelogramAreaT<vec2Type>((lineend - cutlinestart), (cutlineend - cutlinestart));
	d = d1 * d2;
	if (d > 0)
	{
		return false;
	}
	d1 = fabs(d1);
	d2 = fabs(d2);
	d = d1 + d2;
	if (d1 <= 0 || d2 <= 0)
	{
		cutpoint = linestart;
		return true;
	}
	d1 /= d;
	cutpoint.x() = linestart.x() + (lineend.x() - linestart.x()) * d1;
	cutpoint.y() = linestart.y() + (lineend.y() - linestart.y()) * d1;
	return true;
}

osg::Vec3 startpos;
osg::Vec3 normaldir;
float lagdis;

osg::Node* createDir(const osg::Vec3d& center)
{
	std::ifstream ifs("./data/gamv/gamv.par");
	if (!ifs.is_open())
		return nullptr;
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
	std::getline(ifs, strline);
	std::istringstream isslag(strline.c_str());
	int nlags = 0;
	if (!(isslag >> nlags))
		return nullptr;
	std::getline(ifs, strline);
	std::istringstream isslagdis(strline.c_str());
	if (!(isslagdis >> lagdis))
		return nullptr;
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::getline(ifs, strline);
	std::istringstream issdir(strline.c_str());
	float azm = 0.f, atol = 0.f, bandh = 0.f, dip = 0.f, dtol = 0.f, bandv = 0.f;
	if (!(issdir >> azm >> atol >> bandh >> dip >> dtol >> bandv))
		return nullptr;

	azm *= -1;


	osg::Geode* res = new osg::Geode();

	float laglen = lagdis * nlags;
	osg::Matrix mt = osg::Matrix::rotate(osg::DegreesToRadians(azm), osg::Vec3(0, 0, 1)) * osg::Matrix::translate(center);

	startpos = osg::Vec3(0, -laglen / 2, 0) * mt;
	osg::Vec3 endpos = osg::Vec3(0, laglen / 2, 0) * mt;

	osg::Vec3 normal = endpos - startpos;
	normal.normalize();
	normaldir = normal;
	normal = normal ^ osg::Vec3(0, 0, 1);
	osg::Vec3 startposr = startpos + normal * bandh;
	osg::Vec3 endposr = endpos + normal * bandh;
	osg::Vec3 startposl = startpos - normal * bandh;
	osg::Vec3 endposl = endpos - normal * bandh;

	osg::Matrix mtl = osg::Matrix::rotate(osg::DegreesToRadians(azm + atol / 2), osg::Vec3(0, 0, 1)) * osg::Matrix::translate(startpos);
	osg::Matrix mtr = osg::Matrix::rotate(osg::DegreesToRadians(azm - atol / 2), osg::Vec3(0, 0, 1)) * osg::Matrix::translate(startpos);
	osg::Vec3 endposlt = osg::Vec3(0, laglen, 0) * mtl;
	osg::Vec3 endposrt = osg::Vec3(0, laglen, 0) * mtr;

	osg::Vec3 inter;
	inter.z() = startpos.z();
	bool b = LineIntersectT<osg::Vec3>(startpos, endposlt, startposl, endposl,inter);
	endposlt = inter;
	startposl = inter;
	b = LineIntersectT<osg::Vec3>(startpos, endposrt, startposr, endposr, inter);
	endposrt = inter;
	startposr = inter;

	for (size_t i = 1; i <= nlags; i++)
	{
		float startangle = azm - atol / 2;
		float endangle = azm + atol / 2;

		float currangle = startangle;
		std::vector<osg::Vec3> poss;
		while (currangle<endangle)
		{
			float cx = cos(osg::DegreesToRadians(currangle+90)) * i * lagdis + startpos.x();
			float cy = sin(osg::DegreesToRadians(currangle+90)) * i * lagdis + startpos.y();

			bool b1 = ((startposr.x() - cx)* (endposr.y() - cy) - (endposr.x() - cx) * (startposr.y() - cy))>=0;
			bool b2 = ((startposl.x() - cx) * (endposl.y() - cy) - (endposl.x() - cx) * (startposl.y() - cy))<=0;

			if(b1 && b2)
				poss.push_back(osg::Vec3(cx, cy, startpos.z()));
			currangle += 0.1;
		}

		float cx = cos(osg::DegreesToRadians(endangle)) * i * lagdis + startpos.x();
		float cy = sin(osg::DegreesToRadians(endangle)) * i * lagdis + startpos.y();
		bool b1 = ((startposr.x() - cx) * (endposr.y() - cy) - (endposr.x() - cx) * (startposr.y() - cy)) >= 0;
		bool b2 = ((startposl.x() - cx) * (endposl.y() - cy) - (endposl.x() - cx) * (startposl.y() - cy)) <= 0;

		if (b1 && b2)
			poss.push_back(osg::Vec3(cx, cy, startpos.z()));



		res->addDrawable(createLine(poss,true));
	}
	
	res->addDrawable(createLine({ startpos, endpos }));
	res->addDrawable(createLine({ startposl, endposl }));
	res->addDrawable(createLine({ startposr,endposr }));
	res->addDrawable(createLine({ startpos, endposlt }));
	res->addDrawable(createLine({ startpos, endposrt }));
	return res;
}

osg::Node* createOut()
{
	std::ifstream ifs("./data/gamv/gamv.out");
	if (!ifs.is_open())
		return nullptr;
	std::string strline;
	int idx;
	float dis, v, n, p1, p2;
	std::vector<float> vs;
	while (std::getline(ifs, strline))
	{
		std::istringstream  iss(strline.c_str());
		if (iss >> idx >> dis >> v >> n>>p1>>p2)
		{
			vs.push_back(v);
		}
	}

	osg::Geode* geode = new osg::Geode();
	std::vector<osg::Vec3> points;
	for (size_t i = 1; i < vs.size()-1; i++)
	{
		osg::Vec3 p = startpos + normaldir * (i-0.5) * lagdis;
		osg::Vec3 pt = p + osg::Vec3(0, 0, 1) * vs[i]*5;
		osg::Sphere* sp = new osg::Sphere(pt, 10);
		osg::ShapeDrawable* sd = new osg::ShapeDrawable(sp);
		sd->setColor(osg::Vec4(1, 0, 0, 1));
		geode->addDrawable(sd);
		geode->addDrawable(createLine({ p,pt },false,osg::Vec4(0,1,0,1)));
		points.push_back(pt);
	}

	geode->addChild(createLine(points, false, osg::Vec4(1, 0, 0, 1)));
	return geode;
}

int main()
{
	osg::ref_ptr<osg::Node> points = createPoints();
	if (!points)
		return 0;

	osg::BoundingSphere bs = points->getBound();

	osg::Node* dir = createDir(bs.center());
	osg::Node* out = createOut();

	osg::Group* root = new osg::Group();
	root->addChild(points);
	if (dir) root->addChild(dir);
	if (out) root->addChild(out);


	osgViewer::Viewer viewer;
	viewer.getCamera()->setClearColor(osg::Vec4(0, 0, 0, 1));
	viewer.setUpViewInWindow(300, 300, 800, 600);
	viewer.setSceneData(root);

	return viewer.run();
}