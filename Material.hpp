//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "OBJ_Loader.hpp"
#include "Vector.hpp"

enum MaterialType { DIFFUSE, MICROFACET, MIRROR };

class Material {
private:
    // Compute reflection direction
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }

    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; }
        else
        {
            std::swap(etai, etat);
            n = -N;
        }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    // Compute Fresnel equation
    //
    // \param I is the incident view direction
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior, float &kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) { std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1)
        {
            kr = 1;
        }
        else
        {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    Vector3f toWorld(const Vector3f &a, const Vector3f &N)
    {
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y))
        {
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x * invLen);
        }
        else
        {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y * invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
    }

public:
    MaterialType m_type;
    //Vector3f m_color;
    Vector3f m_emission;
    float ior;
    Vector3f Kd, Ks;
    float specularExponent;
    float roughness;
    //Texture tex;
    std::optional<std::string> matName;

    inline Material(MaterialType t = MICROFACET, Vector3f e = Vector3f(0, 0, 0));

    inline Material(const objl::Material& mat);

    inline MaterialType getType();

    //inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);

    inline Vector3f getEmission();

    inline bool hasEmission();

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);

    // given a ray, calculate the PdF of this ray
    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

    // given a ray, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

    void setEmission(const Vector3f e) { m_emission = e; }
};

Material::Material(MaterialType t, Vector3f e)
{
    m_type = t;
    m_emission = e;

    // 让漫反射有一定颜色
    Kd = Vector3f(0.8f, 0.2f, 0.2f);
    // 让镜面系数不为0
    Ks = Vector3f(0.2f, 0.2f, 0.2f);
    // 指数适中，可以再调
    specularExponent = 64.0f;
    // 粗糙度小一点让高光集中
    roughness = 0.05f;
    ior = 1.5f; // 若为玻璃或其它介质
}

Material::Material(const objl::Material& mat)
{
    m_type = MICROFACET;
    Kd = Vector3f(mat.Kd.X, mat.Kd.Y, mat.Kd.Z);
    Ks = Vector3f(mat.Ks.X, mat.Ks.Y, mat.Ks.Z);
    ior = mat.Ni;
    specularExponent = mat.Ns;
    roughness = 0.1f;
    m_emission = Vector3f(0, 0, 0);
    matName = mat.name;
}


MaterialType Material::getType() { return m_type; }
///Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() { return m_emission; }

bool Material::hasEmission()
{
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v)
{
    return Vector3f();
}


Vector3f Material::sample(const Vector3f &wi, const Vector3f &N)
{
    switch (m_type)
    {
        case DIFFUSE:
        case MICROFACET:
        {
            // uniform sample on the hemisphere
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x_1);
            float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
            Vector3f localRay(r * std::cos(phi), r * std::sin(phi), z);
            return toWorld(localRay, N);
            break;
        }
        case MIRROR:
        {
            return -reflect(wi, N);
            break;
        }
    }
}

float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N)
{
    switch (m_type)
    {
        case DIFFUSE:
        case MICROFACET:
        {
            // uniform sample probability 1 / (2 * PI)
            if (dotProduct(wo, N) > 0.0f)
                return 0.5f / M_PI;
            return 0.0f;
            break;
        }
        case MIRROR:
        {
            if (dotProduct(wo, N) > EPSILON)
                return 1.0f;
            return 0.0f;
            break;
        }
    }
}

Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N)
{
    float cosi = std::max(0.f, dotProduct(N, wi));
    float coso = std::max(0.f, dotProduct(N, wo));
    switch (m_type)
    {
        case DIFFUSE:
        {
            // 漫反射部分
            float cosTheta = std::max(0.f, dotProduct(N, wo));
            Vector3f diffuse = Kd / M_PI;
            // 这里你可以根据需要加入 specular 分量
            float cosR = std::max(0.f, dotProduct(reflect(-wi, N), wo));
            Vector3f specular = Ks * std::pow(cosR, specularExponent);
            // 注意：如果 Tr 表示透明度，可以在这里进一步处理混合
            return diffuse + specular;


            // // calculate the contribution of diffuse   model
            // float cosalpha = dotProduct(N, wo);
            // if (cosalpha > 0.0f)
            // {
            //     Vector3f diffuse = Kd / M_PI;
            //     return diffuse;
            // }
            // return {0.0f};
            // break;
        }
        case MIRROR:
        {
            float cosalpha = dotProduct(N, wo);
            if (cosalpha > 0.0f)
            {
                float F;
                fresnel(wi, N, ior, F);
                float divisor = cosalpha;
                if (divisor < 0.001f) return {0.0f};
                Vector3f mirror = 1.f / divisor;
                return F * mirror;
            }
            return {0.0f};
            break;
        }
        case MICROFACET:
        {
            if (coso > 0.0f && cosi > 0.0f)
            {
                // Fresnel
                float Fval;
                fresnel(wi, N, ior, Fval);

                // Cook-Torrance近似: G * D * F / (4 cosi coso)
                // 这里 roughness2 影响法线分布 D
                float roughness2 = roughness * roughness;

                // Geometry term G (Smith)
                float A_wi = -1.0f + std::sqrt(1.0f + roughness2 * (1.0f / (cosi * cosi) - 1.0f));
                float A_wo = -1.0f + std::sqrt(1.0f + roughness2 * (1.0f / (coso * coso) - 1.0f));
                A_wi *= 0.5f;
                A_wo *= 0.5f;
                float G = std::min(1.0f, 1.0f / (A_wi + A_wo + 1.0f));

                // 法线分布函数 D (GGX 近似 or Beckmann)
                Vector3f h = normalize(wi + wo); // 半程向量
                float cosh = std::max(0.f, dotProduct(h, N));
                // 这里简单用 Beckmann / GGX 都行
                // 先用 GGX: D = alpha^2 / [ π * (cosh^2*(alpha^2-1)+1)^2 ]
                float alpha2 = roughness2;
                float denom = cosh * cosh * (alpha2 - 1.0f) + 1.0f;
                float D = alpha2 / (M_PI * denom * denom);

                // 漫反射
                Vector3f diffuse = (1.0f - Fval) * (Kd / M_PI);
                // specular
                float spec = (Fval * G * D) / (4.0f * cosi * coso);

                return diffuse + Vector3f(spec, spec, spec) * Ks;
            }
            return Vector3f(0.f);
            break;
        }
    }
}

#endif //RAYTRACING_MATERIAL_H
