#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

using namespace std;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("minecraft_dirt.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

// функция для расчёта нормали
double* Calculations_normals(double a[],double b[],double c[]) {
	//a = B - A
	//b = C - A
	int a_ms[2];
	int b_ms[2];

	a_ms[0] = b[0] - a[0];
	a_ms[1] = b[1] - a[1];
	a_ms[2] = b[2] - b[2];

	b_ms[0] = c[0] - a[0];
	b_ms[1] = c[1] - a[1];
	b_ms[2] = c[2] - a[2];

	// n_x = a(y)*b(z) - b(y)*a(z) = 7*3 - 7*0 = 21   / 21 = 1
	// n_y = -a(x)*b(z) + b(x)*a(z) = 0*3 + 0*0 = 0   / 21 = 0
	// n_z = a(x)*b(y) - b(x)*a(y) = 0*7 - 0*7 = 0    / 21 = 0
	double n_x_t = 0;
	double n_y_t = 0;
	double n_z_t = 0;

	double n_x = 0;
	double n_y = 0;
	double n_z = 0;

	double ms_rez[2];

	n_x_t = a[1] * b[2] - b[1] * a[2];
	n_y_t = a[0] * b[2] * (-1) + b[0] * a[2];
	n_z_t = a[0] * b[1] - b[0] * a[1];

	n_x = n_x_t / n_x_t;
	n_y = n_y_t / n_y_t;
	n_z = n_z_t / n_z_t;

	ms_rez[0] = n_x;
	ms_rez[1] = n_y;
	ms_rez[2] = n_z;

	return ms_rez;
}


void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
	//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	// 
	// рисунок фигуры
	glBegin(GL_LINES);

	glVertex3d(0, 0, 0);
	glVertex3d(8, 2, 0);

	glVertex3d(8, 2, 0);
	glVertex3d(12, -4, 0);

	glVertex3d(12, -4, 0);
	glVertex3d(10, 2, 0);

	glVertex3d(10, 2, 0);
	glVertex3d(17, 4, 0);

	glVertex3d(17, 4, 0);
	glVertex3d(11, 7, 0);

	glVertex3d(11, 7, 0);
	glVertex3d(8, 4, 0);

	glVertex3d(8, 4, 0);
	glVertex3d(0, 7, 0);

	glVertex3d(0, 7, 0);
	glVertex3d(0, 0, 0);
	glEnd();

	//создание стенок         вектор a = B - A = {0, 7, 0}, вектор b = C - A = {0, 7, 3}
	double A[] = { 0, 0, 0 }; // n_x = a(y)*b(z) - b(y)*a(z) = 7*3 - 7*0 = 21   / 21 = 1
	double B[] = { 0, 7, 0 }; // n_y = -a(x)*b(z) + b(x)*a(z) = 0*3 + 0*0 = 0   / 21 = 0
	double C[] = { 0, 7, 3 }; // n_z = a(x)*b(y) - b(x)*a(y) = 0*7 - 0*7 = 0    / 21 = 0
	double D[] = { 0, 0, 3 };

	double A1[] = { 0, 7, 0 };
	double B1[] = { 8, 4, 0 };
	double C1[] = { 8, 4, 3 };
	double D1[] = { 0, 7, 3 };

	double A2[] = { 8, 4, 0 };
	double B2[] = { 11, 7, 0 };
	double C2[] = { 11, 7, 3 };
	double D2[] = { 8, 4, 3 };

	double A3[] = { 11, 7, 0 };
	double B3[] = { 17, 4, 0 };
	double C3[] = { 17, 4, 3 };
	double D3[] = { 11, 7, 3 };

	double A4[] = { 17, 4, 0 };
	double B4[] = { 10, 2, 0 };
	double C4[] = { 10, 2, 3 };
	double D4[] = { 17, 4, 3 };

	double A5[] = { 10, 2, 0 };
	double B5[] = { 12, -4, 0 };
	double C5[] = { 12, -4, 3 };
	double D5[] = { 10, 2, 3 };

	double A6[] = { 12, -4, 0 };
	double B6[] = { 8, 2, 0 };
	double C6[] = { 8, 2, 3 };
	double D6[] = { 12, -4, 3 };

	double A7[] = { 8, 2, 0 };
	double B7[] = { 0, 0, 0 };
	double C7[] = { 0, 0, 3 };
	double D7[] = { 8, 2, 3 };

	// основание

	double A8[] = { 0, 0, 0 };
	double B8[] = { 0, 7, 0 };
	double C8[] = { 8, 4, 0 };
	double D8[] = { 8, 2, 0 };

	double A9[] = { 8, 4, 0 };
	double B9[] = { 10, 2, 0 };
	double C9[] = { 17, 4, 0 };
	double D9[] = { 11, 7, 0 };

	double A10[] = { 8, 4, 0 };
	double B10[] = { 8, 2, 0 };
	double C10[] = { 12, -4, 0 };
	double D10[] = { 10, 2, 0 };

	//крыша
	double A11[] = { 0, 0, 3 };
	double B11[] = { 0, 7, 3 };
	double C11[] = { 8, 4, 3 };
	double D11[] = { 8, 2, 3 };

	double A12[] = { 8, 4, 3 };
	double B12[] = { 10, 2, 3 };
	double C12[] = { 17, 4, 3 };
	double D12[] = { 11, 7, 3 };

	double A13[] = { 8, 4, 3 };
	double B13[] = { 8, 2, 3 };
	double C13[] = { 12, -4, 3 };
	double D13[] = { 10, 2, 3 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glBegin(GL_QUADS);

	//glColor3d(0.2, 0.7, 0.7);
	//самая левая плоскость рисунка, по G
	glNormal3d(-1, 0, 0); // n_x n_y n_z
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(B);
	glTexCoord2d(1, 1);
	glVertex3dv(C);
	glTexCoord2d(1, 0);
	glVertex3dv(D);

	//glColor3d(0.2, 0.7, 0.7);
	//след грань
	//const int size = 2;
	//double* ms1[size] = { Calculations_normals(A1, B1, C1) };
	//int i_ms0 = int(ms1[0]);
	//int i_ms1 = int(ms1[1]);
	//int i_ms2 = int(ms1[2]);

	glNormal3d(0, 1, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(A1);
	glTexCoord2d(0, 1);
	glVertex3dv(B1);
	glTexCoord2d(1, 1);
	glVertex3dv(C1);
	glTexCoord2d(1, 0);
	glVertex3dv(D1);

	//glColor3d(0.2, 0.7, 0.7);
	//след грань
	glNormal3d(0, 1, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(A2);
	glTexCoord2d(0, 1);
	glVertex3dv(B2);
	glTexCoord2d(1, 1);
	glVertex3dv(C2);
	glTexCoord2d(1, 0);
	glVertex3dv(D2);

	//glColor3d(0.2, 0.7, 0.7);
	//след грань
	glNormal3d(0, 1, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(A3);
	glTexCoord2d(0, 1);
	glVertex3dv(B3);
	glTexCoord2d(1, 1);
	glVertex3dv(C3);
	glTexCoord2d(1, 0);
	glVertex3dv(D3);

	//glColor3d(0.2, 0.7, 0.7);
	//след грань
	glNormal3d(0, -1, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(A4);
	glTexCoord2d(0, 1);
	glVertex3dv(B4);
	glTexCoord2d(1, 1);
	glVertex3dv(C4);
	glTexCoord2d(1, 0);
	glVertex3dv(D4);

	//glColor3d(0.2, 0.7, 0.7);
	//след грань
	glNormal3d(1, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(A5);
	glTexCoord2d(0, 1);
	glVertex3dv(B5);
	glTexCoord2d(1, 1);
	glVertex3dv(C5);
	glTexCoord2d(1, 0);
	glVertex3dv(D5);

	//glColor3d(0.5, 0.7, 0.7);
	//след грань
	glNormal3d(0, -1, 0); // поправить
	glTexCoord2d(0, 0);
	glVertex3dv(A6);
	glTexCoord2d(0, 1);
	glVertex3dv(B6);
	glTexCoord2d(1, 1);
	glVertex3dv(C6);
	glTexCoord2d(1, 0);
	glVertex3dv(D6);

	//glColor3d(0.5, 0.7, 0.7);
	//след грань
	glNormal3d(0, -1, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(A7);
	glTexCoord2d(0, 1);
	glVertex3dv(B7);
	glTexCoord2d(1, 1);
	glVertex3dv(C7);
	glTexCoord2d(1, 0);
	glVertex3dv(D7);

	//основание
	//glColor3d(0.5, 0.3, 0.9);
	//след грань
	glNormal3d(0, 0, -1);
	glTexCoord2d(0, 0);
	glVertex3dv(A8);
	glTexCoord2d(0, 1);
	glVertex3dv(B8);
	glTexCoord2d(1, 1);
	glVertex3dv(C8);
	glTexCoord2d(1, 0);
	glVertex3dv(D8);

	//glColor3d(0.5, 0.3, 0.9);
	//след грань
	glNormal3d(0, 0, -1);
	glTexCoord2d(0, 0);
	glVertex3dv(A9);
	glTexCoord2d(0, 1);
	glVertex3dv(B9);
	glTexCoord2d(1, 1);
	glVertex3dv(C9);
	glTexCoord2d(1, 0);
	glVertex3dv(D9);

	//glColor3d(0.5, 0.3, 0.9);
	//след грань
	glNormal3d(0, 0, -1);
	glTexCoord2d(0, 0);
	glVertex3dv(A10);
	glTexCoord2d(1, 0);
	glVertex3dv(B10);
	glTexCoord2d(1, 1);
	glVertex3dv(C10);
	glTexCoord2d(1, 0);
	glVertex3dv(D10);

	//крыша
	//glColor3d(0.5, 0.3, 0.9);
	//след грань
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex3dv(A11);
	glTexCoord2d(1, 0);
	glVertex3dv(B11);
	glTexCoord2d(1, 1);
	glVertex3dv(C11);
	glTexCoord2d(1, 0);
	glVertex3dv(D11);

	//glColor3d(0.5, 0.3, 0.9);
	//след грань
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex3dv(A12);
	glTexCoord2d(1, 0);
	glVertex3dv(B12);
	glTexCoord2d(1, 1);
	glVertex3dv(C12);
	glTexCoord2d(1, 0);
	glVertex3dv(D12);

	//glColor3d(0.5, 0.3, 0.9);
	//след грань
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex3dv(A13);
	glTexCoord2d(1, 0);
	glVertex3dv(B13);
	glTexCoord2d(1, 1);
	glVertex3dv(C13);
	glTexCoord2d(1, 0);
	glVertex3dv(D13);

	//glBindTexture(GL_TEXTURE_2D, texId);

	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);

	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);

	glEnd();
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}