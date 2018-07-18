## Overview
This is a *.fbx Parser which parses the *.fbx file and reconstructs the model on OpenGL. 

I parsed the information(such as meshes, animations, textures, skeletons, etc) of FBX file by using FBX SDK (version: 2018.1.1) ,then I stored them into some data structures. After that I reconstructed the whole model on OpenGL and implemented **Meshed Skeleton Animation** so that the models can animated like any other characters in the game or movies if they do have animations. In other words, you can read and open FBX file on OpenGL. What's more, I added some interactive features. by using this program, you can:
- drag mouse to move the scene.
- click right button to select the menus, you can select different animations,cameras,and rendering mode.
- press **A** and **D** to move model to the left or right, so as **W** or **S**.

Pictures below show the effect of this program:

**render in shaded mode**

![image](https://github.com/Larry955/FbxParser/blob/master/Photos/select%20menus.png)

**render in wireframe mode**

![image](https://github.com/Larry955/FbxParser/blob/master/Photos/%E8%8F%9C%E5%8D%95%E9%80%89%E6%8B%A92.png)

**watch the model from different perspectives**

![image](https://github.com/Larry955/FbxParser/blob/master/Photos/%E5%A4%9A%E8%A7%86%E8%A7%92%E7%9B%B8%E6%9C%BA.png)

**skeleton animation**

![image](https://github.com/Larry955/FbxParser/blob/master/Photos/%E9%AA%A8%E9%AA%BC%E8%BF%90%E8%A1%8C.PNG)
![image](https://github.com/Larry955/FbxParser/blob/master/Photos/%E9%AA%A8%E9%AA%BC%E5%8A%A8%E7%94%BB.png)

## What's expected...
- **add multiply models into one scene**. Current version only support one model, which is a little simple.
- **export the FBX File data to new files**. I have tried to implement this feature, but only mesh data can be exported now.The animation informations are really hard to export because I don'w know how to store them in a read-friendly way. 

This program is not good enough to use in practice, but I successfully parsed the FBX File and implemented the Skinned Mesh Skeleton, which is a big challenge to me. **There are still some features to be added or problems to be solved, it will be my great honor to get your helps or advices if you are interested in this program, come and join me to make it better!**
