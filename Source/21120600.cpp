#include<iostream>
#include<fstream>
using namespace std;

//cấu trúc phần mở đầu file BMP
struct infomation{
	//header
	int size;								//kích thước toàn bộ file
	int reserve;						//byte dành riêng
	int address;						//địa chỉ bắt đầu lưu dữ liệu điểm ảnh
	
	//DIB infomation
	int DIBsize;						//kích thước DIB
	int width;							//số pixel chiều rộng	
	int height;							//số pixel chiều cao
	short planes;						//số lớp màu
	short bpp;							//số bit trên mỗi pixel
	int compression;				//cách nén ảnh
	int imagesize;					//kích thước phần dữ liệu điểm ảnh
	int Xpixel;							//độ phân giải theo phương ngang
	int Ypixel;							//độ phân giải theo phương dọc
	int ColorUsed;					//các màu được sử dụng
	int ImpColor;					//các màu quan trọng
};

//cấu trúc của một pixel 24 bit (3 byte)
struct pixel24{
	unsigned char B;	
	unsigned char G;
	unsigned char R;
};

//cấu trúc của một pixel 32 bit (4 byte)
struct pixel32{
	unsigned char A;
	unsigned char B;
	unsigned char G;
	unsigned char R;
};

//cấu trúc của toàn bộ ảnh BMP
struct image{
	unsigned char sign[2];				//chữ kí file
	infomation info;						//thông tin phần header và DIB
	char* color;							//thông tin còn lại của phần DIB (chứa bảng màu)
	char* data_image;					//thông tin phần dữ liệu điểm ảnh
};

//đọc file BMP		
int DocFile(char* filename, image*& a){
	a=new image;
	int address,imageSize;
	//mở file
	ifstream fin(filename, ios::binary);
	//kiểm tra tình trạng mở file
	if(!fin) return 0;
	//đọc các thông tin của file ảnh
	fin.read((char*)&a->sign,2);
	fin.read((char*)&a->info,52);
	address=a->info.address;
	imageSize=a->info.imagesize;
	//kiểm tra định dạng file
	if(a->sign[0]!='B' || a->sign[1]!='M') return 0;
	//đọc phần còn lại của DIB (chứa bảng màu)
	a->color=new char[address-54];
	fin.read(a->color,address-54);
	//đọc phần pixel data(dữ liệu điểm ảnh)
	a->data_image=new char[imageSize];
	fin.read(a->data_image,imageSize);
	//đóng file
	fin.close();
	return 1;
}

//ghi file BMP		
int GhiFile( char* filename, image* a){			
	int address,imageSize;
	address=a->info.address;
	imageSize=a->info.imagesize;
	//mở file để ghi thông tin
	ofstream fout(filename, ios::binary);
	//kiểm tra việc mở file
	if(!fout) return 0;
	//kiểm tra định dạng file BMP
	if(a->sign[0]!='B' || a->sign[1]!='M') return 0;
	//ghi phần chữ kí file
	fout.write((char*)&a->sign,2); 
	//ghi phần thông tin của ảnh
	fout.write((char*)&a->info,52);
	//ghi phần còn lại của  file DIB(chứa bảng màu)
	fout.write(a->color,address-54);
	//ghi phần dữ liệu điểm ảnh
	fout.write(a->data_image,imageSize);
	//đóng file
	fout.close();
	//giải phóng vùng nhớ của con trỏ chứa dữ liệu điểm ảnh và phần dư của DIB
	delete[] a->data_image;
	delete[] a->color;
	return 1;
}

//chuyển từ file 24/32 bit sang file 8 bit
image* Convert(image* a){
	//tạo một con trỏ image để lưu kết quả 
	image* out=new image;
	*out=*a;
	//tạo bảng màu cho ảnh 8 bit
	pixel32* ColorTable=new pixel32[256];
	for (int i=0; i<256; i++){
        ColorTable[i].R = i;
        ColorTable[i].G = i;
        ColorTable[i].B = i;
        ColorTable[i].A = i;
	}
	//lưu lại bảng màu vào phần kết quả 
	out->color=(char*)ColorTable;
	//lưu các thông tin cần để chuyển đổi
	int width,height,bpp,t=0;
	char ave=0;
		width=a->info.width;
		height=a->info.height;
		bpp=a->info.bpp;
	int d=bpp/8;
	//tính padding byte trước khi chuyển đổi
	int padding=(a->info.imagesize-d*width*height)/height;
	//tính padding byte sau khi chuyển đổi
	int afpadding;
	if(width%4==0) afpadding=0;
		else afpadding=4-width%4;
	//tạo một vùng nhớ mới để lưu lại phần tính toán của dữ liệu điểm ảnh
	out->data_image=new char[(width+afpadding)*height];
	//xét trường hợp  24 bit
	if(bpp==24){
		pixel24 pi;
		for(int i=0;i<height;++i){
			for(int j=0;j<3*width;j+=3){
				pi.B=a->data_image[i*(width*d+padding)+j];
				pi.G=a->data_image[i*(width*d+padding)+j+1];
				pi.R=a->data_image[i*(width*d+padding)+j+2];
				ave=(pi.B+pi.R+pi.G)/3;		
				out->data_image[t]=ave;
				++t;
			}
			//chèn padding byte sau khi đã chuyển đổi
			while(t<(i+1)*(width+afpadding)){
				out->data_image[t]=0;
				++t;		
			}
		}
	}
	//xét trường hợp 32 bit
	if(bpp==32){
		pixel32 pi;
		for(int i=0;i<height;++i){
			for(int j=0;j<4*width;j+=4){
				pi.B=a->data_image[i*(width*4+padding)+j+1];
				pi.G=a->data_image[i*(width*4+padding)+j+2];
				pi.R=a->data_image[i*(width*4+padding)+j+3];
				ave=(pi.B+pi.G+pi.R)/3;		
				out->data_image[t]=ave;
				++t;
			}
			//chèn padding byte sau khi chuyển đổi
			while(t<(i+1)*(width+afpadding)){
				out->data_image[t]=0;
				++t;
			}
		}
	}
	//cập nhật lại thông tin của file 8 
	out->info.address=54+256*4;
	out->info.DIBsize=40;
	out->info.imagesize=(width+afpadding)*height;
	out->info.bpp=8;
	return out;
}


