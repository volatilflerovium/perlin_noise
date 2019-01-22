#include<iostream>
#include<cmath>
#include<string>
#include<cstdlib>
#include<ctime>
#include <chrono>
#include<thread>
#include<SFML/Graphics.hpp>

#include "helper.h"
#include "framerate.h"
#include "reference_frame.h"

using namespace std;

//###################################################################
//###################################################################
//###################################################################

/*
This class generate integer noise for a grid of R x C of rectangles
of xlg x ylg 

*/


class Noise
{
	private:
		double* NS;
		int rows;
		int cols;
		double xlg;
		double ylg;
		int seedx;
		int seedy;

	public:
		Noise(int C, int R, double lx, double ly);
		Noise() = delete;
		Noise(const Noise&) = delete;
		Noise& operator=(const Noise&) = delete;
		double interpolation(int k, int T = 2) const;
		double smooth_interpolation(int k) const;
		void IntegerNoise(int i, int j, double factor, double cr, double period=50000.0);
		void reset_seeds(int sx, int sy);
		virtual ~Noise();
};

Noise::~Noise(){
	delete[] NS;
}

/*
C the number of columns of the grid
R the number of rows of the grid
lx the width of each square that the grid is made of 
ly the heigh of each square that the grid is made of 

*/

Noise::Noise(int C, int R, double lx, double ly)
		:rows(R), cols(C), xlg(lx), ylg(ly), seedx(10502), seedy(30955){
	NS=new double[rows*cols];
}

/*
the index of the cell to be interpolated with respect to
a square of side T-cells
*/
double Noise::interpolation(int k, int T) const {
	
	int i=k/cols;
	int j=k-cols*i;

	int ti=(T)*(i/T);
	int tj=(T)*(j/T);
		
	int tr=T;
	double TR=ylg*T;
	if(ti+T>=rows){
		tr=rows-ti-1;
		TR=ylg*(tr);
	}

	int tc=T;
	double TC=xlg*T;
	if(tj+T>=cols){
		tc=cols-tj-1;
		TC=xlg*(tc);
	}
		
	int h11=cols*ti+tj; // top left
	int h12=cols*ti+tj+tc; // top right
	int h21=cols*(ti+tr)+tj; // bottom left
	int h22=cols*(ti+tr)+tj+tc; // bottom right
	
	double n=1.0/(TR*TC);
			n*=((TC+xlg*(tj-j))*(NS[h11]*(TR+ylg*(ti-i))+NS[h21]*(i-ti)*ylg)+
				   xlg*(j-tj)*(NS[h12]*(TR+ylg*(ti-i))+NS[h22]*(i-ti)*ylg));
	return n;
}

/*
k is the cell to be interpolated
*/

double Noise::smooth_interpolation(int k) const {
   static int r=9;
   static int iterations[]={5, 11, 23, 31, 53, 83, 113, 163, 211};
   double v=0;
   for(int i=0; i<r; i++)
   {
      v+=interpolation(k, iterations[i]);
   }
   
   return v/(1.0*r);
}

void Noise::reset_seeds(int x, int y){
	seedx=x;
	seedy=y;
}


/*
i, j the (row, col) position of a cell.
factor: how much we want to stretch the result
cr if we want just positive or negative results
period if we need to set a cycle for the randomness... very hight value less cycling effect
*/

void Noise::IntegerNoise(int i, int j, double factor, double cr, double period){
	int k=cols*i+j;
	i+=seedy;
	j+=seedx;
	//j+=30955;
   j = (j >> 13) ^ j;
   i = (i >> 13) ^ i;
   double n = (i*i*(j*j*(i*43943+1256531)+13784891)+j*j*(i*i*(j*184949+5752609)+19990303)+j*i*184949+30671033) & 0x7fffffff;
   //return factor*(sin((x+y)/30000.0)+sin(n)+cr);
   NS[k]=factor*(sin((i+j)/period)+sin(n)+cr);
}


