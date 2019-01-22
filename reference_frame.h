#ifndef REF_FRAME_H
#define REF_FRAME_H

class ReferenceFrame
{
   private:
      double origen_x;
      double origen_y;
      double ration;
      const double px_per_cm;
      ReferenceFrame();	
      ReferenceFrame(const ReferenceFrame&);	
      ReferenceFrame& operator=(const ReferenceFrame& RF); 
      
   public:
   
   	//ReferenceFrame() = delete;
      ReferenceFrame(double x, double y, double size=0.0);
      double operator()(double z, char t);
      const double& rt() const;
};

//-------------------------------------------------------------------

inline const double& ReferenceFrame::rt() const{
   return ration;
}


//-------------------------------------------------------------------

#endif