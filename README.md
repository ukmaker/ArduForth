# ArduForth
A Forth-like interpreter for the Arduino environment. 

## Q&A
Q: Why did you write yet another Forth Duncan?
A: Because I can
A (supplemental): Because I wanted to be able to run a Forth in 
  the arduino environment to help with testing sketches.

Q: How do I use it?
A: Well, there are a few possibilities - see below for details.
  But essentially:
    - You can run the code in your development environment
      (i.e. on your PC) in order to generate core Forth images
      for the VM
    - You can include the ArduForth library in your own sketch
      using either an image you created or one of the images
      I've supplied. For example there's an image with words
      for interacting with a host STM32F401CC (BlackPill).

Q: Sorry, what VM now?
A: Ah. This library implements an abstrat virtual machine
  to run Forth on. This way it's independent of the host
  Arduino processor. It's also quite slow as a result.

## Getting Started

1. Check-out the repository from Github
2. copy the runtime/ArduForth directory into your 
  Arduino library folder
3. Open one of the sketches in the ArduForth/examples folder
4. Enjoy!