#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#import <mach/mach_time.h>

#import "platform.h"

//////////////////////////////////
// Display Link Callback:
static CVReturn GlobalDisplayLinkCallback ( CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*, CVOptionFlags, CVOptionFlags*, void* );


//////////////////////////////////
// AppDelegate Interface:
@interface AppDelegate : NSObject<NSApplicationDelegate>
{	
}
@end


//////////////////////////////////
// AppDelegate Implementation:
@implementation AppDelegate

- (void) applicationWillFinishLaunching: (NSNotification *)notification 
{
	// Next, we need to create the menu bar. You don't need to give the first item in the menubar a name 
	// (it will get the application's name automatically)
	id menubar = [[NSMenu new] autorelease];
	id appMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];

	// Then we add the quit item to the menu. Fortunately the action is simple since terminate: is 
	// already implemented in NSApplication and the NSApplication is always in the responder chain.
	id appMenu = [[NSMenu new] autorelease];
	id appName = [[NSProcessInfo processInfo] processName];
	id quitTitle = [@"Quit " stringByAppendingString:appName];
	id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[appMenu addItem:quitMenuItem];
	[appMenuItem setSubmenu:appMenu];
}

- (void) applicationDidFinishLaunching: (NSNotification *)notification
{
	// Since Snow Leopard, programs without application bundles and Info.plist files don't get a menubar 
	// and can't be brought to the front unless the presentation option is changed
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:YES];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*)sender
{
	return YES;
}

@end


//////////////////////////////////
// View Interface:
@interface View : NSOpenGLView <NSWindowDelegate>
{
@public
	CVDisplayLinkRef displayLink;
	bool running;
	NSRect windowRect;
	NSRecursiveLock* appLock;

	WindowInfo windowInfo;
	InputInfo inputInfo;
	mach_timebase_info_data_t timingInfo;
}
@end


//////////////////////////////////
// View Implementation:
@implementation View

//////////////////////////////////
- (id) initWithFrame: (NSRect)frame
{
	running = true;
	
	// No multisampling:
	unsigned int samples = 0;

	// Keep multisampling attributes at the start of the attribute 
	// lists since code below assumes they are array elements 0 through 4:
	NSOpenGLPixelFormatAttribute windowedAttrs[] = 
	{
		NSOpenGLPFAMultisample,
		NSOpenGLPFASampleBuffers, samples ? 1u : 0,
		NSOpenGLPFASamples, samples,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
		0
	};

	// Try to choose a supported pixel format:
	NSOpenGLPixelFormat* pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:windowedAttrs];
	if (!pf)
	{
		bool valid = false;
		while (!pf && samples > 0)
		{
			samples /= 2;
			windowedAttrs[2] = samples ? 1 : 0;
			windowedAttrs[4] = samples;
			pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:windowedAttrs];
			if (pf)
			{
				valid = true;
				break;
			}
		}
		
		if (!valid)
		{
			NSLog(@"OpenGL pixel format not supported.");
			return nil;
		}
	}
	
	self = [super initWithFrame:frame pixelFormat:[pf autorelease]];
	appLock = [[NSRecursiveLock alloc] init];

	[self setWantsBestResolutionOpenGLSurface:YES];

	// NOTE(Xavier): (2017.11.16) This code adds a 
	// button on top of the opengl view:
	[self setWantsLayer:YES];
	int x = 10;
    int y = 10;
    int width = 130;
    int height = 40;
    NSButton *myButton = [[[NSButton alloc] initWithFrame:NSMakeRect(x, y, width, height)] autorelease];
    [self addSubview: myButton];
    [myButton setTitle: @"Button title!"];
    [myButton setButtonType:NSMomentaryLightButton];
    [myButton setBezelStyle:NSRoundedBezelStyle];
    [myButton setTarget:self];
    [myButton setAction:@selector(buttonPressed)];

	return self;
}

// NOTE(Xavier): (2017.11.16) This is the callback for the button press:
- (void) buttonPressed
{
    NSLog(@"Button pressed!"); 
}

