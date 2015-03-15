# inotifysys
A daemon used to follow system action caused via applications by means of inotify.

Note that these codes are just a simplified routine in Android 4.4 and should be compile in Android source code(external for instance).

When an app handles a specified message in /system, this daemon should report some message in return to device owner.

Besides, framebuffer or surfaceflinger should flush a logo in the screen once tampering actions encounter.

Yet,you should prepare /sbin/res with corresponding resource for logo showing as well in your device,it is quite easy,though.

