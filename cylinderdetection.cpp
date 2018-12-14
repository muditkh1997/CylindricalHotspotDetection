
#include<bits/stdc++.h>
#define earthRadiusKm 6371.0
using namespace std;


double ldiff,bdiff;
struct Circle
{
    double lat,lon,radius;
};

struct Cylinder
{
    double lat,lon,radius,baseyear,heightyear;
};
const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

vector < vector< pair<double,double > > >getCoordinatesFromDataSet(char* fileName)
{

    int i;
    FILE* stream = fopen(fileName, "r");
    char line[1024];
    vector< vector< pair< double,double > > > cord , ans;
    vector< pair< double,double > > a;
    float lastyear=-1.0;
    int m=1;
    while (fgets(line, 1024, stream))
    {

        char* tmp = strdup(line);
        double lat=atof(getfield(tmp,2));
        free(tmp);
        tmp = strdup(line);
        double lon=atof(getfield(tmp,3));
        free(tmp);
        tmp = strdup(line);
        float year=atof(getfield(tmp,1));
        if(lastyear==-1.0)
        {
            a.push_back({lat,lon});
            lastyear=year;
        }
        else
        {
            if(lastyear==year)
            {
                a.push_back({lat,lon});
            }
            else
            {
                cord.push_back(a);
                a.clear();
                a.push_back({lat,lon});
                lastyear=year;
            }
        }
        free(tmp);
    }
    cord.push_back(a);
    a.clear();

    int j=0;
    for(i=0;i<cord.size();i++)
    {
        for(int k=0;k<cord[i].size();k++)
        {
            if(cord[i][k].first != 0 && cord[i][k].second != 0)
            {
                a.push_back(cord[i][k]);
                j++;
            }
        }
        ans.push_back(a);
        a.clear();
    }

    return ans;
}

// This function converts decimal degrees to radians
double deg2rad(double deg) {
  return (deg * M_PI / 180);
}

//  This function converts radians to decimal degrees
double rad2deg(double rad) {
  return (rad * 180 / M_PI);
}

double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(lat1d);
  lon1r = deg2rad(lon1d);
  lat2r = deg2rad(lat2d);
  lon2r = deg2rad(lon2d);
  double delta_phi=deg2rad(lat2d-lat1d);
  double delta_lambda=deg2rad(lon2d-lon1d);
  double a=pow(sin(delta_phi/2.0),2)+cos(lat1r)*cos(lat2r)*pow(sin(delta_lambda/2.0),2);
  double c=2*atan2(sqrt(a),sqrt(1-a));
  double m=earthRadiusKm*1000*c;

  return m;
}



vector < vector< Circle > > getCircles(vector < vector< pair<double,double > > >cord,double rmin)
{
    vector< vector <Circle> > circles;
    vector< Circle > circle;
    int i,j,k;
    for(k=0;k<cord.size();k++)
    {
        for(i=0;i<cord[k].size();i++)
        {
            for(j=0;j<cord[k].size();j++)
            {
                if(i==j)
                    continue;
                else
                {
                    double radius=distanceEarth(cord[k][i].first,cord[k][i].second,cord[k][j].first,cord[k][j].second);
                    if(radius>=rmin)
                    {
                        Circle c;
                        c.lat=cord[k][i].first;
                        c.lon=cord[k][i].second;
                        c.radius=radius;
                        circle.push_back(c);
                    }
                }
            }
        }
        circles.push_back(circle);
        circle.clear();
    }
    return circles;
}

