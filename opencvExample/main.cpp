#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>



using namespace cv;
using namespace std;


//-------Function Define---------------------
static void read_imgList(const string& filename, vector<Mat>& images);


// ======================Example And testing======================
#define example_tag 3 //a b c
#define Total_ConuterOfTest 16 "8+4+4"
int _testSymbol[3]={8,4,4};
unsigned char testSquence[16]={0,0,0,1,0,0,0,0,1,1,1,1,2,2,2,2};
//================================================================

//#define CC_interval 256 // for graylevel image we have 0~255 256 Interval to define they region for arithmetic coding
#define CC_interval 3
class ArithmeticCoding
{
public:
    //---------public region using Coding and Decoding File Propertype...
    fstream fp;
    
    FILE *Readfp;

    int endOfDecoding=0;
    Mat decodingImage;
    
    //---------public region Using Coding initial Data-------------
    unsigned long _TotalCounter=0;  //Total Number of Coding
    unsigned long _CC[CC_interval*2]={0}; //For Image comparess ,Ex: if Image Pixel is 20x20 then CC=20x20-1=309
    unsigned long _maxUPBound; // For Image comparss using World_length=totalCounterx4,World_length>=2^m
    unsigned long _symbolConunter[CC_interval]={0};
    unsigned char _symbolArray[10304]={0};
    unsigned long _CounterSymbol=0;
    unsigned int  status=0;
    unsigned long E3_counter=0;
    unsigned int shiftMax=0;
    unsigned int E3_status=0;
    //==========public region Using decoding initial Data==============
    
    
    //--------------------------------------------------------------
    ArithmeticCoding(){}
    
