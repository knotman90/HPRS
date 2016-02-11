#ifndef __APPLE__
#include <GL/glut.h>
#include <GL/gl.h>
#else
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#endif
#include <IceT.h>
#include <IceTGL.h>
#include <IceTMPI.h>
#include <mpi.h>
#include <boost/lexical_cast.hpp>

/*HBASE includes*/
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <Hbase.h>

#include <server_details.h>
#include <binaryConverter.h>
#include<Frame.h>
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::hadoop::hbase::thrift;
/*-----HBASE includes-----*/

using std::cout;
using std::endl;


#define NUM_TILES_X 2
#define NUM_TILES_Y 2
int TILE_WIDTH= 512;
int TILE_HEIGHT= 512;
static void InitIceT();
static void DoFrame();
static void Draw();
static int winId;
static IceTContext icetContext;


/*Server Setting and other static params*/
std::string rowkey = "97018a43cc1087b2ab701820357d80ca00000003DEM413f6af80000000000000000";
std::string table = "Simulations_Data";

HPRS::server_details<std::string> sd("localhost",9090);
boost::shared_ptr<HbaseClient> client;
boost::shared_ptr<TSocket> T_socket;
boost::shared_ptr<TTransport> T_transport;
boost::shared_ptr<TProtocol> T_protocol;
void initializeHBaseClient(){
    /* Connection to the Thrift Server */
    boost::shared_ptr<TSocket> _socket(new TSocket(sd.hostname, sd.port));
    T_socket= _socket;
    boost::shared_ptr<TTransport> _transport(new TBufferedTransport(T_socket));
    T_transport = _transport;
    boost::shared_ptr<TProtocol> _protocol(new TBinaryProtocol(T_transport));
    T_protocol = _protocol;
    boost::shared_ptr<HbaseClient> _client( new HbaseClient(T_protocol));
    client = _client;

}






/*Create a frame from a timestep*/
template <class T>
auto createFrameHBase(const std::string& rowkey, const std::string& table,  HbaseClient& client){
    std::vector<TRowResult> res;
    std::map<Text, Text>  attributes;
    client.getRow(res, table, rowkey,attributes);

    boost::shared_ptr<Frame<T>> frame(new Frame<T>()) ;

    //binary converter
    converter<int64_t> c_int64;
    converter<double> c_double;

    for(int i=0;i<res.size();i++){
        //foreach particle get its coordinates
        TRowResult a = res[i];
        for(auto it = a.columns.begin(); it != a.columns.end() ; ++it ) {
            std::string key = it->first;

            if(key.find("M:c") != std::string::npos){ //coordinate info for the particle
                //get the particle ID
                Particle<T>* p = new Particle<T>();

                std::string key_suffix = key.substr(10,8); //binario long long
                int64_t p_id =  c_int64.fromBytes(reinterpret_cast<const unsigned char*>(key_suffix.c_str()),true);
                p->id = p_id;

                //read coordinate ffor particle p_id
                std::string value= a.columns.at(key).value; //24 bytes: 3 double




                //firstr coordinate
                std::string value_suffix = value.substr(0,8);
                double res_value =  c_double.fromBytes(reinterpret_cast<const unsigned char*>(value_suffix.c_str()),true);
                p->p.x = res_value;

                //second coordinate
                value_suffix = value.substr(8,8);
                res_value =  c_double.fromBytes(reinterpret_cast<const unsigned char*>(value_suffix.c_str()),true);
                p->p.y = res_value;

                //third coordinate
                value_suffix = value.substr(16,8);
                res_value =  c_double.fromBytes(reinterpret_cast<const unsigned char*>(value_suffix.c_str()),true);
                p->p.z = res_value;

                frame->particles.push_back(*p);
                printf("Coordinate for particle %i are [%.2f,%.2f,%.2f]\n",p->id,p->p.x,p->id,p->p.y,p->id,p->p.z);

            }
        }

    }
    return frame;
}

boost::shared_ptr<Frame<double>> frame;



void reshape(int w,int h){
    TILE_WIDTH = w;
    TILE_HEIGHT = h;
}

