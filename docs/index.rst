knm::vulkan_tools
=================

Helper tools to implement a Vulkan-based C++ application. It handles the following
Vulkan operations:

    - creation of a GLFW window
    - creation of the Vulkan instance, enabling all the required extensions and the
      validation layers (for debug builds only)
    - creation of a window surface, to be used by Vulkan to render into the GLFW
      window
    - selection of the physical device to use (a graphics card)
    - creation of the logical device and its queues
    - creation of the swap chain, with several images

Additionaly, the destruction of all those objects at shutdown is implemented.

The implementation is heavily based on https://vulkan-tutorial.com/. The idea is to
have a quick solution to jump-start a new project. A handful of examples are
available showing how to do common things in Vulkan.

Note that this library isn't a rendering engine/library or an abstraction layer on
top of Vulkan: the goal is to fasten a bit the repetitive task of configuring Vulkan
to render into a window.

The user is still expected to write the actual Vulkan code doing the rendering, which
means to implement its own management code for render passes, graphics pipelines,
framebuffers, vertex & index buffers, command buffers, etc...

Some settings can be modified by changing a configuration structure, other requires
to override some methods to implement your own version (if the choices made by this
implementation doesn't fulfill your needs). A lot of methods can be overriden (in
order to keep a maximum of flexibility), but most of them should be fine as they are
in most scenarios.

**knm::vulkan_tools** is a single-file, header-only C++ library. It relies on **GLFW**
(https://www.glfw.org/) to create and manage the window.

Everything is defined in the ``knm::vk`` namespace (``knm`` for my username, *Kanma*,
and ``vk`` for *Vulkan*).


.. toctree::
   :maxdepth: 2
   :caption: Documentation
   
   usage
   dependencies
   license


.. toctree::
   :maxdepth: 2
   :caption: API
   
   api_application
   api_config
   api_queuefamilyindices
   api_swapchainsupportdetails