    ArithmeticCoding(long totalCounter)
    {
        long World_length=totalCounter*4;
        _TotalCounter=totalCounter;   // Saving the Total Counter of arithmic coding...
        unsigned int i=0;
        while (1)  // Calaulate the Max Up Boundary...
        {
            if(pow(2, i)>=World_length)
            {
                _maxUPBound=pow(2, i)-1;
                break;
            }else i++;
        }
        shiftMax=i;
        shiftMax--;
        printf("shift_max=%d\n",shiftMax);
        
    }
    //This Function is calculate the each symbol interval ....
    bool Counter_Character＿interval(unsigned char symbol)
    {
        
        _symbolConunter[symbol]++;
        _symbolArray[_CounterSymbol++]=symbol;
        printf("key=%5d %5d ",symbol,_CounterSymbol);
        if(_CounterSymbol==_TotalCounter)
        {
            _CounterSymbol=0;
            if(1==(settingEachSymbolInterval()))
            {
                status=Coding();
                
            }else
            {
                printf("Setting Interval Fail......\n");
            }
        }
        return 1;
    }
    // This function setting the each syboml ArithmicCoding UpBoundary and LowBandary...
    int settingEachSymbolInterval()
    {
        int i=0,j=0;
        
        unsigned long tmp=0;
        try
        {
            for(i=0;i<CC_interval;i++)
            {
                if(0==i)
                {
                    _CC[0]=0;    // Saving the low Boundary...
                    _CC[1]=_symbolConunter[i];
                    tmp=_symbolConunter[i];
                    j=2;
                }
                else
                {
                    _CC[j++]=tmp;
                    tmp+=_symbolConunter[i];
                    _CC[j++]=tmp;
                    
                }
                
                
            }
            printf("Interval:\n");
            for(i=0;i<j;i++)
            {
                
                printf("%d %ld %d \n",i,_CC[i],_symbolConunter[i/2]);
            }
            
        } catch (const std::out_of_range& oor) {
            printf("arrary is out of range....\n");
        }
        
        return 1;
    }
    // This function is coding the each syboml
    //
    //
    int Coding()
    {
        unsigned long UpBoundary=_maxUPBound,LowBoundary=0,tempx=0,tempy=0;
        int codingWrold,counter=0;
        for (int i=0; i<_TotalCounter;i++)
        {
            codingWrold=_symbolArray[i];
            tempx=LowBoundary;
            tempy=UpBoundary;
            //printf("input data %3d =[%d] CC,l=%d CC,u=%d\n",i,codingWrold,_CC[codingWrold*2],_CC[codingWrold*2+1]);
            LowBoundary=tempx+(unsigned long)(tempy-tempx+1)*((double)_CC[codingWrold*2]/(double)_TotalCounter);
            UpBoundary=tempx+(unsigned long)(tempy-tempx+1)*((double)_CC[codingWrold*2+1]/(double)_TotalCounter)-1;
            //LowBoundary=LowBoundary+(((double)_CC[codingWrold*2]*_maxUPBound)/_TotalCounter);
            //LowBoundary=LowBoundary+(((double)_CC[codingWrold*2+1]*_maxUPBound)/_TotalCounter)-1;
            while((GetMSB(UpBoundary)==GetMSB(LowBoundary))|| E3_mapping(LowBoundary,UpBoundary))
            {
                //printf("L:%3ld U:%3ld\n",LowBoundary,UpBoundary);
                if(GetMSB(UpBoundary)==GetMSB(LowBoundary))
                {
                    //printf("E1,2\n");
                    send(GetMSB(UpBoundary));
                    UpBoundary<<=1;
                    UpBoundary=UpBoundary & _maxUPBound;
                    UpBoundary+=0x01;
                    LowBoundary<<=1;
                    LowBoundary=LowBoundary & _maxUPBound;
                    while (E3_counter>0)
                    {
                        printf("E3=%d !E3=%d\n",UpBoundary,!GetMSB(UpBoundary));
                        send(!GetMSB(UpBoundary));
                        E3_counter--;
                    }
                }
                if(E3_mapping(LowBoundary, UpBoundary))
                {
                    UpBoundary<<=1;
                    UpBoundary=UpBoundary & _maxUPBound;
                    LowBoundary<<=1;
                    LowBoundary=LowBoundary & _maxUPBound;
                    UpBoundary++;
                    // Complement MSB(l,u)
                    UpBoundary ^= (0x01<<shiftMax);
                    LowBoundary ^= (0x01<<shiftMax);
                    E3_status=0;
                }
                
            }
            //test(32);
            //printf("After L:%3ld U:%3ld %ld\n",LowBoundary,UpBoundary,GetMSB(UpBoundary));
            //UpBoundary=LowBoundary+(unsigned long)()
        }
         fp<<"\n";  //Write '\n' mean coding is end...
        fp.close();
        return 1;
    }
    // parameter 1. Is Decoding File Full Path..
    //
    int Decoding(char *filename)
    {
        unsigned char readByte;
        unsigned long numberReadPos=0;
        unsigned long tag=0,index=0,tag_1=0;
        unsigned long LowBandary=0;
        unsigned long UpBandary=_maxUPBound-1;
        unsigned long tempx=0,tempy=0;
        
       
        fpos_t pos;
        printf("\nStart Decoding of ArimicCode....\n");
        Readfp=fopen(filename,"rb");
        unsigned long decode=0;
        

        pos=numberReadPos;
        for(int i=0;i<=shiftMax;i++)
        {
            numberReadPos++;
            fsetpos(Readfp, &pos);
            fread(&readByte, sizeof(unsigned char), 1, Readfp);
            switch (readByte) {
                case '0':
                    tag<<=1;
                    tag&=_maxUPBound;
                    //printf("0");
                    break;
                case '1':
                    tag<<=1;
                    tag&=_maxUPBound;
                    tag++;
                    //printf("1");
                    break;
                case '\n':
                    //printf("end of Read file");
                    endOfDecoding=1;
                    break;
                default:
                        
                    break;
            }
            printf("%lx \n",tag);
            if(endOfDecoding)
                break;
            pos=numberReadPos;
        }
        tag_1=(unsigned long)((double)((((tag-LowBandary+1)*_TotalCounter)-1)/(double)(UpBandary-LowBandary+1)));
        
        printf("First decode:%d\n",DecodingOfinterval(tag_1));
            
        index=DecodingOfinterval(tag);
        
        tempx=LowBandary;
        tempy=UpBandary;
            
        //if(endOfDecoding)
          //  break;
            
        while(1)
        {
            LowBandary=tempx+(unsigned long)((double)(_maxUPBound*_CC[index])/(double)(_TotalCounter));
            UpBandary=tempx+(unsigned long)((double)(_maxUPBound*_CC[index+1])/(double)(_TotalCounter));
            while((GetMSB(LowBandary)==GetMSB(UpBandary))|| E3_mapping(LowBandary,UpBandary))
            {
                //printf("L:%3ld U:%3ld\n",LowBoundary,UpBoundary);
                if(GetMSB(LowBandary)==GetMSB(UpBandary))   // E1 or E2 mapping sucess....
                {
                    //printf("E1,2\n");
                    printf("E1 E2 low:%ld up:%ld t:%ld",LowBandary,UpBandary,tag);
                    UpBandary<<=1;
                    UpBandary=UpBandary & _maxUPBound;
                    UpBandary+=0x01;
                    LowBandary<<=1;
                    LowBandary=tempx & _maxUPBound;
                    tag<<=1;
                    tag=tag&_maxUPBound;
                    pos=numberReadPos;
                    tag=addNextBit(tag,pos);
                    if(endOfDecoding)
                        break;
                    numberReadPos++;
                    printf("After E1 E2 low:%ld up:%ld t:%ld",LowBandary,UpBandary,tag);
                }
                if(E3_mapping(LowBandary,UpBandary))
                {
                    printf("E3 low:%ld up:%ld t:%ld",LowBandary,UpBandary,tag);
                    UpBandary<<=1;
                    UpBandary=UpBandary & _maxUPBound;
                    UpBandary+=0x01;
                    LowBandary<<=1;
                    LowBandary=tempx & _maxUPBound;
                    tag<<=1;
                    tag=tag & _maxUPBound;
                    
                    pos=numberReadPos;
                    tag=addNextBit(tag,pos);
                    if(endOfDecoding)
                        break;
                    numberReadPos++;
                    UpBandary ^= (0x01<<shiftMax);
                    LowBandary ^= (0x01<<shiftMax);
                    tag^= (0x01<<shiftMax);
                    //E3_status=0;
                }

            
            }
            tag_1=(unsigned long)((double)((((tag-LowBandary+1)*_TotalCounter)-1)/(double)(UpBandary-LowBandary+1)));
            printf("tagIs=%d\n");
            printf("decode:%d\n",DecodingOfinterval(tag_1));
        }
        
        
        return 1;
    }
    
