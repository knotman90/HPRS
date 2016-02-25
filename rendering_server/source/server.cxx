#ifndef __APPLE__
# include <GL/glut.h>
# include <GL/gl.h>
#else // ifndef __APPLE__
# include <GLUT/glut.h>
# include <OpenGL/gl.h>
#endif // ifndef __APPLE__
#include <IceT.h>
#include <IceTGL.h>
#include <IceTMPI.h>
#include <mpi.h>
#include <boost/lexical_cast.hpp>

#include<utils.h>
#include <Hbase.h>
#include <server_details.h>
#include<Particle.h>

#include <iostream>
#include<vector>
using std::cout;
using std::endl;
using glm::vec3;
#define NUM_TILES_X 2
#define NUM_TILES_Y 2
int TILE_WIDTH  = 512;
int TILE_HEIGHT = 512;
static int winId;
static IceTContext icetContext;

uint frame=0;

// iceT functions
static void InitIceT();
static void DoFrame();
static void Draw();
void        reshape(int w, int h) {
  TILE_WIDTH  = w;
  TILE_HEIGHT = h;
}

std::vector<Particle<float>*> particles;


void initparticles(uint nproc, uint chunk ){
    if(nproc <=0 || nproc > 8){
        nproc =2;
    }
    for (int i = 0; i < nproc; ++i)
    {
        for (int i = 0; i < chunk; ++i)
        {
            Particle<float>* p = new Particle<float>();
            float  xx = d_rand<float>(0,1);
            float  yy = d_rand<float>(0,2);
            float  zz = d_rand<float>(0,1);
            p->p = *(new vec3(xx,yy,zz));

            xx = d_rand<float>(0,1);
            yy = d_rand<float>(0,1);
            zz = d_rand<float>(0,1);
            p->v = *(new vec3(xx,yy,zz));

            particles.push_back(p);
        }
    }
}


void cleanParticle(){
    for (Particle<float>* p : particles)
    {
        delete p;
    }
}
int main(int argc, char **argv) {
    srand(time(0));
  int rank, numProc;
  IceTCommunicator icetComm;

  initparticles(numProc,200);
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
  glutInitWindowPosition((rank % NUM_TILES_X) * (TILE_WIDTH + 10),
                         (rank / NUM_TILES_Y) * (TILE_HEIGHT + 50));
  glutInitWindowSize(TILE_WIDTH, TILE_HEIGHT);
  winId = glutCreateWindow("IceT Example");

  /* Setup an IceT context. Since we are only creating one, this context will
   * always be current. */
  icetComm    = icetCreateMPICommunicator(MPI_COMM_WORLD);
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

  icetGetIntegerv(ICET_RANK,          &rank);
  icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
  cout << "INIT ICE T " << num_proc << endl << endl << rank << endl << endl;

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

  // icetBoundingBoxf(-0.5f+rank, 0.5f+rank, -0.5, 0.5, -0.5, 0.5);

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

    //				icetAddTile(0, TILE_HEIGHT, TILE_WIDTH,
    // TILE_HEIGHT, 0);
    //				icetAddTile(TILE_WIDTH, TILE_HEIGHT, TILE_WIDTH,
    // TILE_HEIGHT, 1);
    //				icetAddTile(0, 0, TILE_WIDTH, TILE_HEIGHT, 2);
    //				icetAddTile(TILE_WIDTH, 0, TILE_WIDTH,
    // TILE_HEIGHT, 3);
  }

  /* Tell IceT what strategy to use. The REDUCE strategy is an all-around
   * good performer. */
  icetStrategy(ICET_STRATEGY_SEQUENTIAL);

  // icetSingleImageStrategy(ICET_SINGLE_IMAGE_STRATEGY);

  /* Set up the projection matrix as you normally would. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (double)TILE_WIDTH / (double)TILE_HEIGHT, 1.0, 100.0); //
                                                                              // Set
                                                                              // perspective
  glViewport(0, 0, TILE_WIDTH, TILE_HEIGHT);                                  //
                                                                              // Set
                                                                              // viewport

  // glOrtho(-0.75, 0.75, -0.75, 0.75, -0.75, 0.75);
  // glFrustum(-0.5,+0.5,-0.5,+0.5,0.1,500.0f);

  /* Other normal OpenGL setup. */
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  if (rank % 4 != 0)
  {
    GLfloat color[4];
    color[0] = (float)(rank % 2);
    color[1] = (float)((rank / 2) % 2);
    color[2] = (float)((rank / 4) % 2);
    color[3] = 0.5;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
  }


  icetDiagnostics(ICET_DIAG_WARNINGS); // turn on errors also
}




static void DoFrame() {

	/* In this idle callback, we do a simple animation loop and then exit. */
	static float angle = 0;
	IceTInt rank, num_proc;
	/* We could get these directly from MPI, but it’s just as easy to get them
	 * from IceT. */
	icetGetIntegerv(ICET_RANK, &rank);
	icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
	cout<<"DO FRAME "<<num_proc<<endl<<endl<<rank<<endl<<endl;
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
		angle +=0.1f;
		if(frame == 100){
			frame=0;
		}
		else{
			frame++;
		}
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



	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f((float)(rank%2) , (float)((rank/2)%2) , (float)((rank/4)%2), 0.6f) ;

    uint size = particles.size();
	uint chunk = size/num_proc;

	for(uint i = chunk*rank ; i<chunk*(rank+1) ; i++){
	       auto p = particles[i];
		glPushMatrix();
        glTranslatef(p->p.x, p->p.y, p->p.z);
        glutSolidSphere(0.05,20,20);
		//glutWireSphere(0.3,20,20);
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

	// disable blending

	glDisable(GL_BLEND);

}