double logLikelihood(vector <vector  < pair<double,double > > > cord,Circle c,double areaS,int k)
{
    double P=cord[k].size();
    double areaCircle=3.14*c.radius*c.radius;
    double B=(areaCircle/areaS)*P;
    int i,C=0;
    for(i=0;i<cord[k].size();i++)
    {
        double dist=distanceEarth(c.lat,c.lon,cord[k][i].first,cord[k][i].second);
        if(dist<c.radius)
            C++;

    }
    int I;
    if(C>B)
        I=1;
    else
        I=0;
    double a=(C/B);
    a=pow(a,C);
    double b=(P-C)/(P-B);
    b=pow(b,(P-C));
    double ans=log(a*b*I);
    return ans;
}
double logLikelihoodCylinder(vector < vector <  pair<double,double >  > >cord,Cylinder c,double volumeS, int h)
{
    double P;
    vector < pair<double,double> > mergedcords;
    for(int i=0;i<cord.size();i++)
    {
        for(int j=0;j<cord[i].size();j++)
        {
            mergedcords.push_back(cord[i][j]);
        }
    }
    P=mergedcords.size();
    h=c.heightyear-c.baseyear;
    double volumeCylinder=3.14*c.radius*c.radius*h;
    double B=(volumeCylinder/volumeS)*P;
    int i,C=0;

    for(int j=c.baseyear;j<=c.heightyear;j++)
    {
        for(i=0;i<cord[j].size();i++)
        {
            double dist=distanceEarth(c.lat,c.lon,cord[j][i].first,cord[j][i].second);
            if(dist<c.radius)
                C++;
        }
    }
    int I;
    if(C>B)
        I=1;
    else
        I=0;
    double a=(C/B);
    a=pow(a,C);
    double b=(P-C)/(P-B);
    b=pow(b,(P-C));
    double ans=log(a*b*I);
    return ans;
}

double findfullarea(vector <vector< pair<double,double > > > cord)
{
    set<double> s_x,s_y;
    int i,j;
    for(i=0;i<cord.size();i++)
    {
        for(j=0;j<cord[i].size();j++)
        {
            s_x.insert(cord[i][j].first);
            s_y.insert(cord[i][j].second);
        }

    }
    double min_x,min_y,max_x,max_y;
    min_x=*s_x.begin();
    max_x=*s_x.rbegin();
    min_y=*s_y.begin();
    max_y=*s_y.rbegin();
    /*auto d = s_x.begin();
    min_x = *d;
    d = s_x.end();
    d--;
    max_x = *d;
    d = s_y.begin();
    min_y = *d;
    d = s_y.end();
    d--;
    max_y = *d;*/
    double breadth = distanceEarth(min_x , min_y , min_x , max_y);
    double length = distanceEarth(min_x , min_y , max_x , min_y);
    return length*breadth;
}

double findfullvolume(vector <vector< pair<double,double > > > cord)
{
    set<double> s_x,s_y;
    int i,j;
    for(i=0;i<cord.size();i++)
    {
        for(j=0;j<cord[i].size();j++)
        {
            s_x.insert(cord[i][j].first);
            s_y.insert(cord[i][j].second);
        }

    }
    double min_x,min_y,max_x,max_y;
    min_x=*s_x.begin();
    max_x=*s_x.rbegin();
    min_y=*s_y.begin();
    max_y=*s_y.rbegin();

    ldiff=max_x-min_x;
    bdiff=max_y-min_y;

    double breadth = distanceEarth(min_x , min_y , min_x , max_y);
    double length = distanceEarth(min_x , min_y , max_x , min_y);
    return length*breadth*(cord.size()-1);
}

vector < vector< pair<Circle,double > > > generateCandidateCircle(vector <vector< Circle > >circle,vector <vector< pair<double,double > > >cord,double areaS)
{
    vector < vector< pair<Circle,double > > > cs,candidateCircles;
    vector< pair<Circle,double > > c,candidateCircle;
    int i,j,flag=0;
    //cout<<"circlesize = "<<circle.size()<<endl;
    for(int k=0;k<circle.size();k++)
    {
        for(i=0;i<circle[k].size();i++)
        {
            double LR=logLikelihood(cord,circle[k][i],areaS,k);
            cout<<k<<" "<<circle[k][i].lat<<" "<<circle[k][i].lon<<" "<<circle[k][i].radius<<" "<<LR<<endl;
            if(LR!= -INFINITY)
            {
                c.push_back({circle[k][i],LR});
            }

        }
        cs.push_back(c);
        c.clear();
    }


    for(int k=0;k<cs.size();k++)
    {
        for(i=0;i<cs[k].size();i++)
        {
            flag=0;
            for(j=0;j<cs[k].size();j++)
            {
                double dist=distanceEarth(cs[k][i].first.lat,cs[k][i].first.lon,cs[k][j].first.lat,cs[k][j].first.lon);
                cout<<k<<" "<<cs[k][i].first.lat<<" "<<cs[k][i].first.lon<<" "<<cs[k][j].first.lat<<" "<<cs[k][j].first.lon<<endl;
                if(dist<(cs[k][i].first.radius+cs[k][j].first.radius))
                {
                    if((cs[k][i].second<cs[k][j].second))
                    {
                        flag=1;
                    }

                }
            }
            if(flag==0)
            {
                candidateCircle.push_back(cs[k][i]);
                cout<<"year "<<k<<" lat"<<cs[k][i].first.lat<<endl;
            }
        }
        candidateCircles.push_back(candidateCircle);
        candidateCircle.clear();
    }
    return candidateCircles;
}