    int DecodingOfinterval(unsigned  long tag)
    {
        int IntervalIndex=0;
        for(int i=0;i<_TotalCounter*2;i+=2)
        {
            printf("tag=[%d],_CC[%d]=%d,_CC[%d]=%d\n",tag,i,_CC[i],i+1,_CC[i+1]);

            if(tag>=_CC[i] && tag <_CC[i+1])
            {
                IntervalIndex=i;
                printf("search index=%d\n",i);
                break;
            }
        }
        return IntervalIndex/2;
    }
    
    unsigned long addNextBit(unsigned long tag,fpos_t _pos)
    {
        unsigned char readByte;
        fsetpos(Readfp, &_pos);
        fread(&readByte, sizeof(unsigned char), 1, Readfp);
        switch (readByte) {
            case '0':
                tag<<=1;
                tag&=_maxUPBound;
                //printf("0");
                break;
            case '1':
                tag<<=1;
                tag&=_maxUPBound;
                tag++;
                //printf("1");
                break;
            case '\n':
                //printf("end of Read file");
                endOfDecoding=1;
                break;
            default:
                
                break;
        }

        return tag;
    }
    // --------Setting coding File------
    int settingTheFilePath(char *filename)
    {
        fp.open(filename, ios::out);   //open File
        if(!fp){//如果開啟檔案失敗，fp為0；成功，fp為非0
            cout<<"Fail to open file: "<<filename<<endl;
        }
        cout<<"File Descriptor: "<<&fp<<endl;
        
        return 1;
    }
    
    //---------private region -------------
private:
    unsigned long GetMSB(unsigned long syboml)
    {
        unsigned long x= ((syboml>>shiftMax)&0x01);
        // printf("MSB=%ld\n",x);
        return x ;
    }
    

    
    int E3_mapping(unsigned long x, unsigned long y)
    {
        int x1=0,y1=0;
        x1=(x>>(shiftMax-1)) & 0x01;
        y1=(y>>(shiftMax-1)) & 0x01;
        if((1==x1)&&(0==y1))
        {
            
            //printf("E3 true\n");
            if(E3_status==0)
            {
                E3_counter++;
                E3_status=1;
            }
            return 1;
        }
        return 0;
    }
    
