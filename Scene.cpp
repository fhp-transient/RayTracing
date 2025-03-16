//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

void Scene::buildBVH()
{
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object *> &objects,
    float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear)
        {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // get the intersection
    Intersection intersection = Scene::intersect(ray);

    if (!intersection.happened)
    {
        return {};
    }

    Vector3f L_dir(0);
    Vector3f L_indir(0);

    Vector3f hitPoint = intersection.coords;
    Vector3f N = normalize(intersection.normal);
    Vector3f wo = normalize(-ray.direction);

    // hit light
    if (intersection.happened && intersection.m->hasEmission())
    {
        L_dir += intersection.emit;
    }

    switch (intersection.m->getType())
    {
        case DIELECTRIC:
        {
            if (get_random_float() < RussianRoulette)
            {
                Vector3f wi = normalize(intersection.m->sample(wo, N));
                Vector3f reflectionRayOrig = (dotProduct(wi, N) < 0) ? hitPoint - N * epsilon : hitPoint + N * epsilon;
                Ray reflectionRay(reflectionRayOrig, wi);
                Intersection reflectionInter = Scene::intersect(reflectionRay);
                if (reflectionInter.happened)
                {
                    if (float pdf = intersection.m->pdf(wo, wi, N); pdf > EPSILON)
                    {
                        L_indir = castRay(reflectionRay, depth + 1) * intersection.m->eval(wi, wo, N) *
                                  std::max(0.f, dotProduct(wi, N)) / (pdf * RussianRoulette);
                    }
                }
            }
            break;
        }
        default:
        {
            Intersection lightInter;
            float pdf_light;
            sampleLight(lightInter, pdf_light);
            Vector3f x = lightInter.coords;
            Vector3f NN = normalize(lightInter.normal);
            Vector3f lightDirection = normalize(x - hitPoint);
            float distance = (x - hitPoint).norm();
            Vector3f lightRayOrigin = (dotProduct(lightDirection, N) < 0)
                                          ? hitPoint - N * epsilon
                                          : hitPoint + N * epsilon;
            Ray shadowRay(lightRayOrigin, lightDirection);
            Intersection shadowInter = Scene::intersect(shadowRay);

            if (fabs(distance - shadowInter.distance) < EPSILON)
            {
                L_dir = lightInter.emit * intersection.m->eval(lightDirection, wo, N) *
                        std::max(dotProduct(lightDirection, N), 0.f) * std::max(dotProduct(-lightDirection, NN), 0.f) /
                        (distance * distance * pdf_light);
            }

            if (get_random_float() < RussianRoulette)
            {
                Vector3f wi = normalize(intersection.m->sample(wo, N));
                Vector3f reflectionRayOrig = (dotProduct(wi, N) < 0) ? hitPoint - N * epsilon : hitPoint + N * epsilon;
                Ray reflectionRay(reflectionRayOrig, wi);
                Intersection reflectionInter = Scene::intersect(reflectionRay);
                if (reflectionInter.happened && !reflectionInter.m->hasEmission())
                {
                    if (float pdf = intersection.m->pdf(wo, wi, N); pdf > EPSILON)
                    {
                        L_indir = castRay(reflectionRay, depth + 1) * intersection.m->eval(wi, wo, N) *
                                  std::max(0.f, dotProduct(wi, N)) / (pdf * RussianRoulette);
                    }
                }
            }
            break;
        }
    }
    auto hitColor = L_dir + L_indir;
    // hitColor.x = (clamp(0, 1, hitColor.x));
    // hitColor.y = (clamp(0, 1, hitColor.y));
    // hitColor.z = (clamp(0, 1, hitColor.z));

    return hitColor;
}