vector< pair<Cylinder,double > > generateCandidateCylinders(vector < vector< Circle > >candidateCircles,vector <vector< pair<double,double > > >cord,double volumeS)
{
  vector< pair<Cylinder,double > > c,candidateCylinders,cylinders;
  int i,j;
  vector< pair<double,double > > cords;
    int a=1;
  int height=cord.size()-1;
  int flag;
  for(i=0;i<cord.size();i++)
  {
      for(j=0;j<cord[i].size();j++)
        cords.push_back(cord[i][j]);
  }
  for(i=0;i<candidateCircles.size();i++)
  {
      for(j=0;j<candidateCircles[i].size();j++)
      {
          for(int k=0;k<candidateCircles.size();k++)
          {
              if(k!=i)
              {
                  Cylinder cyl;
                  cyl.baseyear=min(k,i);
                  cyl.heightyear=max(k,i);
                  cyl.lat=candidateCircles[i][j].lat;
                  cyl.lon=candidateCircles[i][j].lon;
                  cyl.radius=candidateCircles[i][j].radius;
                  double LR=logLikelihoodCylinder(cord,cyl,volumeS,height);
                    a++;
                    if(LR!= -INFINITY)
                    {
                        cylinders.push_back({cyl,LR});
                    }
              }
          }
      }
        for(int l=0;l<cylinders.size();l++)
        {
            flag=0;
            for(j=0;j<cylinders.size();j++)
            {
                double dist=distanceEarth(cylinders[l].first.lat,cylinders[l].first.lon,cylinders[j].first.lat,cylinders[j].first.lon);
                //cout<<dist<<endl;
                if(dist<(cylinders[l].first.radius+cylinders[j].first.radius))
                {
                    if((cylinders[l].second<cylinders[j].second))
                    {
                        flag=1;
                    }

                }
            }
            if(flag==0)
            {
                c.push_back(cylinders[l]);

            }
        }
      cylinders.clear();
  }
  for(i=0;i<c.size();i++)
    {
        flag=0;
        for(j=0;j<c.size();j++)
        {
            double dist=distanceEarth(c[i].first.lat,c[i].first.lon,c[j].first.lat,c[j].first.lon);
            int hyear,byear;
            double hdiff;
            if(c[i].first.heightyear>c[j].first.heightyear)
            {
                hyear=c[j].first.heightyear;
                byear=c[i].first.baseyear;
            }
            else
            {
                hyear=c[i].first.heightyear;
                byear=c[j].first.baseyear;
            }
            hdiff=hyear-byear;
            //cout<<dist<<endl;
            if(dist<(c[i].first.radius+c[j].first.radius) && hdiff>0)
            {
                if((c[i].second<c[j].second))
                {
                    flag=1;
                }

            }
        }
        if(flag==0)
        {
            candidateCylinders.push_back(c[i]);
        }
    }
  return candidateCylinders;
}

double rand_double() {
   return rand()/(double)RAND_MAX;
}
int getPoissonRandom(int u)
{
  double L=exp((-1)*u);
  double p = 1.0;
  int k = 0;

  do {
    //cout<<"1"<<endl;
    k++;
    p *= rand_double();
  } while (p > L);

  return k - 1;
}


