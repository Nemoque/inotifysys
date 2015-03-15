#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <signal.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <time.h>
#include <pthread.h>
#include <openssl/md5.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <SkImageDecoder.h>
#include <SkBitmap.h>
#include <android/native_window.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <unistd.h>
#include <ui/DisplayInfo.h>
#include <utils/String8.h>
#include <cutils/klog.h>
#include "event_queue.h"
#include "inotify_utils.h"

#define DEBUG_LOG

#ifdef DEBUG_LOG
#define DEBUG_LOG(fmt,arg...)    printf(fmt,##arg)
#else
#define DEBUG_LOG(fmt,arg...)    do{}while(0);
#endif


using namespace android;
int keep_running = 0;
int sys_tampered = 0;

/*show a crack figure if the system partition has been tampered viciously
 *return NULL if errors happen,otherwise,this handler should exist permanently
 *This handler should be called once in a specified thread
 */
void *display_res_on_screen(void* data) {
    sp<SurfaceComposerClient> client;
    sp<SurfaceControl>        control;
    sp<Surface>               surface;
    SkBitmap                  sbs;

    while(sys_tampered == 0) {
        sleep(10);
    }

    if (false == SkImageDecoder::DecodeFile("/sbin/rpres", &sbs)) {
        DEBUG_LOG("fail load file\n");
        return NULL;
    }

    client = new SurfaceComposerClient();
    control = client->createSurface(String8("bootres"), sbs.width(), sbs.height(), PIXEL_FORMAT_RGBA_8888);
    client->openGlobalTransaction();
    control->setLayer(0x80000000);
    control->setPosition(0,0);
    client->closeGlobalTransaction();

    surface = control->getSurface();
    ANativeWindow_Buffer outBuffer;
    ARect tmpRect;
    tmpRect.left = 0;
    tmpRect.top = 0;
    tmpRect.right = sbs.width();
    tmpRect.bottom = sbs.height();

    surface->lock(&outBuffer, &tmpRect);
    DEBUG_LOG("sbs=%d,%d,%d\n",sbs.width(),sbs.height(),sbs.bytesPerPixel());
    DEBUG_LOG("outbuffer=%d,%d,%d\n",outBuffer.stride,outBuffer.width,outBuffer.height);
    uint8_t* displayPixels = reinterpret_cast<uint8_t*>(sbs.getPixels());
    uint8_t* img = reinterpret_cast<uint8_t*>(outBuffer.bits);
    for (uint32_t y = 0; y < outBuffer.height; y++) {
        for (uint32_t x = 0; x < outBuffer.width; x++) {
            uint8_t* pixel = img + (4 * (y*outBuffer.stride + x));
            uint8_t* display = displayPixels + (4 * (y*sbs.width() + x));
            pixel[0] = display[0];
            pixel[1] = display[1];
            pixel[2] = display[2];
            pixel[3] = display[3];
        }
    }
    surface->unlockAndPost();
    client->dispose();
    while (1) {
        sleep(1000);
    }
    return NULL;
}

/* This program will take as arguments one or more directory 
   or file names, and monitor them, printing event notifications 
   to the console. It will automatically terminate if all watched
   items are deleted or unmounted. Use ctrl-C or kill to 
   terminate otherwise.
*/

/* Signal handler that simply resets a flag to cause termination */
void signal_handler (int signum)
{
  keep_running = 0;
}

int main (int argc, char **argv)
{
    /* This is the file descriptor for the inotify watch */
    int inotify_fd;

    keep_running = 1;
   
    /*Create a thread to handle result for the tampered action*/
    pthread_t figure_tid;
    pthread_create(&figure_tid,NULL,display_res_on_screen,NULL);

    /* Set a ctrl-c signal handler */
    if (signal (SIGINT, signal_handler) == SIG_IGN)
    {
      /* Reset to SIG_IGN (ignore) if that was the prior state */
      signal (SIGINT, SIG_IGN);
    }

    /* First we open the inotify dev entry */
    inotify_fd = open_inotify_fd();
    if (inotify_fd > 0)
    {

        /* We will need a place to enqueue inotify events,
           this is needed because if you do not read events
           fast enough, you will miss them. This queue is 
           probably too small if you are monitoring something
           like a directory with a lot of files and the directory 
           is deleted.
         */
        queue_t q;
        q = queue_create();

        /* This is the watch descriptor returned for each item we are 
         watching. A real application might keep these for some use 
         in the application. This sample only makes sure that none of
         the watch descriptors is less than 0.
         */
        int wd = 0;


        /* Watch all events (IN_ALL_EVENTS) for the directories and 
           files passed in as arguments.
           Read the article for why you might want to alter this for 
           more efficient inotify use in your app.      
         */
        int index;
        DEBUG_LOG("\n");
        for (index = 1; (index < argc) && (wd >= 0); index++) 
	{
	    wd = watch_dir (inotify_fd, argv[index], IN_ALL_EVENTS);
	    /*wd = watch_dir (inotify_fd, argv[index], IN_ALL_EVENTS & ~(IN_CLOSE | IN_OPEN) ); */
	}

        if (wd > 0) 
	{
	    /* Wait for events and process them until a 
               termination condition is detected
             */
	    process_inotify_events (q, inotify_fd);
	}
        DEBUG_LOG("\nTerminating\n");

        /* Finish up by closing the fd, destroying the queue,
           and returning a proper code
         */
        close_inotify_fd (inotify_fd);
        queue_destroy (q);
    }
    return 0;
}

