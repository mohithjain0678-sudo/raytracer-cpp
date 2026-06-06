#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "hittable.h"
#include "material.h"
#include "ray.h"
#include "vec3.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

class camera {
  public:
    double aspect_ratio      = 16.0 / 9.0;
    int    image_width       = 400;
    int    samples_per_pixel = 10;
    int    max_depth         = 50;

    double vfov     = 90;
    point3 lookfrom = point3(0, 0, 0);
    point3 lookat   = point3(0, 0, -1);
    vec3   vup      = vec3(0, 1, 0);

    double defocus_angle = 0;
    double focus_dist    = 10;

    void render(const hittable& world) {
        initialize();
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0,0,0);
                for (int s = 0; s < samples_per_pixel; s++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                write_color(std::cout, pixel_color / samples_per_pixel);
            }
        }
        std::clog << "\rDone.                 \n";
    }

  private:
    int    image_height;
    point3 camera_center;
    point3 pixel00_loc;
    vec3   pixel_delta_u;
    vec3   pixel_delta_v;
    vec3   u, v, w;
    vec3   defocus_disk_u;
    vec3   defocus_disk_v;

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        camera_center = lookfrom;

        auto theta = vfov * 3.14159265 / 180.0;
        auto h = std::tan(theta/2);
        auto viewport_height = 2.0 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        auto viewport_upper_left = camera_center
                                 - (focus_dist * w)
                                 - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        auto defocus_radius = focus_dist * std::tan((defocus_angle/2) * 3.14159265 / 180.0);
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    vec3 random_in_unit_disk() const {
        while (true) {
            auto p = vec3(((double)rand()/RAND_MAX)*2-1,
                          ((double)rand()/RAND_MAX)*2-1, 0);
            if (p.length_squared() < 1) return p;
        }
    }

    ray get_ray(int i, int j) const {
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0)
                        ? camera_center
                        : defocus_disk_sample();
        return ray(ray_origin, pixel_sample - ray_origin);
    }

    point3 defocus_disk_sample() const {
        auto p = random_in_unit_disk();
        return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    vec3 sample_square() const {
        return vec3(((double)rand()/RAND_MAX) - 0.5,
                    ((double)rand()/RAND_MAX) - 0.5, 0);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0) return color(0,0,0);
        hit_record rec;
        if (world.hit(r, 0.001, 1e9, rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth-1, world);
            return color(0,0,0);
        }
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0,1.0,1.0) + a*color(0.5,0.7,1.0);
    }
};

#endif