void generatemontecarlo(int m,double montecarlo[5000],double areaS,double volumeS)
{
    double rmin=0.5;
    int u=10;
    double e=exp((-1)*u);
    int i,j;

    for(i=0;i<m;i++)
    {
        vector < vector< pair<double,double > > > cords;
        vector< pair<double,double > > cord[3],mergedcords;
        for(j=0;j<227;j++)
        {
            int year= rand()%3;
            double lat=fmod(rand(),ldiff);
            double lon=fmod(rand(),bdiff);
            cord[year].push_back({lat,lon});
            mergedcords.push_back({lat,lon});
        }
        for(int k=0;k<3;k++)
        {
            cords.push_back(cord[k]);
        }

        vector < vector< Circle > > circle=getCircles(cords,rmin);
        vector< Cylinder >  cylinders;
        for(int l=0;l<circle.size();l++)
        {
            for(j=0;j<circle[l].size();j++)
            {
                for(int k=0;k<circle.size();k++)
                {
                      if(k!=l)
                      {
                          Cylinder cyl;
                          cyl.baseyear=min(k,l);
                          cyl.heightyear=max(k,l);
                          cyl.lat=circle[l][j].lat;
                          cyl.lon=circle[l][j].lon;
                          cyl.radius=circle[l][j].radius;
                          cylinders.push_back(cyl);
                      }
                }
            }
        }

        double maxll=-999;
        for(j=0;j<cylinders.size();j++)
        {
            double ll=logLikelihoodCylinder(cords,cylinders[j],volumeS,2);

            if(ll>maxll)
                maxll=ll;
        }

        //cout<<maxll<<endl;
        montecarlo[i]=maxll;

    }
}
int main()
{

    double rmin=0.4;
    vector< vector< pair<double,double > > > cord=getCoordinatesFromDataSet("database.csv");

    double areaS =findfullarea(cord);
    double volumeS = findfullvolume(cord);
    vector < vector< Circle > > circle=getCircles(cord,rmin);



    vector < pair<Cylinder, double> > candidateCylinders = generateCandidateCylinders(circle,cord,volumeS);
    //monte carlo
    cout<<candidateCylinders.size()<<endl;
    for(int i=0;i<candidateCylinders.size();i++)
    {
        cout<<candidateCylinders[i].first.lat<<" "<<candidateCylinders[i].first.lon<<" "<<candidateCylinders[i].first.radius<<" "<<candidateCylinders[i].second<<endl;

    }
    srand(time(NULL));

    int m=999;
    double montecarlo[m];
    for(int i=0;i<m;i++)
        montecarlo[i]=0;
    generatemontecarlo(m,montecarlo,areaS,volumeS);
    sort(montecarlo,montecarlo+m,greater<double>());
    for(int i=0;i<m;i++)
    {
        cout<<montecarlo[i]<<" ";
    }
    cout<<endl;
    vector< pair<Cylinder,double > > hotspotCylinder;
    cout<<candidateCylinders.size()<<endl;
    for(int i=0;i<candidateCylinders.size();i++)
    {
        for(int j=0;j<m;j++)
        {
            if(candidateCylinders[i].second>montecarlo[j])
            {
                double pval=((double)j)/((double)m+1);
                cout<<"pval "<<pval<<endl;
                if(pval<=0.001)
                {
                    hotspotCylinder.push_back({candidateCylinders[i].first,candidateCylinders[i].second});
                }
                break;
            }
        }
    }
    FILE *fp;

    fp=fopen("hotspot.csv","w+");
    fprintf(fp,"Lat, Lon, Radius, LLR");
    cout<<hotspotCylinder.size()<<endl;

    for(int i=0;i<hotspotCylinder.size();i++)
    {
        fprintf(fp,"\n%f,%f,%f,%f",hotspotCylinder[i].first.lat,hotspotCylinder[i].first.lon,hotspotCylinder[i].first.radius,hotspotCylinder[i].second);
        cout<<"Hotspot centre: "<<hotspotCylinder[i].first.lat<<","<<hotspotCylinder[i].first.lon<<" radius: "<<hotspotCylinder[i].first.radius<<" height: "<<(hotspotCylinder[i].first.heightyear-hotspotCylinder[i].first.baseyear)<<endl;

    }
    fclose(fp);
}


