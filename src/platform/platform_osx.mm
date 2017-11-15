#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include "platform.h"

//////////////////////////////////
// Display Link Callback:
static CVReturn GlobalDisplayLinkCallback ( CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*, CVOptionFlags, CVOptionFlags*, void* );


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

	return self;
}

//////////////////////////////////
- (void) prepareOpenGL
{
	[super prepareOpenGL];
		
	[[self window] setLevel: NSNormalWindowLevel];
	[[self window] makeKeyAndOrderFront: self];
	
	[[self openGLContext] makeCurrentContext];
	
	// Vsync:
	GLint swapInt = 1; // On (1) / Off (0)
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	// Create a display link capable of being used with all active displays:
	CVDisplayLinkCreateWithActiveCGDisplays( &displayLink );
	// Set the renderer output callback function:
	CVDisplayLinkSetOutputCallback( displayLink, &GlobalDisplayLinkCallback, self );
	
	CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext( displayLink, cglContext, cglPixelFormat );
	
	// NOTE(Xavier): (2017.11.15) I had to diable this because
	// It was causing the glviewport to resize incorrectly.
	// GLint dim[2] = { static_cast<int>(windowRect.size.width), static_cast<int>(windowRect.size.height) };
	// CGLSetParameter( cglContext, kCGLCPSurfaceBackingSize, dim );
	// CGLEnable( cglContext, kCGLCESurfaceBackingSize );
	
	[appLock lock];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
		// NOTE(Xavier): (2017.11.15) This is where the first
		// entry point to the main section of the application is:
		windowInfo.width = windowRect.size.width;
		windowInfo.height = windowRect.size.height;
		windowInfo.hidpi_width = [self convertRectToBacking:[self bounds]].size.width;
		windowInfo.hidpi_height = [self convertRectToBacking:[self bounds]].size.height;
		init( windowInfo );

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
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseMove;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseDragged: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseDrag;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) scrollWheel: (NSEvent*)event
{
	[appLock lock];

		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseScroll;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseScrollDeltaX = [event deltaX];
		inputInfo.mouseScrollDeltaY = [event deltaY];
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseDown: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseDown;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseButton = MouseButton::Left;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseUp: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseUp;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseButton = MouseButton::Left;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) rightMouseDown: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseDown;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseButton = MouseButton::Right;
		input( windowInfo, inputInfo );
	
	[appLock unlock];
}

//////////////////////////////////
- (void) rightMouseUp: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseUp;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseButton = MouseButton::Right;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) otherMouseDown: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseDown;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseButton = MouseButton::Middle;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) otherMouseUp: (NSEvent*)event
{
	[appLock lock];
		
		NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseUp;
		inputInfo.mouseX = point.x;
		inputInfo.mouseY = point.y;
		inputInfo.mouseButton = MouseButton::Middle;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseEntered: (NSEvent*)event
{
	[appLock lock];

		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseEnter;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) mouseExited: (NSEvent*)event
{
	[appLock lock];
		
		InputInfo inputInfo = {};
		inputInfo.type = InputType::MouseExit;
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
- (void) keyDown: (NSEvent*)event
{
	[appLock lock];
		
		if ([event isARepeat] == NO)
		{
			InputInfo inputInfo = {};
			inputInfo.type = InputType::KeyDown;
			inputInfo.keyCode = [event keyCode];
			input( windowInfo, inputInfo );
		}

	[appLock unlock];
}

//////////////////////////////////
- (void) keyUp: (NSEvent*)event
{
	[appLock lock];
		
		InputInfo inputInfo = {};
		inputInfo.type = InputType::KeyUp;
		inputInfo.keyCode = [event keyCode];
		input( windowInfo, inputInfo );

	[appLock unlock];
}

//////////////////////////////////
// Window Render:
- (CVReturn) getFrameForTime: (const CVTimeStamp*)outputTime
{
	[appLock lock];
	[[self openGLContext] makeCurrentContext];
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);

		// NOTE(Xavier): (2017.11.15) This is where the render
		// call to multiplatform section is:
		render( windowInfo );

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
		
		render( windowInfo );
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
		CVDisplayLinkStop(displayLink);
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

	[NSApp terminate:self];
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

	// Since Snow Leopard, programs without application bundles and Info.plist files don't get a menubar 
	// and can't be brought to the front unless the presentation option is changed
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:YES];
	
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
	id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
		action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[appMenu addItem:quitMenuItem];
	[appMenuItem setSubmenu:appMenu];

	// Create app delegate to handle system events:
	View* view = [[[View alloc] initWithFrame:windowRect] autorelease];
	view->windowRect = windowRect;
	[window setAcceptsMouseMovedEvents:YES];

	[window setContentView:view];
	[window setDelegate:view];

	// Set app title:
	[window setTitle:appName];

	// Add fullscreen button:
	[window setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];

	// Show window and run event loop:
	[window orderFrontRegardless]; 
	[NSApp run]; 
	
	[pool drain]; 
 
	return (0); 
}
