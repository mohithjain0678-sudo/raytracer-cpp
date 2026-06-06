#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"
#include "ray.h"
#include "color.h"
#include <cstdlib>
#include <cmath>

struct hit_record;

inline vec3 random_unit_vector() {
    while (true) {
        auto p = vec3(
            ((double)rand()/RAND_MAX)*2 - 1,
            ((double)rand()/RAND_MAX)*2 - 1,
            ((double)rand()/RAND_MAX)*2 - 1
        );
        if (p.length_squared() < 1)
            return p / p.length();
    }
}

inline bool near_zero(const vec3& v) {
    auto s = 1e-8;
    return (fabs(v.e[0]) < s) && (fabs(v.e[1]) < s) && (fabs(v.e[2]) < s);
}

class material {
  public:
    virtual ~material() = default;
    virtual bool scatter(const ray& r_in, const hit_record& rec,
                         color& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
  public:
    lambertian(const color& albedo) : albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec,
                 color& attenuation, ray& scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();
        if (near_zero(scatter_direction))
            scatter_direction = rec.normal;
        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

  private:
    color albedo;
};

class metal : public material {
  public:
    metal(const color& albedo, double fuzz)
        : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec,
                 color& attenuation, ray& scattered) const override {
        vec3 udir = unit_vector(r_in.direction());
        vec3 reflected = udir - 2*dot(udir, rec.normal)*rec.normal;
        reflected = reflected + fuzz * random_unit_vector();
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

  private:
    color albedo;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec,
                 color& attenuation, ray& scattered) const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_dir = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_dir, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > ((double)rand()/RAND_MAX))
            direction = unit_dir - 2*dot(unit_dir, rec.normal)*rec.normal;
        else
            direction = refract(unit_dir, rec.normal, ri);

        scattered = ray(rec.p, direction);
        return true;
    }

  private:
    double refraction_index;

    static double reflectance(double cosine, double ri) {
        auto r0 = (1 - ri) / (1 + ri);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1-cosine), 5);
    }

    static vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
        auto cos_theta = std::fmin(dot(-uv, n), 1.0);
        vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
        vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
        return r_out_perp + r_out_parallel;
    }
};

#endif