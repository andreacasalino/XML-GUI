![What you should see when running the application](https://github.com/andreacasalino/XML-GUI/blob/master/Example.png)

This application allows to **import**, **modify** and **export** **xml** structures.
The application is cross-platform but you should have on your system
[NodeJS](https://nodejs.org/en/) and [npm](https://www.npmjs.com/get-npm).
The application is made of 2 big components:

* **Frontend**, represented by **XML-GUI.html**
* **Backend**, contained in **XML-GUI.js** 

**Install**:

* initialize the git submodule with the 2 commands (from root) 
  * `git submodule init`
  * `git submodule update --recursive --remote`
* run `npm install` from the JS-xml-addon folder, to build and create a node-js addon wrapping [XML-parser](https://github.com/andreacasalino/XML-parser), a C++ library handling xml.

**Run**:

* enter in the terminal from the root folder `node XML-GUI.js`
* open your favourite browser and go to the address 'http://localhost:3000'

