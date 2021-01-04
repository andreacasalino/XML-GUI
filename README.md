![What you should see when running the application](https://github.com/andreacasalino/XML-GUI/blob/master/Example.png)

This application allows to **import**, **modify** and **export** **xml** structures.
The application is completely cross platform thanks to [this](https://github.com/yhirose/cpp-httplib) C++ http server implementation.
The application is made of 2 big components:

* **Frontend**, an **html** script represented by **XML-GUI.html**
* **Backend**, a **C++** application that can be compiled using [Cmake](https://cmake.org) and wrapping [XML-parser](https://github.com/andreacasalino/XML-parser)

**Install**:

* initialize the git submodule with the 2 commands (from root) 
  * `git submodule init`
  * `git submodule update --recursive --remote`

* compile the [Cmake](https://cmake.org) project
  * after a backend application named **XML-GUI** will be appear in root/App

**Run**:

* got to root/App and launch the backend application **XML-GUI**
* open in your favourite browser the script **XML-GUI.html**
* have fun with the GUI :)

