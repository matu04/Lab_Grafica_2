#include <iostream>
#include <FreeImage.h>
#include <cmath>

const double PI = 3.14159265358979323846;

class Vec {
public:
    double x;
    double y;
    double z;

    Vec() : x(0), y(0), z(0) {}
    Vec(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec operator+(const Vec& v) const {
        return Vec(x + v.x, y + v.y, z + v.z);
    }

    Vec operator-(const Vec& v) const {
        return Vec(x - v.x, y - v.y, z - v.z);
    }

    Vec operator*(double s) const {
        return Vec(x * s, y * s, z * s);
    }

    Vec operator/(double s) const {
        return Vec(x / s, y / s, z / s);
    }

    Vec opuesto() const {
        return Vec(-x, -y, -z);
    }

    double productoEscalar(const Vec& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec productoVectorial(const Vec& v) const {
        double a = y * v.z - z * v.y;
        double b = z * v.x - x * v.z;
        double c = x * v.y - y * v.x;
        return Vec(a, b, c);
    }
    double largo() const {
        return sqrt(x * x + y * y + z * z);
    }

    Vec normalizar() const {
        double len = largo();
        return Vec(x / len, y / len, z / len);
    }
};

class Rayo {
public:
    Vec origen;
    Vec direccion;

    Rayo() : origen(), direccion() {}
    Rayo(Vec o, Vec d) : origen(o), direccion(d.normalizar()) {}
};

class Camara {
public:
    Vec posicion;
    Vec adelante;
    Vec arriba;
    Vec derecha;
    double fov;
    int ancho;
    int alto;
    Camara();
    Camara(Vec posicion, Vec objetivo, Vec arriba, double fov, int ancho, int alto);
    Rayo generarRayo(int x, int y);
};

Camara::Camara() {
    posicion = Vec(0, 0, 0);
    adelante = Vec(0, 0, -1);
    arriba = Vec(0, 1, 0);
    derecha = Vec(1, 0, 0);
    fov = 60;
    ancho = 800;
    alto = 600;
}

Camara::Camara(Vec posicion, Vec objetivo, Vec arriba, double fov, int ancho, int alto) {
    this->posicion = posicion;
    adelante = (objetivo - posicion).normalizar();
    derecha = adelante.productoVectorial(arriba).normalizar();
    this->arriba = derecha.productoVectorial(adelante).normalizar();
    this->fov = fov;
    this->ancho = ancho;
    this->alto = alto;
}

Rayo Camara::generarRayo(int x, int y) {
    // coordenadas normalizadas [0,1]
    double u = (x + 0.5) / ancho;
    double v = (y + 0.5) / alto;
    double aspect = (double)ancho / alto;
    // coordenadas viewport [-1,1]
    double px = (2.0 * u - 1.0) * aspect;
    double py = (1.0 - 2.0 * v);
    // aplicar FOV
    double scale = tan(fov * 0.5 * PI / 180.0);
    px *= scale;
    py *= scale;
    Vec dir = (adelante + derecha * px + arriba * py).normalizar();
    return Rayo(posicion, dir);
}

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

class Framebuffer {
public:

    int width;
    int height;

    Color* pixels;

    Framebuffer(int w, int h) {
        width = w;
        height = h;
        pixels = new Color[width * height];
    }

    ~Framebuffer() {
        delete[] pixels;
    }

    void setPixel(int x, int y, const Color& c) {
        pixels[y * width + x] = c;
    }

    Color getPixel(int x, int y) const {
        return pixels[y * width + x];
    }
};

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

struct infoImpacto
{
    bool impacto;
    double t;
    Vec punto;
    Vec normal;
    Material material;
};

void guardarImagen(const Framebuffer& fb, const char* filename) {

    FIBITMAP* image = FreeImage_Allocate(fb.width, fb.height, 24);

    for (int y = 0; y < fb.height; y++) {

        for (int x = 0; x < fb.width; x++) {

            Color c = fb.getPixel(x, y);

            c.acotar();

            RGBQUAD color;

            color.rgbRed = (BYTE)(c.r * 255);
            color.rgbGreen = (BYTE)(c.g * 255);
            color.rgbBlue = (BYTE)(c.b * 255);

            FreeImage_SetPixelColor(image, x, fb.height - y - 1, &color);
        }
    }

    FreeImage_Save(FIF_PNG, image, filename);

    FreeImage_Unload(image);
}

class Objeto
{
public:
    Material material;

    virtual ~Objeto() = default;

    virtual bool interseccion(Rayo rayo, infoImpacto& impacto) = 0;
};

class Esfera : public Objeto
{
public:
    Vec centro;
    double radio;

    Esfera(Vec centro, double radio, Material material)
    {
        this->centro = centro;
        this->radio = radio;
        this->material = material;
    }

    bool interseccion(Rayo rayo, infoImpacto& impacto) override
    {
        const double EPSILON = 0.000001;

        Vec vectorEsferaCamara = rayo.origen - centro;

        //Sustituyo ecuacion del rayo dentro de la de la esfera// 
        double a = rayo.direccion.productoEscalar(rayo.direccion);
        double b = 2.0 * vectorEsferaCamara.productoEscalar(rayo.direccion);
        double c = vectorEsferaCamara.productoEscalar(vectorEsferaCamara) - radio * radio;

        double discriminante = b * b - 4.0 * a * c;

        if (discriminante < 0)
            return false;

        double raiz = sqrt(discriminante);

        double t1 = (-b - raiz) / (2.0 * a);
        double t2 = (-b + raiz) / (2.0 * a);

        double t;

        if (t1 > EPSILON)
            t = t1;
        else if (t2 > EPSILON)
            t = t2;
        else
            return false;

        //Consideras esferas mas lejos// 
        if (t >= impacto.t)
            return false;

        impacto.impacto = true;
        impacto.t = t;
        impacto.punto = rayo.origen + rayo.direccion * t;
        impacto.normal = (impacto.punto - centro).normalizar();
        impacto.material = material;

        return true;
    }
};

class Plano : public Objeto
{
public:
    Vec punto;
    Vec normal;

    Plano(Vec punto, Vec normal, Material material)
    {
        this->punto = punto;
        this->normal = normal;
        this->material = material;
    }

    bool interseccion(Rayo rayo, infoImpacto& impacto) override
    {
        const double EPSILON = 0.000001;

        double escalarRayoNPlano = rayo.direccion.productoEscalar(normal);

        if (abs(escalarRayoNPlano) < EPSILON)
            return false;
        
        //Dado un punto x para que este en el plano, se debe cumplir (x - punto del plano) productoEscalar normal = 0//
        double t = (punto - rayo.origen).productoEscalar(normal) / escalarRayoNPlano;

        if (t <= EPSILON)
            return false;

        if (t >= impacto.t)
            return false;

        impacto.impacto = true;
        impacto.t = t;
        impacto.punto = rayo.origen + rayo.direccion * t;
        impacto.normal = normal;
        impacto.material = material;

        return true;
    }
};

class Triangulo : public Objeto
{
public:
    Vec3 v0, v1, v2;

    Triangulo(Vec3 v0, Vec3 v1, Vec3 v2, Material material)
    {
        this->v0 = v0;
        this->v1 = v1;
        this->v2 = v2;
        this->material = material;
    }

    bool interseccion(Ray rayo, infoImpacto& impacto) override
    {
        const double EPSILON = 0.000001;

        //Punto dentro del triangulo como P = v0 + u * (v1 - v0) + v * (v2 - v0)
        // Donde u >= 0; v >= 0; u + v <= 1//
        Vec3 lado1 = v1 - v0;
        Vec3 lado2 = v2 - v0;

        //Considerando un punto de un rayo se llega a: origen - v0 = u lado1 + v lado2 - t direccion//

        //vectorAux1 corresponde al determinante del sistema//
        Vec3 vectorAux1 = rayo.direccion.productoVectorial(lado2);
        double escalarLadoVectorAux = lado1.productoEscalar(vectorAux1);

        //Chequea si el rayo es paralelo//
        if (abs(escalarLadoVectorAux) < EPSILON)
            return false;

        double invEscalarLVA = 1.0 / escalarLadoVectorAux;

        //Cramer//
        Vec3 vectorV0Origen = rayo.origen - v0;
        double u = invEscalarLVA * vectorV0Origen.productoEscalar(vectorAux1);

        if (u < 0.0 || u > 1.0)
            return false;

        Vec3 vectorAux2 = vectorV0Origen.productoVectorial(lado1);
        double v = invEscalarLVA * rayo.direccion.productoEscalar(vectorAux2);

        if (v < 0.0 || u + v > 1.0)
            return false;

        double t = invEscalarLVA * lado2.productoEscalar(vectorAux2);

        if (t <= EPSILON)
            return false;

        if (t >= impacto.t)
            return false;

        impacto.impacto = true;
        impacto.t = t;
        impacto.punto = rayo.origen + rayo.direccion * t;
        impacto.normal = lado1.productoVectorial(lado2).normalizar();
        impacto.material = material;

        return true;
    }
};

class Escena
{
public:

    Objeto* objetos[100];

    int cantidadObjetos;

    Vec luzPos;

    Camara cam;



    Escena()
    {
        cantidadObjetos = 0;
    }



    void agregarObjeto(Objeto* obj)
    {
        objetos[cantidadObjetos] = obj;

        cantidadObjetos++;
    }
};

void cargarEscena(Escena& escena)
{
    Material mat; mat.diffuse = Color(1, 1, 1);

    Esfera* esfera1 = new Esfera(Vec(1, 1, -5), 1, mat);
    Esfera* esfera2 = new Esfera(Vec(-1, -1, -5), 1, mat);
    Esfera* esfera3 = new Esfera(Vec(0, 0, -4), 0.5, mat);

    Material matPiso; matPiso.diffuse = Color(1, 1, 1);
    Material matTecho; matTecho.diffuse = Color(1, 1, 1);
    Material matIzquierda; matIzquierda.diffuse = Color(1, 0, 0);
    Material matDerecha; matDerecha.diffuse = Color(0, 1, 0);
    Material matFondo; matFondo.diffuse = Color(1, 1, 1);

    Plano* piso = new Plano(Vec(0, -2, 0), Vec(0, 1, 0), matPiso);
    Plano* techo = new Plano(Vec(0, 2, 0), Vec(0, -1, 0), matTecho);
    Plano* izquierda = new Plano(Vec(-3, 0, 0), Vec(1, 0, 0), matIzquierda);
    Plano* derecha = new Plano(Vec(3, 0, 0), Vec(-1, 0, 0), matDerecha);
    Plano* fondo = new Plano(Vec(0, 0, -10), Vec(0, 0, 1), matFondo);

    escena.agregarObjeto(esfera1);
    escena.agregarObjeto(esfera2);
    escena.agregarObjeto(esfera3);

    escena.agregarObjeto(piso);
    escena.agregarObjeto(techo);
    escena.agregarObjeto(izquierda);
    escena.agregarObjeto(derecha);
    escena.agregarObjeto(fondo);

    escena.cam = Camara();

    escena.luzPos = Vec(0, 1, -3);
}

Color traceRay(Escena& escena, Rayo r)
{
    infoImpacto hit;

    hit.t = 999999;

    bool huboImpacto = false;

    Objeto* objetoImpactado = nullptr;

    for (int i = 0; i < escena.cantidadObjetos; i++)
    {
        if (escena.objetos[i]->interseccion(r, hit))
        {
            huboImpacto = true;

            objetoImpactado = escena.objetos[i];
        }
    }

    if (!huboImpacto)
    {
        return Color(0, 0, 0);
    }

    Vec n = hit.normal;

    Vec dirALuz = escena.luzPos - hit.punto;

    Vec L = dirALuz.normalizar();

    Rayo shadowRay(hit.punto + hit.normal * 0.001, dirALuz);

    infoImpacto hitSombra;

    hitSombra.t = 999999;

    bool enSombra = false;

    double distanciaLuz = dirALuz.largo();

    for (int i = 0; i < escena.cantidadObjetos; i++)
    {
        if (escena.objetos[i] == objetoImpactado)
            continue;

        if (escena.objetos[i]->interseccion(shadowRay, hitSombra) && hitSombra.t < distanciaLuz)
        {
            enSombra = true;
        }
    }

    if (enSombra)
    {
        return Color(0, 0, 0);
    }

    double intensidad = n.productoEscalar(L);

    if (intensidad < 0)
        intensidad = 0;

    Color c(intensidad, intensidad, intensidad);

    return c;
}

void renderizar(Escena& escena, Framebuffer& fb)
{
    for (int y = 0; y < fb.height; y++)
    {
        for (int x = 0; x < fb.width; x++)
        {
            Rayo r = escena.cam.generarRayo(x, y);

            Color c = traceRay(escena, r);

            fb.setPixel(x, y, c);
        }
    }
}

int main(int argc, char* argv[])
{
    FreeImage_Initialise();

    int width = 800;

    int height = 600;

    Framebuffer fb(width, height);

    Escena escena;

    cargarEscena(escena);

    renderizar(escena, fb);

    guardarImagen(fb, "imagen.png");

    FreeImage_DeInitialise();

    return 0;
}




//
//// ===================================
//// TRACE RAY (NUCLEO DEL RAY TRACER)
//// ===================================
//
//Color traceRay(Rayo rayo, int depth)
//{
//    if (depth > MAX_DEPTH)
//        return backgroundColor;
//
//    infoImpacto impacto;
//
//    bool huboInterseccion =
//        escena.intersectar(rayo, impacto);
//
//    if (!huboInterseccion)
//        return backgroundColor;
//
//    return shade(impacto, rayo, depth);
//}
//
//
//
//// ===================================
//// SHADING
//// ===================================
//
//Color shade(infoImpacto impacto, Rayo rayo, int depth)
//{
//    Material mat = impacto.material;
//
//    Color finalColor = Color(0, 0, 0);
//
//
//
//    // ============================
//    // 1. AMBIENT
//    // ============================
//
//    finalColor +=
//        mat.ambient * ambientLight;
//
//
//
//    // ============================
//    // 2. LUCES
//    // ============================
//
//    for cada luz
//    {
//        Vec L =
//            (light.position - impacto.punto).normalizar();
//
//
//
//    // ========================
//    // SHADOW RAY
//    // ========================
//
//    Rayo shadowRay(
//        impacto.punto + epsilon * impacto.normal,
//        L
//    );
//
//    bool inShadow =
//        escena.hayInterseccion(
//            shadowRay
//        );
//
//
//
//    if (!inShadow)
//    {
//
//        // ====================
//        // DIFUSO (LAMBERT)
//        // ====================
//
//        double diff =
//            max(0, impacto.normal.productoEscalar(L));
//
//        finalColor +=
//            mat.diffuse *
//            light.color *
//            diff;
//
//
//
//        // ====================
//        // PHONG ESPECULAR
//        // ====================
//
//        Vec R =
//            reflect(L.opuesto(), impacto.normal);
//
//        Vec V =
//            rayo.direccion.opuesto().normalizar();
//
//        double spec =
//            pow(max(impacto.normal.productoEscalar(R),0),
//                mat.shininess);
//
//        finalColor +=
//            mat.specular *
//            light.color *
//            spec;
//    }
//    }
//
//
//
//        // ============================
//        // 3. REFLEXION
//        // ============================
//
//        if (mat.reflectivity > 0)
//        {
//            Vec R =
//                reflect(rayo.direction,
//                    impacto.normal);
//
//            Rayo reflectedRay(
//                impacto.punto + epsilon * impacto.normal,
//                R
//            );
//
//            Color reflectedColor =
//                traceRay(
//                    reflectedRay,
//                    depth + 1
//                );
//
//            finalColor +=
//                reflectedColor *
//                mat.reflectivity;
//        }
//
//
//
//    // ============================
//    // 4. REFRACCION
//    // ============================
//
//    if (mat.transparency > 0)
//    {
//        bool totalInternalReflection;
//
//        Vec T =
//            refract(
//                rayo.direction,
//                impacto.normal,
//                mat.ior,
//                totalInternalReflection
//            );
//
//        if (!totalInternalReflection)
//        {
//            Rayo refractedRay(
//                impacto.punto - epsilon * impacto.normal,
//                T
//            );
//
//            Color refractedColor =
//                traceRay(
//                    refractedRay,
//                    depth + 1
//                );
//
//            finalColor +=
//                refractedColor *
//                mat.transparency;
//        }
//    }
//
//    return finalColor;
//}
//


