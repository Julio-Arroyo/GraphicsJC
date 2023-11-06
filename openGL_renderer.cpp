#include "openGL_renderer.hpp"

void extract_parameters(Scene& scene) {
    Camera camera = scene.getCamera();
    cam_orientation_angle = (float) camera.orientation.theta * 180.0 / M_PI;
    cam_orientation_axis[0] = (float) camera.orientation.x;
    cam_orientation_axis[1] = (float) camera.orientation.y;
    cam_orientation_axis[2] = (float) camera.orientation.z;
    cam_position[0] = (float) camera.pos.x;
    cam_position[1] = (float) camera.pos.y,
    cam_position[2] = (float) camera.pos.z;
    cam_left = camera.left;
    cam_right = camera.right;
    cam_bottom = camera.bottom;
    cam_top = camera.top;
    cam_near = camera.near;
    cam_far = camera.far;

    lights = scene.getLights();
    objects = scene.getObjects();
}

void init() {
    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // std::cout << "l " << cam_left << std::endl;
    // std::cout << "r " << cam_right << std::endl;
    // std::cout << "b " << cam_bottom << std::endl;
    // std::cout << "t " << cam_top << std::endl;
    // std::cout << "n " << cam_near << std::endl;
    // std::cout << "f " << cam_far << std::endl;
    glFrustum(cam_left, cam_right, cam_bottom,
              cam_top, cam_near, cam_far);

    glMatrixMode(GL_MODELVIEW);

    init_lights();
}

void init_lights() {
    glEnable(GL_LIGHTING);

    for (int i = 0; i < lights.size(); i++) {
        int light_id = GL_LIGHT0 + i;
        glEnable(light_id);

        float rgb[3] = {(float) lights[i].color.r,
                        (float) lights[i].color.g,
                        (float) lights[i].color.b};
        glLightfv(light_id, GL_AMBIENT, rgb); 
        glLightfv(light_id, GL_DIFFUSE, rgb);
        glLightfv(light_id, GL_SPECULAR, rgb);

        glLightf(light_id, GL_QUADRATIC_ATTENUATION, (float) lights[i].attenuation);
    }
}

void set_lights() {
    for (int i = 0; i < lights.size(); ++i) {
        int light_id = GL_LIGHT0 + i;
        float pos[4] = {(float) lights[i].pos.x,
                        (float) lights[i].pos.y,
                        (float) lights[i].pos.z,
                        1};

        glLightfv(light_id, GL_POSITION, pos);
    }
}