    void send(unsigned long x)
    {
        if(x==0)
        {
            //printf("Sned 0\n");
            printf("0");
            fp<<"0";//寫入字串
        }
        else
        {
            //printf("Send 1\n");
            printf("1");
            fp<<"1";//寫入字串
        }
    }
    
    void  test(unsigned long x)
    {
        
        printf("%ld",GetMSB(x));
    }
    
};



// Clsss To Temp Image
class ImageClass
{
public:
    Mat img;
    unsigned char ImageData[6]={0,2,111,243,9,10};
    unsigned char *ptr;  //Saving the Image data Point...
    unsigned int Rows;
    unsigned int Cols;
    unsigned int Total_Pixel_counter;
    ImageClass(){}
    ImageClass(Mat Img)
    {
        img=Img;
        Rows=img.rows;
        Cols=img.cols;
        Total_Pixel_counter=Rows*Cols;  //calaues the total pixel in this image...
        
    }
    void Initial(Mat Img)
    {
        img=Img;
        Rows=img.rows;
        Cols=img.cols;
        Total_Pixel_counter=Rows*Cols;
    }
    void dataCatchFromImageTo()
    {
        
        unsigned char imageDataSquence[img.rows*img.cols];
        for (int i=0; i<img.rows;i++)
        {
            for(int j=0;j<img.cols;j++)
            {
                imageDataSquence[(i*j)+j]=img.at<uchar>(j,i);
            }
        }
        ptr=ImageData;
        printf("totoal %3d %3d\n",Rows,Cols);
        for (int i=0; i<img.rows*img.cols; i++)
        {
            printf("ptr[%i]=%3d ",i,*(ptr+i));
        }
       
    
    }
    
    int startArithmeticCodingCoding()
    {
        ArithmeticCoding coding(Total_Pixel_counter);
        coding.settingTheFilePath("/Users/latdmc/Documents/text.txt");
        for(int i=0;i<Total_Pixel_counter;i++)
        {
            coding.Counter_Character＿interval(*(ptr+i));
            
        }
        coding.Coding();
        coding .Decoding("/Users/latdmc/Documents/text.txt");
        return 1;
    
    }
private:
    
    
    
    
};


static  Mat formatImagesForPCA(const vector<Mat> &data)
{
    Mat dst(static_cast<int>(data.size()), data[0].rows*data[0].cols, CV_32F);
    for(unsigned int i = 0; i < data.size(); i++)
    {
        Mat image_row = data[i].clone().reshape(1,1);
        Mat row_i = dst.row(i);
        image_row.convertTo(row_i,CV_32F);
    }
    return dst;
}

static Mat toGrayscale(InputArray _src) {
    Mat src = _src.getMat();
    // only allow one channel
    if(src.channels() != 1) {
        CV_Error(Error::StsBadArg, "Only Matrices with one channel are supported");
    }
    // create and return normalized image
    Mat dst;
    cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
    return dst;
}

struct params
{
    Mat data;
    int ch;
    int rows;
    PCA pca;
    string winName;
};

    ImageClass _imageClass;

static void onTrackbar(int pos, void* ptr)
{

    //cout << "Retained Variance = " << pos << "%   ";
    //cout << "re-calculating PCA..." << std::flush;
    
    double var = pos / 100.0;
    
    struct params *p = (struct params *)ptr;
    
    p->pca = PCA(p->data, cv::Mat(), PCA::DATA_AS_ROW, var);
    cout << p->pca.eigenvalues << endl;
    Mat point = p->pca.project(p->data.row(0));
    Mat reconstruction = p->pca.backProject(point);
    reconstruction = reconstruction.reshape(p->ch, p->rows);
    reconstruction = toGrayscale(reconstruction);
    //point=point.reshape(p->ch, p->rows);
    //point=point.reshape(p->ch, p->rows);
    imshow("testing",point);
    //cout << p->ch << endl;
    imshow(p->winName, reconstruction);
    //----Start coding----
    //_imageClass.Initial(reconstruction);
    //_imageClass.dataCatchFromImageTo();
    //_imageClass.startArithmeticCodingCoding();
    //imshow("ClassImage",_imageClass.img);
    //-----end of coding---
    cout << "done!   # of principal components: " << p->pca.eigenvectors.rows << endl;
}






