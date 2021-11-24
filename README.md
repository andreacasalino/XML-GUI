![What you should see when running the application](https://github.com/andreacasalino/XML-GUI/blob/master/Example.png)

This application allows to **import**, **modify** and **export** **xml** structures.
The application is completely cross platform thanks to [this](https://github.com/yhirose/cpp-httplib) C++ http server implementation.
The application is made of 2 big components:

* **Frontend**, an **html** script represented by **XML-GUI.html**
* **Backend**, a **C++** application that can be compiled using [Cmake](https://cmake.org) which wraps [XML-parser](https://github.com/andreacasalino/XML-parser) to handle operations on **xml**

**Compile**:

* compile the [Cmake](https://cmake.org) project an run the **INSTALL** command
  * after that, the backend application named **XML-GUI** will be appear in the installation folder, under bin

**Run**:

* got to the installation folder and then inside bin.
* run the application named **XML-GUI**
* open in your favourite browser the script **XML-GUI.html**
* have fun with the GUI :)

