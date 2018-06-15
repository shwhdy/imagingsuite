#include <sstream>

#include <QString>
#include <QtTest>
#include <QDebug>

#include <base/timage.h>
#include <base/index2coord.h>
#include <math/tcenterofgravity.h>
#include <math/circularhoughtransform.h>
#include <math/nonlinfit.h>
#include <filters/filter.h>
#include <drawing/drawing.h>
#include <math/statistics.h>
#include <math/mathfunctions.h>

#include <io/io_tiff.h>

class TKiplMathTest : public QObject
{
    Q_OBJECT

public:
    TKiplMathTest();
    static bool qFuzzyCompare(double p1, double p2, double delta=0.000000000001)
    {
        return (qAbs(p1 - p2) <= delta * qMin(qAbs(p1), qAbs(p2)));
    }

private Q_SLOTS:
    void testCOG();
    void testCircularHoughTransform();
    void testNonLinFit_GaussianFunction();
    void testNonLinFit_fitter();

    void testNonLinFit_fitter2();
    void testStatistics();
    void testSignFunction();
};

TKiplMathTest::TKiplMathTest()
{
}

void TKiplMathTest::testCOG()
{
    kipl::math::CenterOfGravity cog;

    size_t dims[3]={100,100,100};
    kipl::base::TImage<float,3> img(dims);
    kipl::base::coords3Df center;
    center.x=54;
    center.y=45;
    center.z=60;
    float R2=400;

    size_t i=0UL;
    float *pImg=img.GetDataPtr();
    for (size_t z=0; z<dims[2]; ++z)
    {
        for (size_t y=0; y<dims[1]; ++y)
        {
            for (size_t x=0; x<dims[0]; ++x, ++i)
            {
                pImg[i]=((x-center.x)*(x-center.x)+
                        (y-center.y)*(y-center.y)+
                        (z-center.z)*(z-center.z))<R2;
            }
        }
    }

    kipl::base::coords3Df cogcenter=cog.findCenter(img, false);
    ostringstream msg;
    msg<<center.x<<"!="<<cogcenter.x;
    QVERIFY2(cogcenter.x==center.x,msg.str().c_str());

    msg.str("");
    msg<<center.y<<"!="<<cogcenter.y;
    QVERIFY2(cogcenter.y==center.y,msg.str().c_str());
    msg.str("");
    msg<<center.z<<"!="<<cogcenter.z;
    QVERIFY2(cogcenter.z==center.z,msg.str().c_str());

    float k[30];
    std::fill(k,k+30,1.0f);
    size_t kd[3]={3,3,3};
    kipl::filters::TFilter<float,3> box(k,kd);

    kipl::base::TImage<float,3> fimg=box(img,kipl::filters::FilterBase::EdgeValid);
    cogcenter=cog.findCenter(fimg, true);

    msg.str("");
    msg<<center.x<<"!="<<cogcenter.x;
    float delta=0.01f;
    QVERIFY2(qFuzzyCompare(cogcenter.x,center.x,delta),msg.str().c_str());

    msg.str("");
    msg<<center.y<<"!="<<cogcenter.y;
    QVERIFY2(qFuzzyCompare(cogcenter.y,center.y,delta),msg.str().c_str());
    msg.str("");
    msg<<center.z<<"!="<<cogcenter.z;
    QVERIFY2(qFuzzyCompare(cogcenter.z,center.z,delta),msg.str().c_str());
}

void TKiplMathTest::testCircularHoughTransform()
{
    size_t dims[2]={100,100};

    kipl::base::TImage<float,2> img(dims);

    kipl::drawing::Circle<float> circ1(10.0);
    kipl::drawing::Circle<float> circ2(5.0);

    circ1.Draw(img,50,50,1.1);

    circ1.Draw(img,75,25,0.6);
    circ1.Draw(img,70,80,2.3);
    circ2.Draw(img,25,75,1.5);

    kipl::math::CircularHoughTransform cht;

    kipl::base::TImage<float,2> chm=cht(img,10.0f);

    kipl::io::WriteTIFF32(img,"cht_orig.tif");
    kipl::io::WriteTIFF32(chm,"cht_map.tif");

    chm=cht(img,10.0f,true);

    kipl::io::WriteTIFF32(img,"chtg_orig.tif");
    kipl::io::WriteTIFF32(chm,"chtg_map.tif");
}

