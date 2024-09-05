
import controlP5.*; //import ControlP5 library
import processing.serial.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.*;

Serial port;
Chart myChart1;
Chart myChart2;
Chart myChart3;
ControlP5 cp5; //create ControlP5 object
//PFont font;
int nl = 10; 
String received = null;
String [] readings;
int stop_flag = 0;
int start_flag = 0;
int i = 0;
PrintWriter save_data;
PFont myFont;

void setup(){ 

  size(800, 450);    //window size, (width, height)
  
  printArray(Serial.list());   //prints all available serial ports
  
  port = new Serial(this, "COM9", 115200);  //i have connected arduino to /dev/cu.usbmodem14201
  
  //lets add buttons and charts to empty window
  
  cp5 = new ControlP5(this);
  myFont = createFont("calibri", 20);  // custom fonts for buttons and title
  //textFont(font);
  
  cp5.addButton("Start")     //"Start" is the name of button
    .setPosition(80, 120)  //x and y coordinates of upper left corner of button
    .setSize(120, 70)      //(width, height)
    .setFont(myFont);   

  cp5.addButton("Stop")     //"Stop" is the name of button
    .setPosition(80, 220)  //x and y coordinates of upper left corner of button
    .setSize(120, 70)      //(width, height)
    .setFont(myFont);
  
  cp5.addButton("Save")     //"Save" is the name of button
    .setPosition(80, 320)  //x and y coordinates of upper left corner of button
    .setSize(120, 70)      //(width, height)
    .setFont(myFont);
  
  myChart1= cp5.addChart("pulse waveform 1")
    .setPosition(260, 10)
    .setSize(500, 125)
    .setView(Chart.LINE)
    .setRange(19000,22000)
    .setStrokeWeight(1.5);

  myChart1.addDataSet("incoming");
  myChart1.setData("incoming", new float[100]);
  
  myChart2 = cp5.addChart("pulse waveform 2")
    .setPosition(260, 155)
    .setSize(500, 125)
    .setView(Chart.LINE)
    .setRange(20000,22000)
    ;

  myChart2.addDataSet("incoming");
  myChart2.setData("incoming", new float[100]);
  
  myChart3 = cp5.addChart("pulse waveform 3")
    .setPosition(260, 300)
    .setSize(500, 125)
    .setView(Chart.LINE)
    .setRange(23000,26000)
    ;

  myChart3.addDataSet("incoming");
  myChart3.setData("incoming", new float[100]);
  
  save_data = createWriter(day() + "-" + month() + "-" + year() + "/sensordata" + "("+ hour() + "," + minute() + ")"  + ".txt");
  i++;
}

void draw(){  //same as loop in arduino

  background(0, 0, 0); // background color of window (r, g, b) or (0 to 255)
  
  //lets give title to our window
  fill(0, 255, 0);               //text color (r, g, b)
  textFont(myFont);
  text("PULSE WAVEFORM", 60, 80);  // ("text", x coordinate, y coordinat)
  
  received = port.readStringUntil(nl); 
  if(received != null && start_flag == 1 && stop_flag == 0){
  //  println(readings);
    readings = received.split(",",3);
    //println(readings);
    println(readings[0],readings[1],readings[2]);
    //saveStrings("sensor_data.txt", readings);
    save_data.println(hour()+"."+"0"+minute()+"."+second()+"."+millis()+" : "+readings[0]+","+readings[1]+","+readings[2]);
    myChart1.push("incoming", float(readings[0]));
    myChart2.push("incoming", float(readings[1]));
    myChart3.push("incoming", float(readings[2]));
  }
  //println(received);
  
}

//lets add some functions to our buttons
//so when you press any button, it sends particular string over serial port

void Start(){
  start_flag = 1;
  stop_flag = 0;
}

void Stop(){
    stop_flag = 1;
    start_flag = 0;
    
    //exit();
}

void Save(){
  save_data.flush();
  save_data.close();
  //save_data = createWriter("sensordata"+"(i)"+".txt");
 // save_data.println(hour()+"."+"0"+minute()+"."+second()+"."+millis()+" : "+readings[0]+","+readings[1]+","+readings[2]);
  exit();
}
