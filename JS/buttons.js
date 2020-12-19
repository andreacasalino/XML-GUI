function get_image(x, y, W, H, relative_path){
    let img_temp = document.createElementNS("http://www.w3.org/2000/svg","image");
    img_temp.setAttributeNS(null, "x", x);
    img_temp.setAttributeNS(null, "y", y);
    img_temp.setAttributeNS(null, "width", W);
    img_temp.setAttributeNS(null, "height", H);
    img_temp.setAttributeNS(null, "preserveAspectRatio", "none");
    img_temp.setAttributeNS(null, "href", relative_path);
    return img_temp;
}

function get_SVG_canvas(viewBox = "0 0 100 100"){
    let SVG_frame  = document.createElementNS("http://www.w3.org/2000/svg","svg");
    SVG_frame.setAttributeNS(null, "width" , "100%");
    SVG_frame.setAttributeNS(null, "height" , "100%");
    SVG_frame.setAttributeNS(null, "viewBox", viewBox);
    return SVG_frame;
}

function get_button(img_path, description){
    let D=document.createElement("div");
    D.style.height = "50px";
    D.setAttribute("class" , "bord");
    let cols = new cols_div(D , [2,8]);
    cols.at(0).style.backgroundColor = "#545454";
    cols.at(1).style.backgroundColor = "#545454";
    D.style.marginTop = "6px";
    D.style.marginBottom = "6px";

    let canvas_img = get_SVG_canvas();
    cols.at(0).appendChild(canvas_img);
    canvas_img.appendChild(get_image(0,0,100,100, img_path));

    let descr = document.createElement("font");
    descr.innerHTML = description;
    descr.setAttribute("color", "white");
    descr.setAttribute("size", 5);
    cols.at(1).appendChild(descr);

    D.style.cursor = "pointer";
    D.addEventListener("mouseover" , function(){ D.style.opacity = 0.5; });
    D.addEventListener("mouseout" , function(){ D.style.opacity = 1; });

    return D;
}