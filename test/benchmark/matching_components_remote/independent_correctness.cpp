#include "candidate.hpp"
#include "baseline.hpp"
#include <bit>
#include <iostream>
#include <random>
#include <functional>
using namespace std;
int brute(const vector<unsigned>& e) {
 vector<int> dp(1u<<e.size(),-1); dp[0]=0;
 function<int(unsigned)> f=[&](unsigned s) {if(dp[s]>=0)return dp[s]; int u=countr_zero(s); unsigned t=s^(1u<<u), a=e[u]&t; int r=f(t); while(a){unsigned b=a&-a; a-=b;r=max(r,1+f(t^b));}return dp[s]=r;};
 return f((1u<<e.size())-1);
}
int main(){
 mt19937 r(1828193); long checks=0;
 for(int trial=0;trial<3000;++trial){int n=trial%17;GeneralMatching a(n);BaselineMatching b(n);vector<unsigned> e(n); vector<pair<int,int>> edges;
 vector<int> group(n);for(auto &g:group)g=r()%5;
 for(int u=0;u<n;++u)for(int v=u;v<n;++v)if((trial%3==0||group[u]==group[v])&&r()%4)edges.emplace_back(u,v);
 shuffle(edges.begin(),edges.end(),r);
 auto check=[&]{int x=a.build(),y=b.build();assert(x==y&&a.mate==b.mate&&a.edges()==b.edges());assert(x==brute(e));++checks;};
 check();for(auto [u,v]:edges){a.add_edge(u,v);b.add_edge(u,v);if(u!=v)e[u]|=1u<<v,e[v]|=1u<<u;if(r()%3==0){a.add_edge(v,u);b.add_edge(v,u);}if(r()%7==0)check();}check();check();
 auto ac=a;auto bc=b;assert(ac.build()==bc.build()&&ac.mate==bc.mate);
 for(int u=1;u<n;++u){a.add_edge(u,u-1);b.add_edge(u,u-1);e[u]|=1u<<(u-1);e[u-1]|=1u<<u;check();}
 }
 for(int trial=0;trial<400;++trial){int n=100+(r()%301);GeneralMatching a(n);BaselineMatching b(n);vector<int> p(n);iota(p.begin(),p.end(),0);shuffle(p.begin(),p.end(),r);vector<pair<int,int>> edges;
 // Overlapping odd cycles, triangles, deficient stars, and isolated vertices.
 for(int s=0;s+13<n;s+=17){for(int j=0;j<9;++j)edges.emplace_back(p[s+j],p[s+(j+1)%9]);for(int j=1;j<13;++j)edges.emplace_back(p[s],p[s+j]);edges.emplace_back(p[s+2],p[s+4]);edges.emplace_back(p[s+4],p[s+6]);}
 shuffle(edges.begin(),edges.end(),r);auto ck=[&]{assert(a.build()==b.build());assert(a.mate==b.mate&&a.edges()==b.edges());++checks;};
 for(auto [u,v]:edges){a.add_edge(u,v);b.add_edge(u,v);}ck();ck();for(int j=1;j<n;++j){a.add_edge(p[j-1],p[j]);b.add_edge(p[j-1],p[j]);if(j%37==0)ck();}ck();
 }
 cout<<"passed "<<checks<<" differential checks\n";
}