///////////////////////
// Main
int main(int argc, char** argv)
{
    
    string imgList = "/Users/latdmc/Downloads/att_faces/s1/1.pgm /Users/latdmc/Downloads/att_faces/s1/1.pgm ";
    // vector to hold the images
    vector<Mat> images;
  
    // Read in the data. This can fail if not valid
    try {
        read_imgList(imgList, images);
    } catch (cv::Exception& e) {
        cerr << "Error opening file \"" << imgList << "\". Reason: " << e.msg << endl;
        exit(1);
    }
    
    // Quit if there are not enough images for this demo.
    if(images.size() <= 1) {
        string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
        CV_Error(Error::StsError, error_message);
    }
    
    // Reshape and stack images into a rowMatrix
    Mat data = formatImagesForPCA(images);
    
    // perform PCA
    PCA pca(data, cv::Mat(), PCA::DATA_AS_ROW, 0.95); // trackbar is initially set here, also this is a common value for retainedVariance
    
    // Demonstration of the effect of retainedVariance on the first image
    Mat point = pca.project(data.row(0)); // project into the eigenspace, thus the image becomes a "point"
    Mat reconstruction = pca.backProject(point); // re-create the image from the "point"
    reconstruction = reconstruction.reshape(images[0].channels(), images[0].rows); // reshape from a row vector into image shape
    reconstruction = toGrayscale(reconstruction); // re-scale for displaying purposes
    
    // init highgui window
    string winName = "Reconstruction | press 'q' to quit";
    namedWindow(winName, WINDOW_NORMAL);
    
    // params struct to pass to the trackbar handler
    params p;
    p.data = data;
    p.ch = images[0].channels();
    p.rows = images[0].rows;
    p.pca = pca;
    p.winName = winName;
    
    
    
    //printf("ttt %d ttt\n",(unsigned long)((double)((((196-0+1)*50)-1)/(double)(255-0+1))));
    
    ArithmeticCoding x(16);
    x.settingTheFilePath("/Users/latdmc/Documents/text.txt");
    for (int i=0;i<16;i++)
    {
        x.Counter_Character＿interval(testSquence[i]);
    }
    x.Decoding("/Users/latdmc/Documents/text.txt");
    
    // create the tracbar
    int pos = 95;
    createTrackbar("Retained Variance (%)", winName, &pos, 100, onTrackbar, (void*)&p);
    
    // display until user presses q
    imshow(winName, reconstruction);
    
    int key = 0;
    while(key != 'q')
        key = waitKey();
    
    return 0;
}



///////////////////////
// Functions  loading eginVector Image
static void read_imgList(const string& filename, vector<Mat>& images) {
    /*
     std::ifstream file(filename.c_str(), ifstream::in);
     if (!file) {
     string error_message = "No valid input file was given, please check the given filename.";
     CV_Error(Error::StsBadArg, error_message);
     }*/
    string line;
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s1/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s2/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s3/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s4/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s5/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s6/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s7/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s8/9.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/1.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/2.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/3.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/4.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/5.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/6.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/7.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/8.pgm", 0));
    images.push_back(imread("/Users/latdmc/Downloads/att_faces/s9/9.pgm", 0));
    
}



/*

void tt()
{
    int nEigens=1;
    CvTermCriteria calcLimit = cvTermCriteria( CV_TERMCRIT_ITER, nEigens, 1);
    
    // Compute average image, eigenvectors (eigenfaces) and eigenvalues (ratios).
    cvCalcEigenObjects(nTrainFaces, (void*)faceImgArr, (void*)eigenVectArr,
                       CV_EIGOBJ_NO_CALLBACK, 0, 0, &calcLimit,
                       pAvgTrainImg, eigenValMat->data.fl);
    
    // Normalize the matrix of eigenvalues.
    cvNormalize(eigenValMat, eigenValMat, 1, 0, CV_L1, 0);
    
    // Project each training image onto the PCA subspace.
    CvMat projectedTrainFaceMat = cvCreateMat( nTrainFaces, nEigens, CV_32FC1 );
    int offset = projectedTrainFaceMat->step / sizeof(float);
    for(int i=0; i<nTrainFaces; i++) {
        cvEigenDecomposite(faceImgArr[i], nEigens, eigenVectArr, 0, 0,
                           pAvgTrainImg, projectedTrainFaceMat->data.fl + i*offset);
}*/
