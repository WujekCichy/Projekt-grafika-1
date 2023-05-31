/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <vector>
#include <windows.h>
#include <time.h>
#include <iostream>
using namespace std;

float speed = PI; //Prędkość kątowa obrotu obiektu
//int direction = 0;
//glm::mat4 snake[30];
//int points = 0;
const int grid_size = 22;
glm::mat4 M2 = glm::mat4(1.0f);
glm::mat4 edges[grid_size * 8];
glm::mat4 field = glm::mat4(1.0f);

char grid[grid_size][grid_size];
int x_obstacle, y_obstacle, x_head, y_head, x_apple, y_apple,snake_a , snake_b;
int direction = 0;
// 4-UP, 1-RIGHT, 2-DOWN,3-LEFT
vector <pair<int, int>> snake;
bool found_apple = 0;//0 when not found


void generate_apple()
{
    x_apple = (rand() % (grid_size - 2)) + 1;
    y_apple = (rand() % (grid_size - 2)) + 1;
    while (grid[x_apple][y_apple] != ' ')
    {
        x_apple = (rand() % (grid_size - 2)) + 1;
        y_apple = (rand() % (grid_size - 2)) + 1;
    }
    grid[x_apple][y_apple] = '+';
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_LEFT) {
            direction = 4;
            
        }
        if (key == GLFW_KEY_RIGHT) {
            direction = 2;
            
        }
        if (key == GLFW_KEY_UP) {   
            direction = 1;
            
        }
        if (key == GLFW_KEY_DOWN) { 
            direction = 3; 
            
        }
    }
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    M2 = glm::scale(M2, glm::vec3(0.5f, 0.5f, 0.5f));//przeskalowanie segmentu weza do mniejszego rozmiaru
    M2 = glm::translate(M2, glm::vec3(0.0f, 0.0f, -5.0f));//przesuniecie weza "w gore"

    field = glm::scale(field, glm::vec3(12.0f, 12.0f, 12.0f));//tło/podłoże snake'a
    field = glm::translate(field, glm::vec3(0.0f, 0.0f, 1.0f));

    //edge = glm::scale(edge, glm::vec3(0.5f, 0.5f, 0.5f));//przeskalowanie krawedzi do mniejszego rozmiaru
    //edge = glm::translate(edge, glm::vec3(0.0f, 0.0f, -5.0f));//przesuniecie krawedzi "w gore"

    for (int i = 0; i < (grid_size * 8); i++)//wyzerowanie i wyskalowanie przestrzeni obiektow tworzacych krawedzie
    {
        edges[i] = glm::mat4(1.0f);
        edges[i] = glm::scale(edges[i], glm::vec3(0.5f, 0.5f, 0.5f));
        // edges[i] = glm::translate(edges[i], glm::vec3(i,-20, -5.0f));//przesuniecie krawedzi "w gore"
    }


    //ustawianie krawedzi
    for (int i = 0; i < grid_size; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(i % grid_size, -20, -5.0f));
    }
    for (int i = grid_size; i < grid_size * 2; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(i % grid_size, 20, -5.0f));
    }
    for (int i = grid_size * 2; i < grid_size * 3; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(-i % grid_size, -20, -5.0f));
    }
    for (int i = grid_size * 3; i < grid_size * 4; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(-i % grid_size, 20, -5.0f));
    }


    for (int i = grid_size * 4; i < grid_size * 5; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(-20, i % grid_size, -5.0f));
    }
    for (int i = grid_size * 5; i < grid_size * 6; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(20, i % grid_size, -5.0f));
    }
    for (int i = grid_size * 6; i < grid_size * 7; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(20, -i % grid_size, -5.0f));
    }
    for (int i = grid_size * 7; i < grid_size * 8; i++)
    {
        edges[i] = glm::translate(edges[i], glm::vec3(-20, -i % grid_size, -5.0f));
    }
    //koniec ustawiania krawedzi

    for (int i = 0; i < grid_size; i++)
    {
        for (int j = 0; j < grid_size; j++)
        {
            grid[i][j] = ' ';
        }
    }

    for (int i = 0; i < grid_size; i++)
    {
        grid[i][0] = '#';
        grid[i][grid_size - 1] = '#';
        grid[0][i] = '#';
        grid[grid_size - 1][i] = '#';
    }
    /////////////////////////////////////////////
//////////////////////////////////
    //set obstacles
    srand(time(NULL));
    x_obstacle = (rand() % (grid_size - 2)) + 1;
    y_obstacle = (rand() % (grid_size - 2)) + 1;

    for (int i = 0; i < grid_size / 2; i++)
    {
        x_obstacle = (rand() % (grid_size - 2)) + 1;
        y_obstacle = (rand() % (grid_size - 2)) + 1;
        if (grid[x_obstacle][y_obstacle] == ' ')// if found free space
        {
            grid[x_obstacle][y_obstacle] = '#';//set obstacle
        }
        else
        {
            i--;
        }
    }
    generate_apple();
    // set starting point of snake
    x_head = grid_size / 2;
    y_head = grid_size / 2;
    while (grid[x_head][y_head] != ' ')
    {
        x_head++;
        y_head++;
    }
    //grid[x_head][y_head] = char(219);
    //vector <pair<int,int>> snake;
    snake.push_back(make_pair(x_head, y_head));

    initShaders();
    //************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
    glClearColor(0, 0, 0, 1);//Ustaw czarny kolor czyszczenia ekranu
    glEnable(GL_DEPTH_TEST); //Włącz test głębokości pikseli
    glfwSetKeyCallback(window, keyCallback);
}




