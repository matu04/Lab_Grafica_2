
#include <cmath>



class Vec{
    public: 
        double x;
        double y;
        double z;

        Vec() : x(0), y(0), z(0) {}
        Vec(double x, double y, double z) : x(x), y(y), z(z) {}

        Vec operator+(const Vec& v){
            return Vec(x + v.x, y + v.y, z + v.z);
        }

        Vec operator-(const Vec& v){
            return Vec(x - v.x, y - v.y, z - v.z);
        }

        Vec operator*(double s){
            return Vec(x * s, y * s, z * s);
        }

        Vec operator/(double s){
            return Vec(x / s, y / s, z / s);
        }

        double productoEscalar(const Vec& v){
            return x * v.x + y * v.y + z * v.z;
        }

        Vec productoVectorial(const Vec& v){
            double a = y * v.z - z * v.y;
            double b = z * v.x - x * v.z;
            double c = x * v.y - y * v.x;
            return Vec(a, b, c);
        }
        double largo(){
            return sqrt(x * x + y * y + z * z);
        }

        Vec normalizar(){
            double len = largo();
            return Vec(x / len, y / len, z / len);
        }
};

class Color{
    public:
        double r;
        double g;
        double b;

        Color() : r(0), g(0), b(0) {}
        Color(double r, double g, double b) : r(r), g(g), b(b) {}

        Color operator+(const Color& c){
            return Color(r + c.r, g + c.g, b + c.b);
        }

        Color operator*(const Color& c){
            return Color(r * c.r, g * c.g, b * c.b);
        }

        Color operator*(double s){
            return Color(r * s, g * s, b * s);
        }

        void acotar(){
            if (r < 0.0) r = 0.0;
            if (r > 1.0) r = 1.0;

            if (g < 0.0) g = 0.0;
            if (g > 1.0) g = 1.0;

            if (b < 0.0) b = 0.0;
            if (b > 1.0) b = 1.0;
        }
};

class Ray{
    public:
        Vec origen;
        Vec direccion;

        Ray() : origen(), direccion() {}
        Ray(Vec o, Vec d) : origen(o), direccion(d.normalizar()) {}
};

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
        Vec L =
            normalizar(light.position - hit.point);



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
            max(0, hit.normal.productoEscalar(L));

        finalColor +=
            mat.diffuse *
            light.color *
            diff;



        // ====================
        // PHONG ESPECULAR
        // ====================

        Vec R =
            reflect(-L, hit.normal);

        Vec V =
            normalizar(-ray.direction);

        double spec =
            pow(max(hit.normal.productoEscalar(R),0),
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
            Vec R =
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

        Vec T =
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
    Vec center;
    double radius;

    intersect(...)
};



class Plane : public Object
{
    Vec point;
    Vec normal;

    intersect(...)
};



class Triangle : public Object
{
    Vec v0, v1, v2;

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

    Vec point;

    Vec normal;

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

    Vec3 dir = normalize(adelante + px * derecha + py * arriba);

    return Ray(posicion, dir);
}

// ===================================
// LUZ
// ===================================

struct Light
{
    Vec position;

    Color color;

    double intensity;
};



// ===================================
// MATEMATICA
// ===================================

Vec reflect(I, N)
{
    return I - 2 * productoEscalar(I, N) * N;
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