//////////////////////////////////
- (void) prepareOpenGL
{
	[super prepareOpenGL];
		
	[[self window] setLevel: NSNormalWindowLevel];
	[[self window] makeKeyAndOrderFront: self];
	
	[[self openGLContext] makeCurrentContext];
	
	// Vsync:
	GLint swapInt = 0; // On (1) / Off (0)
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	// Create a display link capable of being used with all active displays:
	CVDisplayLinkCreateWithActiveCGDisplays( &displayLink );
	// Set the renderer output callback function:
	CVDisplayLinkSetOutputCallback( displayLink, &GlobalDisplayLinkCallback, self );
	
	CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext( displayLink, cglContext, cglPixelFormat );

	GLint sync = 0;
	CGLSetParameter(cglContext, kCGLCPSwapInterval, &sync);
	
	[appLock lock];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
		inputInfo = { 0 };
		// NOTE(Xavier): (2017.11.15) This is where the first
		// entry point to the main section of the application is:
		windowInfo.width = windowRect.size.width;
		windowInfo.height = windowRect.size.height;
		windowInfo.hidpi_width = [self convertRectToBacking:[self bounds]].size.width;
		windowInfo.hidpi_height = [self convertRectToBacking:[self bounds]].size.height;
		windowInfo.deltaTime = 0;
		init( windowInfo );

		if ( mach_timebase_info (&timingInfo) != KERN_SUCCESS ) { printf ("mach_timebase_info failed\n"); }

	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]); 
	[appLock unlock];

	// Activate the display link:
	CVDisplayLinkStart(displayLink);
}

////////////////////////////////////////////
// Tell the window to accept input events:
- (BOOL)acceptsFirstResponder { return YES; }

//////////////////////////////////
- (void)mouseMoved:(NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseDragged: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;

	[appLock unlock];
}

//////////////////////////////////
- (void) scrollWheel: (NSEvent*)event
{
	[appLock lock];

		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseScrollDeltaX = [event deltaX];
		inputInfo.mouseScrollDeltaY = [event deltaY];

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseDown: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.activeMouseButtons[0] = true;
		inputInfo.downMouseButtons[0] = true;

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseUp: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.activeMouseButtons[0] = false;
		inputInfo.upMouseButtons[0] = true;

	[appLock unlock];
}

//////////////////////////////////
- (void) rightMouseDown: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.activeMouseButtons[1] = true;
		inputInfo.downMouseButtons[1] = true;
	
	[appLock unlock];
}

//////////////////////////////////
- (void) rightMouseUp: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.activeMouseButtons[1] = false;
		inputInfo.upMouseButtons[1] = true;

	[appLock unlock];
}

//////////////////////////////////
- (void) otherMouseDown: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.activeMouseButtons[2] = true;
		inputInfo.downMouseButtons[2] = true;

	[appLock unlock];
}

//////////////////////////////////
- (void) otherMouseUp: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.activeMouseButtons[2] = false;
		inputInfo.upMouseButtons[2] = true;

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseEntered: (NSEvent*)event
{
	[appLock lock];

		// TODO(Xavier): (2017.11.23) Some testing needs to be
		// done before implementing this, because it appears to not be working.

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseExited: (NSEvent*)event
{
	[appLock lock];

		// TODO(Xavier): (2017.11.23) Some testing needs to be
		// done before implementing this, because it appears to not be working.

	[appLock unlock];
}

//////////////////////////////////
- (void) keyDown: (NSEvent*)event
{
	[appLock lock];
	
		if ([event isARepeat] == NO)
		{
			unsigned int key = [event keyCode];
			inputInfo.activeKeys[ key ] = true;
			inputInfo.downKeys[ key ] = true;
		}

	[appLock unlock];
}

//////////////////////////////////
- (void) keyUp: (NSEvent*)event
{
	[appLock lock];

		unsigned int key = [event keyCode];
		inputInfo.activeKeys[ key ] = false;
		inputInfo.upKeys[ key ] = true;

	[appLock unlock];
}

//////////////////////////////////
// NOTE(Xavier): (2017.11.23) This is a helper
// method to clear input:
- (void) clearInputAfterFrame
{
	for ( std::size_t i = 0; i < KEY_COUNT; ++i )
	{
		inputInfo.downKeys[i] = false;
		inputInfo.upKeys[i] = false;
	}

	for ( std::size_t i = 0; i < MOUSE_BUTTON_COUNT; ++i )
	{
		inputInfo.downMouseButtons[i] = false;
		inputInfo.upMouseButtons[i] = false;
	}
}

//////////////////////////////////
// Window Render:
- (CVReturn) getFrameForTime: (const CVTimeStamp*)outputTime
{
	[appLock lock];
	[[self openGLContext] makeCurrentContext];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);

		// NOTE(Xavier): (2017.11.15) This is where the render and input
		// call to multiplatform section is:
		static std::size_t startTime = mach_absolute_time();
		std::size_t endTime = mach_absolute_time();
		std::size_t elapsedTime = endTime - startTime;
		startTime = mach_absolute_time();
		float millisecs = (elapsedTime * timingInfo.numer / timingInfo.denom) / 1000000;
		windowInfo.deltaTime = millisecs;
		
		input_and_render( windowInfo, &inputInfo );

		[self clearInputAfterFrame];

		CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);

	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	[appLock unlock];
	
	return kCVReturnSuccess;
}

