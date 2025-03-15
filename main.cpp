#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>
#include <unordered_map>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char **argv)
{
    // Change the definition here to change resolution
    Scene scene(1380, 720);

    Material* red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    // Material* light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * Vector3f(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *Vector3f(0.737f+0.642f,0.737f+0.159f,0.737f)));
    Material* light = new Material(DIFFUSE, Vector3f(34, 24, 8));
    light->Kd = Vector3f(0.65f);
    Material* microfacet = new Material(MICROFACET, Vector3f(0.0f));
    microfacet->roughness = 0.1f;
    microfacet->Ks = Vector3f(0.45, 0.45, 0.45);
    microfacet->Kd = Vector3f(0.3, 0.3, 0.25);
    microfacet->ior = 12.85;
    Material* mirror = new Material(MIRROR, Vector3f(0.0f));
    mirror->Ks = Vector3f(0.45, 0.45, 0.45);
    mirror->Kd = Vector3f(0.3, 0.3, 0.25);
    mirror->ior = 12.85;

    std::unordered_map<std::string, Vector3f> emissionMapping1 = {
        {"light1", Vector3f(300, 300, 300)},
        {"light2", Vector3f(50, 50, 50)},
        {"light3", Vector3f(20, 20, 20)},
        {"light4", Vector3f(10, 10, 10)},
    };

    std::unordered_map<std::string, Vector3f> emissionMapping2 = {
        {"Light", Vector3f(34.0, 24.0, 8.0)},
    };

    // // load
    // std::string modelPath = "D:/Assignment7/models/veach-mis/veach-mis.obj";
    // objl::Loader loader;
    // loader.LoadFile(modelPath);
    //
    // // add
    // auto emission = Vector3f(0.0f);
    // for (auto mesh: loader.LoadedMeshes)
    // {
    //     if (emissionMapping1.find(mesh.MeshMaterial.value().name) != emissionMapping1.end())
    //         emission = emissionMapping1[mesh.MeshMaterial.value().name];
    //     MeshTriangle *meshTriangle = new MeshTriangle(mesh, emission);
    //     scene.Add(meshTriangle);
    //     emission = Vector3f(0.0f);
    // }

    // MeshTriangle cornellbox("D:/Assignment7/models/cornell-box/cornell-box.obj", white, emissionMapping);

    MeshTriangle floor("D:/Assignment7/models/cornellbox/floor.obj", white);
    MeshTriangle shortbox("D:/Assignment7/models/cornellbox/shortbox.obj", white);
    MeshTriangle tallbox("D:/Assignment7/models/cornellbox/tallbox.obj", white);
    MeshTriangle left("D:/Assignment7/models/cornellbox/left.obj", red);
    MeshTriangle right("D:/Assignment7/models/cornellbox/right.obj", green);
    MeshTriangle light_("D:/Assignment7/models/cornellbox/light.obj", light);
    Sphere sphere1(Vector3f(150, 100, 200), 100, mirror);

    scene.Add(&floor);
    // scene.Add(&shortbox);
    // scene.Add(&tallbox);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);
    scene.Add(&sphere1);
    // scene.Add(&cornellbox);

    scene.buildBVH();

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() <<
            " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() <<
            " seconds\n";

    return 0;
}
