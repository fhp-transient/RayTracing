#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "OBJ_Loader.hpp"
#include "Vector.hpp"
#include <optional>
#include <cmath>
#include <algorithm>
#include <memory>

#include "ConstantTexture.h"
#include "global.hpp"
#include "imageTexture.h"
#include "Texture.h"
#include "stb_image.h"


struct Intersection;

enum MaterialType { DIFFUSE, MICROFACET, DIELECTRIC };

class Material
{
private:
    Vector3f reflect(const Vector3f& I, const Vector3f& N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }

    Vector3f refract(const Vector3f& I, const Vector3f& N, const float& ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0)
        {
            cosi = -cosi;
        }
        else
        {
            std::swap(etai, etat);
            n = -N;
        }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? Vector3f(0, 0, 0) : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    void fresnel(const Vector3f& I, const Vector3f& N, const float& ior, float& kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) { std::swap(etai, etat); }
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
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
    }

    Vector3f toWorld(const Vector3f& a, const Vector3f& N)
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
    Vector3f m_emission;
    float ior;
    Vector3f Kd, Ks;
    float specularExponent;
    float roughness;
    float pDiffuse, pSpecular;
    std::optional<std::string> matName;
    std::shared_ptr<Texture> diffuseTexture;
    std::shared_ptr<Texture> specularTexture;

    inline Material(MaterialType t = MICROFACET, Vector3f e = Vector3f(0, 0, 0));
    inline Material(const objl::Material& mat);
    inline MaterialType getType();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();
    inline Vector3f sample(const Vector3f& wi, const Vector3f& N);
    inline float pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N);
    inline Vector3f eval(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, Vector2f& tcoords);
    void setEmission(const Vector3f e) { m_emission = e; }
};

Material::Material(MaterialType t, Vector3f e)
{
    m_type = t;
    m_emission = e;
    Kd = Vector3f(0.8f, 0.2f, 0.2f);
    Ks = Vector3f(0.2f, 0.2f, 0.2f);
    specularExponent = 64.0f;
    roughness = 0.05f;
    ior = 1.5f;
    // 默认漫反射和镜面反射各占一半
    pDiffuse = 0.5f;
    pSpecular = 0.5f;
}

Material::Material(const objl::Material& mat)
{
    Vector3f kd(mat.Kd.X, mat.Kd.Y, mat.Kd.Z);
    Vector3f ks(mat.Ks.X, mat.Ks.Y, mat.Ks.Z);
    float ns = mat.Ns;
    float ni = mat.Ni;
    matName = mat.name;
    Kd = kd;
    Ks = ks;
    ior = ni;
    specularExponent = ns;
    roughness = 4.f / ns;
    m_emission = Vector3f(0, 0, 0);

    specularTexture = std::make_shared<ConstantTexture>(ks);
    // TODO: implement Texture, path:mat.map_Kd
    if (!mat.map_Kd.empty())
    {
        int nx, ny, nn;
        // load texture data
        std::string texturePath = "E:/GAMES101/RayTracing/models/bathroom2/" + mat.map_Kd;
        std::cout << "Get diffuse map : " << texturePath << std::endl;
        unsigned char* tex_data = stbi_load(texturePath.c_str(), &nx, &ny, &nn, 0);
        diffuseTexture = std::make_shared<ImageTexture>(tex_data, nx, ny, nn);
    }
    else
    {
        diffuseTexture = std::make_shared<ConstantTexture>(kd);
    }

    // 根据漫反射和镜面反射分量的大小计算混合权重
    float kdLen = sqrt(dotProduct(Kd, Kd));
    float ksLen = sqrt(dotProduct(Ks, Ks));
    float sum = kdLen + ksLen;
    if (sum > 1e-6)
    {
        pDiffuse = kdLen / sum;
        pSpecular = ksLen / sum;
    }
    else
    {
        pDiffuse = 1.0f;
        pSpecular = 0.0f;
    }
    if (fabs(ior - 1.0f) < 1e-3)
    {
        if (ns > 200.f)
        {
            m_type = DIELECTRIC;
            ior = 12.85;
            diffuseTexture = std::make_shared<ConstantTexture>(Vector3f(0.45, 0.45, 0.45));
            specularTexture = std::make_shared<ConstantTexture>(Vector3f(0.3, 0.3, 0.25));
        }
        else if (fabs(ns - 1.0f) > 1e-6)
            m_type = MICROFACET;
        else
            m_type = DIFFUSE;
    }
    else
    {
        m_type = DIFFUSE;
    }
}

// GGX NDF
inline float GGXDistribution(float cosTheta, float alpha)
{
    return (alpha * alpha) / (M_PI * pow((alpha * alpha - 1.0f) * cosTheta * cosTheta + 1.0f, 2.0f));
}