int main(int argc, char **argv){

     initializeHBaseClient();
    try{
        T_transport->open();



        if(!client){
            printf("ERROR\n");
        }

        frame = createFrameHBase<double>(rowkey,table,*client);

    }catch (const TException &tx) {
        std::cerr << "ERROR: " << tx.what() << std::endl;
    }

    int rank, numProc;
    IceTCommunicator icetComm;
    /* Setup MPI. */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);
    /* Setup a window and OpenGL context. Normally you would just place all the
     * windows at 0, 0 (and probably full screen in tile display mode) to a local
     * display, but since this is an example we are assuming that they are all
     *  going to one screen for display. */

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);
    glutInitWindowPosition((rank%NUM_TILES_X)*(TILE_WIDTH+10),
                           (rank/NUM_TILES_Y)*(TILE_HEIGHT+50));
    glutInitWindowSize(TILE_WIDTH, TILE_HEIGHT);
    winId = glutCreateWindow("IceT Example");
    /* Setup an IceT context. Since we are only creating one, this context will
     * always be current. */
    icetComm = icetCreateMPICommunicator(MPI_COMM_WORLD);
    icetContext = icetCreateContext(icetComm);
    icetDestroyMPICommunicator(icetComm);
    /* Prepare for using the OpenGL layer. */
    icetGLInitialize();
    glutDisplayFunc(InitIceT);
    glutIdleFunc(DoFrame);
    glutReshapeFunc(reshape);
    /* Glut will only draw in the main loop. This will simply call our idle
     * callback which will in turn call icetGLDrawFrame. */
    glutMainLoop();
    return 0;
}


static void InitIceT()
{
    IceTInt rank, num_proc;
    /* We could get these directly from MPI, but it’s just as easy to get them
     * from IceT. */

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    cout<<num_proc<<endl<<endl;
    /* We should be able to set any color we want, but we should do it BEFORE
     * icetGLDrawFrame() is called, not in the callback drawing function.
     * There may also be limitations on the background color when performing
     * color blending. */
    glClearColor(0.2f, 0.5f, 0.1f, 1.0f);
    /* Give IceT a function that will issue the OpenGL drawing commands. */
    icetGLDrawCallback(Draw);
    glutReshapeFunc(reshape);
    /* Give IceT the bounds of the polygons that will be drawn. Note that
     * we must take into account any transformation that happens within the
     * draw function (but IceT will take care of any transformation that
     * happens before icetGLDrawFrame). */
    //icetBoundingBoxf(-0.5f+rank, 0.5f+rank, -0.5, 0.5, -0.5, 0.5);
    /* Set up the tiled display. Normally, the display will be fixed for a
     * given installation, but since this is a demo, we give two specific
     * examples. */
    if (num_proc < 4)
    {
        /* Here is an example of a "1 tile" case. This is functionally
         * identical to a traditional sort last algorithm. */
        icetResetTiles();
        icetAddTile(0, 0, TILE_WIDTH, TILE_HEIGHT, 0);
    }
    else
    {
        /* Here is an example of a 4x4 tile layout. The tiles are displayed
         * with the following ranks:
         *
         * +---+---+
         * | 0 | 1 |
         * +---+---+
         * | 2 | 3 |
         * +---+---+
         *
         * Each tile is simply defined by grabing a viewport in an infinite
         * global display screen. The global viewport projection is
         * automatically set to the smallest region containing all tiles.
         *
         * This example also shows tiles abutted against each other.
         * Mullions and overlaps can be implemented by simply shifting tiles
         * on top of or away from each other.
         */
        icetResetTiles();
        icetAddTile(0, 0, TILE_WIDTH, TILE_HEIGHT, 0);
        //				icetAddTile(0, TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, 0);
        //				icetAddTile(TILE_WIDTH, TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, 1);
        //				icetAddTile(0, 0, TILE_WIDTH, TILE_HEIGHT, 2);
        //				icetAddTile(TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT, 3);
    }
    /* Tell IceT what strategy to use. The REDUCE strategy is an all-around
     * good performer. */
    icetStrategy(ICET_STRATEGY_REDUCE);
    /* Set up the projection matrix as you normally would. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)TILE_WIDTH/(double)TILE_HEIGHT, 1.0, 100.0); // Set perspective
    glViewport(0, 0, TILE_WIDTH, TILE_HEIGHT); // Set viewport

    //glOrtho(-0.75, 0.75, -0.75, 0.75, -0.75, 0.75);
    //glFrustum(-0.5,+0.5,-0.5,+0.5,0.1,500.0f);
    /* Other normal OpenGL setup. */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    if (rank%8 != 0)
    {
        GLfloat color[4];
        color[0] = (float)(rank%2);
        color[1] = (float)((rank/2)%2);
        color[2] = (float)((rank/4)%2);
        color[3] = 1.0;
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    }


    icetDiagnostics(ICET_DIAG_WARNINGS);//turn on errors also
}