void draw_objects() {
    // std::cout << "draw_objects " << objects.size() << std::endl;
    for (int i = 0; i < objects.size(); i++) {
        // std::cout << "objects " << i << std::endl;
        glPushMatrix();
        {
        for (int j = objects[i]->transSeq.size() - 1; j  >= 0; j--) {
            // std::cout << "help bro " << i << std::endl;
            TransformationRecord tr = objects[i]->transSeq[j];
            switch (tr.tt) {
                case Type::TRANSLATION_MAT: {
                    // std::cout << "TRANS "
                    //           << tr.params[0] << " "
                    //           << tr.params[1] << " "
                    //           << tr.params[2] << std::endl;
                    glTranslatef(tr.params[0], tr.params[1], tr.params[2]);
                    break;
                }
                case Type::ROTATION_MAT: {
                    float angle = tr.params[3] * 180 / M_PI;
                    // std::cout << "ROT "
                    //           << angle << " "
                    //           << tr.params[0] << " "
                    //           << tr.params[1] << " "
                    //           << tr.params[2] << std::endl;
                    glRotatef(angle, tr.params[0], tr.params[1], tr.params[2]);
                    break;
                }
                case Type::SCALING_MAT: {
                    // std::cout << "SCALE "
                    //           << tr.params[0] << " "
                    //           << tr.params[1] << " "
                    //           << tr.params[2] << std::endl;
                    glScalef(tr.params[0], tr.params[1], tr.params[2]);
                    break;
                }
                default: {
                    std::cerr << "draw_objects: unexpected geometric transformation.";
                    break;
                }
            }

        }

        float ambient_reflect[3] = {(float) objects[i]->ambient.r,
                                    (float) objects[i]->ambient.g,
                                    (float) objects[i]->ambient.b};
        float diffuse_reflect[3] = {(float) objects[i]->diffuse.r,
                                    (float) objects[i]->diffuse.g,
                                    (float) objects[i]->diffuse.b};
        float specular_reflect[3] = {(float) objects[i]->specular.r,
                                     (float) objects[i]->specular.g,
                                     (float) objects[i]->specular.b};
        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_reflect);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_reflect);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular_reflect);
        glMaterialf(GL_FRONT, GL_SHININESS, (float) objects[i]->shininess);
        
        // Draw vertices & normals (use 1-index to skip dummy)
        std::vector<Vertex> vertices = objects[i]->getVertices();
        std::vector<Vertex> normals = objects[i]->getNormals();
        std::vector<Face> faces = objects[i]->getFaces();
        std::vector<Float3> vertex_buffer;
        std::vector<Float3> normal_buffer;
        for (Face face : faces) {
            Vertex v1 = vertices[face.v.i1];
            Float3 v1f = {(float) v1.x, (float) v1.y, (float) v1.z};
            Vertex v2 = vertices[face.v.i2];
            Float3 v2f = {(float) v2.x, (float) v2.y, (float) v2.z};
            Vertex v3 = vertices[face.v.i3];
            Float3 v3f = {(float) v3.x, (float) v3.y, (float) v3.z};
            // std::cout << "v " << v1f.x << " " << v1f.y << " " << v1f.z << std::endl;
            // std::cout << "v " << v2f.x << " " << v2f.y << " " << v2f.z << std::endl;
            // std::cout << "v " << v3f.x << " " << v3f.y << " " << v3f.z << std::endl;
            vertex_buffer.push_back(v1f);
            vertex_buffer.push_back(v2f);
            vertex_buffer.push_back(v3f);

            Vertex n1 = normals[face.n.i1];
            Float3 n1f = {(float) n1.x, (float) n1.y, (float) n1.z};
            Vertex n2 = normals[face.n.i2];
            Float3 n2f = {(float) n2.x, (float) n2.y, (float) n2.z};
            Vertex n3 = normals[face.n.i3];
            Float3 n3f = {(float) n3.x, (float) n3.y, (float) n3.z};
            normal_buffer.push_back(n1f);
            normal_buffer.push_back(n2f);
            normal_buffer.push_back(n3f);
        }
        glVertexPointer(3, GL_FLOAT, 0, &vertex_buffer[0]);
        glNormalPointer(GL_FLOAT, 0, &normal_buffer[0]);
        glDrawArrays(GL_TRIANGLES, 0, vertex_buffer.size());
        }

        glPopMatrix();
    }

    /* The following code segment uses OpenGL's built-in sphere rendering
     * function to render the blue-ground that you are walking on when
     * you run the program. The blue-ground is just the surface of a big
     * sphere of radius 100.
     */
    glPushMatrix();
    {
        glTranslatef(0, -103, 0);
        glutSolidSphere(100, 100, 100);
    }
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camera rotations caused by the mouse
    // glRotatef(y_view_angle, 1, 0, 0);
    // glRotatef(x_view_angle, 0, 1, 0);

    // float params[4] = lastRotation.getParams();
    // glRotatef(params[0], params[1], params[2], params[3]);

    float* rotationMatrix = lastRotation.toMatrix();
    glMultMatrixf(rotationMatrix);

    // std::cout << "angle " << cam_orientation_angle << std::endl
    //           << "or " << cam_orientation_axis[0]
    //           << " " << cam_orientation_axis[1]
    //           << " " << cam_orientation_axis[2] << std::endl;
    // Inverse camera rotation by its orientation angle and axis
    glRotatef(-cam_orientation_angle, cam_orientation_axis[0],
              cam_orientation_axis[1], cam_orientation_axis[2]);

    // std::cout
    //           << "pos " << cam_position[0]
    //           << " " << cam_position[1]
    //           << " " << cam_position[2] << std::endl;
    // inverse camera translation
    glTranslatef(-cam_position[0], -cam_position[1], -cam_position[2]);

    set_lights();
    draw_objects();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    width = width == 0 ? 1 : width;
    height = height == 0 ? 1 : height;

    glViewport(0, 0, width, height);

    mouse_scale_x = (float) (cam_right - cam_left) / (float) width;
    mouse_scale_y = (float) (cam_top - cam_bottom) / (float) height;
    glutPostRedisplay();
}

void mouse_pressed(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouse_x = x;
        mouse_y = y;
        is_pressed = true;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        lastRotation = currentRotation*lastRotation;
        currentRotation = Quaternion();
        mouse_x = x;
        mouse_y = y;
        glutPostRedisplay();

        is_pressed = false;
    }
}

void mouse_moved(int x, int y) {
    if (is_pressed) {
        // x_view_angle += ((float) x - (float) mouse_x) * mouse_scale_x * x_view_step;
        // float temp_y_view_angle = y_view_angle +
        //                           ((float) y - (float) mouse_y) * mouse_scale_y * y_view_step;
        // y_view_angle = (temp_y_view_angle > 90 || temp_y_view_angle < -90) ?
        //                y_view_angle : temp_y_view_angle;

        currentRotation = Quaternion(x, y, mouse_x, mouse_y);

        // mouse_x = x;
        // mouse_y = y;
        // glutPostRedisplay();
    }
}

void key_pressed(unsigned char key, int x, int y) {
    if (key == 'q') {
        exit(0);
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Wrong number of arguments." << std::endl;
    }

    std::string sceneDesFname = argv[1];
    int xres = std::stoi(argv[2]);
    int yres = std::stoi(argv[3]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(xres, yres);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Rendering");

    Scene scene(sceneDesFname, xres, yres);
    extract_parameters(scene);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse_pressed);
    glutMotionFunc(mouse_moved);
    glutKeyboardFunc(key_pressed);
    glutMainLoop();

    return 0;
}