//####################################################################
//####################################################################
//####################################################################
//####################################################################
//*
void do_something(int h, int k, int nl, double lg, sf::RectangleShape* Grid, Noise* Ns){
	cout<<"Thread: "<<h<<"===================="<<endl;
	int ti,tj;
   for(int i=h; i<k; i++){
    	Grid[i]=sf::RectangleShape(sf::Vector2f(lg, lg));
    	ti=i/nl;
     	tj=i-ti*nl;
     	Grid[i].setPosition(lg*(1.0*ti), lg*(1.0*tj));
     	Ns->IntegerNoise(ti, tj, 60, 2.0);
   }
	cout<<"Thread: "<<h<<"++++++++++++++++++++++"<<endl;
}

void do_more(int h, int k, sf::RectangleShape* Grid, Noise* Ns){
	cout<<"Thread more: "<<h<<endl;
	double ty=30;
	for(int i=h; i<k; i++){
		//ty=Ns.interpolation(i, 40);
		ty=Ns->smooth_interpolation(i);
		
		Grid[i].setFillColor(sf::Color(ty, ty, 200, 255));
	}
}


void draw_things(int h, int k, sf::RenderWindow* window, sf::RectangleShape* Grid){
	for(int i=h; i<k; i++){
		window->draw(Grid[i]);
	}
}


//####################################################################
//####################################################################
//####################################################################
//####################################################################

int main(){
   srand(time(0));
   sf::ContextSettings settings;
   settings.antialiasingLevel = 8;

	cout<<"Display settings: "<< sf::VideoMode::getDesktopMode().width << ", " << sf::VideoMode::getDesktopMode().height<<endl;

   sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "My window",sf::Style::Default, settings);
   
   ReferenceFrame RF(0, WINDOW_HEIGHT, 10);

//   Line::set_reference_frame(&RF); 

   int nl=800;
   double lg=WINDOW_HEIGHT/(nl*1.0);
   sf::RectangleShape* GRID=new sf::RectangleShape[nl*nl];
   
 	Noise Ns(nl, nl, lg, lg);
   //Ns.reset_seeds(1, 3);
   
   auto start = chrono::high_resolution_clock::now();

	int max_threads=thread::hardware_concurrency();
	int max_op=150000;
	int spw=min(1+(nl*nl)/max_op, max_threads);	
	
	thread thd[spw];
	int h=0, k=nl*nl/spw;
	for(short c=0; c<spw; c++){
		thd[c]=thread(do_something, h, k, nl, lg, GRID, &Ns);		
		h=k;
		k=min(k+k, nl*nl);
	}

	for(short c=0; c<spw; c++){
		cout<<"joining thread "<<c<<endl;
		std::mem_fn(&std::thread::join)(thd[c]);
		cout<<"thread "<<c<<" joined"<<endl;
	}

	h=0, k=nl*nl/spw;
	for(short c=0; c<spw; c++){
		thd[c]=thread(do_more, h, k, GRID, &Ns);
		h=k;
		k=min(k+k, nl*nl);
	}

	for(short c=0; c<spw; c++){
		thd[c].join();
	}

	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	cout<<"Total Elapsed time: "<<microseconds/1000000.0<<endl;

   FrameRate Framerate(4);
   double t;
	while(window.isOpen()){
		sf::Event event;
      while(window.pollEvent(event))
      {
         if(event.type==sf::Event::Closed)
         {
            window.close();
         }
      }

      if(Framerate()>=0)//
      {
         Framerate.reset();

         window.clear(sf::Color::White);
        
         //################################################################
         //draw things here ###############################################
         for(int i=0; i<nl*nl; i++)
         {
            window.draw(GRID[i]);
         }
         // END of drawing ###############################################
         //###############################################################
         t=0;
         Framerate.reset();
      }
      window.display();
      
   }
   

   delete[] GRID;

   return 0;
}