static void DoFrame()
{
    /* In this idle callback, we do a simple animation loop and then exit. */
    static float angle = 0;
    IceTInt rank, num_proc;
    /* We could get these directly from MPI, but it’s just as easy to get them
     * from IceT. */
    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    if (angle <= 720)
    {
        /* We can set up a modelview matrix here and IceT will factor this
         * in determining the screen projection of the geometry. Note that
         * there is further transformation in the draw function that IceT
         * cannot take into account. That transformation is handled in the
         * application by deforming the bounds before giving them to
         * IceT. */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        gluLookAt(0, 0, 5, 0, 2, -1, 0, 1, 0);
        glRotatef(angle, 0.0, 1.0, 0.0);
        //glScalef(1.0f/num_proc, 1.0, 1.0);
        //glTranslatef(-(num_proc-1)/2.0f, 0.0, 0.0);
        /* Instead of calling Draw() directly, call it indirectly through
         * icetGLDrawFrame(). IceT will automatically handle image compositing. */
        icetGLDrawFrame();
        /* For obvious reasons, IceT should be run in double-buffered frame
         * mode. After calling icetGLDrawFrame, the application should do a
         * synchronize (a barrier is often about as good as you can do) and
         * then a swap buffers. */
        glutSwapBuffers();
        angle +=0.4f;

    }
    else
    {
        /* We are done with the animation. Bail out of the program here. Clean
         * up IceT and the other libraries we used. */
        icetDestroyContext(icetContext);
        glutDestroyWindow(winId);
        MPI_Finalize();
        exit(0);
    }
}

template <typename T, typename Y, typename Z>
T clip( T n,  Y lower,  Z upper){
    return std::max(lower, std::min(n, upper));
}
static void Draw()
{
    IceTInt rank, num_proc;
    /* We could get these directly from MPI, but it’s just as easy to get them
     * from IceT. */
    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /* When changing the modelview matric in the draw function, you must be
     * wary of two things. First, make sure the modelview matrix is restored
     * to what is was when the function is called. Remember, the draw
     * function may be called multiple times and transformations may be
     * commuted. Also, the bounds of the drawn geometry must be correctly
     * transformed before given to IceT. IceT has no way of knowing about
     * transformations done here. It is an error to change the projection
     * matrix in the draw function. */
    glMatrixMode(GL_MODELVIEW);


    std::vector<Particle<double>>* particles = &frame->particles;
    uint size = particles->size();
    uint chunk = size/num_proc;
    for(uint i = chunk*rank ; i<chunk*(rank+1) ; i++){
        auto p = (*particles)[i];

        glPushMatrix();

        glTranslatef(p.p.x*100, p.p.y*100, p.p.z*100);
        glutSolidSphere(0.05, 20, 20);

        //glutWireSphere(0.5,30,30);
        glPopMatrix();
    }

    //	int chunk = 100;
    //	const int NUM_BALLS=chunk*num_proc;
    //	const float dim=0.05f;
    //	float rn;
    //	int sign = rank %2 == 0 ? 1 : -1;
    //	for(int i=rank*chunk;i<(1+rank)*(chunk);i++){
    //		for(int j=rank*chunk;j<(1+rank)*(chunk);j++){
    //
    //			glPushMatrix();
    //
    //			glTranslatef((float)dim*2*i, (float)rank/3+dim*2*j*sign, 0);
    //			glutSolidSphere(dim, 20, 20);
    //
    //			//glutWireSphere(0.5,30,30);
    //			glPopMatrix();
    //		}
    //	}



}