//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyszczenie bufora kolorów i bufora głębokości

    glm::vec3 cameraPosition(y_head -11, x_head -13, -20.0f);
    glm::vec3 cameraTarget(y_head-11, x_head-8, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    
    float rotationAngle = 0.0f;  // Kąt obrotu kamery

 // Dostosowanie kąta obrotu kamery na podstawie kierunku ruchu snake'a
    if (direction == 4) { // Ruch w górę
        rotationAngle = 90.0f;  // Kąt obrotu 0 stopni (patrzy w górę)
    }
    else if (direction == 1) { // Ruch w prawo
        rotationAngle = -90.0f;  // Kąt obrotu -90 stopni (patrzy w prawo)
    }
    else if (direction == 2) { // Ruch w dół
        rotationAngle = -90.0f;  // Kąt obrotu 180 stopni (patrzy w dół)
    }
    else if (direction == 3) { // Ruch w lewo
        rotationAngle = 90.0f;  // Kąt obrotu 90 stopni (patrzy w lewo)
    }

    // Tworzenie macierzy widoku (view matrix) z uwzględnieniem obrotu kamery
    glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
    viewMatrix = glm::rotate(viewMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));

    // Przypisanie macierzy widoku do zmiennej 'V'
    glm::mat4 V = viewMatrix;

    glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 50.0f); //Wyliczenie macierzy rzutowania
    //glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //Wyliczenie macierzy widoku

    spLambert->use();//Aktywacja programu cieniującego
    glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P)); //Załadowanie macierzy rzutowania do programu cieniującego
    glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V)); //Załadowanie macierzy widoku do programu cieniującego



    for (int i = 0; i < 18; i++)//animacja przemieszczania
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(field));
        glUniform4f(spLambert->u("color"), 0, 1, 0, 1); //Ustaw kolor zieony
        Models::cube.drawSolid();//narysuj podloze

        //edge = glm::translate(edge, glm::vec3(0.1f, 0.0f, 0.0f));
        for (int j = 0; j < grid_size * 8; j++)//rysowanie krawedzi
        {
            glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(edges[j]));
            glUniform4f(spLambert->u("color"), 1, 0, 0, 1); //Ustaw kolor czerwony
            Models::cube.drawSolid();//narysuj krawedz
        }

        if (direction == 4) { M2 = glm::translate(M2, glm::vec3(angle, 0.0f, 0.0f)); }//left
        if (direction == 1) { M2 = glm::translate(M2, glm::vec3(0.0f, angle, 0.0f)); }//UP
        if (direction == 2) { M2 = glm::translate(M2, glm::vec3(-angle, 0.0f, 0.0f)); }//RIGHT
        if (direction == 3) { M2 = glm::translate(M2, glm::vec3(0.0f, -angle, 0.0f)); }//DOWN

        glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M2));//Załadowanie macierzy modelu do programu cieniującego
        glUniform4f(spLambert->u("color"), 1, 0, 1, 1); //Ustaw kolor czerwony
        Models::cube.drawSolid();//narysowanie glowy snake'a

        glfwSwapBuffers(window);
        Sleep(10);
    }


    //Sleep(10);

//if (M2 == M) { printf("%s", "KOLIZJAAAAAAAAAAAA"); }
}


int main(void)
{
    GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

    glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

    if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
        fprintf(stderr, "Nie można zainicjować GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

    if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
    {
        fprintf(stderr, "Nie można utworzyć okna.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
    glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

    if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
        fprintf(stderr, "Nie można zainicjować GLEW.\n");
        exit(EXIT_FAILURE);
    }

    initOpenGLProgram(window); //Operacje inicjujące

    float angle = 0; //Aktualny kąt obrotu obiektu


    glfwSetTime(0); //Wyzeruj timer
    //Główna pętla
    while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
    {

        int snake_length = snake.size();
        x_head = snake[snake_length - 1].first;
        y_head = snake[snake_length - 1].second;
       

        if (direction == 1) { x_head++; }
        else if (direction == 2) { y_head--; }
        else if (direction == 3) { x_head--; }
        else if (direction == 4) { y_head++; }

        if (grid[x_head][y_head] != ' ' && grid[x_head][y_head] != '+' && grid[x_head][y_head] != char(219)) {
            printf("%s \n", "PRZEGRANA");
            printf("%c \n", grid[x_head][y_head]);
            return 0;
        }//jesli snake jest na innym polu niz jablko lub puste
        if (grid[x_head][y_head] == '+') { found_apple = 1; generate_apple(); }// jesli snake jest na jablku
        snake.push_back(make_pair(x_head, y_head));
        // delete tail (if snake steps on '+') do not proceed
        if (found_apple == 0)
        {
            grid[snake[0].first][snake[0].second] = ' ';
            snake.erase(snake.begin());
            //found_apple = 0;
        }
        found_apple = 0;
        system("cls");
        //draw grid
        for (int i = 0; i < snake.size(); i++)
        {
            grid[snake[i].first][snake[i].second] = char(219);
        }
        for (int i = 0; i < grid_size; i++)
        {
            for (int j = 0; j < grid_size; j++)
            {
                cout << grid[i][j];

            }
            cout << endl;
        }

        cout << "Direction: " << direction << endl;




        /*cout << endl;
        cout << snake[0].first << " " << snake[0].second << endl;
        cout<< ">"<<grid[snake[0].first][snake[0].second]<<"<"<<endl;*/
        //cout << found_apple << endl;
        //Sleep(300);

        //angle += speed * glfwGetTime()/5; //Oblicz przyrost kąta po obrocie
        //printf("%f \n", angle);
        //if (angle > 1) { angle = 0; }
        glfwSetTime(0); //Wyzeruj timer
        drawScene(window, 0.1); //Wykonaj procedurę rysującą
        glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
    }

    freeOpenGLProgram(window);

    glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
    glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
    exit(EXIT_SUCCESS);
}
