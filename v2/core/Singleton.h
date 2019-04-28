#ifndef DEF_SINGLETON
#define DEF_SINGLETON

template<typename T> class Singleton
{
public:
       
    static T* GetInstance(void)
    {
        if(singleton==0)
	    {
		singleton = new T;     
	    }
        return singleton;
    }
 
    static void Kill()
    {
        if(singleton!=0)
	    {
		delete singleton ;
		singleton=0;
	    }
    }
  
protected:
 
    Singleton()
    {
    }
 
    virtual ~Singleton()
    {
    }
    
    static T *singleton;  
};
 
template <typename T> T* Singleton<T>::singleton = 0;

#endif