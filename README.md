# FlashHax

This is an exploit in the Wii's Internet Channel.
This exploit is entirely loaded over the internet so you don't even need an SD card for this.

Unfortunately this doesn't work for the vWii since Nintendo doesn't allow you to download the Internet Channel on the vWii. Although if you install the channel through homebrew it should in theory work (untested tho).

Currently the only payload that is prepared is the HackMii Installer (v1.2) so you can install homebrew on a Wii that has a broken SD card reader.
Although good luck trying to get any homebrew apps to play nice without their SD card support.

Here is the current guide to FlashHax.

WARNING: A good Internet connection is highly recommended to increase the success rate of the exploit.
EDIT: Internet connection speed should no longer matter but I can't say for 100% sure I got all the randomness out of it so a good speed might still help.
1. Download the Internet Channel if you don't already have it from the store.
2. In the Internet Channel go to flashhax.com
3. When the page loads it should select your region and when the site prompts you bookmark the page.
If you get this far then you can start from here if the exploit fails.
4. Click the page label "FlashHax" that just got added in the favorites menu.
You should be booted into the HackMii installer after a few seconds.
If you get stuck at 95%+ on the download then hold down the power button on your Wii to try again.

From now on all you have to do is click the Exploit page from the favorites menu to be launched into the installer.
As I said above this isn't that useful to do multiple times however I hope to have a landing page where you can select what app you want to launch. (Coming soon... maybe... I haven't really started on this yet)

Questions people will probably ask
>So if this isn't available to the vWii and we already have an entry point that works without any channel and isn't patched then what's the point of this?
This was just a fun project that I decided to work on. There wasn't really a need for this, it's just a project to teach me more about PowerPC and how to exploit it.

>Is the source code available?
Yes. See the Github page below.

How to build your own payload into a webpage:
This requires your payload to be in an elf format. It should be possible to convert it from dol to elf however the tools to do that aren't in this.
1. If you are running on Windows then you need to install Cygwin to run some of the build commands. DevkitPro will also need to be installed IN THE CYGWIN CONTAINER!
(Optional as of V2.1) 2. You will also need an application that can compile the fla into a flash swf file. I use Adobe Flash Professional CS5.5 (11.5.1). Just open the fla and click File->Publish. This should create exploit.swf in the same folder.
3. Once you have the swf files just double click the exploit.bat file to copy it into the server folder.
4. In Cygwin make sure you have installed the devkitpro ppc toolchain as well as a native toolchain for your system and Python.
6. Copy your boot.elf file of choosing to the payload folder!
5. Open a Cygwin terminal and navigate to the payload folder and run the buildNdump.sh file.
6. At this point the server folder should have been populated with the swf files and payload.bin along with the index.html and start.sh that were already there.
7. Run the start.sh file from Cygwin to start a temporary server on port 8080 that you can test with.
8. To navigate to this on your Wii's Internet Channel go to http://YOUR LOCAL IP HERE:8080/
9. You can now continue like normal from step 3 on the current guide to flashhax section to launch the payload.
If these steps fail after testing with a normal boot.elf (like the HackMii installer one) then please leave comment with what is going wrong. This hasn't been tested very much so there are likely to be a couple problems with the instructions.

Technical explanation for the exploit:
This is actually a fairly recent flash exploit that was discovered by Google's Project Zero team and patched in early 2016. (CVE-2016-0974) (https://bugs.chromium.org/p/project-zero/issues/detail?id=667)
If you look at the source for the exploit "exploit.fla" and check the actionscript on frame 0 you will find the attack there.