void TKiplMathTest::testNonLinFit_GaussianFunction()
{
    Nonlinear::SumOfGaussians sog(1);
    QCOMPARE(sog.getNpars(),3); // Three parameters as we have position, amplitude and width.
    QCOMPARE(sog.getNpars2fit(),3); // Default all will be fitted

    int lv[]={0,1,1};
    sog.setLock(lv);
    for (int i=0; i<sog.getNpars(); ++i)
        QCOMPARE(sog.isLocked(i),lv[i]);

    QCOMPARE(sog.getNpars2fit(),1);
    sog[0]=1;
    sog[1]=0;
    sog[2]=1;

    QCOMPARE(sog(0.0),(long double)1.0);
    QVERIFY(fabs(sog(1)-(long double)0.367879441171)<(long double)1.0e-7);

    Nonlinear::SumOfGaussians sog2(2);
    sog2[0]=1;
    sog2[1]=0;
    sog2[2]=1;
    sog2[3]=2;
    sog2[4]=0.5;
    sog2[5]=0.5;
//    qDebug() << (double)sog2(0);
//    qDebug() << (double)sog2(1);
    QVERIFY(fabs(sog2(0)-(long double)1.73575888234)<(long double)1.0e-7);
    QVERIFY(fabs(sog2(1)-(long double)1.10363832351)<(long double)1.0e-7);

    long double x=1;
    long double y0=0;
    long double y1=0;
    Array1D<long double> dyda(3);

    y0=sog(x);
    sog(x,y1,dyda);
    QCOMPARE(y0,y1);
}

void TKiplMathTest::testNonLinFit_fitter()
{
    int N=100;
//    long double *x=new long double[N];
//    long double *y=new long double[N];
//    long double *sig=new long double[N];
    double *x=new double[N];
    double *y=new double[N];
    double *sig=new double[N];

    Nonlinear::SumOfGaussians sog0(1),sog(1);
    sog0[0]=2; //A
    sog0[1]=0; //m
    sog0[2]=1; //s

    for (int i=0; i<N; ++i)
    {
        x[i]=(i-N/2)*0.2;
        y[i]=sog0(x[i]);
        sig[i]=1.0;
    }

    sog[0]=2.1; //A
    sog[1]=.1; //m
    sog[2]=0.9; //s

    Nonlinear::LevenbergMarquardt(x,y,N,sog,1e-15);

 //   Nonlinear::mrqmin(x,y,sig, N, covar, alpha, chisq, sog,alambda);
    sog.printPars();
    QCOMPARE(sog[0],sog0[0]);
    QCOMPARE(sog[1],sog0[1]);
    QCOMPARE(sog[2],sog0[2]);

    delete [] x;
    delete [] y;
    delete [] sig;
}

