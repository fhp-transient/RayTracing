//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"
#include <omp.h>


inline float deg2rad(const float &deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00016;
const float epsilon = 0.00001;

int prog = 0;
omp_lock_t lock;

void Renderer::ompCastRay(const Scene &scene, std::vector<Vector3f> &framebuffer, int start, int end)
{
    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float) scene.height;
    Vector3f eye_pos(278, 273, -800);

    int widthPixel, heightPixel;
    widthPixel = heightPixel = sqrt(spp);
    float step = 1.f / widthPixel;
    for (int j = start; j < end; j++)
    {
        for (int i = 0; i < scene.width; i++)
        {
            int index = j * scene.width + i;
            for (int k = 0; k < spp; k++)
            {
                float x = (2 * (i + step * (k % widthPixel) + step / 2) / (float) scene.width - 1) *
                          imageAspectRatio * scale;
                float y = (1 - 2 * (j + step * (k / heightPixel) + step / 2) / (float) scene.height) * scale;
                Vector3f dir = normalize(Vector3f(-x, y, 1));
                framebuffer[index] += scene.castRay(Ray(eye_pos, dir), 0) / spp;
            }
        }
        omp_set_lock(&lock);
        UpdateProgress(++prog / (float) scene.height);
        omp_unset_lock(&lock);
    }

}

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene &scene)
{
    omp_init_lock(&lock);

    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    const int threadNum = 20;
    const int threadStep = scene.height / threadNum;
    const int remainder = scene.height % threadNum;

    // change the spp value to change sample ammount
    spp = 1024;
    std::cout << "SPP: " << spp << "\n";

#pragma omp parallel for
    for (int i = 0; i < threadNum; i++)
    {
        // Calculate the start and end row for each thread
        int startRow = i * threadStep;
        int endRow = (i + 1) * threadStep;

        // For the last thread, add the remainder to ensure all rows are processed
        if (i == threadNum - 1)
        {
            endRow += remainder;
        }

        ompCastRay(scene, framebuffer, startRow, endRow);
    }

    UpdateProgress(1.f);

    // save framebuffer to file
    FILE *fp = fopen("sphere3.ppm", "wb");
    (void) fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i)
    {
        static unsigned char color[3];
        color[0] = (unsigned char) (255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char) (255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char) (255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);

    omp_destroy_lock(&lock);
}