The first thing to explain is what happens if you just trigger the PoC from Google (see link above).
This will actually crash the Wii. But what is interesting is if you enable panic handlers in Dolphin you will see it crashes trying to read memory at an address like 0x00740065. Hmm, that looks like a weird address and if we run the exploit several more times we consistently get this address over and over.
So lets take a closer look at that address 0074 0065. Those numbers are valid ascii letters and if it's unicode that might actually be a string!
Sure enough if we translate those addresses to characters in ascii we get "te". Now where do we see "te" in our exploit?
Right, the string returned from the function that removes its parent (func) starts with te! So lets confirm this by changing those characters to AA.
Sure enought we get a invalid read at address 0x00410041. (The addresses read might be +- 4 depending on what it's trying to read so it might be a little off but still), this confirms that we control where it is read from. Now how do we turn this into an exploit?

Well the first thing we need to do is get it to actually read something that doesn't start with 00. Luckly flash lets us include almost any uncode characters we want in a string if we just use escape charaters. So putting in "\u4242\u4242" will result in a two character unicode string that in memory looks like 0x42424242. Perfect!
Run the exploit again and sure enought we now crash on 0x42424242. So now we control where we now need to know what this thing is actually doing with these addresses.

To spare you from the hours of reverse engineering that this took I'll just explain what's going on.
The object we are overwriting with our use-after-free is cleared when garbage collection happens (I think, no way to really confirm tho).
This object has a very simple layout when going forwards:

0x(pointer to next object)

However after it reads this pointer the object is initialized like this:

0x00000002 (some small number)
0x(pointer to previous object)
0x00000000 (might get changed to pointer to something)
0x00000000 (might get changed to pointer to something)
0x00000000 (might get changed to pointer to something)
0x00000010

This is a pretty bad primitive if you just look at it like this. You can see a pointer to the previous object is written to +4 from where ever the next object is so there is some hope for this to be useful.
This means that we just setup a pointer to an address we want to point back to us then subtract 4 so that the back link will point to our object.

Unfortunately this primitive has a major downside. Flash will continue to follow this chain of pointers to objects and after every follow it will read the next address then overwrite it with the second layout I described.
This has the side effect that if you don't take control before flash reads the next object and it turns out to be an invalid pointer then the Wii will just crash referencing it.

Somehow we need to either stop it from reading anymore objects in the chain, be the last link in the chain, or somehow break that code that is looping this without crashing the Wii.

Unfortunately after some more reverse engineering it turns out it isn't possible to stop the chain prematurely as our pointer is never check before being dereferenced and none of the other fields in our string are even read so there isn't even a chance that it is looking for some other end of chain marker.

This means we have either the code method or finding out how many times this thing loops and being at the end of it. After looking at the code to see if there was some way to break the loop with the write I have concluded that it just doesn't seem feasible without more direct control over what is being written. The best I could get with this method was a single instruction from a limited list being executed before a guaranteed crash.

This leaves us with somehow being the end of the chain. Spoiler alert this method ends up working however it comes at the cost of having to do a heap spray with little to no chance of using any type of nop-sled. The first thing to try is finding out how many times the loop is called so we know when the last link in the chain will be and won't have to worry about it's pointer at that location being a bad address. For example if the last link in the chain is pointed at 0x80003400 then flash will overwrite 0x80003404 with a pointer back to the last object but WON'T follow the pointer it found at 0x80003400, This is great because it means that the Wii has time to do a context switch to another thread which involves following pointer to find saved thread contexts.

So how do we find out how many times the function is called? Well if you just set a break point at the function you will actually see it called hundreds of times a second. This might make it seem like there is no way we could predict when the last chain is however, taking a closer look we see that r4 is dereferenced to obtain the pointer that it saves before it initializes the object. Only looking at the times that our chain is the one being iterated over we see that r4 actually is the same address each time while a different address for other chains. (Note that this doesn't mean r4 is a static address just that determines what chain we are following). If we just pay attention to our chain then we quickly see that it is only iterated over 6 times before stopping. This is the key we need to take control without crashing the Wii!

At first you might think that we can just overwrite a saved stack pointer. Unfortunately this doesn't work since don't know where these threads stacks are. If you investigate the function which is looping over the objects you will find that it's sp is pointed at an address in MEM2 and each time you run it the stack will be at a different location. Fortunately there is a static address that contains a pointer to a thread context that we can overwrite with out primitive.

Lastly since we know we are going to have to guide the loop ourselves this means we can't just use one string to do the exploit since we need to predict its location. After searching in the heap for a while I was able to find a location that we always allocated when the favorites menu was used to load a webpage and when a heap spray occurred there was a high chance that just after this allocated area would be our target string. I really didn't think that this would ever work reliably since we need to count the number of loops that chain has to do so no nop sled but somehow this little statically allocated (not really static but very likely) region saves the whole thing.

With all of this in mind we can now craft out exploit chain.

First we create a string that will guide the garbage collection loop through our chain the correct amount of times.

StringBaseAddress:
0: 0x(StringBaseAddress+0x18)
4: 0x41414141
8: 0x41414141
c: 0x41414141
10: 0x41414141
14: 0x41414141
18: 0x(StringBaseAddress+(0x18*2))
1c: 0x41414141
20: 0x41414141
24: 0x41414141
28: 0x41414141
2c: 0x41414141
30: 0x(StringBaseAddress+(0x18*3))
34: 0x41414141
38: 0x41414141
3c: 0x41414141
40: 0x41414141
44: 0x41414141

Assuming that we start reading at 0 then this chain will guide the collection routine 3 times before reaching the next pointer. At this point we will actually have used up 4 loops since the initial pivot to this string will have taken one.
Next we need to decide where we are writing our address to. As I stated above there is a nice pointer in the thread swapping function that we can overwrite and will be used when the next thread switch happens and is suppose to point to the stored thread state. This means we control most of the registers when the thread switches. Since this is our 5th iteration in the loop when it goes around for the last time it will overwrite the pointer with the back link to our string and stop after that.

Once a thread switch occurs with out string as the stored thread context we control most of the registers. (See the fla file for the exact layout of registers stored in a thread context)

Unfortunately since we clobbered the next couple bytes in our string this makes it impossible to directly control the sp register with this since it is loaded from offset 4 which is in the middle of the clobbered region. We do however control registers 6 and up as well as all the special purpose registers such as lr, ctr, and pc. This means that with some rop trickery we can use a value stored in one of our registers we do control and swap it with sp.
The exact chain isn't really interesting but you can take a look at the addresses in the file. At this point since we control sp we can build a rop chain that will write a payload to somewhere in MEM1 which is mapped as executable in the BAT registers and jump to it. From there it is game over. The rop chain in this exploit writes an egg hunter that looks for the payload downloaded earlier by flash and copies it to MEM1 before jumping to it. This allows us to get around the annoying no unicode null restrictions on flash strings.

That about does it for this exploit. Just to note this is by far the worst exploit primitive I have ever successfully used and man was it such a pain. Definitely worth it in the end to get it working tho :).

Source: https://github.com/Fullmetal5/FlashHax

All code is under the GPL-V3 unless otherwise stated.

Changelog:
All change logs from here on are on github (https://github.com/Fullmetal5/FlashHax)

V2.1.2:
Forgot that the payload is now payload.bin instead of payload.jpg so don't keep naming it that in the builder.

V2.1.1:
Take out some accidentally included code and fix javascript callback

V2.1:
Split the exploit off into it's own file that can be maintained easier.
Made menu more presentable and cleaned up lots of the loader.
Added arguments that can be passed to the loader from flash or javascript.
Made html file sane again. Unfortunately breaks bookmarks again. (sorry)

V2.0.1:
Fixed some formatting and fixed broken embedded payload address check.
Updated index.html to minified version. (This will break previous bookmarks)

V2:
Added instructions on how to add your own boot.elf
