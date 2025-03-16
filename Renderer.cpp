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
    // 计算视野缩放系数和长宽比
    // float scale = tan(deg2rad(20.1143 * 0.5));  // 使用 scene.fovy，单位为度
    float scale = tan(deg2rad(35.9834 * 0.5));  // 使用 scene.fovy，单位为度
    float imageAspectRatio = scene.width / (float) scene.height;

    // 使用场景中的摄像机参数
    Vector3f eye_pos = Vector3f(4.443147659301758, 16.934431076049805, 49.91023254394531);      // scene.camera.lookfrom
    Vector3f forward = normalize(Vector3f(-2.5734899044036865, 9.991769790649414, -10.588199615478516) - eye_pos);
    Vector3f right = normalize(crossProduct(forward, Vector3f(0, 1, 0)));
    Vector3f cameraUp = crossProduct(right, forward);

    int widthPixel, heightPixel;
    widthPixel = heightPixel = sqrt(spp);
    float step = 1.f / widthPixel;

    // 对每一行像素进行处理
    for (int j = start; j < end; j++)
    {
        for (int i = 0; i < scene.width; i++)
        {
            int index = j * scene.width + i;
            // 对每个像素进行多重采样（抗锯齿）
            for (int k = 0; k < spp; k++)
            {
                // 在单个像素内部做随机偏移（这里均匀分布在小网格中）
                float jitterX = step * (k % widthPixel) + step / 2;
                float jitterY = step * (k / heightPixel) + step / 2;
                // 将像素坐标转换为 NDC 坐标 [0,1]
                float ndcX = (i + jitterX) / (float) scene.width;
                float ndcY = (j + jitterY) / (float) scene.height;
                // 将 NDC 映射到屏幕空间 [-1,1]，注意水平要乘上长宽比和 scale
                float pixelScreenX = (2 * ndcX - 1) * imageAspectRatio * scale;
                float pixelScreenY = (1 - 2 * ndcY) * scale;
                // 计算射线方向：在摄像机坐标系下，x 轴沿 right，y 轴沿 cameraUp，z 轴指向 forward
                Vector3f dir = normalize(pixelScreenX * right + pixelScreenY * cameraUp + forward);
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
    spp = 32;
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
    FILE *fp = fopen("test_bathroom1.ppm", "wb");
    (void) fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i)
    {
        static unsigned char color[3];
        color[0] = (unsigned char) (255.99 * std::sqrt(clamp(0, 1, framebuffer[i].x)));
        color[1] = (unsigned char) (255.99 * std::sqrt(clamp(0, 1, framebuffer[i].y)));
        color[2] = (unsigned char) (255.99 * std::sqrt(clamp(0, 1, framebuffer[i].z)));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);

    omp_destroy_lock(&lock);
}