//zoom nhỏ ảnh tỉ lệ 1/f 
image* Zoom(image* a){
	//nhập vào tỉ lệ muốn zoom
	int f;
	cout<<"ban muon thu nho lai bao nhieu lan: ";
	cin>>f;
	//tạo con trỏ image mới để lưu kết quả trả về
	image* out=new image;
	*out=*a;
	//lưu các thông số trước và sau khi zoom
	int height,bpp,d,width,afpadding,padding,afheight,afwidth,imagesize,afimagesize;
		height=a->info.height;
		width=a->info.width;
		bpp=a->info.bpp;
		d=bpp/8;
		imagesize=a->info.imagesize;
		padding=imagesize/height-width*d;
		afheight=(height%f==0)?(height/f):(height/f+1);
		afwidth=(width%f==0)?(width/f):(width/f+1);
		afpadding=(4-afwidth%4)%4;
		afimagesize=(afwidth*d+afpadding)*afheight;
	//số pixel thừa theo chiều ngang và chiều dọc khi chia theo tỉ lệ f
	int Xdu,Ydu;
	Xdu=width%f;
	Ydu=height%f;
	int i,j,t,jj,ii,tam=0,count;
	char s;
	//cấp phát vùng nhớ mới để lưu phần dữ liệu điểm ảnh sau khi zoom
	out->data_image=NULL;
	out->data_image=new char[afimagesize];
	//xét trường hợp thường (số ô cần chia nhỏ là fxf)
	for(j=0;j<(height-height%f);j+=f){
		//trường hợp có fxf pixel
		for(i=0;i<width*d-(width%f)*d;i+=f*d)
			for(t=0;t<d;++t){
				out->data_image[tam]=a->data_image[(j+f-1)*(imagesize/height)+i+(f-1)*d+t];
				++tam;		
			}
		//trường hợp số pixel theo chiều ngang  bị thiếu khi chia theo tỉ lệ f 
		if(Xdu!=0){
			for(t=0;t<d;++t){
				out->data_image[tam]=a->data_image[(j+f-1)*(imagesize/height)+i+(Xdu-1)*d+t];
				++tam;
			}
		}
		
		//thêm padding byte cho mỗi dòng đang xét
		count=0;
			while(count<afpadding){
				out->data_image[tam]=0;
					++count;
				++tam;
			}
	}
	//xét trường hợp số pixel theo chiều dọc bị thiếu khi chia theo tỉ lệ f
	if(Ydu!=0){
		for(i=0;i<width*d-(width%f)*d;i+=f*d) 
			for(t=0;t<d;++t){
				out->data_image[tam]=a->data_image[(j+Ydu-1)*(imagesize/height)+i+(f-1)*d+t];
				++tam;
			}
	
		//xét trường hợp bị thiếu cả số pixel theo chiều ngang và cả chiều dọc
		if(Xdu!=0){
			for(t=0;t<d;++t){
				out->data_image[tam]=a->data_image[(j+Ydu-1)*(imagesize/height)+i+(Xdu-1)*d+t];
				++tam;
			}
		}
		//thêm padding byte
		count=0;
		while(count<afpadding){
			out->data_image[tam]=0;
			++count;
			++tam;
		}
	}
	//cập nhật lại thông tin cho kết quả trả về
	out->info.imagesize=afimagesize;
	out->info.height=afheight;
	out->info.width=afwidth;
	out->info.size=a->info.size-(imagesize-afimagesize);
	return out;
}


int main(int argc, char* argv[]){
	image* a;
	image* out;
	if(argc!=4){	
		cout<<"Error!!!";
		return 0;
	}
	else{
		int i=DocFile(argv[2],a);
		if(argv[1]==string("-conv"))
			out=Convert(a);
		else if(argv[1]==string("-zoom"))
			out=Zoom(a);
			else{
				cout<<"Error!!!";
				return 0;
			}
		int j=GhiFile(argv[3],out);
		cout<<"xong"<<endl;
	}
	return 0;
}