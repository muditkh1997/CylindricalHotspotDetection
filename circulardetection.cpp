#include<bits/stdc++.h>
#define earthRadiusKm 6371.0
using namespace std;

double length,breadth;
double ldiff,bdiff;

struct Circle
{
    double lat,lon,radius;
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

vector< pair<double,double > > getCoordinatesFromDataSet(char* fileName)
{
    int i;
    FILE* stream = fopen(fileName, "r");
    char line[1024];
    vector< pair<double,double > > cord , ans;
    int flag = 0;
    while (fgets(line, 1024, stream))
    {
        char* tmp = strdup(line);
        if(flag == 0)
        {
            cord.push_back({atof(getfield(tmp,1)),atof(getfield(tmp,2))});
            flag = 1;
        }
        else
            flag = 0;

        free(tmp);
    }
    int j=0;
    for(i=0;i<cord.size();i++)
    {
        cout<<cord[i].first<<","<<cord[i].second<<endl;
        if(cord[i].first != 0 && cord[i].second != 0)
        {
            ans.push_back(cord[i]);
            j++;
        }
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

/*double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(lat1d);
  lon1r = deg2rad(lon1d);
  lat2r = deg2rad(lat2d);
  lon2r = deg2rad(lon2d);
  u = sin((lat2r - lat1r)/2);
  v = sin((lon2r - lon1r)/2);
  return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}*/

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

  return m/1000.0;
}

/*double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
double u=lat1d-lat2d;
double v=lon1d-lon2d;
double dist=u*u+v*v;
dist=pow(dist,0.5);
return dist;
}*/

vector< Circle > getCircles(vector< pair<double,double > > cord,double rmin)
{
    vector< Circle > circle;
    int i,j;
    for(i=0;i<cord.size();i++)
    {
        for(j=0;j<cord.size();j++)
        {
            if(i==j)
                continue;
            else
            {
                double radius=distanceEarth(cord[i].first,cord[i].second,cord[j].first,cord[j].second);
                if(radius>=rmin)
                {
                    Circle c;
                    c.lat=cord[i].first;
                    c.lon=cord[i].second;
                    c.radius=radius;
                    circle.push_back(c);
                    //cout<<c.lat<<" "<<c.lon<<" "<<c.radius<<endl;
                }
            }
        }
    }
    return circle;
}

double logLikelihood(vector< pair<double,double > > cord,Circle c,double areaS)
{
    double P=cord.size();
    double areaCircle=3.14*c.radius*c.radius;
    double B=(areaCircle/areaS)*P;
    int i,C=0;
    for(i=0;i<cord.size();i++)
    {
        double dist=distanceEarth(c.lat,c.lon,cord[i].first,cord[i].second);
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
double findfullarea(vector< pair<double,double > > cord)
{
    set<double> s_x,s_y;
    int i;
    for(i=0;i<cord.size();i++)
    {
        s_x.insert(cord[i].first);
        s_y.insert(cord[i].second);
    }
    double min_x,min_y,max_x,max_y;
    min_x=*s_x.begin();
    max_x=*s_x.rbegin();
    min_y=*s_y.begin();
    max_y=*s_y.rbegin();

    ldiff=max_x-min_x;
    bdiff=max_y-min_y;
    return length*breadth;
}

vector< pair<Circle,double > > generateCandidateCircle(vector< Circle > circle,vector< pair<double,double > > cord,double areaS)
{
    vector< pair<Circle,double > > c,candidateCircle;
    int i,j,flag=0;
    for(i=0;i<circle.size();i++)
    {
        double LR=logLikelihood(cord,circle[i],areaS);

        if(LR!= -INFINITY)
        {
            c.push_back({circle[i],LR});
        }

    }
    for(i=0;i<c.size();i++)
    {
        flag=0;
        for(j=0;j<c.size();j++)
        {
            double dist=distanceEarth(c[i].first.lat,c[i].first.lon,c[j].first.lat,c[j].first.lon);
            //cout<<dist<<endl;
            if(dist<(c[i].first.radius+c[j].first.radius))
            {
                if((c[i].second<c[j].second))
                {
                    flag=1;
                }

            }
        }
        if(flag==0)
        {
            candidateCircle.push_back(c[i]);
           // cout<<c[i].first.lat<<" "<<c[i].first.lon<<" "<<c[i].first.radius<<endl;
        }
    }
    return candidateCircle;
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

void generatemontecarlo(int m,double montecarlo[5000],double areaS)
{
    double rmin=0.5;
    int u=10;
    int i,j;

    for(i=0;i<m;i++)
    {
        vector< pair<double,double > > cord;
        cout<<"New dataset"<<endl;
        for(j=0;j<60;j++)
        {
            double lat=fmod(rand(),ldiff);
            double lon=fmod(rand(),bdiff);
            cord.push_back({lat,lon});
            cout<<lat<<" "<<lon<<endl;
        }
        vector< Circle > circle=getCircles(cord,rmin);
        double maxll=-999;
        for(j=0;j<circle.size();j++)
        {
            double ll=logLikelihood(cord,circle[j],areaS);

            if(ll>maxll)
                maxll=ll;
        }
        //cout<<maxll<<endl;
        montecarlo[i]=maxll;

    }
}



int main()
{
    double rmin=2.5;
    vector< pair<double,double > > cord=getCoordinatesFromDataSet("Book6.csv");
    double areaS = findfullarea(cord);
    vector< Circle > circle=getCircles(cord,rmin);
    vector< pair<Circle,double > > candidateCircles =generateCandidateCircle(circle,cord,areaS);


    srand(time(NULL));

    int m=999;
    double montecarlo[m];
    for(int i=0;i<m;i++)
        montecarlo[i]=0;
    generatemontecarlo(m,montecarlo,areaS);
    sort(montecarlo,montecarlo+m,greater<double>());

    vector< pair<Circle,double > > hotspotCircles;
    for(int i=0;i<candidateCircles.size();i++)
    {
        for(int j=0;j<m;j++)
        {
            if(candidateCircles[i].second>montecarlo[j])
            {
                double pval=((double)j)/((double)m+1);
                //cout<<pval<<endl;
                if(pval<=0.001)
                {
                    hotspotCircles.push_back({candidateCircles[i].first,candidateCircles[i].second});
                }
                break;
            }
        }
    }
    //for(int i=0;i<hotspotCircles.size();i++)
    //{
    //    cout<<"Hotspot centre: "<<hotspotCircles[i].first.lat<<","<<hotspotCircles[i].first.lon<<" radius: "<<candidateCircles[i].first.radius<<endl;

    //}

    FILE *fp;

    fp=fopen("hotspot1.csv","w+");
    fprintf(fp,"Lat, Lon, Radius, LLR");
    cout<<hotspotCircles.size()<<endl;

    for(int i=0;i<hotspotCircles.size();i++)
    {
        fprintf(fp,"\n%f,%f,%f,%f",hotspotCircles[i].first.lat,hotspotCircles[i].first.lon,hotspotCircles[i].first.radius,hotspotCircles[i].second);
        cout<<"Hotspot centre: "<<hotspotCircles[i].first.lat<<","<<hotspotCircles[i].first.lon<<" radius: "<<hotspotCircles[i].first.radius<<endl;

    }
    fclose(fp);
}