void TKiplMathTest::testNonLinFit_fitter2()
{
    int N=100;

    Array1D<double> x(N);
    Array1D<double> y(N);
    Array1D<double> sig(N);
    Array1D<double> aa(3);
    Array1D<double> aa0(3);
    Array1D<double> dyda(3);

//    Nonlinear::SumOfGaussians sog0(1);
//    sog0[0]=2; //A
//    sog0[1]=0; //m
//    sog0[2]=1; //s

        aa0[0]=2; //A
        aa0[1]=0; //m
        aa0[2]=1; //s


    for (int i=0; i<N; ++i)
    {
        x[i]=(i-N/2)*0.2;
        NonlinearDev::fgauss(x[i],aa0,y[i],dyda);
      //  qDebug() << "Data: x="<<x[i]<<", y="<<y[i];
        sig[i]=1.0;
    }

    aa[0]=2; //A
    aa[1]=1; //m
    aa[2]=0.1; //s

    NonlinearDev::LevenbergMarquardt mrq(x,y,sig,aa,&NonlinearDev::fgauss,1e-15);

    mrq.fit();

    qDebug() <<aa[0]<<", "<<aa[1]<<", "<<aa[2];

    QCOMPARE(aa[0], aa0[0]);
    QCOMPARE(aa[1], aa0[1]);
    QCOMPARE(aa[2], aa0[2]);
//    QVERIFY(qFuzzyCompare(aa[0], aa0[0]));
//    QVERIFY(qFuzzyCompare(aa[1], aa0[1]));
//    QVERIFY(qFuzzyCompare(aa[2], aa0[2]));

}

void TKiplMathTest::testStatistics()
{
    kipl::math::Statistics stats;

    QVERIFY(stats.n()==0UL);
    QVERIFY(stats.E()==0.0f);
    QVERIFY(stats.s()==0.0f);
    QVERIFY(stats.Min()==0.0f);
    QVERIFY(stats.Max()==0.0f);
    QVERIFY(stats.V()==0.0f);
    QVERIFY(stats.Sum()==0.0f);
    QVERIFY(stats.Sum2()==0.0f);

    double data[10]={ 4.49201549,  3.63910658,  1.14245381,  2.42364998,  2.12082273,
                     -0.89899808,  2.27052317,  0.62877943,  5.10328637,  4.0204556};

    for (int i=0; i<10; ++i)
        stats.put(data[i]);
    double eps=0.0001;
    QVERIFY(fabs(stats.Sum()==24.942095070011739)<eps);
    QVERIFY(fabs(stats.Sum2()==93.664900413805682)<eps);
    QVERIFY(stats.n()==10UL);
    QVERIFY(fabs(stats.E()-2.4942095070011741)<eps);
    cout<<stats.s()<<std::endl;
    QVERIFY(fabs(stats.s()-1.7735300889935668)<eps);
    QVERIFY(fabs(stats.Min()==-0.89899808236334788)<eps);
    QVERIFY(fabs(stats.Max()==5.1032863747302493)<eps);
    QVERIFY(fabs(stats.V()==3.1454089765655291)<eps);


}

void TKiplMathTest::testSignFunction()
{
    std::ostringstream msg;
    for (int i=-10; i<10; ++i) {
        msg.str("");
        msg<<"Sign for "<<i<<" is "<<kipl::math::sign(i);
        if (i<0)
            QVERIFY2(kipl::math::sign(i)==int(-1),msg.str().c_str());
        else {
        if (i==0)
            QVERIFY2(kipl::math::sign(i)==0,msg.str().c_str());
        else {
            if (i>0)
                QVERIFY2(kipl::math::sign(i)==1,msg.str().c_str());
            }
        }
    }

    for (float f=-10.0f; f<10.0f; ++f) {
        msg.str("");
        msg<<"Sign for "<<f<<" is "<<kipl::math::sign(f);
        if (f<0.0f)
            QVERIFY2(kipl::math::sign(f)==-1,msg.str().c_str());
        else {
            if (f==0.0f)
                QVERIFY2(kipl::math::sign(f)==0,msg.str().c_str());
            else {
                if (f>0.0f)
                    QVERIFY2(kipl::math::sign(f)==1,msg.str().c_str());
            }
        }

    }

    QVERIFY2(kipl::math::sign(std::numeric_limits<float>::infinity())==1,"Plus inf");
    QVERIFY2(kipl::math::sign(-std::numeric_limits<float>::infinity())==-1,"Minus inf");

}

QTEST_APPLESS_MAIN(TKiplMathTest)

#include "tst_tkiplmathtest.moc"