//////////////////////////////////
// NOTE(Xavier): This is implemented
// bceause when the window resizes, without this it flickers.
- (void) drawRect:(NSRect)rect
{
	[appLock lock];
	[[self openGLContext] makeCurrentContext];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
		
		input_and_render( windowInfo, &inputInfo );

		[self clearInputAfterFrame];

		CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);

	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	[appLock unlock];
}

//////////////////////////////////
// Window Resize:
- (void) windowDidResize: (NSNotification*)notification
{
	NSSize size = [ [ _window contentView ] frame ].size;
	[appLock lock];
	[[self openGLContext] makeCurrentContext];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
		windowRect.size.width = size.width;
		windowRect.size.height = size.height;
		
		// NOTE(Xavier): (2017.11.15) This is the call to the resize
		// fucntion in the multiplatform section of the application:
		windowInfo.width = size.width;
		windowInfo.height = size.height;
		windowInfo.hidpi_width = [self convertRectToBacking:[self bounds]].size.width;
		windowInfo.hidpi_height = [self convertRectToBacking:[self bounds]].size.height;
		resize( windowInfo );
	
	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]); 
	[appLock unlock];
}

//////////////////////////////////
- (void) resumeDisplayRenderer
{
	[appLock lock];
		CVDisplayLinkStart(displayLink);
	[appLock unlock];
}

//////////////////////////////////
- (void) haltDisplayRenderer
{
	[appLock lock];
		CVDisplayLinkStop(displayLink);
	[appLock unlock];
}

//////////////////////////////////////////////////
// Terminate window when the red 'x' is pressed:
- (void) windowWillClose: (NSNotification *)notification
{
	if ( running )
	{
		running = false;

		[appLock lock];

			// NOTE(Xavier): (2017.11.15): This function is called
			// to cleanup the program upon closing.
			// It is implemented in the multiplatform section of the application.
			cleanup( windowInfo );

			CVDisplayLinkStop(displayLink);
			CVDisplayLinkRelease(displayLink);

		[appLock unlock];
	}
}

//////////////////////////////////
- (void) dealloc
{
	[appLock release]; 
	[super dealloc];
}

@end


//////////////////////////////////
static CVReturn GlobalDisplayLinkCallback ( CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext )
{
	CVReturn result = [(View*)displayLinkContext getFrameForTime:outputTime];
	return result;
}


////////////////////////////////////////////
// This function will be called from the
// multiplatform section of the application
// when the program wants to quit.
void close_window ()
{
	[[NSApp mainWindow] close];
}


//////////////////////////////////
int main( int argc, const char *argv[] )
{ 
	// Autorelease Pool: 
	// Objects declared in this scope will be automatically 
	// released at the end of it, when the pool is "drained". 
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; 
 
	// Create a shared app instance. 
	// This will initialize the global variable 
	// 'NSApp' with the application instance. 
	[NSApplication sharedApplication];

	[NSApp setDelegate:[[AppDelegate alloc] init]];
	[NSApp finishLaunching];
 	
	// Window bounds (x, y, width, height):
	NSRect screenRect = [[NSScreen mainScreen] frame];
	NSRect viewRect = NSMakeRect(0, 0, 800, 600); 
	NSRect windowRect = NSMakeRect(NSMidX(screenRect) - NSMidX(viewRect),
								 NSMidY(screenRect) - NSMidY(viewRect),
								 viewRect.size.width, 
								 viewRect.size.height);

	// Style flags:
	NSUInteger windowStyle = NSWindowStyleMaskTitled  | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable; 

	// Create window: 
	NSWindow *window = [[NSWindow alloc] initWithContentRect:windowRect 
						styleMask:windowStyle 
						backing:NSBackingStoreBuffered 
						defer:NO]; 
	[window autorelease]; 
 
	// Window controller:
	NSWindowController *windowController = [[NSWindowController alloc] initWithWindow:window]; 
	[windowController autorelease]; 

	// Create app delegate to handle system events:
	View* view = [[[View alloc] initWithFrame:windowRect] autorelease];
	view->windowRect = windowRect;
	[window setAcceptsMouseMovedEvents:YES];

	[window setContentView:view];
	[window setDelegate:view];

	// Set app title:
	[window setTitle:[[NSProcessInfo processInfo] processName]];

	// Add fullscreen button:
	[window setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];

	// Show window and run event loop:
	[window orderFrontRegardless];
	[NSApp run];
	
	[pool drain];
	
	return 0;
}