inline Vector3f FresnelSchlick(float cosTheta, const Vector3f& F0)
{
    return F0 + (Vector3f(1.0f, 1.0f, 1.0f) - F0) * pow(1.0f - cosTheta, 5.0f);
}

inline float chiGGX(float v)
{
    return (v > 0.0f) ? 1.0f : 0.0f;
}

inline float GGX_PartialGeometryTerm(const Vector3f& v, const Vector3f& n, const Vector3f& h, float alpha)
{
    float VoH = dotProduct(v, h);
    float chi = chiGGX(VoH / dotProduct(v, n));
    float VoH2 = VoH * VoH;
    float tan2 = (1 - VoH2) / VoH2;
    return (chi * 2.0f) / (1.0f + sqrtf(1.0f + alpha * alpha * tan2));
}

MaterialType Material::getType() { return m_type; }
Vector3f Material::getEmission() { return m_emission; }
bool Material::hasEmission() { return (m_emission.norm() > 1e-6); }
Vector3f Material::getColorAt(double u, double v) { return Kd; }

Vector3f Material::sample(const Vector3f& wi, const Vector3f& N)
{
    if (m_type == DIFFUSE || m_type == MICROFACET)
    {
        float x1 = get_random_float(), x2 = get_random_float();
        float r = sqrtf(x1);
        float theta = 2 * M_PI * x2;
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        float z = sqrtf(std::max(0.f, 1.0f - x1));
        Vector3f localRay(x, y, z);
        return toWorld(localRay, N);
    }
    else if (m_type == DIELECTRIC)
    {
        return -reflect(wi, N);
    }
    return Vector3f(0, 0, 0);
}

float Material::pdf(const Vector3f& wi, const Vector3f& wo, const Vector3f& N)
{
    // DIFFUSE
    if (m_type == DIFFUSE)
    {
        if (dotProduct(wo, N) > 0.0f)
            return dotProduct(wo, N) / M_PI;
        else
            return 0.0f;
    }
    // MICROFACET: mix of diffuse and specular
    else if (m_type == MICROFACET)
    {
        float pdf_diffuse = 0.0f;
        if (dotProduct(wo, N) > 0.0f)
            pdf_diffuse = dotProduct(wo, N) / M_PI;
        else
            pdf_diffuse = 0.0f;

        Vector3f l = normalize(wo);
        Vector3f h = normalize(wi + wo);
        float cosTheta_h = fabs(dotProduct(h, N));
        float pdf_spec = 0.0f;
        if (fabs(dotProduct(wo, h)) > 1e-6)
        {
            pdf_spec = GGXDistribution(cosTheta_h, roughness) * cosTheta_h / (4.0f * fabs(dotProduct(wo, h)));
        }
        // MIX
        return pDiffuse * pdf_diffuse + pSpecular * pdf_spec;
    }
    // DIELECTRIC
    else if (m_type == DIELECTRIC)
    {
        if (dotProduct(wo, N) > EPSILON)
            return 1.0f;
        return 0.0f;
    }
    return 0.0f;
}

Vector3f Material::eval(const Vector3f& wi, const Vector3f& wo, const Vector3f& N, Vector2f& tcoords)
{
    float cosi = std::max(0.f, dotProduct(N, wi));
    float coso = std::max(0.f, dotProduct(N, wo));
    Vector3f Kd = diffuseTexture->Evaluate(tcoords.x, tcoords.y);
    Vector3f Ks = specularTexture->Evaluate(tcoords.x, tcoords.y);

    if (m_type == DIFFUSE)
    {
        return (coso > 0.0f ? (Kd / M_PI) : Vector3f(0, 0, 0));
    }
    else if (m_type == MICROFACET)
    {
        if (cosi <= 0.0f || coso <= 0.0f)
            return Vector3f(0, 0, 0);

        Vector3f diffuse(0), specular(0);
        if (pDiffuse > 1e-8)
        {
            diffuse = (coso / M_PI) * Kd;
        }
        if (pSpecular > 1e-8)
        {
            // Half vector
            Vector3f h = normalize(wi + wo);
            float cosTheta = std::max(0.f, dotProduct(h, N));

            // D = GGX
            float D = GGXDistribution(cosTheta, roughness);

            Vector3f F0 = Ks;
            // Fresnel
            Vector3f F = FresnelSchlick(dotProduct(wi, h), F0);

            // G
            float G = GGX_PartialGeometryTerm(wi, N, h, roughness);

            // Cook-Torrance specular
            float spec = (G * D) / (4.0f * cosi * coso);
            specular = F * spec;
        }
        return diffuse * pDiffuse + specular * pSpecular;
    }
    else if (m_type == DIELECTRIC)
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
    }
    return Vector3f(0, 0, 0);
}


#endif // RAYTRACING_MATERIAL_H
