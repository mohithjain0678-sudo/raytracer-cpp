#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include <memory>
#include <cstdlib>

int main() {
    hittable_list world;

    // Ground
    auto mat_ground = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(std::make_shared<sphere>(point3(0, -1000, 0), 1000, mat_ground));

    // Hundreds of random small spheres
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double choose_mat = (double)rand()/RAND_MAX;
            point3 center(a + 0.9*(double)rand()/RAND_MAX,
                          0.2,
                          b + 0.9*(double)rand()/RAND_MAX);

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {
                    // diffuse
                    color albedo(
                        ((double)rand()/RAND_MAX) * ((double)rand()/RAND_MAX),
                        ((double)rand()/RAND_MAX) * ((double)rand()/RAND_MAX),
                        ((double)rand()/RAND_MAX) * ((double)rand()/RAND_MAX)
                    );
                    world.add(std::make_shared<sphere>(center, 0.2,
                        std::make_shared<lambertian>(albedo)));
                } else if (choose_mat < 0.95) {
                    // metal
                    color albedo(
                        0.5 + 0.5*((double)rand()/RAND_MAX),
                        0.5 + 0.5*((double)rand()/RAND_MAX),
                        0.5 + 0.5*((double)rand()/RAND_MAX)
                    );
                    double fuzz = 0.5 * ((double)rand()/RAND_MAX);
                    world.add(std::make_shared<sphere>(center, 0.2,
                        std::make_shared<metal>(albedo, fuzz)));
                } else {
                    // glass
                    world.add(std::make_shared<sphere>(center, 0.2,
                        std::make_shared<dielectric>(1.5)));
                }
            }
        }
    }

    // Three big spheres
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0,
        std::make_shared<dielectric>(1.5)));
    world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0,
        std::make_shared<lambertian>(color(0.4, 0.2, 0.1))));
    world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0,
        std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0)));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vfov        = 20;
    cam.lookfrom    = point3(13, 2, 3);
    cam.lookat      = point3(0, 0, 0);
    cam.vup         = vec3(0, 1, 0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
}