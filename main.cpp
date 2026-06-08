// =============================
// WHITTED RAY TRACER - ESQUELETO
// =============================

main()
{
    cargarEscenaXML();

    Camara cam;

    inicializarFramebuffer();

    renderizar();

    guardarImagen();
}





// ===================================
// RENDER PRINCIPAL
// ===================================

renderizar()
{
    for cada pixel(x, y)
    {
        Ray r = camara.generarRayo(x, y);

        Color c = traceRay(r, 0);

        framebuffer[x][y] = c;
    }
}



// ===================================
// TRACE RAY (NUCLEO DEL RAY TRACER)
// ===================================

Color traceRay(Ray ray, int depth)
{
    if (depth > MAX_DEPTH)
        return backgroundColor;

    HitInfo hit;

    bool huboInterseccion =
        escena.intersectar(ray, hit);

    if (!huboInterseccion)
        return backgroundColor;

    return shade(hit, ray, depth);
}



// ===================================
// SHADING
// ===================================

Color shade(HitInfo hit, Ray ray, int depth)
{
    Material mat = hit.material;

    Color finalColor = Color(0, 0, 0);



    // ============================
    // 1. AMBIENT
    // ============================

    finalColor +=
        mat.ambient * ambientLight;



    // ============================
    // 2. LUCES
    // ============================

    for cada luz
    {
        Vec3 L =
            normalize(light.position - hit.point);



    // ========================
    // SHADOW RAY
    // ========================

    Ray shadowRay(
        hit.point + epsilon * hit.normal,
        L
    );

    bool inShadow =
        escena.hayInterseccion(
            shadowRay
        );



    if (!inShadow)
    {

        // ====================
        // DIFUSO (LAMBERT)
        // ====================

        double diff =
            max(0, dot(hit.normal, L));

        finalColor +=
            mat.diffuse *
            light.color *
            diff;



        // ====================
        // PHONG ESPECULAR
        // ====================

        Vec3 R =
            reflect(-L, hit.normal);

        Vec3 V =
            normalize(-ray.direction);

        double spec =
            pow(max(dot(R,V),0),
                mat.shininess);

        finalColor +=
            mat.specular *
            light.color *
            spec;
    }
    }



        // ============================
        // 3. REFLEXION
        // ============================

        if (mat.reflectivity > 0)
        {
            Vec3 R =
                reflect(ray.direction,
                    hit.normal);

            Ray reflectedRay(
                hit.point + epsilon * hit.normal,
                R
            );

            Color reflectedColor =
                traceRay(
                    reflectedRay,
                    depth + 1
                );

            finalColor +=
                reflectedColor *
                mat.reflectivity;
        }



    // ============================
    // 4. REFRACCION
    // ============================

    if (mat.transparency > 0)
    {
        bool totalInternalReflection;

        Vec3 T =
            refract(
                ray.direction,
                hit.normal,
                mat.ior,
                totalInternalReflection
            );

        if (!totalInternalReflection)
        {
            Ray refractedRay(
                hit.point - epsilon * hit.normal,
                T
            );

            Color refractedColor =
                traceRay(
                    refractedRay,
                    depth + 1
                );

            finalColor +=
                refractedColor *
                mat.transparency;
        }
    }

    return finalColor;
}



// ===================================
// ESCENA
// ===================================

Scene
{
    vector<Object*> objects;

    vector<Light> lights;

    Camera camera;



    bool intersectar(Ray r, HitInfo& hit)
    {
        buscar la interseccion mas cercana
    }



    bool hayInterseccion(Ray r)
    {
        usado para sombras
    }
};



// ===================================
// OBJETO ABSTRACTO
// ===================================

class Object
{
    Material material;

    virtual bool intersect(
        Ray ray,
        HitInfo& hit
    ) = 0;
};



// ===================================
// OBJETOS GEOMETRICOS
// ===================================

class Sphere : public Object
{
    Vec3 center;
    double radius;

    intersect(...)
};



class Plane : public Object
{
    Vec3 point;
    Vec3 normal;

    intersect(...)
};



class Triangle : public Object
{
    Vec3 v0, v1, v2;

    intersect(...)
};



class Cylinder : public Object
{
    ...
};



// ===================================
// HIT INFO
// ===================================

struct HitInfo
{
    bool hit;

    double t;

    Vec3 point;

    Vec3 normal;

    Material material;
};



// ===================================
// MATERIAL
// ===================================

struct Material
{
    Color ambient;

    Color diffuse;

    Color specular;

    double shininess;

    double reflectivity;

    double transparency;

    double ior; // indice refraccion
};



// ===================================
// CAMERA
// ===================================


class Camara {
public:
    Vec3 posicion;
    Vec3 adelante;
    Vec3 arriba;
    Vec3 derecha;
    double fov;
    int ancho;
    int alto;
    Camara();
    Camara(Vec3 posicion, Vec3 objetivo, Vec3 arriba, double fov, int ancho, int alto);
    Ray generarRayo(int x, int y);
};
Camara::Camara() {
    posicion = Vec3(0, 0, 0);
    adelante = Vec3(0, 0, -1);
    arriba = Vec3(0, 1, 0);
    derecha = Vec3(1, 0, 0);
    fov = 60;
    ancho = 800;
    alto = 600;
}
Camara::Camara(Vec3 posicion, Vec3 objetivo, Vec3 arriba, double fov, int ancho, int alto) {
    this->posicion = posicion;
    adelante = normalize(objetivo - posicion);
    derecha = normalize(cross(adelante, arriba));
    this->arriba = normalize(cross(derecha, adelante));
    this->fov = fov;
    this->ancho = ancho;
    this->alto = alto;
}

Camara::generarRayo(int x, int y) {
   // Coordenadas normalizadas [0,1]
    double u = (x + 0.5) / width;
    double v = (y + 0.5) / height;

    double aspect = (double)width / height;

    // Coordenadas entre [-1,1]
    double px = (2.0 * u - 1.0) * aspect;
    double py = (1.0 - 2.0 * v);

    double scale = tan(fov * 0.5 * M_PI / 180.0);
    px *= scale;
    py *= scale;

    Vec3 dir = normalize(adelante +px * derecha +py * arriba);

    return Ray(posicion, dir);
}


// ===================================
// LUZ
// ===================================

struct Light
{
    Vec3 position;

    Color color;

    double intensity;
};



// ===================================
// MATEMATICA
// ===================================

Vec3 reflect(I, N)
{
    return I - 2 * dot(I, N) * N;
}



Vec3 refract(...)
{
    usar ley de Snell
}



// ===================================
// XML
// ===================================

cargarEscenaXML()
{
cargar:
    -objetos
        - materiales
        - luces
        - camara
        - resolucion
}



// ===================================
// EXPORTAR IMAGEN
// ===================================

guardarImagen()
{
    png / bmp
}


