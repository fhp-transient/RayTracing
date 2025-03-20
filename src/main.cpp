#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>
#include <unordered_map>

int main(int argc, char** argv)
{
    // Change the definition here to change resolution
    Scene scene(1280, 720);

    // Material* red = new Material(DIFFUSE, Vector3f(0.0f));
    // red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    // red->diffuseTexture = std::make_shared<ConstantTexture>(red->Kd);
    // red->specularTexture = std::make_shared<ConstantTexture>(red->Ks);
    // Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    // green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    // green->diffuseTexture = std::make_shared<ConstantTexture>(green->Kd);
    // green->specularTexture = std::make_shared<ConstantTexture>(green->Ks);
    // Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    // white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    // white->diffuseTexture = std::make_shared<ConstantTexture>(white->Kd);
    // white->specularTexture = std::make_shared<ConstantTexture>(white->Ks);
    // // Material* light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * Vector3f(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *Vector3f(0.737f+0.642f,0.737f+0.159f,0.737f)));
    // Material* light = new Material(DIFFUSE, Vector3f(34, 24, 8));
    // light->Kd = Vector3f(0.65f);
    // light->diffuseTexture = std::make_shared<ConstantTexture>(light->Kd);
    // light->specularTexture = std::make_shared<ConstantTexture>(light->Ks);
    // Material* microfacet = new Material(MICROFACET, Vector3f(0.0f));
    // microfacet->roughness = 0.1f;
    // microfacet->Ks = Vector3f(0.45, 0.45, 0.45);
    // microfacet->Kd = Vector3f(0.3, 0.3, 0.25);
    // microfacet->ior = 12.85;
    // microfacet->diffuseTexture = std::make_shared<ConstantTexture>(microfacet->Ks);
    // microfacet->specularTexture = std::make_shared<ConstantTexture>(microfacet->Kd);
    // Material* mirror = new Material(DIELECTRIC, Vector3f(0.0f));
    // mirror->Ks = Vector3f(0.45, 0.45, 0.45);
    // mirror->Kd = Vector3f(0.3, 0.3, 0.25);
    // mirror->ior = 12.85;
    // mirror->diffuseTexture = std::make_shared<ConstantTexture>(mirror->Ks);
    // mirror->specularTexture = std::make_shared<ConstantTexture>(mirror->Kd);

    std::unordered_map<std::string, Vector3f> emissionMapping1 = {
        {"light1", Vector3f(300, 300, 300)},
        {"light2", Vector3f(50, 50, 50)},
        {"light3", Vector3f(20, 20, 20)},
        {"light4", Vector3f(10, 10, 10)},
    };

    std::unordered_map<std::string, Vector3f> emissionMapping2 = {
        {"Light", Vector3f(34.0, 24.0, 8.0)},
    };

    std::unordered_map<std::string, Vector3f> emissionMapping3 = {
        {"Light", Vector3f(125.0, 100.0, 75.0)},
    };

    // load
    std::string modelPath = "E:/GAMES101/RayTracing/models/bathroom2/bathroom2.obj";
    objl::Loader loader;
    loader.LoadFile(modelPath);

    // add
    auto emission = Vector3f(0.0f);
    for (auto mesh : loader.LoadedMeshes)
    {
        if (emissionMapping3.find(mesh.MeshMaterial.value().name) != emissionMapping3.end())
            emission = emissionMapping3[mesh.MeshMaterial.value().name];
        MeshTriangle* meshTriangle = new MeshTriangle(mesh, emission);
        scene.Add(meshTriangle);
        emission = Vector3f(0.0f);
    }

    // MeshTriangle cornellbox("D:/Assignment7/models/cornell-box/cornell-box.obj", white, emissionMapping);

    // MeshTriangle floor("E:/GAMES101/RayTracing/models/cornellbox/floor.obj", white);
    // MeshTriangle shortbox("E:/GAMES101/RayTracing/models/cornellbox/shortbox.obj", white);
    // MeshTriangle tallbox("E:/GAMES101/RayTracing/models/cornellbox/tallbox.obj", white);
    // MeshTriangle left("E:/GAMES101/RayTracing/models/cornellbox/left.obj", red);
    // MeshTriangle right("E:/GAMES101/RayTracing/models/cornellbox/right.obj", green);
    // MeshTriangle light_("E:/GAMES101/RayTracing/models/cornellbox/light.obj", light);
    // Sphere sphere1(Vector3f(150, 100, 200), 100, mirror);
    //
    // scene.Add(&floor);
    // // scene.Add(&shortbox);
    // // scene.Add(&tallbox);
    // scene.Add(&left);
    // scene.Add(&right);
    // scene.Add(&light_);
    // scene.Add(&sphere1);
    // // scene.Add(&cornellbox);

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
